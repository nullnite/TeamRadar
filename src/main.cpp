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
    coords teamLocations[teamSize];
    getTeamLocations(teamLocations);

    coords myLocation = getLocation();

    // Share location of this device with others
    sendLocation(myLocation);

    // Get direction from current location to each team member - bearings
    int teamBearings[teamSize];
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