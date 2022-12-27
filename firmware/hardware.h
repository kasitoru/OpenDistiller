/*
    OpenDistiller Project
    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
    URL: https://github.com/kasitoru/OpenDistiller
*/

#ifndef HARDWARE_H_
#define HARDWARE_H_

// Дисплей
#define HW_DISPLAY_SCK_BIT    PB5
#define HW_DISPLAY_SCK_PIN    PINB
#define HW_DISPLAY_SCK_DDR    DDRB
#define HW_DISPLAY_SCK_PORT   PORTB

#define HW_DISPLAY_MOSI_BIT   PB3
#define HW_DISPLAY_MOSI_PIN   PINB
#define HW_DISPLAY_MOSI_DDR   DDRB
#define HW_DISPLAY_MOSI_PORT  PORTB

#define HW_DISPLAY_CS_BIT     PB2
#define HW_DISPLAY_CS_PIN     PINB
#define HW_DISPLAY_CS_DDR     DDRB
#define HW_DISPLAY_CS_PORT    PORTB

// Кнопки
#define HW_BUTTON_1_BIT       PB0
#define HW_BUTTON_1_PIN       PINB
#define HW_BUTTON_1_DDR       DDRB
#define HW_BUTTON_1_PORT      PORTB

#define HW_BUTTON_2_BIT       PD7
#define HW_BUTTON_2_PIN       PIND
#define HW_BUTTON_2_DDR       DDRD
#define HW_BUTTON_2_PORT      PORTD

#define HW_BUTTON_3_BIT       PD6
#define HW_BUTTON_3_PIN       PIND
#define HW_BUTTON_3_DDR       DDRD
#define HW_BUTTON_3_PORT      PORTD

#define HW_BUTTON_4_BIT       PD5
#define HW_BUTTON_4_PIN       PIND
#define HW_BUTTON_4_DDR       DDRD
#define HW_BUTTON_4_PORT      PORTD

#define HW_BUTTON_5_BIT       PD4
#define HW_BUTTON_5_PIN       PIND
#define HW_BUTTON_5_DDR       DDRD
#define HW_BUTTON_5_PORT      PORTD

// Датчики
#define HW_SENSOR_WATER_BIT   PC0
#define HW_SENSOR_WATER_PIN   PINC
#define HW_SENSOR_WATER_DDR   DDRC
#define HW_SENSOR_WATER_PORT  PORTC

#define HW_SENSOR_TSA_BIT     PC1
#define HW_SENSOR_TSA_PIN     PINC
#define HW_SENSOR_TSA_DDR     DDRC
#define HW_SENSOR_TSA_PORT    PORTC

#define HW_SENSOR_TSARGA_BIT  PC2
#define HW_SENSOR_TSARGA_PIN  PINC
#define HW_SENSOR_TSARGA_DDR  DDRC
#define HW_SENSOR_TSARGA_PORT PORTC

#define HW_SENSOR_CUBE_BIT    PD3
#define HW_SENSOR_CUBE_PIN    PIND
#define HW_SENSOR_CUBE_DDR    DDRD
#define HW_SENSOR_CUBE_PORT   PORTD

// Звук
#define HW_BUZZER_BIT         PB1
#define HW_BUZZER_PIN         PINB
#define HW_BUZZER_DDR         DDRB
#define HW_BUZZER_PORT        PORTB

// Реле
#define HW_RELAY_1_INVERTED   1
#define HW_RELAY_1_BIT        PC5
#define HW_RELAY_1_PIN        PINC
#define HW_RELAY_1_DDR        DDRC
#define HW_RELAY_1_PORT       PORTC

#define HW_RELAY_2_INVERTED   1
#define HW_RELAY_2_BIT        PC4
#define HW_RELAY_2_PIN        PINC
#define HW_RELAY_2_DDR        DDRC
#define HW_RELAY_2_PORT       PORTC

#define HW_RELAY_3_INVERTED   1
#define HW_RELAY_3_BIT        PC3
#define HW_RELAY_3_PIN        PINC
#define HW_RELAY_3_DDR        DDRC
#define HW_RELAY_3_PORT       PORTC

// Bluetooth
#define HW_BLUETOOTH_EN_BIT   PD2
#define HW_BLUETOOTH_EN_PIN   PIND
#define HW_BLUETOOTH_EN_DDR   DDRD
#define HW_BLUETOOTH_EN_PORT  PORTD

#endif /* HARDWARE_H_ */
