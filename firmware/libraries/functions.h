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
#ifdef DEBUG
uint16_t free_ram();
#endif
void restart_atmega();

#endif /* FUNCTIONS_H_ */
