/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

void init_millis();
uint32_t get_millis();
uint8_t crc8(uint8_t *buffer, uint8_t size);
void restart_atmega();

// Изменить состояние порта
#define SET_PIN_STATE(port, bit, inverted, state) (((!!inverted) != state) ? (port |= _BV(bit)) : (port &= ~_BV(bit)))

// Число с фиксированной запятой (int16_t)
#define FPN_FORMAT "%i.%04i" // Формат вывода числа для printf
#define FPN_GBD(n) (n >> 4) // Получить часть числа до запятой
#define FPN_GAD(n) ((n & 0xF) * 625) // Получить часть числа после запятой
#define FPN_SBD(v) (v << 4); // Установить часть числа до запятой
#define FPN_SAD(v) (v / 625); // Установить часть числа после запятой

#endif /* FUNCTIONS_H_ */
