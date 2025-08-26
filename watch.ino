#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <GyverSegment.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ===== WiFi настройки =====
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== WeatherAPI =====
const String city = "YOUR_CITY";  
const String apiKey = "YOUR_API_KEY"; 

// ===== Настройки NTP =====
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3 * 3600;
const int   daylightOffset_sec = 0;

// ===== Дисплеи =====
Disp1637Colon clockDisp(D2, D1, true);    // CLK=D1, DIO=D2 (часы)
Disp1637Colon weatherDisp(D6, D5, false); // CLK=D5, DIO=D6 (погода)

// Бегущая строка для дисплея погоды
SegRunner weatherRunner(&weatherDisp);

// Глобальные переменные
String weatherString = "WAIT";
unsigned long lastWeatherUpdate = 0;
unsigned long lastRtcSync = 0;   // для повторной синхронизации RTC

// ===== DS1302 =====
ThreeWire myWire(D7, D0, D8); // IO=D7, SCLK=D0, CE=D8 (DAT, CLK, RST)
RtcDS1302<ThreeWire> Rtc(myWire);

// ===== Функция для логирования времени RTC =====
void logRtcTime() {
  RtcDateTime now = Rtc.GetDateTime();
  
  Serial.print("RTC Time: ");
  Serial.printf("%04u-%02u-%02u %02u:%02u:%02u",
    now.Year(), now.Month(), now.Day(),
    now.Hour(), now.Minute(), now.Second());
  
  if (!now.IsValid()) {
    Serial.print(" - INVALID TIME");
  }
  
  Serial.println();
}

// ===== Синхронизация RTC с NTP =====
void syncRtcWithNtp() {
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
      RtcDateTime t = RtcDateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      );
      Rtc.SetDateTime(t);
      Serial.println("RTC synced with NTP");
      logRtcTime();
    } else {
      Serial.println("Failed to get NTP time");
    }
  } else {
    Serial.println("WiFi not connected, RTC not synced");
  }
}

// ===== Функция получения погоды (WeatherAPI) =====
void updateWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    weatherString = "NO WiFi";
    weatherRunner.setText(weatherString);
    weatherRunner.start();
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + city + "&aqi=no";

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float temp = doc["current"]["temp_c"];
      String weather = doc["current"]["condition"]["text"].as<String>();
      char buf[32];
      snprintf(buf, sizeof(buf), "%.0f* %s", temp, weather.c_str());
      weatherString = String(buf);
    } else {
      weatherString = "JSON ERR";
    }
  } else {
    weatherString = "HTTP ERR";
  }

  weatherRunner.setText(weatherString);
  weatherRunner.start();
  http.end();
}

// ===== Функция обновления дисплея часов =====
void updateClockDisplay() {
    RtcDateTime now = Rtc.GetDateTime();
    if (!now.IsValid()) {
        clockDisp.setCursor(0);
        clockDisp.print("ERR");
        clockDisp.update();
        return;
    }

    char buf[5];
    sprintf(buf, "%02d%02d", now.Hour(), now.Minute());

    clockDisp.setCursor(0);
    clockDisp.print(buf);
    clockDisp.colon(now.Second() % 2 == 0);
    clockDisp.update();
}

// ===== Настройки =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Starting ===");

  // Инициализация дисплеев
  clockDisp.brightness(7);
  weatherDisp.brightness(7);

  // Тест дисплея
  clockDisp.print("LOH");
  clockDisp.colon(true);
  delay(500);
  clockDisp.clear();
  delay(500);

  Rtc.Begin();
  logRtcTime();

  // Подключаемся к WiFi
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(250);
    static bool dots = false;
    clockDisp.print("CONN");
    clockDisp.colon(dots);
    dots = !dots;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    syncRtcWithNtp();  // сразу при старте
  } else {
    Serial.println("WiFi connection failed");
    updateClockDisplay();
  }

  // Инициализация бегущей строки
  weatherRunner.setText(weatherString);
  weatherRunner.start();

  updateWeather(); // первая загрузка
  Serial.println("Setup complete");
}

// ===== Основной цикл =====
void loop() {
    updateClockDisplay();
    weatherRunner.tick();

    // Обновляем погоду каждые 10 минут
    if (millis() - lastWeatherUpdate > 600000) {
        updateWeather();
        lastWeatherUpdate = millis();
    }

    // Синхронизируем RTC с NTP каждые 10 минут
    if (millis() - lastRtcSync > 600000) {
        syncRtcWithNtp();
        lastRtcSync = millis();
    }

    delay(10);
}
