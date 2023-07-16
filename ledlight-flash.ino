#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "credentials.h"

const char* ssid       = WIFI_SSID;
const char* password   = WIFI_PASSWD;

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
#define FLASH_GPIO_NUM 4

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

// when to start watering
const int   hourToWater = 12;
const int   minuteToWater = 00;
// in milliters
const double waterToDisplace = 1600;
// x mililiter per second, the system displaces 1 liter every 75 seconds.
const double throughput = 1000 / 75;

bool shouldWater(int h, int m, int s) {
  // Too early
  if(h < hourToWater) {
    return false;
  }
  if(h == hourToWater && m < minuteToWater) {
    return false;
  }
  double wateringSeconds = (double)waterToDisplace / throughput;
  int startEpoch = hourToWater * 3600 + minuteToWater * 60;
  int endEpoch = startEpoch + wateringSeconds;
  int currEpoch = h * 3600 + m * 60 + s;
  double totalDisplaced = (currEpoch - startEpoch) * throughput;
  Serial.print("Displaced: ");
  Serial.print(totalDisplaced);
  Serial.println("Mililiters.");
  return currEpoch < endEpoch;
}

void setIdle() {
  Serial.println("Idle...");
  digitalWrite(FLASH_GPIO_NUM, HIGH);
}
void setWatering() {
  Serial.println("Watering...");
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
  Serial.println(h);
  Serial.println(m);

  if(shouldWater(h, m, s)) {
    setWatering();
  } else {
    setIdle();
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // initialize digital pin ledPin as an output
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  setIdle();
  //disconnect WiFi as it's no longer needed
  delay(10 * 1000);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  Serial.println("Starting....");
}

void loop() {
  delay(1000);
  printLocalTime();
}
