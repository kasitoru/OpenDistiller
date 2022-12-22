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
#include "libraries/u8g2/csrc/mui_u8g2.h" // U8g2
#include "libraries/tone/tone.h" // Звуки
#include "libraries/functions.h" // Разное

#if defined(UART) || defined(DEBUG)
    #include "libraries/uart/uart.h" // Последовательный порт
    #ifdef DEBUG
        #include "libraries/debug/debug.h" // Функции отладки
    #endif
#endif

// Глобальные переменные
u8g2_t u8g2; // Дисплей
mui_t mui; // Интерфейс

float water_temperature = 0; // Температура на выходе водяного охлаждения (датчик 1)
float tsa_temperature = 0; // Температура трубки связи с атмосферой (датчик 2)
float reflux_temperature = 0; // Температура флегмы в узле отбора (датчик 3)
float tsarga_temperature = 0; // Температура в царге (датчик 4)
float cube_temperature = 0; // Температура в кубе (датчик 5)

uint32_t now_millis = 0; // Текущее время с начала включения
uint32_t rect_start_time = 0; // Время начала ректификации
uint32_t last_event_time = 0; // Время последнего события
float target_tsarga_temp = 0; // Целевая температура царги
float target_reflux_temp = 0; // Целевая температура УО

void loop(uint8_t *is_redraw); // Основной цикл

