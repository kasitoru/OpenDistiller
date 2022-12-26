/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#include "libraries/translate/translate.h" // Перевод

#define GUI_NONE_FORM        0 // Зарезервировано
#define GUI_SETTING_FORM     1 // Параметры ректификации
#define GUI_SENSORS_FORM     2 // Датчики отбора
#define GUI_WORKING_FORM     3 // Работа на себя
#define GUI_CUBETEMP_FORM    4 // Температура в кубе
#define GUI_SAFETY_FORM      5 // Безопасность
#define GUI_BLUETOOTH_FORM   6 // Bluetooth модуль
#define GUI_RESET_FORM       7 // Сброс настроек
#define GUI_ABOUT_FORM       8 // О проекте
#define GUI_RECTIFICATE_FORM 100 // Процесс ректификации
#define GUI_CIRCULATE_FORM   150 // Отбор оборотного спирта
#define GUI_FINISH_FORM      200 // Ректификация завершена

#define GUI_ENWATER_BUTTON   101 // Кнопка "ВКЛ ВОДУ"
#define GUI_SKIP_BUTTON      102 // Кнопка "ПРОПУСТИТЬ"
#define GUI_GETBODY_BUTTON   103 // Кнопка "ОТБОР ТЕЛА"
#define GUI_WORKING_BUTTON   104 // Кнопка "НА СЕБЯ"
#define GUI_FINISH_BUTTON    105 // Кнопка "ЗАВЕРШИТЬ"

// https://github.com/olikraus/u8g2/issues/1988
#define _MUI_FORM(n) MUI_FORM(n)
#define _MUI_GOTO(x, y, n, text) MUI_GOTO(x, y, n, text)
#define _MUI_XYA(id, x, y, a) MUI_XYA(id, x, y, a)
#define _MUI_XYT(id, x, y, text) MUI_XYT(id, x, y, text)
#define _MUI_XYAT(id, x, y, a, text) MUI_XYAT(id, x, y, a, text)

void set_working_mode(uint8_t mode, uint8_t form); // Изменить текущий режим

// Обычный шрифт
const uint8_t mui_style_font_normal(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            u8g2_SetFont(mui_get_U8g2(ui), I18N_NORMAL_FONT);
            break;
    }
    return 0;
}

// Жирный шрифт
const uint8_t mui_style_font_bold(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            u8g2_SetFont(mui_get_U8g2(ui), I18N_BOLD_FONT);
            break;
    }
    return 0;
}

// Текстовая надпись
#define TEXTLABEL_ALIGN_NONE   0 // Без выравнивания
#define TEXTLABEL_ALIGN_LEFT   1 // По левому краю
#define TEXTLABEL_ALIGN_CENTER 2 // По центру
#define TEXTLABEL_ALIGN_RIGHT  3 // По правому краю
uint8_t mui_text_label(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            u8g2_t *u8g2 = mui_get_U8g2(ui);
            u8g2_uint_t x = mui_get_x(ui);
            switch(ui->arg) { // Выравнивание
                case TEXTLABEL_ALIGN_LEFT:   x = 0 + mui_get_x(ui); break;
                case TEXTLABEL_ALIGN_CENTER: x = (u8g2_GetDisplayWidth(u8g2) - (u8g2_GetUTF8Width(u8g2, ui->text))) / 2; break;
                case TEXTLABEL_ALIGN_RIGHT:  x = u8g2_GetDisplayWidth(u8g2) - u8g2_GetUTF8Width(u8g2, ui->text) - mui_get_x(ui); break;
            }
            u8g2_DrawUTF8(u8g2, x, mui_get_y(ui), ui->text); // Отображаем текст
            break;
    }
    return 0;
}

