# OpenDistiller

[![Donate](https://img.shields.io/badge/donate-YooMoney-blueviolet.svg)](https://yoomoney.ru/to/4100110221014297)

Открытая система автоматизации для ректификационной колонны, основанная на микроконтроллере [Arduino Nano](https://docs.arduino.cc/hardware/nano) ([ATmega328P](https://www.microchip.com/en-us/product/ATMEGA328P)).

В качестве датчиков температуры используются популярные сенсоры [DS18B20](https://www.analog.com/en/products/ds18b20.html). Более продробную информацию об аппаратном обеспечении можно получить на соответствующей [странице](/hardware).

Инструкции по самостоятельной сборке прошивки и загрузке ее в микроконтроллер находятся [здесь](/firmware).

## Описание настроек

* **Температура в кубе**
  * **Старт подачи воды** - значение температуры в кубе, при достижении которого подается сигнал на включение реле управления охлаждением;
  * **Товарный спирт до** - значение температуры в кубе, при достижении которого происходит приостановка процесса ректификации и вывод информационного сообщения о необходимости замены принимающей емкости;
  * **Окончание работы** - значение температуры в кубе, при достижении которого происходит окончание процесса ректификации.
* **Работа на себя**
  * **Темп.старта (царга)** - значение температуры датчика царги, при достижении которого начинается первоначальная "работа на себя";
  * **Длительность (мин)** - значение времени в минутах, в течении которого происходит первоначальная "работа на себя".
* **Отбор спирта**
  * **Дельта отбора** - значение дельты температуры датчика царги с шагом в 0.0625 °C, при превышении которого происходит приостановка отбора продукта и начало промежуточной "работы на себя" (процесс восстановления теоретических тарелок в царге);
  * **Восстановление (мин)** - значение времени в минутах, в течении которого происходит промежуточная "работа на себя".
* **Безопасность**
  * **Контроль датчиков** - включение или отключение функции контроля работоспособности датчиков температуры;
  * **Защита темп. ТСА** - значение максимальной температуры в трубке связи с атмосферой (ТСА), при достижении которого происходит аварийная остановка процесса ректификации;
  * **Защита темп. воды** - значение максимальной температуры воды на выходе системы охлаждения, при достижении которого происходит аварийная остановка процесса ректификации;
  * **Время сброса флегмы** - значение времени в минутах, в течении которого происходит доохлаждение дефлегматора после завершения процесса ректификации.
* **Bluetooth модуль**
  * **Включить** - включение передачи служебной информации через беспроводную сеть;
  * **Отключить** - отключение передачи служебной информации через беспроводную сеть.
* **Сброс настроек** - восстановление настроек на значения по умолчанию и перезагрузка системы.
* **О проекте...** - вывод информации об авторе и версии программного обеспечения.

## Алгоритм ректификации

Сильно упрощенный, но, тем не менее, достаточный для понимания общих принципов работы алгоритм представлен ниже:

![Алгоритм](/algorithm.png)

## Интерфейс ректификации
