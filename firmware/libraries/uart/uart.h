/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef UART_H_
#define UART_H_

void uart_init(uint32_t speed);
uint8_t uart_getChar();
void uart_printf(const char *format, ...);
//void uart_float(float value);

#endif /* UART_H_ */
