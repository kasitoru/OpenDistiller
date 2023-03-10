/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#include <stddef.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring.c

#define MICROSECONDS_PER_TIMER0_OVERFLOW ((64 * 256) / (F_CPU / 1000000L))
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile uint32_t timer0_millis = 0;
static uint8_t timer0_fract = 0;

// Прерывание таймера счетчика
ISR(TIMER0_OVF_vect) {
    uint32_t m = timer0_millis;
    uint8_t f = timer0_fract;
	m += MILLIS_INC;
	f += FRACT_INC;
	if(f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}
	timer0_fract = f;
	timer0_millis = m;
}

// Инициализация счетчика
void init_millis() {
    sei(); // Разрешаем прерывания
    // Обычный режим
    TCCR0A = 0x00;
    // Предделитель 64
    TCCR0B = 0x00;
    TCCR0B |= _BV(CS01) | _BV(CS00);
    // Прерывание по переполнению
    TIMSK0 = 0x00;
    TIMSK0 |= _BV(TOIE0);
}

// Безопасный доступ к счетчику
uint32_t get_millis() {
	unsigned long m;
	uint8_t oldSREG = SREG;
	cli();
	m = timer0_millis;
	SREG = oldSREG;
	return m;
}

// Подсчет количества цифр в числе
size_t count_digits(int16_t number) {
    size_t count = 0;
    do {
        number /= 10;
        ++count;
    } while(number != 0);
    return count;
}

// Обрезать целое число
int16_t truncate_integer(int16_t number, size_t length) {
    while(count_digits(number) > length) {
        int16_t n = (number / 10);
        number = ((abs(number - (n * 10)) < 5) ? (n) : ((n < 0) ? (n - 1) : (n + 1)));
    }
    return number;
}

// Рассчет контрольной суммы
// https://alexgyver.ru/lessons/crc/
uint8_t crc8(uint8_t *buffer, size_t size) {
    uint8_t crc = 0;
    for(uint8_t i = 0; i < size; i++) {
        uint8_t data = buffer[i];
        for(size_t j = 8; j > 0; j--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }
    return crc;
}

// Перезагрузка микроконтроллера
// https://www.avrfreaks.net/forum/reset-atmega-software
void restart_atmega() {
    wdt_enable(WDTO_15MS); // Включаем сторожевой таймер
    wdt_reset(); // Сбрасываем его значение
    while(1); // Активируем срабатывание
}
