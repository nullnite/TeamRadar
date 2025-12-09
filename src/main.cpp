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

    int heading = getHeading();

    drawCompass(heading);
}