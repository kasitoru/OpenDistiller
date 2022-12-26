/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef VERSION
    #define VERSION ""
#endif

// Шрифты
#define I18N_NORMAL_FONT              u8g2_font_5x8_t_cyrillic
#define I18N_BOLD_FONT                u8g2_font_6x13B_t_cyrillic

// Параметры ректификации
#define I18N_SETTING_MENU             "Параметры"
#define I18N_START_MENU               "СТАРТ"
#define I18N_CUBETEMP_MENU            "Температура в кубе"
#define I18N_WORKING_MENU             "Работа на себя"
#define I18N_GETALCOHOL_MENU          "Отбор спирта"
#define I18N_SAFETY_MENU              "Безопасность"
#define I18N_BLUETOOTH_MENU           "Bluetooth модуль"
#define I18N_RESET_MENU               "Сброс настроек"
#define I18N_ABOUT_MENU               "О проекте..."

// Температура в кубе
#define I18N_WATER_CUBE_TEMPERATURE   "Старт подачи воды:"
#define I18N_ETHANOL_CUBE_TEMPERATURE "Товарный спирт до:"
#define I18N_FINAL_CUBE_TEMPERATURE   "Окончание работы:"

// Работа на себя
#define I18N_WORKING_TEMPERATURE      "Темп.старта (царга):"
#define I18N_WORKING_INITIAL_TIME     "Длительность (мин):"

// Отбор спирта
#define I18N_SELECTION_DELTA          "Дельта отбора:"
#define I18N_DECIMAL_SEPARATOR        "."
#define I18N_WORKING_INTERIM_TIME     "Восстановление (мин):"
#define I18N_OK_BUTTON                "Ок"

// Безопасность
#define I18N_SENSORS_PROTECTION       "Контроль датчиков"
#define I18N_TSA_PROTECTION           "Защита темп. ТСА:"
#define I18N_WATER_PROTECTION         "Защита темп. воды:"
#define I18N_PHLEGM_WAIT_TIME         "Время сброса флегмы:"

// Bluetooth модуль
#define I18N_ENABLE_LABEL             "Включить"
#define I18N_DISABLE_LABEL            "Отключить"

// Сброс настроек
#define I18N_RESET_TEXT_1             "Установить"
#define I18N_RESET_TEXT_2             "значения по-умолчанию?"
#define I18N_NO_BUTTON                "Нет"
#define I18N_YES_BUTTON               "Да"

// О проекте
#define I18N_ABOUT_TEXT_1             "OpenDistiller " VERSION
#define I18N_ABOUT_TEXT_2             "Автор: Сергей Авдеев"
#define I18N_ABOUT_TEXT_3             "HTTPS://KASITO.RU"

// Процесс ректификации
#define I18N_HEATING_TITLE            "Нагрев куба"
#define I18N_WORKING_TITLE            "Работа на себя"
#define I18N_GETHEAD_TITLE            "Отбор голов"
#define I18N_GETBODY_TITLE            "Отбор тела"
#define I18N_WATER_LABEL              "ВОДА:"
#define I18N_TSA_LABEL                "ТСА:"
#define I18N_CUBE_LABEL               "КУБ:"
#define I18N_TARGET_LABEL             "ЦЕЛЬ:"
#define I18N_TSARGA_LABEL             "ЦАРГА:"
#define I18N_ENWATER_BUTTON           "ВКЛ ВОДУ"
#define I18N_SKIP_BUTTON              "ПРОПУСТИТЬ"
#define I18N_GETBODY_BUTTON           "ОТБОР ТЕЛА"
#define I18N_WORKING_BUTTON           "НА СЕБЯ"
#define I18N_FINISH_BUTTON            "ЗАВЕРШИТЬ"

// Отбор оборотного спирта
#define I18N_CIRCULATE_TITLE          "Смените емкость"
#define I18N_CIRCULATE_TEXT_1         "Установите новую емкость"
#define I18N_CIRCULATE_TEXT_2         "для оборотного спирта"
#define I18N_CIRCULATE_TEXT_3         "или завершите отбор!"
#define I18N_CONTINUE_BUTTON          "ПРОДОЛЖИТЬ"

// Ректификация завершена
#define I18N_ERROR_TITLE              "Ошибка!"
#define I18N_DONE_TITLE               "Готово!"
#define I18N_FINISH_TITLE             "Завершено"
#define I18N_FINISH_TEXT_1            "Ректификация завершена!"
#define I18N_FINISH_TEXT_2            "Пожалуйста, подождите..."
#define I18N_FINISH_TEXT_3            "СРАБОТАЛА ЗАЩИТА!"
#define I18N_FINISH_TEXT_4            "НЕ ОТКЛЮЧАЙТЕ ПОДАЧУ ВОДЫ"
#define I18N_FINISH_TEXT_5            "Сброс остатков флегмы"
#define I18N_FINISH_TEXT_6            "ВНИМАНИЕ: КОЛОННА ГОРЯЧАЯ"
#define I18N_FINISH_TIME              "ВРЕМЯ: %iч %iм %iс"
#define I18N_DONE_BUTTON              "ГОТОВО"
