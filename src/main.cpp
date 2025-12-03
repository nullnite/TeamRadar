#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <Arduino.h>
#include <SPI.h>

// Screen parameters
constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;

constexpr int8_t TFT_CS = WB_SPI_CS;
constexpr int8_t TFT_RST = 27;
constexpr int8_t TFT_DC = WB_SW1;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void drawCompass() {
    constexpr int16_t radius = 100;
    constexpr uint16_t background_color = ST77XX_BLACK;
    constexpr uint16_t color = ST77XX_GREEN;

    tft.fillScreen(background_color);
    tft.drawCircle(TFT_WIDTH / 2, TFT_HEIGHT / 2, radius, color);
}

void setup() {
    Serial.begin(9600);
    Serial.print(F("Serial initialized"));

    tft.init(TFT_WIDTH, TFT_HEIGHT);
    Serial.println(F("Screen initialized"));
}

void loop() {
    uint32_t time = millis();
    drawCompass();
    Serial.printf("Drew compass at %d\n", time);
    delay(500);
}