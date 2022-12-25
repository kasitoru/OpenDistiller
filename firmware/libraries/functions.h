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

#define SET_PIN_STATE(port, bit, inverted, state) (((!!inverted) != state) ? (port |= _BV(bit)) : (port &= ~_BV(bit)))

#endif /* FUNCTIONS_H_ */
