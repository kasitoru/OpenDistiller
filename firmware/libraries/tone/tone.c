/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

// https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Tone.cpp

#include <avr/interrupt.h>

volatile uint32_t _tone_toggle_count;

// Воспроизведение заданной частоты с определенной длительностью
void tone(uint16_t frequency, uint32_t duration) {
    // Настройка ШИМ
    TCCR2A = 0; TCCR2B = 0;
    TCCR2A |= _BV(WGM21);
    TCCR2B |= _BV(CS20);
    // Рассчитываем частоту и режим тактирования ШИМ
    uint32_t ocr = F_CPU / frequency / 2 - 1;
    uint8_t prescalarbits = 0b001;
    if(ocr > 255) {
        ocr = F_CPU / frequency / 2 / 8 - 1;
        prescalarbits = 0b010;
        if(ocr > 255) {
            ocr = F_CPU / frequency / 2 / 32 - 1;
            prescalarbits = 0b011;
            if(ocr > 255) {
                ocr = F_CPU / frequency / 2 / 64 - 1;
                prescalarbits = 0b100;
                if(ocr > 255) {
                    ocr = F_CPU / frequency / 2 / 128 - 1;
                    prescalarbits = 0b101;
                    if(ocr > 255) {
                        ocr = F_CPU / frequency / 2 / 256 - 1;
                        prescalarbits = 0b110;
                        if(ocr > 255) {
                            ocr = F_CPU / frequency / 2 / 1024 - 1;
                            prescalarbits = 0b111;
                        }
                    }
                }
            }
        }
    }
    // Устанавливаем частоту и режим тактирования
    TCCR2B = (TCCR2B & 0b11111000) | prescalarbits;
    OCR2A = ocr;
    // Рассчитываем количество запусков таймера
    if(duration > 0) {
        _tone_toggle_count = 2 * frequency * duration / 1000;
    } else {
        _tone_toggle_count = -1;
    }
    // Запускаем таймер
    TIMSK2 |= _BV(OCIE2A);
}

// Остановить воспроизведение
void noTone() {
    _tone_toggle_count = 0;
}

// Таймер
ISR(TIMER2_COMPA_vect) {
    if(_tone_toggle_count != 0) {
        PORTB ^= _BV(PB1);
        if(_tone_toggle_count > 0)
            _tone_toggle_count--;
    } else {
        // Отключаем таймер
        TIMSK2 &= ~_BV(OCIE2A);
        // Отключаем ШИМ
        TCCR2A = _BV(WGM20);
        TCCR2B = (TCCR2B & 0b11111000) | _BV(CS22);
        OCR2A = 0;
        // Обесточиваем порт
        PORTB &= ~_BV(PB1);
    }
}
