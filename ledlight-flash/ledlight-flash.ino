#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "credentials.h"
#include "OTA.h"
#include "RemoteConfiguration.h"

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
RemoteConfiguration rc;
bool shouldWater(int h, int m, int s) {
  // Too early
  if(h < rc.getHourToWater()) {
    return false;
  }
  if(h == rc.getHourToWater() && m < rc.getMinuteToWater()) {
    return false;
  }
  int startEpoch = rc.getHourToWater() * 3600 + rc.getMinuteToWater() * 60;
  int endEpoch = startEpoch + rc.getWateringSeconds();
  int currEpoch = h * 3600 + m * 60 + s;
  if(currEpoch >= endEpoch) {
    return false;
  }
  double totalDisplaced = (currEpoch - startEpoch) * rc.getThroughput();
  Serial.print("Displaced: ");
  Serial.print(totalDisplaced);
  Serial.println("Mililiters.");
  return true;
}

void setIdle() {
  Serial.println("Idle...");
  digitalWrite(ONBOARD_LED,LOW);
  digitalWrite(FLASH_GPIO_NUM, HIGH);
}
void setWatering() {
  Serial.println("Watering...");
  digitalWrite(ONBOARD_LED,HIGH);
  digitalWrite(FLASH_GPIO_NUM, LOW);
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  time_t now = time(0);
  struct tm* timeInfo = localtime(&now);
  int h = timeInfo->tm_hour;
  int m = timeInfo->tm_min;
  int s = timeInfo->tm_sec;

  if(shouldWater(h, m, s)) {
    setWatering();
  } else {
    setIdle();
  }
}

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
  ota.startServer();
  // initialize digital pin ledPin as an output
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  setIdle();
  //disconnect WiFi as it's no longer needed
  delay(10 * 1000);

  Serial.println("Starting....");
}

void loop() {
  ota.handleRequests();
  delay(1000);
  printLocalTime();
  rc.refresh();
}
