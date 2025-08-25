#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <GyverSegment.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// ===== WiFi настройки =====
const char* ssid     = "wifi ssid";
const char* password = "wifi password";

// ===== OpenWeather =====
const String city = "Krasnodar";
const String apiKey = "OpenWeather apiKey";
const String units = "metric";
const String lang  = "en";

// ===== Настройки NTP =====
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3 * 3600;
const int   daylightOffset_sec = 0;

// ===== Дисплеи =====
Disp1637Colon clockDisp(D2, D1, true);    // CLK=D1, DIO=D2 (часы)
Disp1637Colon weatherDisp(D6, D5, false); // CLK=D5, DIO=D6 (погода)

// Бегущая строка для дисплея погоды
SegRunner weatherRunner(&weatherDisp);

// Глобальная переменная для текста погоды
String weatherString = "WAIT";
unsigned long lastClockUpdate = 0;
unsigned long lastWeatherUpdate = 0;
// ===== DS1302 =====
ThreeWire myWire(D7, D0, D8); // IO=D7, SCLK=D0, CE=D8 (DAT, CLK, RST)
RtcDS1302<ThreeWire> Rtc(myWire);

// ===== Функция для логирования времени RTC =====
void logRtcTime() {
  RtcDateTime now = Rtc.GetDateTime();
  
  Serial.print("RTC Time: ");
  Serial.print(now.Year());
  Serial.print("-");
  Serial.print(now.Month());
  Serial.print("-");
  Serial.print(now.Day());
  Serial.print(" ");
  Serial.print(now.Hour());
  Serial.print(":");
  Serial.print(now.Minute());
  Serial.print(":");
  Serial.print(now.Second());
  
  if (!now.IsValid()) {
    Serial.print(" - INVALID TIME");
  }
  
  Serial.println();
}

// ===== Функция получения погоды =====
void updateWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    weatherString = "NO WiFi";
    weatherRunner.setText(weatherString);
    weatherRunner.start();
    return;
  }

  WiFiClient client;
  HTTPClient http;

  String url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + apiKey + "&units=" + units + "&lang=" + lang;

  http.begin(client, url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float temp = doc["main"]["temp"];
      String weather = doc["weather"][0]["main"].as<String>();
      char buf[20];
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
    clockDisp.colon(now.Second() % 2 == 0);  // точки
    clockDisp.update();  // ОБЯЗАТЕЛЬНО
}


// ===== Настройки =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Starting ===");

  // Инициализация дисплеев
  clockDisp.brightness(7);
  weatherDisp.brightness(7);

  // Короткий тест дисплея
  clockDisp.print("8888");
  clockDisp.colon(true);
  delay(500);
  clockDisp.clear(); // Очищаем дисплей
  delay(500);

  Rtc.Begin();
  logRtcTime();

  // Подключаемся к WiFi
  WiFi.begin(ssid, password);
  
  // Показываем процесс подключения
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
    
    // Получаем время NTP
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 5000)) {
      // Синхронизируем RTC
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
      
      // Сразу обновляем дисплей после синхронизации
      updateClockDisplay();
    }
  } else {
    Serial.println("WiFi connection failed");
    // Используем время из RTC если WiFi не подключился
    updateClockDisplay();
  }

  // Инициализация бегущей строки
  weatherRunner.setText(weatherString);
  weatherRunner.start();

  // Первая загрузка погоды
  updateWeather();

  Serial.println("Setup complete");
}

// ===== Основной цикл =====
void loop() {
    updateClockDisplay();
    weatherRunner.tick();

    if (millis() - lastWeatherUpdate > 600000) {
        updateWeather();
        lastWeatherUpdate = millis();
    }

    delay(10);
}
