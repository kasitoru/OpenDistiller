/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

// https://github.com/kasitoru/acebox_v1x/blob/master/c_project/libraries/uart/uart.c

#include <stdio.h>
#include <avr/io.h>
#include <math.h>

// Инициализация UART порта
void uart_init(uint32_t speed) {
	DDRD &= ~_BV(PD0); // Пин RX как вход
	DDRD |= _BV(PD1); // Пин TX как выход
	speed = (F_CPU / (speed * 16L))-1; // Скорость порта
	UBRR0H = (uint8_t) (speed >> 8);
	UBRR0L = (uint8_t) (speed);
	UCSR0C |= _BV(UMSEL01); // Для доступа к регистру UCSRC
	UCSR0C &= ~_BV(UCSZ02); UCSR0C |= _BV(UCSZ01) | _BV(UCSZ00); // 8 бит
	UCSR0C &= ~_BV(UPM01); UCSR0C &= ~_BV(UPM00); // Без контроля четности
	UCSR0C &= ~_BV(USBS0); // 1 стоп бит
	UCSR0B = _BV(TXEN0) | _BV(RXEN0); // Разрешаем передачу данных
}

// Чтение одного байта
uint8_t uart_getChar() {
	// Ждем окончание приема данных
	while((UCSR0A & _BV(RXC0)) == 0);
	// Возвращаем прочитанный байт
	return UDR0;
}

// Отправка отформатированной строки
void uart_printf(const char *format, ...) {
    char byte, buffer[256];
    va_list params;
    va_start(params, format);
    if(vsnprintf(buffer, (sizeof(buffer) / sizeof(*buffer)), format, params) != -1) {
        int8_t pointer = 0;
        while((byte = buffer[pointer]) != 0x00) {
            while((UCSR0A & _BV(UDRE0)) == 0);
            UDR0 = byte;
            pointer++;
        }
    }
    va_end(params);
}
/*
// Отправка вещественного числа
void uart_float(float value) {
    int16_t integer = (int16_t)value; // Целая часть
    int16_t remainder = round((value-integer)*100); // Остаток (два знака после запятой)
    uart_printf("%i.%i", integer, remainder);
}
*/