// Заголовок экрана
uint8_t mui_header_label(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            // Выбираем нужный текст, в зависимости от текущей формы
            switch(mui_GetCurrentFormId(ui)) {
                case GUI_SETTING_FORM:   strcpy(ui->text, I18N_SETTING_MENU);   break;
                case GUI_SENSORS_FORM:   strcpy(ui->text, I18N_SENSORS_MENU);   break;
                case GUI_WORKING_FORM:   strcpy(ui->text, I18N_WORKING_MENU);   break;
                case GUI_CUBETEMP_FORM:  strcpy(ui->text, I18N_CUBETEMP_MENU);  break;
                case GUI_SAFETY_FORM:    strcpy(ui->text, I18N_SAFETY_MENU);    break;
                #ifdef BLUETOOTH
                    case GUI_BLUETOOTH_FORM: strcpy(ui->text, I18N_BLUETOOTH_MENU); break;
                #endif
                case GUI_RESET_FORM:     strcpy(ui->text, I18N_RESET_MENU);     break;
                case GUI_ABOUT_FORM:     strcpy(ui->text, I18N_ABOUT_MENU);     break;
                default:
                    // Текст в зависимости от режима работы
                    switch(WORKING_MODE) {
                        case WM_HEATING:
                        case WM_WATERING:  strcpy(ui->text, I18N_HEATING_TITLE);   break;
                        case WM_WORKING:   strcpy(ui->text, I18N_WORKING_TITLE);   break;
                        case WM_GETHEAD:   strcpy(ui->text, I18N_GETHEAD_TITLE);   break;
                        case WM_GETBODY:   strcpy(ui->text, I18N_GETBODY_TITLE);   break;
                        case WM_CIRCULATE: strcpy(ui->text, I18N_CIRCULATE_TITLE); break;
                        case WM_ERROR:     strcpy(ui->text, I18N_ERROR_TITLE);     break;
                        case WM_DONE:      strcpy(ui->text, I18N_DONE_TITLE);      break;
                        case WM_FINISH:    strcpy(ui->text, I18N_FINISH_TITLE);    break;
                    }
            }
            // Рендеринг заголовка
            u8g2_t *u8g2 = mui_get_U8g2(ui);
            const uint8_t *font = u8g2->font; // Запоминаем текущий шрифт
            mui_style_font_bold(ui, msg); // Устанавливаем жирный шрифт
            ui->x = 0; // Отступ слева
            ui->y = 9; // Положение по вертикали
            ui->arg = TEXTLABEL_ALIGN_LEFT; // Выравнивание по левому краю
            mui_text_label(ui, msg); // Отображаем заголовок
            u8g2_DrawHLine(u8g2, 0, 12, u8g2_GetDisplayWidth(u8g2)); // Рисуем горизонтальную линию
            u8g2_SetFont(u8g2, font); // Восстанавливаем предыдущий шрифт
            break;
    }
    return 0;
}

// Значение температуры
#define TEMP_VALUE_WATER  1 // Температура на выходе водяного охлаждения (датчик 1)
#define TEMP_VALUE_TSA    2 // Температура трубки связи с атмосферой (датчик 2)
#define TEMP_VALUE_TSARGA 3 // Температура в царге (датчик 3)
#define TEMP_VALUE_CUBE   4 // Температура в кубе (датчик 4)
#define TEMP_VALUE_TARGET 5 // Целевая температура царги
uint8_t mui_temp_value(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            float temperature = 0;
            // Определяем, значение с какого именно датчика необходимо отобразить
            switch(ui->arg) {
                case TEMP_VALUE_WATER:  temperature = water_temperature; break;
                case TEMP_VALUE_TSA:    temperature = tsa_temperature; break;
                case TEMP_VALUE_TSARGA: temperature = tsarga_temperature; break;
                case TEMP_VALUE_CUBE:   temperature = cube_temperature; break;
                case TEMP_VALUE_TARGET: temperature = target_temperature; break;
            }
            // Выравнивание
            if(strcmp(ui->text, "C") == 0) { ui->arg = TEXTLABEL_ALIGN_CENTER; } // По центру
            else if(strcmp(ui->text, "R") == 0) { ui->arg = TEXTLABEL_ALIGN_RIGHT; } // По правому краю
            else { ui->arg = TEXTLABEL_ALIGN_LEFT; } // По левому краю
            // Формируем текстовую надпись во временный буффер
            char buffer[MUI_MAX_TEXT_LEN + 1];
            if(snprintf(buffer, (sizeof(buffer)/sizeof(*buffer)), "%.2f", temperature) != -1) {
                // Передаем этот буффер в функцию отрисовки
                strcpy(ui->text, buffer);
                return mui_text_label(ui, msg);
            }
            break;
    }
    return 0;
}

