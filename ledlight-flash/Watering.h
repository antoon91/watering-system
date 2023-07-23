#include <Arduino.h>
#include <iostream>
#include <string>
#include "RemoteConfiguration.h"
#include "time.h"


class Watering {
    private:
        int indicatorPin;
        int pumpPin;
        RemoteConfiguration rc;
        void setIdle();
        void setWatering();
        bool shouldWater(int h, int m, int s);
        void water();
    public:
        void begin(int lightPin, int outputPin);
        void run();
}; 