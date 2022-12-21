@REM    OpenDistiller Project
@REM    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
@REM    URL: https://github.com/kasitoru/OpenDistiller

@ECHO off
CHCP 65001 > nul

SET "CROSS_COMPILE=C:\Program Files\avr-gcc\bin\avr-"
SET "CCFLAGS=-Wall -Os -mmcu=atmega328p -DF_CPU=16000000L -ffunction-sections -fdata-sections -Wl,--gc-sections,-u,vfprintf -lprintf_flt --param=min-pagesize=0"

DEL /F /Q firmware.hex
RMDIR /S /Q temp
MKDIR temp
CD temp

ECHO Компиляция libraries/u8g2...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -DU8X8_USE_PINS -DMUI_MAX_TEXT_LEN=64 -c ../libraries/u8g2/csrc/*.c"

ECHO Компиляция libraries/u8x8_avr...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -DU8X8_USE_PINS -I ../libraries/u8g2/csrc/ -c ../libraries/u8x8_avr/*.c"

ECHO Компиляция libraries/avr-ds18b20...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -DONEWIRE_AUTO_CLI -I ../libraries/avr-ds18b20/include/ -c ../libraries/avr-ds18b20/src/*.c"

ECHO Компиляция libraries/uart...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -c ../libraries/uart/*.c"

ECHO Компиляция libraries/tone...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -c ../libraries/tone/*.c"

ECHO Компиляция libraries/functions.c...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -c ../libraries/functions.c"

ECHO Компиляция main.c...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% -DLANG=RU -DU8X8_USE_PINS -DMUI_MAX_TEXT_LEN=64 -I ../libraries/u8g2/csrc/ -c ../main.c"

ECHO Линковка файлов...
CMD /C ""%CROSS_COMPILE%gcc.exe" %CCFLAGS% *.o -o firmware.o"

ECHO Сборка прошивки...
CMD /C ""%CROSS_COMPILE%objcopy.exe" -j .text -j .data -O ihex firmware.o ../firmware.hex"
ECHO:
CMD /C ""%CROSS_COMPILE%size.exe" --mcu=atmega328p -C firmware.o"

CD ..
RMDIR /S /Q temp

pause
