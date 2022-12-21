# Прошивка для микроконтроллера Arduino Nano (ATmega328P)

[![Donate](https://img.shields.io/badge/donate-YooMoney-blueviolet.svg)](https://yoomoney.ru/to/4100110221014297)

В качестве хост-машины я использовал компьютер с Windows 10 (x64).

## Необходимые инструменты

* **Свежие сборки AVR-GCC:**
  * https://github.com/ZakKemble/avr-gcc-build/releases
  * https://blog.zakkemble.net/avr-gcc-builds/
* **AVR 8-Bit Toolchain (Windows):**
  * https://www.microchip.com/en-us/tools-resources/develop/microchip-studio/gcc-compilers
  * https://www.microchip.com/en-us/tools-resources/archives/avr-sam-mcus#AVR%20and%20Arm-Based%20MCU%20Toolchains
* **Git for Windows:**
  * https://gitforwindows.org
  * https://github.com/git-for-windows/git/releases

## Подготовка окружения

1. Скачайте последнюю сборку AVR-GCC для Windows (на момент составления инструкции это была версия 12.1.0) и распакуйте ее в каталог `C:\Program Files\avr-gcc\`;
2. Скачайте официальную версию AVR 8-Bit Toolchain (Windows) и распакуйте из архива файл `bin\avr-size.exe` в каталог `C:\Program Files\avr-gcc\bin\`, подтвердив замену;
3. Скачайте и установите Git для Windows, следуя официальной [инструкции](https://github.com/git-guides/install-git#install-git-on-windows) от GitHub.

## Обновление библиотек

*Этот пункт необязателен и вы можете его пропустить.*

В данном проекте используются некоторые сторонние библиотеки, версии которых (при необходимости) вы можете обновить:

* **U8g2** (работа с дисплеем, кнопками и интерфейсом): https://github.com/olikraus/u8g2
* **avr-ds18b20** (получение значений с датчиков температуры): https://github.com/Jacajack/avr-ds18b20

1. Запустите файл `libraries\update.bat` и дождитесь сообщения `Нажмите любую клавишу...`.

## Сборка прошивки

1. Запустите файл `build.bat` и дождитесь окончания процесса;
2. Файл прошивки будет иметь название `firmware.hex` и находиться в корневом каталоге.

## Прошивка микроконтроллера

1. Подключите Arduino Nano к компьютеру с помощью внутрисхемного программатора [USBasp](https://alii.pub/6jxa4k);
2. Зажмите кнопку `Reset` на плате и запустите файл `flash.bat`;
3. Не отпускайте зажатую кнопку до окончания процесса прошивки.

## Дополнительная информация

Для удобства можно воспользоваться файлом `all.bat`, который последовательно выполнит все необходимые операции (обновит библиотеки, соберет прошивку и загрузит ее на микроконтроллер).
