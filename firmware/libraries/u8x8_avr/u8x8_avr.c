/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

// https://github.com/olikraus/u8g2/blob/master/sys/avr/avr-libc/lib/u8x8_avr.c

#include <util/delay.h>
#include "u8x8_avr.h"
#include "../../hardware.h" // Номера портов

#define P_CPU_NS (1000000000UL / F_CPU)

uint8_t u8x8_byte_avr_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    uint8_t *data;
    switch(msg) {
        case U8X8_MSG_BYTE_INIT:
            HW_DISPLAY_SCK_DDR |= _BV(HW_DISPLAY_SCK_BIT);
            HW_DISPLAY_MOSI_DDR |= _BV(HW_DISPLAY_MOSI_BIT);
            HW_DISPLAY_CS_PORT |= _BV(HW_DISPLAY_CS_BIT);
            HW_DISPLAY_CS_DDR |= _BV(HW_DISPLAY_CS_BIT);
            SPCR = (_BV(SPE) | _BV(MSTR));
            switch(u8x8->display_info->spi_mode) {
                case 0:
                    break;
                case 1:
                    SPCR |= _BV(CPHA);
                    break;
                case 2:
                    SPCR |= _BV(CPOL);
                    break;
                case 3:
                    SPCR |= _BV(CPOL);
                    SPCR |= _BV(CPHA);
                    break;
            };
            switch(F_CPU / u8x8->display_info->sck_clock_hz) {
                case 2:
                    SPSR |= _BV(SPI2X);
                    break;
                case 4:
                    break;
                case 8:
                    SPSR |= _BV(SPI2X);
                    SPCR |= _BV(SPR0);
                    break;
                case 16:
                    SPCR |= _BV(SPR0);
                    break;
                case 32:
                    SPSR |= _BV(SPI2X);
                    SPCR |= _BV(SPR1);
                    break;
                case 64:
                    SPCR |= _BV(SPR1);
                    break;
                case 128:
                    SPCR |= _BV(SPR1);
                    SPCR |= _BV(SPR0);
                    break;
            }
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            break;
        case U8X8_MSG_BYTE_SET_DC:
            u8x8_gpio_SetDC(u8x8, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
            u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
            break;
        case U8X8_MSG_BYTE_SEND:
            data = (uint8_t *) arg_ptr;
            while(arg_int > 0) {
                SPDR = (uint8_t) *data;
                while(!(SPSR & _BV(SPIF)));
                data++;
                arg_int--;
            }
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL);
            u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
            break;
        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_avr_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    uint8_t cycles;
    switch(msg) {
        case U8X8_MSG_DELAY_NANO: // delay arg_int * 1 nano second
            // At 20Mhz, each cycle is 50ns, the call itself is slower.
            break;
        case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
            // Approximate best case values...
            #define CALL_CYCLES 26UL
            #define CALC_CYCLES 4UL
            #define RETURN_CYCLES 4UL
            #define CYCLES_PER_LOOP 4UL
            cycles = (100UL * arg_int) / (P_CPU_NS * CYCLES_PER_LOOP);
            if(cycles > CALL_CYCLES + RETURN_CYCLES + CALC_CYCLES) {
                break;
            }
			__asm__ __volatile__ (
                "1: sbiw %0,1" "\n\t" // 2 cycles
                "brne 1b" : "=w" (cycles) : "0" (cycles) // 2 cycles
            );
            break;
        case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
            while(arg_int--) _delay_us(10);
            break;
        case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
            while(arg_int--) _delay_ms(1);
            break;
        case U8X8_MSG_GPIO_CS:
            if(arg_int) {
                HW_DISPLAY_CS_PORT |= _BV(HW_DISPLAY_CS_BIT);
            } else {
                HW_DISPLAY_CS_PORT &= ~_BV(HW_DISPLAY_CS_BIT);
            }
            break;
        default:
            if(msg >= U8X8_MSG_GPIO(0)) {
                switch(u8x8_GetPinIndex(u8x8, msg)) {
                    case U8X8_PIN_MENU_PREV:
                        u8x8_SetGPIOResult(u8x8, !(HW_BUTTON_PREV_PIN & _BV(HW_BUTTON_PREV_BIT)));
                        break;
                    case U8X8_PIN_MENU_NEXT:
                        u8x8_SetGPIOResult(u8x8, !(HW_BUTTON_NEXT_PIN & _BV(HW_BUTTON_NEXT_BIT)));
                        break;
                    case U8X8_PIN_MENU_SELECT:
                        u8x8_SetGPIOResult(u8x8, !(HW_BUTTON_SELECT_PIN & _BV(HW_BUTTON_SELECT_BIT)));
                        break;
                    case U8X8_PIN_MENU_DOWN:
                        u8x8_SetGPIOResult(u8x8, !(HW_BUTTON_DOWN_PIN & _BV(HW_BUTTON_DOWN_BIT)));
                        break;
                    case U8X8_PIN_MENU_UP:
                        u8x8_SetGPIOResult(u8x8, !(HW_BUTTON_UP_PIN & _BV(HW_BUTTON_UP_BIT)));
                        break;
                    default:
                        return 0;
                }
            } else { return 0; }
    }
    return 1;
}