// Итоги ректификации (средняя надпись)
uint8_t mui_result_label_1(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            switch(WORKING_MODE) { // Определяем текущий процесс
                case WM_ERROR: // Ошибка
                    strcpy(ui->text, I18N_FINISH_TEXT_3);
                    break;
                case WM_DONE: // Готово
                    strcpy(ui->text, I18N_FINISH_TEXT_4);
                    break;
                case WM_FINISH: // Завершено
                    char buffer[MUI_MAX_TEXT_LEN + 1];
                    uint8_t hours = (last_event_time - rect_start_time) / 1000 / 3600; // Часы
                    uint8_t minutes = ((last_event_time - rect_start_time) / 1000 - hours * 3600) / 60; // Минуты
                    uint8_t seconds = (last_event_time - rect_start_time) / 1000 - hours * 3600 - minutes * 60; // Секунды
                    if(snprintf(buffer, (sizeof(buffer) / sizeof(*buffer)), I18N_FINISH_TIME, hours, minutes, seconds) != -1) {
                        strcpy(ui->text, buffer);
                    }
                    break;
            }
            return mui_text_label(ui, msg); // Отображаем надпись
    }
    return 0;
}

// Итоги ректификации (нижняя надпись)
uint8_t mui_result_label_2(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW:
            switch(WORKING_MODE) { // Определяем текущий процесс
                case WM_ERROR: // Ошибка
                case WM_DONE: // Готово
                    strcpy(ui->text, I18N_FINISH_TEXT_5);
                    break;
                case WM_FINISH: // Завершено
                    strcpy(ui->text, I18N_FINISH_TEXT_6);
                    break;
            }
            return mui_text_label(ui, msg); // Отображаем надпись
    }
    return 0;
}

