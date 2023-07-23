#include <iostream>
#include <string>


class RemoteConfiguration {
    private:
        unsigned long lastPullTime = 0;
        // Set timer to 5 seconds (5000)
        unsigned long pullTimerDelay = 1 * 60 * 1000;
        // when to start watering
        int hourToWater = 12;
        int minuteToWater = 00;
        // in milliters (two pumps)
        double waterToDisplace = 800;
        // x mililiter per second, the system displaces 1 liter every 75 seconds.
        const double throughput = 1000 / 75;
    public:
        void refresh();
        int getHourToWater();
        int getMinuteToWater();
        double getWaterToDisplace();
        double getThroughput();
        double getWateringSeconds();
};
 