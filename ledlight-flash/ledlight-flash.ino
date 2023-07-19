#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include "credentials.h"
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid       = WIFI_SSID;
const char* password   = WIFI_PASSWD;

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
#define FLASH_GPIO_NUM 4
#define ONBOARD_LED  2

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

String configFileLocation = "https://raw.githubusercontent.com/antoon91/watering-system/main/ledlight-flash/config";
unsigned long lastPullTime = 0;
// Set timer to 5 seconds (5000)
unsigned long pullTimerDelay = 5000;

// when to start watering
const int   hourToWater = 18;//12;
const int   minuteToWater = 44;//00;
// in milliters (two pumps)
const double waterToDisplace = 800;
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
  Serial.println(h);
  Serial.println(m);

  if(shouldWater(h, m, s)) {
    setWatering();
  } else {
    setIdle();
  }
}

void readRemoteConfig() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastPullTime) > pullTimerDelay) {
    //Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(configFileLocation.c_str());
      
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode == 200) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        JSONVar myObject = JSON.parse(payload);
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastPullTime = millis();
  }
}

void setup()
{
  Serial.begin(115200);
  pinMode(ONBOARD_LED,OUTPUT);
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
  // WiFi.disconnect(true);
  // WiFi.mode(WIFI_OFF);

  Serial.println("Starting....");
}

void loop() {
  delay(1000);
  printLocalTime();
  readRemoteConfig();
}