// Кнопка навигации
uint8_t mui_goto_button(mui_t *ui, uint8_t msg) {
    switch(msg) {
        case MUIF_MSG_DRAW: // Отрисовка кнопок навигации
        case MUIF_MSG_CURSOR_ENTER: // Навели фокус на кнопку
            switch(WORKING_MODE) { // Для режимов
                case WM_HEATING: // Нагрев куба
                    switch(ui->arg) {
                        case GUI_ENWATER_BUTTON: // Кнопка "ВКЛ ВОДУ"
                        case GUI_FINISH_BUTTON: // Кнопка "ЗАВЕРШИТЬ"
                            break; // Отображаем
                        default: // Все остальные кнопки
                            return 255; // Игнорируем
                    }
                    break;
                case WM_WATERING: // Включение воды
                case WM_GETBODY: // Отбор тела
                    switch(ui->arg) {
                        case GUI_WORKING_BUTTON: // Кнопка "НА СЕБЯ"
                        case GUI_FINISH_BUTTON: // Кнопка "ЗАВЕРШИТЬ"
                            break; // Отображаем
                        default: // Все остальные кнопки
                            return 255; // Игнорируем
                    }
                    break;
                case WM_WORKING: // Работа на себя
                    switch(ui->arg) {
                        case GUI_SKIP_BUTTON: // Кнопка "ПРОПУСТИТЬ"
                        case GUI_FINISH_BUTTON: // Кнопка "ЗАВЕРШИТЬ"
                            break; // Отображаем
                        default: // Все остальные кнопки
                            return 255; // Игнорируем
                    }
                    break;
                case WM_GETHEAD: // Отбор голов
                    switch(ui->arg) {
                        case GUI_GETBODY_BUTTON: // Кнопка "ОТБОР ТЕЛА"
                        case GUI_FINISH_BUTTON: // Кнопка "ЗАВЕРШИТЬ"
                            break; // Отображаем
                        default: // Все остальные кнопки
                            return 255; // Игнорируем
                    }
                    break;
                case WM_ERROR: // Ошибка
                case WM_DONE: // Готово
                    // Только для отрисовки
                    if(msg == MUIF_MSG_DRAW) {
                        u8g2_t *u8g2 = mui_get_U8g2(ui);
                        const uint8_t *font = u8g2->font; // Запоминаем текущий шрифт
                        mui_style_font_normal(ui, msg); // Устанавливаем обычный шрифт
                        strcpy(ui->text, I18N_FINISH_TEXT_2); // Текст надписи
                        ui->arg = TEXTLABEL_ALIGN_CENTER; // Выравнивание по центру
                        mui_text_label(ui, msg); // Отображаем надпись
                        u8g2_SetFont(u8g2, font); // Восстанавливаем предыдущий шрифт
                        return 0; // Кнопки не отображаем
                    }
                    break;
            }
            break;
        case MUIF_MSG_CURSOR_SELECT: // Нажатие на одну из кнопок
            // Выбираем нужное действие, в зависимости от текущей формы
            switch(mui_GetCurrentFormId(ui)) {
                case GUI_SETTING_FORM: // Параметры ректификации
                    if(ui->arg == GUI_RECTIFICATE_FORM) { // Если нажали кнопку "СТАРТ"
                        // Устанавливаем ручной или автоматический режим
                        set_working_mode(WM_HEATING, GUI_RECTIFICATE_FORM);
                        // Запоминаем время старта
                        rect_start_time = now_millis;
                    }
                case GUI_SENSORS_FORM:   // Датчики отбора
                case GUI_WORKING_FORM:   // Время "работы на себя"
                case GUI_CUBETEMP_FORM:  // Температура в кубе
                case GUI_SAFETY_FORM:    // Безопасность
                #ifdef BLUETOOTH
                    case GUI_BLUETOOTH_FORM: // Bluetooth модуль
                #endif
                case GUI_RESET_FORM:     // Сброс настроек
                    // Рассчитываем контрольную сумму
                    if(ui->arg == GUI_RESET_FORM) { // Если нужно сбросить настройки
                        CONFIG.crc = 0x00; // Устанавливаем заведомо неправильную контрольную сумму
                    } else { // Если же нужно сохранить настройки
                        CONFIG.crc = crc8((uint8_t *) &CONFIG, sizeof(CONFIG) - 1); // Получаем CRC8
                    } 
                    // Сохраняем настройки в памяти
                    eeprom_busy_wait(); // Ждем доступности EEPROM
                    eeprom_write_block(&CONFIG, 0, sizeof(CONFIG)); // Записываем данные
                    // После сброса настроек устройство нужно перезагрузить
                    if(CONFIG.crc == 0x00) {
                        // При старте, настройки из памяти не пройдут проверку
                        // по CRC и будут восстановлены до значений по-умолчанию
                        restart_atmega();
                    }
                    break;
                case GUI_RECTIFICATE_FORM: // Процесс ректификации
                    switch(ui->arg) { // Нажатая кнопка
                        case GUI_FINISH_BUTTON: // Кнопка "ЗАВЕРШИТЬ" (автоматический режим)
                            set_working_mode(WM_DONE, GUI_FINISH_FORM); // Завершаем отбор
                            break;
                        case GUI_ENWATER_BUTTON: // Кнопка "ВКЛ ВОДУ" (нагрев куба)
                            set_working_mode(WM_WATERING, GUI_RECTIFICATE_FORM); // Включаем воду
                            break;
                        case GUI_SKIP_BUTTON: // Кнопка "ПРОПУСТИТЬ" (работа на себя)
                            target_temperature = tsarga_temperature; // Запоминаем целевую температуру царги
                            // В зависимости от текущего статуса ректификации
                            switch(REFLUX_STATUS) {
                                case RS_NOTHEAD: // Отбора голов еще не было
                                    REFLUX_STATUS = RS_YESHEAD; // Отбор голов в процессе
                                case RS_YESHEAD: // Отбор голов в процессе
                                    set_working_mode(WM_GETHEAD, GUI_RECTIFICATE_FORM); // Переходим к отбору голов
                                    break;
                                default: // Отбор тела
                                    if(REFLUX_STATUS == RS_NOTBODY) { // Если отбора тела еще не было
                                        REFLUX_STATUS = RS_YESBODY; // Отбор тела в процессе
                                    }
                                    set_working_mode(WM_GETBODY, GUI_RECTIFICATE_FORM); // Переходим к отбору тела
                            }
                            break;
                        case GUI_GETBODY_BUTTON: // Кнопка "ОТБОР ТЕЛА" (отбор голов)
                            target_temperature = tsarga_temperature; // Запоминаем целевую температуру царги
                            REFLUX_STATUS = RS_NOTBODY; // Отбор голов закончили, но к отбору тела еще не приступили
                        case GUI_WORKING_BUTTON: // Кнопка "НА СЕБЯ" (отбор тела)
                            set_working_mode(WM_WORKING, GUI_RECTIFICATE_FORM); // Начинаем "работать на себя"
                    }
                    return 0;
                case GUI_CIRCULATE_FORM: // Подтверждение начала отбора оборотного спирта
                    switch(ui->arg) { // Нажатая кнопка
                        case GUI_RECTIFICATE_FORM: // Кнопка "ПРОДОЛЖИТЬ"
                            REFLUX_STATUS = RS_YESCIRC; // Начинаем отбор оборотки
                            set_working_mode(WM_GETBODY, GUI_RECTIFICATE_FORM); // Переходим к отбору
                        case GUI_FINISH_FORM: // Кнопка "ЗАВЕРШИТЬ"
                            set_working_mode(WM_DONE, GUI_FINISH_FORM); // Завершаем отбор
                    }
                    return 0;
                case GUI_FINISH_FORM: // Ректификация завершена
                    switch(WORKING_MODE) { // Определяем текущий процесс
                        case WM_FINISH: // Завершено
                            restart_atmega(); // Перезагружаем микроконтроллер
                    }
                    return 0;
            }
            break;
        case MUIF_MSG_VALUE_DECREMENT: // Кнопка "-"
        case MUIF_MSG_VALUE_INCREMENT: // Кнопка "+"
            return 0; // Ничего не делаем
    }
    return mui_u8g2_btn_goto_w2_fi(ui, msg); // Вызываем нативный callback
}

