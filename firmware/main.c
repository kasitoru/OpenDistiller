/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#include <stdio.h>
#include <limits.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#include "libraries/avr-ds18b20/include/ds18b20/ds18b20.h" // Датчики
#include "libraries/u8x8_avr/u8x8_avr.h" // Дисплей
#include "libraries/u8g2/csrc/u8g2.h" // U8g2
#include "libraries/u8g2/csrc/mui_u8g2.h" // MUI
#include "libraries/uart/uart.h" // UART порт
#include "libraries/tone/tone.h" // Звуки
#include "libraries/functions.h" // Разное
#include "hardware.h" // Номера портов

// Глобальные переменные
u8g2_t u8g2; // Дисплей
mui_t mui; // Интерфейс

int16_t water_temperature = 0; // Температура на выходе водяного охлаждения
int16_t tsa_temperature = 0; // Температура трубки связи с атмосферой
int16_t tsarga_temperature = 0; // Температура в царге
int16_t cube_temperature = 0; // Температура в кубе
int16_t target_temperature = 0; // Целевая температура царги

uint32_t now_millis = 0; // Текущее время с начала включения
uint32_t rect_start_time = 0; // Время начала ректификации
uint32_t last_event_time = 0; // Время последнего события

void loop(uint8_t *is_redraw); // Основной цикл

// Параметры ректификации
typedef struct {
    uint8_t water_cube_temperature; // Температура старта подачи воды
    uint8_t ethanol_cube_temperature; // Максимальная температура для товарного спирта
    uint8_t final_cube_temperature; // Температура окончания работы
    uint8_t itself_working_temperature; // Температура начала "работы на себя"
    uint8_t itself_working_initial_time; // Длительность "работы на себя" (мин)
    int16_t target_temperature_delta; // Дельта целевой температуры
    uint8_t target_recovery_time; // Время восстановления (мин)
    uint8_t sensors_protection; // Включить контроль работоспособности датчиков
    uint8_t tsa_protection; // Включить защиту по температуре ТСА
    uint8_t tsa_max_temperature; // Максимальная температура ТСА
    uint8_t water_protection; // Включить защиту по температуре воды
    uint8_t water_max_temperature; // Максимальная температура воды
    uint8_t phlegm_wait_time; // Время сброса флегмы (мин)
    uint8_t use_bluetooth; // Использовать Bluetooth модуль
    uint8_t crc; // Контрольная сумма настроек
} CFG_DATA;
CFG_DATA CONFIG;

// Статус ректификации
#define RS_NOTHEAD 0 // Отбора голов еще не было
#define RS_YESHEAD 1 // Отбор голов в процессе
#define RS_NOTBODY 2 // Отбора тела еще не было
#define RS_YESBODY 3 // Отбор тела в процессе
#define RS_NOTCIRC 4 // Отбора оборотки еще не было
#define RS_YESCIRC 5 // Отбор оборотки в процессе
uint8_t REFLUX_STATUS = RS_NOTHEAD; // Первоначальный статус

// Текущие режимы работы
uint8_t WORKING_MODE;
#define WM_SETTING   0  // Параметры
#define WM_HEATING   1  // Нагрев куба
#define WM_WATERING  2  // Включение воды
#define WM_WORKING   3  // Работа на себя
#define WM_GETHEAD   4  // Отбор голов
#define WM_GETBODY   5  // Отбор тела
#define WM_CIRCULATE 6  // Оборотный спирт
#define WM_ERROR     7  // Ошибка
#define WM_DONE      8  // Готово
#define WM_FINISH    9  // Завершено

// Подключаем интерфейс
#include "interface.c"

// Изменить текущий режим
void set_working_mode(uint8_t mode, uint8_t form) {
    last_event_time = now_millis; // Обновляем время последнего события
    WORKING_MODE = mode; // Устанавливаем режим на новый
    mui_GotoFormAutoCursorPosition(&mui, form); // Переходим к нужной форме
    uart_printf("MODE:%i\n", mode); // Отправляем данные в UART
    // Устанавливаем фокус на первый элемент формы
    if(mui_GetCurrentFormId(&mui) == GUI_RECTIFICATE_FORM) {{ // Только для рабочего процесса
        uint8_t element = UCHAR_MAX; // Позиция самого первого элемента
        for(;;) { // В бесконечном цикле
            uint8_t cursor = mui_GetCurrentCursorFocusPosition(&mui); // Получаем текущее положение
            if(cursor < element) { // Если оно меньше сохраненного
                element = cursor; // Обновляем значение
            } else if(cursor == element) { // Если равно, значит уже прошли полный круг
                break; // Выходим из цикла
            }
            mui_NextField(&mui); // Переходим к следующему элементу
        }
    }}
    // Проигрываем звук смены режима
    tone(NOTE_E4, 115); _delay_ms(115);
    tone(NOTE_F4, 115); _delay_ms(115);
    tone(NOTE_FS4, 115); _delay_ms(115);
    tone(NOTE_GS4, 115); _delay_ms(115);
}