// Параметры ректификации
typedef struct {
    uint8_t rectification_mode; // Режим ректификации (0 = автоматический, 1 = ручной)
    uint8_t use_tsarga_sensor; // Использовать датчик в царге для контроля отбора
    uint8_t use_reflux_sensor; // Использовать датчик УО для контроля отбора
    uint8_t delta_tsarga_before; // Дельта датчика в царге (до запятой)
    uint8_t delta_tsarga_after; // Дельта датчика в царге (после запятой)
    uint8_t delta_reflux_before; // Дельта датчика УО (до запятой)
    uint8_t delta_reflux_after; // Дельта датчика УО (после запятой)
    uint8_t itself_working_temperature; // Температура начала "работы на себя"
    uint8_t itself_working_initial_time; // Время начальной "работы на себя" (мин)
    uint8_t itself_working_interim_time; // Время промежуточной "работы на себя" (мин)
    uint8_t water_cube_temperature; // Температура старта подачи воды
    uint8_t ethanol_cube_temperature; // Максимальная температура для товарного спирта
    uint8_t final_cube_temperature; // Температура окончания работы
    uint8_t sensors_protection; // Включить контроль работоспособности датчиков
    uint8_t tsa_protection; // Включить защиту по температуре ТСА
    uint8_t tsa_max_temperature; // Максимальная температура ТСА
    uint8_t water_protection; // Включить защиту по температуре воды
    uint8_t water_max_temperature; // Максимальная температура воды
    uint8_t phlegm_wait_time; // Время сброса флегмы (мин)
    #ifdef BLUETOOTH
        uint8_t use_bluetooth; // Использовать Bluetooth модуль
    #endif
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
#define WM_MANUAL    1  // Ручной режим
#define WM_HEATING   2  // Нагрев куба
#define WM_WATERING  3  // Включение воды
#define WM_WORKING   4  // Работа на себя
#define WM_GETHEAD   5  // Отбор голов
#define WM_GETBODY   6  // Отбор тела
#define WM_CIRCULATE 7  // Оборотный спирт
#define WM_ERROR     8  // Ошибка
#define WM_DONE      9  // Готово
#define WM_FINISH    10 // Завершено

// Подключаем интерфейс
#include "interface.c"

// Изменить текущий режим
void set_working_mode(uint8_t mode, uint8_t form) {
    last_event_time = now_millis; // Обновляем время последнего события
    WORKING_MODE = mode; // Устанавливаем режим на новый
    mui_GotoFormAutoCursorPosition(&mui, form); // Переходим к нужной форме
    #ifdef UART
        uart_printf("MODE:%i\n", mode); // Отправляем данные в UART
    #endif
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
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_SPI_CLOCK, U8X8_AVR_PB5);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_SPI_DATA, U8X8_AVR_PB3);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_CS, U8X8_AVR_PB2);
    u8g2_InitDisplay(&u8g2);
    u8g2_ClearDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    // Кнопки
    DDRD &= ~_BV(PD4); u8g2_SetMenuPrevPin(&u8g2, U8X8_AVR_PB0); // Кнопка №1
    DDRD &= ~_BV(PD5); u8g2_SetMenuNextPin(&u8g2, U8X8_AVR_PD7); // Кнопка №2
    DDRD &= ~_BV(PD6); u8g2_SetMenuSelectPin(&u8g2, U8X8_AVR_PD6); // Кнопка №3
    DDRD &= ~_BV(PD7); u8g2_SetMenuDownPin(&u8g2, U8X8_AVR_PD5); // Кнопка №4
    DDRB &= ~_BV(PB0); u8g2_SetMenuUpPin(&u8g2, U8X8_AVR_PD4); // Кнопка №5
    // Датчики
    DDRC &= ~_BV(PC0); ds18b20wsp(&PORTC, &DDRC, &PINC, _BV(PC0), NULL, -55, 125, DS18B20_RES12); // Датчик №1
    DDRC &= ~_BV(PC1); ds18b20wsp(&PORTC, &DDRC, &PINC, _BV(PC1), NULL, -55, 125, DS18B20_RES12); // Датчик №2
    DDRC &= ~_BV(PC2); ds18b20wsp(&PORTC, &DDRC, &PINC, _BV(PC2), NULL, -55, 125, DS18B20_RES12); // Датчик №3
    DDRD &= ~_BV(PD3); ds18b20wsp(&PORTD, &DDRD, &PIND, _BV(PD3), NULL, -55, 125, DS18B20_RES12); // Датчик №4
    DDRD &= ~_BV(PD2); ds18b20wsp(&PORTD, &DDRD, &PIND, _BV(PD2), NULL, -55, 125, DS18B20_RES12); // Датчик №5
    // Звук
    DDRB |= _BV(PB1); // Зуммер
    // Реле
    DDRC |= _BV(PC5); PORTC |= _BV(PC5); // Реле №1
    DDRC |= _BV(PC4); PORTC |= _BV(PC4); // Реле №2
    DDRC |= _BV(PC3); PORTC |= _BV(PC3); // Реле №3
    #ifdef BLUETOOTH
        // Bluetooth модуль
        DDRB |= _BV(PB4); // Управление питанием
    #endif
    #if defined(UART) || defined(DEBUG)
        // Последовательный порт
        uart_init(9600);
    #endif
    // Читаем настройки из памяти
    eeprom_busy_wait(); // Ждем доступности EEPROM
    eeprom_read_block(&CONFIG, 0, sizeof(CONFIG)); // Получаем данные
    if(crc8((uint8_t *) &CONFIG, sizeof(CONFIG) - 1) != CONFIG.crc) { // Если контрольная сумма не совпадает
        // Устанавливаем настройки по-умолчанию
        CONFIG.rectification_mode = 0; // Режим ректификации (0 = автоматический, 1 = ручной)
        CONFIG.use_tsarga_sensor = 1; // Использовать датчик в царге для контроля отбора (0 = нет, 1 = да)
        CONFIG.use_reflux_sensor = 0; // Использовать датчик УО для контроля отбора (0 = нет, 1 = да)
        CONFIG.delta_tsarga_before = 0; // Дельта датчика в царге до запятой (от 0 до 9)
        CONFIG.delta_tsarga_after = 10; // Дельта датчика в царге после запятой (от 0 до 99)
        CONFIG.delta_reflux_before = 0; // Дельта датчика УО до запятой (от 0 до 9)
        CONFIG.delta_reflux_after = 10; // Дельта датчика УО после запятой (от 0 до 99)
        CONFIG.itself_working_temperature = 85; // Температура начала "работы на себя" (от 70 до 85)
        CONFIG.itself_working_initial_time = 30; // Время начальной "работы на себя" в минутах (от 10 до 60)
        CONFIG.itself_working_interim_time = 1; // Время промежуточной "работы на себя" в минутах (от 1 до 30)
        CONFIG.water_cube_temperature = 75; // Температура старта подачи воды (от 60 до 80)
        CONFIG.ethanol_cube_temperature = 95; // Максимальная температура для товарного спирта (от 85 до 110)
        CONFIG.final_cube_temperature = 100; // Температура окончания работы (от 85 до 110)
        CONFIG.sensors_protection = 0; // Включить контроль работоспособности датчиков (0 = нет, 1 = да)
        CONFIG.tsa_protection = 1; // Включить защиту по температуре ТСА (0 = нет, 1 = да)
        CONFIG.tsa_max_temperature = 40; // Максимальная температура ТСА (от 40 до 60)
        CONFIG.water_protection = 1; // Включить защиту по температуре воды (0 = нет, 1 = да)
        CONFIG.water_max_temperature = 60; // Максимальная температура воды (от 40 до 60)
        CONFIG.phlegm_wait_time = 1; // Время сброса флегмы в минутах (от 1 до 10) // FIXME
        #ifdef BLUETOOTH
            CONFIG.use_bluetooth = 0; // Использовать Bluetooth модуль (0 = нет, 1 = да)
        #endif
        CONFIG.crc = 0x00; // Контрольная сумма CRC8
    }
    // Очищаем дисплей
    u8g2_ClearDisplay(&u8g2);
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
    #ifdef DEBUG
        // Если включен режим отладки
        if((now_millis % 1000) == 0) { // Каждую секунду
            // Отправляем количество свободной оперативной памяти
            uart_printf("RAM:%i\n", free_ram());
        }
    #endif
    #ifdef BLUETOOTH
        // Управление Bluetooth модулем
        if(CONFIG.use_bluetooth) {
            PORTB |= _BV(PB4); // Включаем
        } else {
            PORTB &= ~_BV(PB4); // Отключаем
        }
    #endif
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
        case U8X8_MSG_GPIO_MENU_SELECT: // Выбор пункта меню
            tone(NOTE_AS4, 50);
            mui_SendSelect(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_DOWN: // Уменьшить значение
            tone(NOTE_AS4, 50);
            mui_SendValueDecrement(&mui);
            *is_redraw = 1;
            break;
        case U8X8_MSG_GPIO_MENU_UP: // Увеличить значение
            tone(NOTE_AS4, 50);
            mui_SendValueIncrement(&mui);
            *is_redraw = 1;
            break;
    }
    // Получаем данные с датчиков температуры
    if((now_millis - ds18b20read_time) > ((uint32_t) 1 * 1000)) { // Если с момента последнего опроса прошло более 1с
        if(ds18b20read_time > 0) { // Если был хотя бы один запрос значений, считываем температуру с датчиков
            int16_t temperature_integer = 0; // Временная переменная для чтения значения датчиков
            float temperature_float = 0; // Временная переменная для расчета значения датчиков
            // Температура на выходе водяного охлаждения (датчик 1)
            ds18b20read(&PORTC, &DDRC, &PINC, _BV(PC0), NULL, &temperature_integer);
            temperature_float = ((float) temperature_integer / 16);
            if((temperature_float != water_temperature)/* && !(temperature_integer == 1360 && !water_temperature)*/) {
                water_temperature = temperature_float;
                #ifdef UART
                    uart_printf("WATER:%.2f\n", water_temperature);
                #endif
                *is_redraw = 1;
            }
            // Температура трубки связи с атмосферой (датчик 2)
            ds18b20read(&PORTC, &DDRC, &PINC, _BV(PC1), NULL, &temperature_integer);
            temperature_float = ((float) temperature_integer / 16);
            if((temperature_float != tsa_temperature)/* && !(temperature_integer == 1360 && !tsa_temperature)*/) {
                tsa_temperature = temperature_float;
                #ifdef UART
                    uart_printf("TSA:%.2f\n", tsa_temperature);
                #endif
                *is_redraw = 1;
            }
            // Температура флегмы в узле отбора (датчик 3)
            ds18b20read(&PORTC, &DDRC, &PINC, _BV(PC2), NULL, &temperature_integer);
            temperature_float = ((float) temperature_integer / 16);
            if((temperature_float != reflux_temperature)/* && !(temperature_integer == 1360 && !reflux_temperature)*/) {
                reflux_temperature = temperature_float;
                #ifdef UART
                    uart_printf("REFLUX:%.2f\n", reflux_temperature);
                #endif
                *is_redraw = 1;
            }
            // Температура в царге (датчик 4)
            ds18b20read(&PORTD, &DDRD, &PIND, _BV(PD3), NULL, &temperature_integer);
            temperature_float = ((float) temperature_integer / 16);
            if((temperature_float != tsarga_temperature)/* && !(temperature_integer == 1360 && !tsarga_temperature)*/) {
                tsarga_temperature = temperature_float;
                #ifdef UART
                    uart_printf("TSARGA:%.2f\n", tsarga_temperature);
                #endif
                *is_redraw = 1;
            }
            // Температура в кубе (датчик 5)
            ds18b20read(&PORTD, &DDRD, &PIND, _BV(PD2), NULL, &temperature_integer);
            temperature_float = ((float) temperature_integer / 16);
            if((temperature_float != cube_temperature)/* && !(temperature_integer == 1360 && !cube_temperature)*/) {
                cube_temperature = temperature_float;
                #ifdef UART
                    uart_printf("CUBE:%.2f\n", cube_temperature);
                #endif
                *is_redraw = 1;
            }
        }
        // Запрашиваем значения температуры у датчиков
        ds18b20convert(&PORTC, &DDRC, &PINC, _BV(PC0), NULL); // Датчик 1
        ds18b20convert(&PORTC, &DDRC, &PINC, _BV(PC1), NULL); // Датчик 2
        ds18b20convert(&PORTC, &DDRC, &PINC, _BV(PC2), NULL); // Датчик 3
        ds18b20convert(&PORTD, &DDRD, &PIND, _BV(PD3), NULL); // Датчик 4
        ds18b20convert(&PORTD, &DDRD, &PIND, _BV(PD2), NULL); // Датчик 5
        // Запоминаем время опроса
        ds18b20read_time = now_millis;
    }
    // Логика работы
    switch(WORKING_MODE) {
        case WM_SETTING: // Параметры
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC |= _BV(PC4); // Отключаем нагрев
            PORTC |= _BV(PC5); // Отключаем воду
            break;
        #ifdef MANUAL
            case WM_MANUAL: // Ручной режим
                break;
        #endif
        case WM_HEATING: // Нагрев куба
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC &= ~_BV(PC4); // Включаем нагрев
            PORTC |= _BV(PC5); // Отключаем воду
            // Проверяем, не пора ли включить подачу воды
            if(cube_temperature >= ((float) CONFIG.water_cube_temperature)) {
                set_working_mode(WM_WATERING, GUI_RECTIFICATE_FORM); // Включаем воду
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_WATERING: // Включение воды
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC &= ~_BV(PC4); // Включаем нагрев
            PORTC &= ~_BV(PC5); // Включаем воду
            // Проверяем, не пора ли начать "работать на себя"
            if(cube_temperature >= ((float) CONFIG.itself_working_temperature)) {
                set_working_mode(WM_WORKING, GUI_RECTIFICATE_FORM); // Режим "работа на себя"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_WORKING: // Работа на себя
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC &= ~_BV(PC4); // Включаем нагрев
            PORTC &= ~_BV(PC5); // Включаем воду
            // Если отбора голов еще не было
            if(REFLUX_STATUS == RS_NOTHEAD) {
                // Начальная "работа на себя" заданное время
                if((now_millis - last_event_time) >= ((uint32_t) CONFIG.itself_working_initial_time * 60 * 1000)) {
                    target_tsarga_temp = tsarga_temperature; // Запоминаем целевую температуру царги
                    target_reflux_temp = reflux_temperature; // Запоминаем целевую температуру УО
                    set_working_mode(WM_GETHEAD, GUI_RECTIFICATE_FORM); // Переходим к отбору голов
                    REFLUX_STATUS = RS_YESHEAD; // Отбор голов в процессе
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            } else { // Отбор голов или тела в процессе, но сейчас нужна промежуточная "поработать на себя"
                if((now_millis - last_event_time) > ((uint32_t) CONFIG.itself_working_interim_time * 60 * 1000)) { // Если за заданное время
                    // Если значения температур датчиков меняются не больше суммы целевой температуры и дельты
                    if((!CONFIG.use_tsarga_sensor || (CONFIG.use_tsarga_sensor && ((tsarga_temperature - target_tsarga_temp) <= ((float) CONFIG.delta_tsarga_after / 100 + CONFIG.delta_tsarga_before)))) // Царга
                       &&
                       (!CONFIG.use_reflux_sensor || (CONFIG.use_reflux_sensor && ((reflux_temperature - target_reflux_temp) <= ((float) CONFIG.delta_reflux_after / 100 + CONFIG.delta_reflux_before)))) // УО
                    ) {
                        // Если отбор голов в процессе
                        if(REFLUX_STATUS == RS_YESHEAD) {
                            set_working_mode(WM_GETHEAD, GUI_RECTIFICATE_FORM); // Возвращаемся к отбору голов
                        } else {
                            // Отбор голов закончили, но к отбору тела еще не приступили
                            if(REFLUX_STATUS == RS_NOTBODY) {
                                target_tsarga_temp = tsarga_temperature; // Запоминаем целевую температуру царги
                                target_reflux_temp = reflux_temperature; // Запоминаем целевую температуру УО
                                REFLUX_STATUS = RS_YESBODY; // Теперь начинаем отбор тела
                            }
                            set_working_mode(WM_GETBODY, GUI_RECTIFICATE_FORM); // Приступаем к отбору тела
                        }
                        *is_redraw = 1; // Нужна перерисовка интерфейса
                    }
                    // Отбор голов в процессе или закончен, но к отбору тела еще не приступили
                    if(REFLUX_STATUS == RS_YESHEAD || REFLUX_STATUS == RS_NOTBODY) {
                        target_tsarga_temp = tsarga_temperature; // Запоминаем целевую температуру царги
                        target_reflux_temp = reflux_temperature; // Запоминаем целевую температуру УО
                    }
                    last_event_time = now_millis; // Обновляем время последнего события
                }
            }
            break;
        case WM_GETHEAD: // Отбор голов
        case WM_GETBODY: // Отбор тела
            // Управляем реле
            PORTC &= ~_BV(PC3); // Включаем отбор
            PORTC &= ~_BV(PC4); // Включаем нагрев
            PORTC &= ~_BV(PC5); // Включаем воду
            // Если значения температур датчиков стали больше суммы целевой температуры и дельты
            if((!CONFIG.use_tsarga_sensor || (CONFIG.use_tsarga_sensor && ((tsarga_temperature - target_tsarga_temp) > ((float) CONFIG.delta_tsarga_after / 100 + CONFIG.delta_tsarga_before)))) // Царга
               &&
               (!CONFIG.use_reflux_sensor || (CONFIG.use_reflux_sensor && ((reflux_temperature - target_reflux_temp) > ((float) CONFIG.delta_reflux_after / 100 + CONFIG.delta_reflux_before)))) // УО
            ) {
                set_working_mode(WM_WORKING, GUI_RECTIFICATE_FORM); // Начинаем "работать на себя"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            // Во время отбора тела
            if(REFLUX_STATUS == RS_YESBODY) {
                // Если пора завершить отбор товарного спирта
                if(cube_temperature >= CONFIG.ethanol_cube_temperature) {
                    set_working_mode(WM_CIRCULATE, GUI_CIRCULATE_FORM); // Подтверждение сбора оборотки
                    REFLUX_STATUS = RS_NOTCIRC; // Но сам отбор пока не начинаем
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            }
            // Во время отбора оборотного спирта
            if(REFLUX_STATUS == RS_YESCIRC) {
                // Если пора завершать ректификацию
                if(cube_temperature >= CONFIG.final_cube_temperature) {
                    set_working_mode(WM_DONE, GUI_FINISH_FORM); // Режим "Готово"
                    *is_redraw = 1; // Нужна перерисовка интерфейса
                }
            }
            break;
        case WM_CIRCULATE: // Подтверждение начала сбора оборотного спирта
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC &= ~_BV(PC4); // Включаем нагрев
            PORTC &= ~_BV(PC5); // Включаем воду
            break;
        case WM_ERROR: // Ошибка
            if((now_millis % 1000) == 0) { // Каждую секунду
                tone(NOTE_AS4, 500); // Звуковой сигнал 500мс
            }
        case WM_DONE: // Готово
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC |= _BV(PC4); // Отключаем нагрев
            PORTC &= ~_BV(PC5); // Включаем воду
            // Сбрасываем флегму в течении заданного времени
            if((now_millis - last_event_time) > ((uint32_t) CONFIG.phlegm_wait_time * 60 * 1000)) {
                set_working_mode(WM_FINISH, GUI_FINISH_FORM); // Режим "Завершено"
                *is_redraw = 1; // Нужна перерисовка интерфейса
            }
            break;
        case WM_FINISH: // Завершено
            // Управляем реле
            PORTC |= _BV(PC3); // Отключаем отбор
            PORTC |= _BV(PC4); // Отключаем нагрев
            PORTC |= _BV(PC5); // Отключаем воду
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
                    ((WORKING_MODE != WM_MANUAL) && CONFIG.use_reflux_sensor && !reflux_temperature) // Датчик температуры в узле отбора
                    ||
                    ((WORKING_MODE != WM_MANUAL) && CONFIG.use_tsarga_sensor && !tsarga_temperature) // Датчик температуры в царге
                    ||
                    ((WORKING_MODE != WM_MANUAL) && !cube_temperature) // Датчик температуры в кубе
               )) ||
               (CONFIG.tsa_protection && (tsa_temperature > CONFIG.tsa_max_temperature)) // Превышение температуры ТСА
               ||
               (CONFIG.water_protection && (water_temperature > CONFIG.water_max_temperature)) // Превышение температуры воды
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
