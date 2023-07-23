#include "RemoteConfiguration.h"
#include <Arduino_JSON.h>
#include <HTTPClient.h>

const String configFileLocation = "https://raw.githubusercontent.com/antoon91/watering-system/main/ledlight-flash/config";

void RemoteConfiguration::refresh() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastPullTime) > pullTimerDelay) {
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
    }
    // Free resources
    http.clearAllCookies();
    http.end();

    lastPullTime = millis();
  }
}
int RemoteConfiguration::getHourToWater() {
    return hourToWater;
}
int RemoteConfiguration::getMinuteToWater() {
    return minuteToWater;
}
double RemoteConfiguration::getWaterToDisplace() {
    return waterToDisplace;
}
double RemoteConfiguration::getThroughput() {
    return throughput;
}
double RemoteConfiguration::getWateringSeconds() {
    return waterToDisplace / throughput;
}