// Главная функция
int main(void) {
    MCUSR = 0; // Сбрасываем флаг срабатывания WDT
    wdt_disable(); // Отключаем сторожевой таймер
    init_millis(); // Инициализация счетчика миллисекунд
    // Дисплей
    u8g2_Setup_st7920_s_128x64_1(&u8g2, U8G2_R0, u8x8_byte_avr_hw_spi, u8x8_avr_gpio_and_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearDisplay(&u8g2);
    // Кнопки
    HW_BUTTON_1_DDR &= ~_BV(HW_BUTTON_1_BIT); // Кнопка №1
    HW_BUTTON_2_DDR &= ~_BV(HW_BUTTON_2_BIT); // Кнопка №2
    HW_BUTTON_3_DDR &= ~_BV(HW_BUTTON_3_BIT); // Кнопка №3
    HW_BUTTON_4_DDR &= ~_BV(HW_BUTTON_4_BIT); // Кнопка №4
    HW_BUTTON_5_DDR &= ~_BV(HW_BUTTON_5_BIT); // Кнопка №5
    // Датчики
    HW_SENSOR_WATER_DDR &= ~_BV(HW_SENSOR_WATER_BIT);
    HW_SENSOR_TSA_DDR &= ~_BV(HW_SENSOR_TSA_BIT);
    HW_SENSOR_TSARGA_DDR &= ~_BV(HW_SENSOR_TSARGA_BIT);
    HW_SENSOR_CUBE_DDR &= ~_BV(HW_SENSOR_CUBE_BIT);
    ds18b20wsp(&HW_SENSOR_WATER_PORT, &HW_SENSOR_WATER_DDR, &HW_SENSOR_WATER_PIN, _BV(HW_SENSOR_WATER_BIT), NULL, -55, 125, DS18B20_RES12);
    ds18b20wsp(&HW_SENSOR_TSA_PORT, &HW_SENSOR_TSA_DDR, &HW_SENSOR_TSA_PIN, _BV(HW_SENSOR_TSA_BIT), NULL, -55, 125, DS18B20_RES12);
    ds18b20wsp(&HW_SENSOR_TSARGA_PORT, &HW_SENSOR_TSARGA_DDR, &HW_SENSOR_TSARGA_PIN, _BV(HW_SENSOR_TSARGA_BIT), NULL, -55, 125, DS18B20_RES12);
    ds18b20wsp(&HW_SENSOR_CUBE_PORT, &HW_SENSOR_CUBE_DDR, &HW_SENSOR_CUBE_PIN, _BV(HW_SENSOR_CUBE_BIT), NULL, -55, 125, DS18B20_RES12);
    // Звук
    HW_BUZZER_DDR |= _BV(HW_BUZZER_BIT); // Зуммер
    // Реле
    HW_RELAY_WATER_DDR |= _BV(HW_RELAY_WATER_BIT);
    HW_RELAY_HEAT_DDR |= _BV(HW_RELAY_HEAT_BIT);
    HW_RELAY_ALCO_DDR |= _BV(HW_RELAY_ALCO_BIT);
    SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 0);
    SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 0);
    SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0);
    // Bluetooth модуль
    HW_BLUETOOTH_EN_DDR |= _BV(HW_BLUETOOTH_EN_BIT); // Управление питанием
    // Последовательный порт
    uart_init(9600);
    // Читаем настройки из памяти
    eeprom_busy_wait(); // Ждем доступности EEPROM
    eeprom_read_block(&CONFIG, 0, sizeof(CONFIG)); // Получаем данные
    if(crc8((uint8_t *) &CONFIG, sizeof(CONFIG) - 1) != CONFIG.crc) { // Если контрольная сумма не совпадает
        // Устанавливаем настройки по-умолчанию
        CONFIG.water_cube_temperature = 75; // Температура старта подачи воды (от 60 до 80)
        CONFIG.ethanol_cube_temperature = 95; // Максимальная температура для товарного спирта (от 85 до 110)
        CONFIG.final_cube_temperature = 100; // Температура окончания работы (от 85 до 110)
        CONFIG.itself_working_temperature = 70; // Температура начала "работы на себя" (от 60 до 80)
        CONFIG.itself_working_initial_time = 30; // Длительность "работы на себя" в минутах (от 10 до 60)
        CONFIG.target_temperature_delta = 1; // Дельта целевой температуры (ТЕМП = (ЗНАЧ * 625) / 10000)
        CONFIG.target_recovery_time = 1; // Время восстановления в минутах (от 1 до 30)
        CONFIG.sensors_protection = 0; // Включить контроль работоспособности датчиков (0 = нет, 1 = да)
        CONFIG.tsa_protection = 1; // Включить защиту по температуре ТСА (0 = нет, 1 = да)
        CONFIG.tsa_max_temperature = 40; // Максимальная температура ТСА (от 40 до 60)
        CONFIG.water_protection = 1; // Включить защиту по температуре воды (0 = нет, 1 = да)
        CONFIG.water_max_temperature = 60; // Максимальная температура воды (от 40 до 60)
        CONFIG.phlegm_wait_time = 3; // Время сброса флегмы в минутах (от 1 до 10)
        CONFIG.use_bluetooth = 0; // Использовать Bluetooth модуль (0 = нет, 1 = да)
        CONFIG.crc = 0x00; // Контрольная сумма CRC8
    }
    // Инициализация MUI-интерфейса
    mui_Init(&mui, &u8g2, fds_data, muif_list, sizeof(muif_list)/sizeof(muif_t));
    // Открываем форму настроек
    set_working_mode(WM_SETTING, GUI_SETTING_FORM);
    // Бесконечный основной цикл
    uint8_t is_redraw = 1;
    for(;;) loop(&is_redraw);
}

