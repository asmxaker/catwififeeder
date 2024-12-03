#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "time.h"
#include "sntp.h"
#include <ESP32Servo.h>

Servo servo1;

unsigned long timing;
int servoPin = D4;

// unsigned long dayDelay = 14400000;    // 4 часов
// unsigned long nightDelay = 28800000;  // 8 часов
unsigned long dayDelay = 4000;    // 4 часов
unsigned long nightDelay = 8000;  // 8 часов

unsigned long timerVal = 1;
int pos = 2;

const char* ssid = "wifiname";
const char* password = "pass";

WebServer server(80);


const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

const char* time_zone = "UTC-3";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


void handleRoot() {

  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  char buffer[80];

  struct tm* timeinfo;
  if (!getLocalTime(timeinfo)) {
    Serial.println("No time available (yet)");
  }

  strftime(buffer, 80, "%Y-%m-%d% H:%M:%S", timeinfo);

  snprintf(temp, 400,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>Demo</title>\
  </head>\
  <body>\
    <h1>Hello!</h1>\
    <p>time work: %02d:%02d:%02d</p>\
    <p>Feed #:  %02d</p>\
    <p>Current time:  %02d</p>\
  </body>\
</html>",
           hr,
           min % 60,
           sec % 60,
           pos,
           buffer);
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  server.send(404, "text/plain", message);
}

void openCloseLid() {
  for (int i = 0; i <= 3; i++) {
    servo1.write(150);
    delay(400);
    servo1.write(10);
    delay(400);
  }
}

void timeavailable(struct timeval* t) {
  Serial.println("Got time adjustment from NTP!");
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  pinMode(servoPin, OUTPUT);
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  sntp_set_time_sync_notification_cb(timeavailable);
  sntp_servermode_dhcp(1);  // (optional)
  configTzTime(time_zone, ntpServer1, ntpServer2);
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  Serial.println("Servo is ready feed no " + String(pos));
  servo1.attach(servoPin);
  servo1.write(10);
  // delay(1800000); пол часа до старта
  //delay(10000);
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(1000);

  // tmRes = printLocalTime();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
  }
  Serial.println(&timeinfo, "%Y-%m-%d% H:%M:%S");
  Serial.println("HTTP server started");
  delay(2);  //allow the cpu to switch to other tasks

  feedProc();
}


/**/
// printLocalTime() {
//   struct tm timeinfo;
//   if (!getLocalTime(&timeinfo)) {
//     // Serial.println("No time available (yet)");
//     return;
//   }
//     // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//   return timeinfo;
// }

void feedProc() {
  if (pos == 5) {
    timerVal = nightDelay;
  } else {
    timerVal = dayDelay;
  }
  if (millis() - timing > timerVal) {  // значение паузы
    timing = millis();
    Serial.println("current feed  " + String(pos) + " pos ");
    openCloseLid();
    pos--;
    if (pos == 0) {
      pos = 5;
    }
  }
}
