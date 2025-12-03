#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Arduino.h>
#include <DFRobot_QMC5883.h>
#include <SPI.h>

// Screen parameters
constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;

constexpr int8_t TFT_CS = WB_SPI_CS;
constexpr int8_t TFT_RST = 27;
constexpr int8_t TFT_DC = WB_SW1;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

DFRobot_QMC5883 compass(&Wire, QMC5883_ADDRESS);

void drawCompass() {
    constexpr int16_t radius = 100;
    constexpr uint16_t background_color = ST77XX_BLACK;
    constexpr uint16_t color = ST77XX_GREEN;

    tft.fillScreen(background_color);
    tft.drawCircle(TFT_WIDTH / 2, TFT_HEIGHT / 2, radius, color);
}

float readMagnetomer() {
    // Magnetic declination for Kaunas, Lithuania
    // Formula: (deg + (min / 60.0)) / (180 / PI);
    constexpr float declinationAngle = (8.0 + (19.0 / 60.0)) / (180 / PI);
    compass.setDeclinationAngle(declinationAngle);

    sVector_t mag = compass.readRaw();
    compass.getHeadingDegrees();

    return mag.HeadingDegress;
}

void setup() {
    Serial.begin(9600);
    Serial.print(F("Serial initialized"));

    tft.init(TFT_WIDTH, TFT_HEIGHT);
    Serial.println(F("Screen initialized"));

    while (!compass.begin()) {
        Serial.println("Could not find a valid QMC5883 sensor, check wiring!");
        delay(500);
    }
    Serial.println("Magnetometer initialized");

    delay(1000);
}

void loop() {
    float heading = readMagnetomer();
    Serial.printf("Heading: %f\n", heading);

    drawCompass();
    uint32_t time = millis();
    Serial.printf("Drew compass at %d\n", time);

    delay(500);
}