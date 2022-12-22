/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#include <stdint.h>

// Свободная оперативная память
// https://robocraft.ru/arduino/531
uint16_t free_ram() {
    extern uint16_t __heap_start, *__brkval;
    uint16_t v;
    return (uint16_t) &v - (__brkval == 0 ? (uint16_t) &__heap_start : (uint16_t) __brkval);
}
