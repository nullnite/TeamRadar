#include <Arduino.h>

#include "display.h"
#include "gps.h"
#include "imu.h"
#include "lora.h"

gnss_data gnss_fix = {0};
coords myLocation = {0, 0};
coords teamLocations[teamSize] = {0};
uint32_t lastSent = 0;

constexpr int averages = 10;
int headingIndex = 0;
float headings[averages] = {0};

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

    // Calculate moving average of current direction - heading
    headings[headingIndex] = getHeading();
    headingIndex++;
    if (headingIndex == averages) headingIndex = 0;

    float sumSin = 0;
    float sumCos = 0;
    for (int i = 0; i < averages; i++) {
        float rad = headings[i] * DEG_TO_RAD;
        sumSin += sin(rad);
        sumCos += cos(rad);
    }

    float average_radians = atan2(sumSin / averages, sumCos / averages);
    float average_heading = average_radians * RAD_TO_DEG;

    if (average_heading < 0) average_heading += 360;

    // Draw indicators for directions
    drawCompass(average_heading, teamBearings, &gnss_fix);
}