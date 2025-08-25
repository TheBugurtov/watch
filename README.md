# Wemos D1 Mini Clock + Weather Display

Простая прошивка для Wemos D1 Mini с дисплеем 4x7 сегментов и RTC DS1302, показывающая время и погоду.

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

### WiFi
- Настройки внутри кода:  
  SSID: `WIFINAME`  
  PASSWORD: `WIFIPASS`

## Использование
1. Подключить все компоненты по пинам выше.
2. Залить прошивку на Wemos D1 Mini.
3. Открыть Serial Monitor для логов RTC и WiFi.
