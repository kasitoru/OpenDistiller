@REM    OpenDistiller Project
@REM    Author: Sergey Avdeev (avdeevsv91@yandex.ru)
@REM    URL: https://github.com/kasitoru/OpenDistiller

@ECHO off
CHCP 65001 > nul

SET "AVRDUDE=C:\Program Files\avr-gcc\bin\avrdude.exe"

ECHO Прошивка микроконтроллера...
CMD /C ""%AVRDUDE%" -p atmega328p -c usbasp -U lfuse:w:0xDF:m -U hfuse:w:0xDF:m -U efuse:w:0xFD:m -U lock:w:0xFF:m -U flash:w:firmware.hex"

pause