// Функция основного цикла
uint32_t ds18b20read_time = 0; // Время последнего опроса датчиков
void loop(uint8_t *is_redraw) {
    // Запоминаем текущее время
    now_millis = get_millis();
    // Управление Bluetooth модулем
    SET_PIN_STATE(HW_BLUETOOTH_EN_PORT, HW_BLUETOOTH_EN_BIT, 0, CONFIG.use_bluetooth);
    // Опрос кнопок
    switch(u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2))) {
        case U8X8_MSG_GPIO_MENU_PREV: // Назад
            tone(NOTE_AS4, 50);
            mui_PrevField(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_NEXT: // Далее
            tone(NOTE_AS4, 50);
            mui_NextField(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_SELECT: // Выбор
            tone(NOTE_AS4, 50);
            mui_SendSelect(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_DOWN: // Уменьшить
            tone(NOTE_AS4, 50);
            mui_SendValueDecrement(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_UP: // Увеличить
            tone(NOTE_AS4, 50);
            mui_SendValueIncrement(&mui);
            *is_redraw = 1;
            break;
    }
    // Получаем данные с датчиков температуры
    if((now_millis - ds18b20read_time) > ((uint32_t) 1 * 1000)) { // Если с момента последнего опроса прошло более 1с
        if(ds18b20read_time > 0) { // Если был хотя бы один запрос значений, считываем температуру с датчиков
            int16_t read_temperature = 0; // Временная переменная для чтения значения датчиков
            // Температура на выходе водяного охлаждения
            ds18b20read(&HW_SENSOR_WATER_PORT, &HW_SENSOR_WATER_DDR, &HW_SENSOR_WATER_PIN, _BV(HW_SENSOR_WATER_BIT), NULL, &read_temperature);
            if((read_temperature != water_temperature) && !(read_temperature == 1360 /* 85.0 */ && !water_temperature)) {
                water_temperature = read_temperature;
                uart_printf("WATER:" FPN_FORMAT "\n", FPN_FRMT_BD(water_temperature), FPN_FRMT_AD(water_temperature));
                *is_redraw = 1;
            }
            // Температура трубки связи с атмосферой
            ds18b20read(&HW_SENSOR_TSA_PORT, &HW_SENSOR_TSA_DDR, &HW_SENSOR_TSA_PIN, _BV(HW_SENSOR_TSA_BIT), NULL, &read_temperature);
            if((read_temperature != tsa_temperature) && !(read_temperature == 1360 /* 85.0 */ && !tsa_temperature)) {
                tsa_temperature = read_temperature;
                uart_printf("TSA:" FPN_FORMAT "\n", FPN_FRMT_BD(tsa_temperature), FPN_FRMT_AD(tsa_temperature));
                *is_redraw = 1;
            }
            // Температура в царге
            ds18b20read(&HW_SENSOR_TSARGA_PORT, &HW_SENSOR_TSARGA_DDR, &HW_SENSOR_TSARGA_PIN, _BV(HW_SENSOR_TSARGA_BIT), NULL, &read_temperature);
            if((read_temperature != tsarga_temperature) && !(read_temperature == 1360 /* 85.0 */ && !tsarga_temperature)) {
                tsarga_temperature = read_temperature;
                uart_printf("TSARGA:" FPN_FORMAT "\n", FPN_FRMT_BD(tsarga_temperature), FPN_FRMT_AD(tsarga_temperature));
                *is_redraw = 1;
            }
            // Температура в кубе
            ds18b20read(&HW_SENSOR_CUBE_PORT, &HW_SENSOR_CUBE_DDR, &HW_SENSOR_CUBE_PIN, _BV(HW_SENSOR_CUBE_BIT), NULL, &read_temperature);
            if((read_temperature != cube_temperature) && !(read_temperature == 1360 /* 85.0 */ && !cube_temperature)) {
                cube_temperature = read_temperature;
                uart_printf("CUBE:" FPN_FORMAT "\n", FPN_FRMT_BD(cube_temperature), FPN_FRMT_AD(cube_temperature));
                *is_redraw = 1;
            }
        }
        // Запрашиваем значения температуры у датчиков
        ds18b20convert(&HW_SENSOR_WATER_PORT, &HW_SENSOR_WATER_DDR, &HW_SENSOR_WATER_PIN, _BV(HW_SENSOR_WATER_BIT), NULL); // Датчик воды
        ds18b20convert(&HW_SENSOR_TSA_PORT, &HW_SENSOR_TSA_DDR, &HW_SENSOR_TSA_PIN, _BV(HW_SENSOR_TSA_BIT), NULL); // Датчик ТСА
        ds18b20convert(&HW_SENSOR_TSARGA_PORT, &HW_SENSOR_TSARGA_DDR, &HW_SENSOR_TSARGA_PIN, _BV(HW_SENSOR_TSARGA_BIT), NULL); // Датчик царги
        ds18b20convert(&HW_SENSOR_CUBE_PORT, &HW_SENSOR_CUBE_DDR, &HW_SENSOR_CUBE_PIN, _BV(HW_SENSOR_CUBE_BIT), NULL); // Датчик куба
        // Запоминаем время опроса
        ds18b20read_time = now_millis;
    }
    // Логика работы
    switch(WORKING_MODE) {
        case WM_SETTING: // Параметры
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 0); // Отключаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 0); // Отключаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            break;
        case WM_HEATING: // Нагрев куба
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 0); // Отключаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 1); // Включаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            // Проверяем, не пора ли включить подачу воды
            if(cube_temperature >= FPN_SBD(CONFIG.water_cube_temperature)) {
                set_working_mode(WM_WATERING, GUI_RECTIFICATE_FORM); // Включаем воду
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_WATERING: // Включение воды
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 1); // Включаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 1); // Включаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            // Проверяем, не пора ли начать "работать на себя"
            if(tsarga_temperature >= FPN_SBD(CONFIG.itself_working_temperature)) {
                set_working_mode(WM_WORKING, GUI_RECTIFICATE_FORM); // Режим "работа на себя"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_WORKING: // Работа на себя
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 1); // Включаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 1); // Включаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            // Если отбора голов еще не было
            if(REFLUX_STATUS == RS_NOTHEAD) {
                // Начальная "работа на себя" заданное время
                if((now_millis - last_event_time) >= ((uint32_t) CONFIG.itself_working_initial_time * 60 * 1000)) {
                    target_temperature = tsarga_temperature; // Запоминаем целевую температуру царги
                    set_working_mode(WM_GETHEAD, GUI_RECTIFICATE_FORM); // Переходим к отбору голов
                    REFLUX_STATUS = RS_YESHEAD; // Отбор голов в процессе
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            } else { // Отбор голов или тела в процессе, но сейчас нужна промежуточная "поработать на себя"
                if((now_millis - last_event_time) > ((uint32_t) CONFIG.target_recovery_time * 60 * 1000)) { // Если за заданное время
                    // Если значение температуры датчика изменилось не более суммы целевой температуры и дельты
                    if((tsarga_temperature - target_temperature) <= CONFIG.target_temperature_delta) {
                        // Если отбор голов в процессе
                        if(REFLUX_STATUS == RS_YESHEAD) {
                            set_working_mode(WM_GETHEAD, GUI_RECTIFICATE_FORM); // Возвращаемся к отбору голов
                        } else {
                            // Отбор голов закончили, но к отбору тела еще не приступили
                            if(REFLUX_STATUS == RS_NOTBODY) {
                                target_temperature = tsarga_temperature; // Запоминаем целевую температуру царги
                                REFLUX_STATUS = RS_YESBODY; // Теперь начинаем отбор тела
                            }
                            set_working_mode(WM_GETBODY, GUI_RECTIFICATE_FORM); // Приступаем к отбору тела
                        }
                        *is_redraw = 1; // Нужна перерисовка интерфейса
                    }
                    // Отбор голов в процессе или закончен, но к отбору тела еще не приступили
                    if(REFLUX_STATUS == RS_YESHEAD || REFLUX_STATUS == RS_NOTBODY) {
                        target_temperature = tsarga_temperature; // Запоминаем целевую температуру царги
                    }
                    last_event_time = now_millis; // Обновляем время последнего события
                }
            }
            break;
        case WM_GETHEAD: // Отбор голов
        case WM_GETBODY: // Отбор тела
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 1); // Включаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 1); // Включаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 1); // Включаем отбор
            // Если значение температуры датчика стало больше суммы целевой температуры и дельты
            if((tsarga_temperature - target_temperature) > CONFIG.target_temperature_delta) {
                set_working_mode(WM_WORKING, GUI_RECTIFICATE_FORM); // Начинаем "работать на себя"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            // Во время отбора тела
            if(REFLUX_STATUS == RS_YESBODY) {
                // Если пора завершить отбор товарного спирта
                if(cube_temperature >= FPN_SBD(CONFIG.ethanol_cube_temperature)) {
                    set_working_mode(WM_CIRCULATE, GUI_CIRCULATE_FORM); // Подтверждение сбора оборотки
                    REFLUX_STATUS = RS_NOTCIRC; // Но сам отбор пока не начинаем
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            }
            // Во время отбора оборотного спирта
            if(REFLUX_STATUS == RS_YESCIRC) {
                // Если пора завершать ректификацию
                if(cube_temperature >= FPN_SBD(CONFIG.final_cube_temperature)) {
                    set_working_mode(WM_DONE, GUI_FINISH_FORM); // Режим "Готово"
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            }
            break;
        case WM_CIRCULATE: // Подтверждение начала сбора оборотного спирта
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 1); // Включаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 1); // Включаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            break;
        case WM_ERROR: // Ошибка
            if((now_millis % 1000) == 0) { // Каждую секунду
                tone(NOTE_AS4, 500); // Звуковой сигнал 500мс
            }
        case WM_DONE: // Готово
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 1); // Включаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 0); // Отключаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            // Сбрасываем флегму в течении заданного времени
            if((now_millis - last_event_time) > ((uint32_t) CONFIG.phlegm_wait_time * 60 * 1000)) {
                set_working_mode(WM_FINISH, GUI_FINISH_FORM); // Режим "Завершено"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_FINISH: // Завершено
            // Управляем реле
            SET_PIN_STATE(HW_RELAY_WATER_PORT, HW_RELAY_WATER_BIT, HW_RELAY_WATER_INVERT, 0); // Отключаем воду
            SET_PIN_STATE(HW_RELAY_HEAT_PORT, HW_RELAY_HEAT_BIT, HW_RELAY_HEAT_INVERT, 0); // Отключаем нагрев
            SET_PIN_STATE(HW_RELAY_ALCO_PORT, HW_RELAY_ALCO_BIT, HW_RELAY_ALCO_INVERT, 0); // Отключаем отбор
            break;
    }

    // Защита от опасных ситуаций
    switch(WORKING_MODE) { // Для режимов
        case WM_SETTING: // Параметры
        case WM_ERROR: // Ошибка
        case WM_DONE: // Готово
        case WM_FINISH: // Завершено
            break; // Защита не нужна
        default: // А для остальных режимов (во время ректификации)
            if((CONFIG.sensors_protection && ( // Контроль работоспособности датчиков
                    (CONFIG.water_protection && !water_temperature) // Датчик температуры воды
                    ||
                    (CONFIG.tsa_protection && !tsa_temperature) // Датчик температуры ТСА
                    ||
                    (!tsarga_temperature) // Датчик температуры в царге
                    ||
                    (!cube_temperature) // Датчик температуры в кубе
               )) ||
               (CONFIG.tsa_protection && (tsa_temperature > FPN_SBD(CONFIG.tsa_max_temperature))) // Превышение температуры ТСА
               ||
               (CONFIG.water_protection && (water_temperature > FPN_SBD(CONFIG.water_max_temperature))) // Превышение температуры воды
            ) {
                set_working_mode(WM_ERROR, GUI_FINISH_FORM); // Режим "ошибка"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
    }
    // Отрисовка интерфейса
    if(mui_IsFormActive(&mui)) {
        if(*is_redraw) {
            u8g2_FirstPage(&u8g2);
            do {
                mui_Draw(&mui);
            } while(u8g2_NextPage(&u8g2));
            *is_redraw = 0;
        }
    }
}
