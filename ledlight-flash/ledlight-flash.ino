#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "credentials.h"
#include "OTA.h"
#include "Watering.h"
#include <string>

const char* ssid       = WIFI_SSID;
const char* password   = WIFI_PASSWD;

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
#define FLASH_GPIO_NUM 4
#define ONBOARD_LED  2

// for updating local time from the web.
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

OTA ota;
Watering watering;

void connectToWifi() {
  digitalWrite(ONBOARD_LED, HIGH);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
}
void disconnectWifi() {
  digitalWrite(ONBOARD_LED, LOW);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("DISCONNECTED");
}
void setup()
{
  Serial.begin(115200);
  pinMode(ONBOARD_LED,OUTPUT);
  connectToWifi();
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // start the OTA server.
  ota.startServer();
  watering.begin(ONBOARD_LED, FLASH_GPIO_NUM);

  Serial.println("Starting....");
}

void loop() {
  ota.handleRequests();
  delay(1000);
  watering.run();
}
