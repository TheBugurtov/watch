# ESP8266 Clock & Weather Display

Отображает текущее время и температуру/погодные условия на двух 7-сегментных дисплеях: часы и бегущая строка с погодой. Используется модуль RTC DS1302 для хранения времени и синхронизация с NTP через WiFi.

## Подключение
- **Дисплей часов:** CLK → D1, DIO → D2
- **Дисплей погоды:** CLK → D5, DIO → D6
- **RTC DS1302:** DAT (IO) → D7, CLK (SCLK) → D0, RST (CE) → D8

## Настройки кода
**WiFi:**
```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```
**WeatherAPI:**
API ключ получить на https://www.weatherapi.com
```cpp
const String city = "YOUR_CITY";
const String apiKey = "YOUR_API_KEY";
```
**Часовой пояс (GMT)::**
```cpp
const long  gmtOffset_sec = 3 * 3600;  // GMT+3
const int   daylightOffset_sec = 0;
```
## Настройки кода
- Подключить к питанию ESP8266.
- Дисплей часов покажет время из RTC.
- Дисплей погоды покажет температуру и погодные условия из WeatherAPI.
- RTC синхронизируется с NTP каждые 10 минут.
- Погода обновляется каждые 10 минут.

## Библиотеки
- ESP8266WiFi
- ESP8266HTTPClient
- ArduinoJson
- GyverSegment
- ThreeWire
- RtcDS1302
