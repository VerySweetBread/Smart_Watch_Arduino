#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "SSC"
#define STAPSK  "nyan_cat"
#endif

#define TFT_CS D1
#define TFT_RST D0
#define TFT_DC D2

const uint16_t grey = 0x5AEB;   //цвет
const uint16_t white = 0xFFFF;  //цвет
const uint16_t black = 0x0000;  //цвет

const char* ssid     = STASSID;
const char* password = STAPSK;

int time_timer = 0;
int temp_timer = 0;

ESP8266WebServer server(80);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  // setSyncProvider(requestSync);
  WiFi.begin(ssid, password);

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);      // Поворот текста
  tft.setTextWrap(false);
  tft.fillScreen(black);
  tft.setTextColor(ST7735_RED); // цвет
  tft.setTextSize(1, 1);         // размер
  tft.setCursor(10, 50);        // Позиция на экране
  tft.println(utf8rus("Подключение к WiFi: ")); //Вывод на экран

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("."); 
  }

  tft.fillScreen(black);
  tft.setCursor(10, 50);
  tft.print(utf8rus("Подключено к: "));
  tft.println(ssid);
  tft.setCursor(10, 60);
  tft.print("IP: ");
  tft.println(WiFi.localIP());

  server.onNotFound(handleNotFound);
  server.on("/meow", meow);
  server.on("/time", time_str_func);
  server.on("/set_time", set_time);

  server.begin();
}

void loop() {
  WiFiClient client;
  HTTPClient http;

  if (timeStatus()!= timeNotSet) {
    if (time_timer <= millis() - 950) {
      tft.fillRect(0, 10, tft.width(), 20, black);
      tft.setCursor(10, 10);
      tft.print(digitalClockDisplay());  
      time_timer = millis();
    }

    if (temp_timer <= millis() - 10000) {
      if (http.begin(client, "http://188.243.185.142:5000/API/temperature")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
          String payload = http.getString();
          tft.fillRect(0, 60, tft.width(), 20, black);
          tft.setCursor(10, 60);
          tft.print(utf8rus(payload));  
        }
      }
      temp_timer = millis();
    }
  }

  server.handleClient();
}


String digitalClockDisplay() {
  // digital clock display of the time

  String time_str = printDigits(hour());  time_str += ":";
  time_str += printDigits(minute());      time_str += ":";
  time_str += printDigits(second());

  time_str += " ";

  time_str += printDigits(day());         time_str += ".";
  time_str += printDigits(month());       time_str += ".";
  time_str += printDigits(year()); 

  return time_str;
}

void time_str_func() {
  String time_str = digitalClockDisplay();
  server.send(200, "text/plain", time_str);
}

String printDigits(int digits) {
  String out = String(digits);
  if(digits < 10)
    out = '0' + out;
  return out;
}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);
  server.send(404, "text/plain", message);
}

void meow() {
  tft.fillScreen(black);
  server.send(200, "text/plain", "meow");
}

void set_time() {
  String str_time = server.arg(0);
  int time = str_time.toInt();
  setTime(time);
  server.send(200, "text/plain", "OK");
}