// Устанавливаем callback`и элементов интерфейса
static const muif_t muif_list[] MUI_PROGMEM = {
    MUIF_STYLE(0, mui_style_font_normal), // Обычный шрифт
    MUIF_STYLE(1, mui_style_font_bold), // Жирный шрифт
    MUIF_RO("HL", mui_header_label), // Заголовок экрана
    MUIF_RO("TL", mui_text_label), // Текстовая надпись
    MUIF_RO("TV", mui_temp_value), // Значение температуры
    MUIF_GOTO(mui_goto_button), // Кнопка навигации
    // Параметры ректификации
    MUIF_RO("ML", mui_u8g2_goto_data), // Список элементов меню параметров
    MUIF_BUTTON("MI", mui_u8g2_goto_form_w1_pi), // Выбор текущего элемента меню
    // Датчики отбора
    MUIF_U8G2_U8_MIN_MAX("TB", &CONFIG.target_delta_before, 0, 9, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: дельта датчика царги (до запятой)
    MUIF_U8G2_U8_MIN_MAX("TA", &CONFIG.target_delta_after, 0, 99, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: дельта датчика царги (после запятой)
    // Работа на себя
    MUIF_U8G2_U8_MIN_MAX("WT", &CONFIG.itself_working_temperature, 80, 99, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: температура начала "работы на себя"
    MUIF_U8G2_U8_MIN_MAX("IW", &CONFIG.itself_working_initial_time, 10, 60, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: время начальной "работы на себя" (мин)
    MUIF_U8G2_U8_MIN_MAX("RT", &CONFIG.itself_working_interim_time, 1, 30, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: время промежуточной "работы на себя" (мин)
    // Температура в кубе
    MUIF_U8G2_U8_MIN_MAX("IC", &CONFIG.water_cube_temperature, 60, 80, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: температура старта подачи воды
    MUIF_U8G2_U8_MIN_MAX("EC", &CONFIG.ethanol_cube_temperature, 85, 110, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: макс. температура для товарного спирта
    MUIF_U8G2_U8_MIN_MAX("FC", &CONFIG.final_cube_temperature, 85, 110, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: температура окончания работы
    // Безопасность
    MUIF_VARIABLE("SP", &CONFIG.sensors_protection, mui_u8g2_u8_chkbox_wm_pi), // Чекбокс: включить контроль работоспособности датчиков
    MUIF_VARIABLE("TP", &CONFIG.tsa_protection, mui_u8g2_u8_chkbox_wm_pi), // Чекбокс: включить защиту по температуре ТСА
    MUIF_U8G2_U8_MIN_MAX("TM", &CONFIG.tsa_max_temperature, 40, 60, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: максимальная температура воды
    MUIF_VARIABLE("WP", &CONFIG.water_protection, mui_u8g2_u8_chkbox_wm_pi), // Чекбокс: включить защиту по температуре воды
    MUIF_U8G2_U8_MIN_MAX("WM", &CONFIG.water_max_temperature, 40, 60, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: максимальная температура воды
    MUIF_U8G2_U8_MIN_MAX("FT", &CONFIG.phlegm_wait_time, 1, 10, mui_u8g2_u8_min_max_wm_mse_pi), // Ввод числа: время сброса флегмы (мин)
    #ifdef BLUETOOTH
        // Bluetooth модуль
        MUIF_VARIABLE("BT", &CONFIG.use_bluetooth, mui_u8g2_u8_radio_wm_pi), // Переключатель: использовать Bluetooth модуль
    #endif
    // Ректификация завершена
    MUIF_RO("RU", mui_result_label_1), // Итоги ректификации (средняя надпись)
    MUIF_RO("RD", mui_result_label_2), // Итоги ректификации (нижняя надпись)
};

// Описание форм интерфейса
static const fds_t fds_data[] MUI_PROGMEM =
    // Параметры ректификации
    _MUI_FORM(GUI_SETTING_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    MUI_DATA("ML",
        MUI_100 I18N_START_MENU "|"     /* GUI_RECTIFICATE_FORM */
        MUI_2   I18N_SENSORS_MENU "|"   /* GUI_SENSORS_FORM */
        MUI_3   I18N_WORKING_MENU "|"   /* GUI_WORKING_FORM */
        MUI_4   I18N_CUBETEMP_MENU "|"  /* GUI_CUBETEMP_FORM */
        MUI_5   I18N_SAFETY_MENU "|"    /* GUI_SAFETY_FORM */
        #ifdef BLUETOOTH
            MUI_6 I18N_BLUETOOTH_MENU "|" /* GUI_BLUETOOTH_FORM */
        #endif
        MUI_7   I18N_RESET_MENU "|"     /* GUI_RESET_FORM */
        MUI_8   I18N_ABOUT_MENU "|"     /* GUI_ABOUT_FORM */
    )
    _MUI_XYA("MI", 5, 22, 0)
    _MUI_XYA("MI", 5, 31, 1)
    _MUI_XYA("MI", 5, 40, 2)
    _MUI_XYA("MI", 5, 49, 3)
    _MUI_XYA("MI", 5, 58, 4)
    
    // Датчики отбора
    _MUI_FORM(GUI_SENSORS_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_LEFT, I18N_SELECTION_DELTA)
    MUI_XY("TB", 105, 22)
    _MUI_XYAT("TL", 110, 22, TEXTLABEL_ALIGN_LEFT, I18N_DECIMAL_SEPARATOR)
    MUI_XY("TA", 115, 22)
    _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    
    // Работа на себя
    _MUI_FORM(GUI_WORKING_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_LEFT, I18N_WORKING_TEMPERATURE)
    MUI_XY("WT", 115, 22)
    _MUI_XYAT("TL", 5, 31, TEXTLABEL_ALIGN_LEFT, I18N_WORKING_INITIAL_TIME)
    MUI_XY("IW", 115, 31)
    _MUI_XYAT("TL", 5, 40, TEXTLABEL_ALIGN_LEFT, I18N_WORKING_INTERIM_TIME)
    MUI_XY("RT", 115, 40)
    _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    
    // Температура в кубе
    _MUI_FORM(GUI_CUBETEMP_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_LEFT, I18N_WATER_CUBE_TEMPERATURE)
    MUI_XY("IC", 115, 22)
    _MUI_XYAT("TL", 5, 31, TEXTLABEL_ALIGN_LEFT, I18N_ETHANOL_CUBE_TEMPERATURE)
    MUI_XY("EC", 110, 31)
    _MUI_XYAT("TL", 5, 40, TEXTLABEL_ALIGN_LEFT, I18N_FINAL_CUBE_TEMPERATURE)
    MUI_XY("FC", 110, 40)
    _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    
    // Безопасность
    _MUI_FORM(GUI_SAFETY_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYT("SP", 5, 22, I18N_SENSORS_PROTECTION)
    _MUI_XYT("TP", 5, 31, I18N_TSA_PROTECTION)
    MUI_XY("TM", 115, 31)
    _MUI_XYT("WP", 5, 40, I18N_WATER_PROTECTION)
    MUI_XY("WM", 115, 40)
    _MUI_XYAT("TL", 5, 49, TEXTLABEL_ALIGN_LEFT, I18N_PHLEGM_WAIT_TIME)
    MUI_XY("FT", 115, 49)
    _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    
    #ifdef BLUETOOTH
        // Bluetooth модуль
        _MUI_FORM(GUI_BLUETOOTH_FORM)
        MUI_AUX("HL")
        MUI_STYLE(0)
        _MUI_XYAT("BT", 5, 22, 0, I18N_ENABLE_LABEL)
        _MUI_XYAT("BT", 5, 31, 1, I18N_DISABLE_LABEL)
        _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    #endif
    
    // Сброс настроек
    _MUI_FORM(GUI_RESET_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    #ifdef I18N_RESET_TEXT_1
        _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_CENTER, I18N_RESET_TEXT_1)
    #endif
    #ifdef I18N_RESET_TEXT_2
        _MUI_XYAT("TL", 5, 31, TEXTLABEL_ALIGN_CENTER, I18N_RESET_TEXT_2)
    #endif
    #ifdef I18N_RESET_TEXT_3
        _MUI_XYAT("TL", 5, 40, TEXTLABEL_ALIGN_CENTER, I18N_RESET_TEXT_3)
    #endif
    #ifdef I18N_RESET_TEXT_4
        _MUI_XYAT("TL", 5, 49, TEXTLABEL_ALIGN_CENTER, I18N_RESET_TEXT_4)
    #endif
    _MUI_GOTO(33, 60, GUI_SETTING_FORM, I18N_NO_BUTTON)
    _MUI_GOTO(95, 60, GUI_RESET_FORM, I18N_YES_BUTTON)
    
    // О проекте
    _MUI_FORM(GUI_ABOUT_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    #ifdef I18N_ABOUT_TEXT_1
        _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_CENTER, I18N_ABOUT_TEXT_1)
    #endif
    #ifdef I18N_ABOUT_TEXT_2
        _MUI_XYAT("TL", 5, 31, TEXTLABEL_ALIGN_CENTER, I18N_ABOUT_TEXT_2)
    #endif
    #ifdef I18N_ABOUT_TEXT_3
        _MUI_XYAT("TL", 5, 40, TEXTLABEL_ALIGN_CENTER, I18N_ABOUT_TEXT_3)
    #endif
    #ifdef I18N_ABOUT_TEXT_4
        _MUI_XYAT("TL", 5, 49, TEXTLABEL_ALIGN_CENTER, I18N_ABOUT_TEXT_4)
    #endif
    _MUI_GOTO(64, 60, GUI_SETTING_FORM, I18N_OK_BUTTON)
    
    // Процесс ректификации
    _MUI_FORM(GUI_RECTIFICATE_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYAT("TL", 0, 22, TEXTLABEL_ALIGN_LEFT, I18N_WATER_LABEL)
    _MUI_XYAT("TL", 30, 22, TEXTLABEL_ALIGN_RIGHT, I18N_TSA_LABEL)
    _MUI_XYAT("TV", 30, 22, TEMP_VALUE_WATER, "L")
    _MUI_XYAT("TV", 0, 22, TEMP_VALUE_TSA, "R")
    _MUI_XYAT("TL", 0, 35, TEXTLABEL_ALIGN_LEFT, I18N_CUBE_LABEL)
    _MUI_XYAT("TL", 0, 35, TEXTLABEL_ALIGN_CENTER, I18N_TARGET_LABEL)
    _MUI_XYAT("TL", 0, 35, TEXTLABEL_ALIGN_RIGHT, I18N_TSARGA_LABEL)
    MUI_STYLE(1)
    _MUI_XYAT("TV", 0, 48, TEMP_VALUE_CUBE, "L")
    _MUI_XYAT("TV", 0, 48, TEMP_VALUE_TARGET, "C")
    _MUI_XYAT("TV", 0, 48, TEMP_VALUE_TSARGA, "R")
    MUI_STYLE(0)
    _MUI_GOTO(33, 60, GUI_ENWATER_BUTTON, I18N_ENWATER_BUTTON)
    _MUI_GOTO(33, 60, GUI_SKIP_BUTTON, I18N_SKIP_BUTTON)
    _MUI_GOTO(33, 60, GUI_GETBODY_BUTTON, I18N_GETBODY_BUTTON)
    _MUI_GOTO(33, 60, GUI_WORKING_BUTTON, I18N_WORKING_BUTTON)
    _MUI_GOTO(95, 60, GUI_FINISH_BUTTON, I18N_FINISH_BUTTON)
    
    // Отбор оборотного спирта
    _MUI_FORM(GUI_CIRCULATE_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    #ifdef I18N_CIRCULATE_TEXT_1
        _MUI_XYAT("TL", 5, 22, TEXTLABEL_ALIGN_CENTER, I18N_CIRCULATE_TEXT_1)
    #endif
    #ifdef I18N_CIRCULATE_TEXT_2
        _MUI_XYAT("TL", 5, 31, TEXTLABEL_ALIGN_CENTER, I18N_CIRCULATE_TEXT_2)
    #endif
    #ifdef I18N_CIRCULATE_TEXT_3
        _MUI_XYAT("TL", 5, 40, TEXTLABEL_ALIGN_CENTER, I18N_CIRCULATE_TEXT_3)
    #endif
    #ifdef I18N_CIRCULATE_TEXT_4
        _MUI_XYAT("TL", 5, 49, TEXTLABEL_ALIGN_CENTER, I18N_CIRCULATE_TEXT_4)
    #endif
    _MUI_GOTO(33, 60, GUI_RECTIFICATE_FORM, I18N_CONTINUE_BUTTON)
    _MUI_GOTO(95, 60, GUI_FINISH_FORM, I18N_FINISH_BUTTON)
    
    // Ректификация завершена
    _MUI_FORM(GUI_FINISH_FORM)
    MUI_AUX("HL")
    MUI_STYLE(0)
    _MUI_XYAT("TL", 0, 23, TEXTLABEL_ALIGN_CENTER, I18N_FINISH_TEXT_1)
    _MUI_XYA("RU", 0, 33, TEXTLABEL_ALIGN_CENTER)
    _MUI_XYA("RD", 0, 43, TEXTLABEL_ALIGN_CENTER)
    _MUI_GOTO(64, 60, GUI_NONE_FORM, I18N_DONE_BUTTON)
;
