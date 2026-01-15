#include <Arduino.h>

#include "display.h"
#include "gps.h"
#include "imu.h"
#include "lora.h"

gnss_data gnss_fix = {0};
coords myLocation = {0, 0};
coords teamLocations[teamSize] = {0};
uint32_t lastSent;

void setup() {
    Serial.begin(115200);
    initDisplay();
    initGPS();
    initIMU();
    initLora();
}

void loop() {
    // Get current location of team members
    getTeamLocations(teamLocations);

    // Update my location, but only if new GNSS data has valid position fix
    getGNSSData(&gnss_fix);
    if (gnss_fix.quality == 1) {
        myLocation = gnss_fix.coordinates;
    }

    // Share location of this device with others
    constexpr uint32_t update_period = 10000;
    uint32_t now = millis();
    if (now - lastSent > update_period) {
        sendLocation(myLocation);
        lastSent = now;
    }

    // Get direction from current location to each team member - bearings
    int teamBearings[teamSize];
    for (int i = 0; i < teamSize; i++) {
        if (teamLocations[i].latitude_dec != 0) {
            teamBearings[i] = calculateBearing(myLocation, teamLocations[i]);
        } else {
            teamBearings[i] = -1;
        }
    }

    // Get current direction - heading
    float heading = 0;
    constexpr int averages = 10;
    for (int i = 0; i < averages; i++) {
        heading += getHeading();
    }
    heading /= averages;

    // Draw indicators for directions
    drawCompass(heading, teamBearings, &gnss_fix);
}