# Wemos D1 Mini Clock + Weather Display

Прошивка для Wemos D1 Mini с двумя 4x7 сегментными дисплеями и RTC DS1302. Показывает время и погоду.

## Что делает код
- Часы отображают текущее время с RTC DS1302.
- RTC синхронизируется с NTP при подключении к WiFi.
- Второй дисплей показывает температуру и погодное состояние (например, `22* Clear`).
- Погода обновляется каждые 10 минут через OpenWeather API.
- Логи времени RTC выводятся в Serial Monitor.

## Пины подключения

### Дисплей часов (TM1637)
- CLK → D1
- DIO → D2

### Дисплей погоды (TM1637)
- CLK → D5
- DIO → D6

### RTC DS1302
- DAT → D7
- CLK → D0
- RST → D8

## Настройки WiFi
- SSID: `Wi-Fi NAME`
- PASSWORD: `Wi-Fi PASS`

## Настройки погоды
- Город: `Krasnodar` (можно поменять)
- API Key: получить на [https://openweathermap.org](https://openweathermap.org)
- Единицы измерения: `metric`
- Язык: `en`

## Использование
1. Подключить все компоненты по указанным пинам.
2. Получить OpenWeather API ключ и вставить в код.
3. Залить прошивку на Wemos D1 Mini.
4. Открыть Serial Monitor для логов RTC и WiFi.
