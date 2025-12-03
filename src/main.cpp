#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Arduino.h>
#include <QMC5883LCompass.h>
#include <SPI.h>

// Screen parameters
constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;

constexpr int8_t TFT_CS = WB_SPI_CS;
constexpr int8_t TFT_RST = 27;
constexpr int8_t TFT_DC = WB_SW1;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas = GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);

QMC5883LCompass compass;

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

    compass.read();
    int heading = compass.getAzimuth() + declinationAngle;

    return heading;
}

void setup() {
    Serial.begin(9600);
    Serial.print(F("Serial initialized"));

    tft.init(TFT_WIDTH, TFT_HEIGHT);
    Serial.println(F("Screen initialized"));

    compass.init();
    compass.setCalibrationOffsets(922.00, 584.00, -2426.00);
    compass.setCalibrationScales(1.39, 1.03, 0.76);
    Serial.println("Magnetometer initialized");

    delay(1000);
}

void loop() {
    int heading = readMagnetomer();
    // Serial.printf("Heading: %f\n", heading);

    drawCompass(heading);
    // uint32_t time = millis();
    // Serial.printf("Drew compass at %d\n", time);
}