#include <Arduino.h>

#include "display.h"
#include "gps.h"
#include "imu.h"
#include "lora.h"

void setup() {
    Serial.begin(115200);
    initDisplay();
    initGPS();
    initIMU();
    initLora();
}

void loop() {
    // Get current direction of this and every other device
    // coords myLocation = getLocation();
    coords myLocation = {54.868, 23.938};

    coords teamLocations[teamSize] = {{54.868, 23.936},
                                      {54.868, 23.938}};
    // getTeamLocations(teamLocations);

    int teamBearings[teamSize] = {0, 0};

    // Get direction from current location to each team member - bearings
    for (int i = 0; i < teamSize; i++) {
        teamBearings[i] = calculateBearing(myLocation, teamLocations[i]);
    }

    // Get current direction - heading
    float heading = 0;
    constexpr int averages = 10;
    for (int i = 0; i < averages; i++) {
        heading += getHeading();
        delay(10);
    }
    heading /= averages;

    // Draw indicators for directions
    drawCompass(heading, teamBearings);
}