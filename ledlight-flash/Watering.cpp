#include "Watering.h"

void Watering::setIdle() {
  Serial.println("Idle...");
  digitalWrite(indicatorPin, LOW);
  digitalWrite(pumpPin, HIGH);
}
void Watering::setWatering() {
  Serial.println("Watering...");
  digitalWrite(indicatorPin, HIGH);
  digitalWrite(pumpPin, LOW);
}

bool Watering::shouldWater(int h, int m, int s) {
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

void Watering::water() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
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
void Watering::begin(int lightPin, int outputPin) {
    indicatorPin = lightPin;
    pumpPin = outputPin;
    pinMode(pumpPin, OUTPUT);
    setIdle();
}
void Watering::run() {
    rc.refresh();
    water();
}
