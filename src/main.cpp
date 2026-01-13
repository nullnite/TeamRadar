#include <Arduino.h>

#include "display.h"
#include "gps.h"
#include "imu.h"
#include "lora.h"

void setup() {
    Serial.begin(115200);
    initDisplay();
    initLora();
    initGPS();
    initIMU();
}

void loop() {
    getCoordinates();

    // Magnetic declination for Kaunas, Lithuania
    // Formula: (deg + (min / 60.0)) / (180 / PI);
    constexpr int declinationAngle = (8.0 + (19.0 / 60.0)) / (180 / PI);

    float heading = 0;
    constexpr int averages = 10;
    for (int i = 0; i < averages; i++) {
        heading += getHeading();
    }
    heading /= averages;

    drawCompass(heading);
}