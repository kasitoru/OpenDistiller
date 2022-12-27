/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

void init_millis();
uint32_t get_millis();
size_t count_digits(int16_t number);
int16_t truncate_integer(int16_t number, size_t length);
uint8_t crc8(uint8_t *buffer, size_t size);
void restart_atmega();

// Изменить состояние порта
#define SET_PIN_STATE(port, bit, inverted, state) (((!!inverted) != state) ? (port |= _BV(bit)) : (port &= ~_BV(bit)))

// Число с фиксированной запятой (int16_t)
#define FPN_ISN(n) (!!(n & 0xF000)) // Является ли число отрицательным
#define FPN_GBD(n) (n >> 4) // Получить часть числа до запятой
#define FPN_GAD(n) ((n & 0xF) * 625) // Получить часть числа после запятой
#define FPN_SBD(v) (v << 4) // Установить часть числа до запятой
#define FPN_SAD(v) (v / 625) // Установить часть числа после запятой

// Вывод значения числа с фиксированной запятой
#define FPN_FORMAT "%i.%02i" // Формат вывода для printf
#define FPN_FRMT_BD(n) (FPN_ISN(n) ? (0) : FPN_GBD(n)) // Часть числа до запятой
#define FPN_FRMT_AD(n) (FPN_ISN(n) ? (0) : truncate_integer(FPN_GAD(n), (count_digits(FPN_GAD(n)) - 2))) // Часть числа после запятой

#endif /* FUNCTIONS_H_ */
