#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Arduino.h>
#include <ICM_20948.h>
#include <SPI.h>

// Display
constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;

constexpr int8_t TFT_CS = PIN_QSPI_CS;
constexpr int8_t TFT_RST = PIN_LED1;
constexpr int8_t TFT_DC = PIN_NFC2;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas = GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);

// IMU
constexpr bool AD0_VAL = 0;
ICM_20948_I2C myICM;

int roundUp(int numToRound, int multiple) {
    if (multiple == 0)
        return numToRound;

    int remainder = abs(numToRound) % multiple;
    if (remainder == 0)
        return numToRound;

    if (numToRound < 0)
        return -(abs(numToRound) - remainder);
    else
        return numToRound + multiple - remainder;
}

void drawCompass(int heading) {
    constexpr int16_t radius = 100;
    constexpr uint16_t background_color = ST77XX_BLACK;
    constexpr uint16_t color = ST77XX_GREEN;

    constexpr int16_t center_x = TFT_WIDTH / 2;
    constexpr int16_t center_y = TFT_HEIGHT / 2;

    canvas.fillScreen(background_color);
    canvas.drawCircle(center_x, center_y, radius, color);

    const int rounded_heading = roundUp(heading, 40);

    const float heading_radians1 = (rounded_heading - 20) * PI / 180;
    const int16_t heading_x1 = center_x + radius * sin(heading_radians1);
    const int16_t heading_y1 = center_y + radius * cos(heading_radians1);

    const float heading_radians2 = (rounded_heading + 20) * PI / 180;
    const int16_t heading_x2 = center_x + radius * sin(heading_radians2);
    const int16_t heading_y2 = center_y + radius * cos(heading_radians2);

    canvas.fillTriangle(center_x, center_y, heading_x1, heading_y1, heading_x2, heading_y2, color);
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}

int readMagnetomer() {
    // Magnetic declination for Kaunas, Lithuania
    // Formula: (deg + (min / 60.0)) / (180 / PI);
    constexpr int declinationAngle = (8.0 + (19.0 / 60.0)) / (180 / PI);

    // compass.read();
    // int heading = compass.getAzimuth() + declinationAngle;
    int heading = 0;

    return heading;
}

void printPaddedInt16b(int16_t val) {
    if (val > 0) {
        Serial.print(" ");
        if (val < 10000) {
            Serial.print("0");
        }
        if (val < 1000) {
            Serial.print("0");
        }
        if (val < 100) {
            Serial.print("0");
        }
        if (val < 10) {
            Serial.print("0");
        }
    } else {
        Serial.print("-");
        if (abs(val) < 10000) {
            Serial.print("0");
        }
        if (abs(val) < 1000) {
            Serial.print("0");
        }
        if (abs(val) < 100) {
            Serial.print("0");
        }
        if (abs(val) < 10) {
            Serial.print("0");
        }
    }
    Serial.print(abs(val));
}

void printRawAGMT(ICM_20948_AGMT_t agmt) {
    Serial.print("RAW. Acc [ ");
    printPaddedInt16b(agmt.acc.axes.x);
    Serial.print(", ");
    printPaddedInt16b(agmt.acc.axes.y);
    Serial.print(", ");
    printPaddedInt16b(agmt.acc.axes.z);
    Serial.print(" ], Gyr [ ");
    printPaddedInt16b(agmt.gyr.axes.x);
    Serial.print(", ");
    printPaddedInt16b(agmt.gyr.axes.y);
    Serial.print(", ");
    printPaddedInt16b(agmt.gyr.axes.z);
    Serial.print(" ], Mag [ ");
    printPaddedInt16b(agmt.mag.axes.x);
    Serial.print(", ");
    printPaddedInt16b(agmt.mag.axes.y);
    Serial.print(", ");
    printPaddedInt16b(agmt.mag.axes.z);
    Serial.print(" ], Tmp [ ");
    printPaddedInt16b(agmt.tmp.val);
    Serial.print(" ]");
    Serial.println();
}

void printFormattedFloat(float val, uint8_t leading, uint8_t decimals) {
    float aval = abs(val);
    if (val < 0) {
        Serial.print("-");
    } else {
        Serial.print(" ");
    }
    for (uint8_t indi = 0; indi < leading; indi++) {
        uint32_t tenpow = 0;
        if (indi < (leading - 1)) {
            tenpow = 1;
        }
        for (uint8_t c = 0; c < (leading - 1 - indi); c++) {
            tenpow *= 10;
        }
        if (aval < tenpow) {
            Serial.print("0");
        } else {
            break;
        }
    }
    if (val < 0) {
        Serial.print(-val, decimals);
    } else {
        Serial.print(val, decimals);
    }
}

void printScaledAGMT(ICM_20948_I2C* sensor) {
    Serial.print("Scaled. Acc (mg) [ ");
    printFormattedFloat(sensor->accX(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->accY(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->accZ(), 5, 2);
    Serial.print(" ], Gyr (DPS) [ ");
    printFormattedFloat(sensor->gyrX(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->gyrY(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->gyrZ(), 5, 2);
    Serial.print(" ], Mag (uT) [ ");
    printFormattedFloat(sensor->magX(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->magY(), 5, 2);
    Serial.print(", ");
    printFormattedFloat(sensor->magZ(), 5, 2);
    Serial.print(" ], Tmp (C) [ ");
    printFormattedFloat(sensor->temp(), 5, 2);
    Serial.print(" ]");
    Serial.println();
}

void setup() {
    Serial.begin(115200);

    tft.init(TFT_WIDTH, TFT_HEIGHT);

    Wire.begin();
    Wire.setClock(400000);

    bool initialized = false;
    while (!initialized) {
        myICM.begin(Wire, AD0_VAL);

        Serial.print(F("Initialization of the sensor returned: "));
        Serial.println(myICM.statusString());
        if (myICM.status != ICM_20948_Stat_Ok) {
            Serial.println("Trying again...");
            delay(500);
        } else {
            initialized = true;
        }
    }
}

void loop() {
    int heading = readMagnetomer();
    drawCompass(heading);

    if (myICM.dataReady()) {
        myICM.getAGMT();
        // printRawAGMT(myICM.agmt);
        printScaledAGMT(&myICM);
        delay(30);
    } else {
        Serial.println("Waiting for data");
        delay(500);
    }
}