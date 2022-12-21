/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef U8X8_AVR_H_
#define U8X8_AVR_H_

#include <u8g2.h>
#include <u8x8.h>
#include <stdint.h>

// PORT B
#define U8X8_AVR_PB0 1
#define U8X8_AVR_PB1 2
#define U8X8_AVR_PB2 3
#define U8X8_AVR_PB3 4
#define U8X8_AVR_PB4 5
#define U8X8_AVR_PB5 6

// PORT C
#define U8X8_AVR_PC0 7
#define U8X8_AVR_PC1 8
#define U8X8_AVR_PC2 9
#define U8X8_AVR_PC3 10
#define U8X8_AVR_PC4 11
#define U8X8_AVR_PC5 12
#define U8X8_AVR_PC6 13

// PORT D
#define U8X8_AVR_PD0 14
#define U8X8_AVR_PD1 15
#define U8X8_AVR_PD2 16
#define U8X8_AVR_PD3 17
#define U8X8_AVR_PD4 18
#define U8X8_AVR_PD5 19
#define U8X8_AVR_PD6 20
#define U8X8_AVR_PD7 21

#define U8X8_AVR_DDR(P) ((P>=U8X8_AVR_PB0 && P<=U8X8_AVR_PB5)?(&DDRB):((P>=U8X8_AVR_PC0 && P<=U8X8_AVR_PC6)?(&DDRC):((P>=U8X8_AVR_PD0 && P<=U8X8_AVR_PD7)?(&DDRD):(NULL))))
#define U8X8_AVR_PORT(P) ((P>=U8X8_AVR_PB0 && P<=U8X8_AVR_PB5)?(&PORTB):((P>=U8X8_AVR_PC0 && P<=U8X8_AVR_PC6)?(&PORTC):((P>=U8X8_AVR_PD0 && P<=U8X8_AVR_PD7)?(&PORTD):(NULL))))
#define U8X8_AVR_PIN(P) ((P>=U8X8_AVR_PB0 && P<=U8X8_AVR_PB5)?(&PINB):((P>=U8X8_AVR_PC0 && P<=U8X8_AVR_PC6)?(&PINC):((P>=U8X8_AVR_PD0 && P<=U8X8_AVR_PD7)?(&PIND):(NULL))))
#define U8X8_AVR_BIT(P) ((P>=U8X8_AVR_PB0 && P<=U8X8_AVR_PB5)?(P-U8X8_AVR_PB0):((P>=U8X8_AVR_PC0 && P<=U8X8_AVR_PC6)?(P-U8X8_AVR_PC0):((P>=U8X8_AVR_PD0 && P<=U8X8_AVR_PD7)?(P-U8X8_AVR_PD0):(U8X8_PIN_NONE))))

uint8_t u8x8_byte_avr_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_avr_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif /* U8X8_AVR_H_ */
