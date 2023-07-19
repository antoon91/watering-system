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
unsigned long pullTimerDelay = 30 * 60 * 1000;

// when to start watering
int   hourToWater = 12;
int   minuteToWater = 00;
// in milliters (two pumps)
double waterToDisplace = 800;
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
  if(currEpoch >= endEpoch) {
    return false;
  }
  double totalDisplaced = (currEpoch - startEpoch) * throughput;
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

void readRemoteConfig() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastPullTime) > pullTimerDelay) {
    connectToWifi();
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
        Serial.println(payload);
        JSONVar myObject = JSON.parse(payload);
        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          http.clearAllCookies();
          http.end();
          return;
        }
        hourToWater = myObject["hour"];
        minuteToWater = myObject["minute"];
        waterToDisplace = myObject["water_amount"];
        Serial.print("Pulled new data:");
        Serial.println(hourToWater);
        Serial.println(minuteToWater);
        Serial.println(waterToDisplace);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
        disconnectWifi();
      }
      // Free resources
      http.clearAllCookies();
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastPullTime = millis();
    disconnectWifi();
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

  // initialize digital pin ledPin as an output
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  setIdle();
  //disconnect WiFi as it's no longer needed
  delay(10 * 1000);
  disconnectWifi();

  Serial.println("Starting....");
}

void loop() {
  delay(1000);
  printLocalTime();
  readRemoteConfig();
}
