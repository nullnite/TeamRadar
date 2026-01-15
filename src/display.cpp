#include "display.h"

constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;
constexpr uint8_t TFT_ROTATION = 2;

constexpr int8_t TFT_CS = PIN_QSPI_CS;
constexpr int8_t TFT_RST = PIN_LED1;
constexpr int8_t TFT_DC = PIN_NFC2;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas = GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);

void initDisplay() {
    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(TFT_ROTATION);
}

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

void drawCompass(int heading, int bearings[], gnss_data* gnss_fix) {
    constexpr int16_t radius = 100;
    constexpr uint16_t background_color = ST77XX_BLACK;
    constexpr uint16_t color = ST77XX_GREEN;

    constexpr int16_t center_x = TFT_WIDTH / 2;
    constexpr int16_t center_y = TFT_HEIGHT / 2;

    // Outer circle
    canvas.fillScreen(background_color);
    canvas.drawCircle(center_x, center_y, radius, color);

    // Heading marker
    const int rounded_heading = roundUp(heading, 30);

    const float heading_radians1 = (rounded_heading - 15) * DEG_TO_RAD;
    const int16_t heading_x1 = center_x + radius * sin(heading_radians1);
    const int16_t heading_y1 = center_y + radius * cos(heading_radians1);

    const float heading_radians2 = (rounded_heading + 15) * DEG_TO_RAD;
    const int16_t heading_x2 = center_x + radius * sin(heading_radians2);
    const int16_t heading_y2 = center_y + radius * cos(heading_radians2);

    // canvas.fillTriangle(center_x, center_y, heading_x1, heading_y1, heading_x2, heading_y2, color);

    // Heading indicator
    canvas.setCursor(center_x - 18, TFT_HEIGHT - 24);
    canvas.setTextColor(color);
    canvas.setTextSize(3);
    canvas.print(heading);

    // Team indicators
    for (int i = 0; i < teamSize; i++) {
        if (bearings[i] != -1) {
            canvas.setCursor(center_x + i * 18, center_y + i * 24);
            canvas.println(bearings[i]);
        }
    }

    // GNSS data indicators
    constexpr uint16_t bad_color = ST77XX_RED;
    constexpr uint16_t fair_color = ST77XX_YELLOW;
    constexpr uint16_t good_color = ST77XX_GREEN;

    // Number of GNSS satellites
    canvas.setCursor(0 + 18, 0 + 24 / 2);
    if (gnss_fix->satellites == 0) {
        canvas.setTextColor(bad_color);
    } else if (gnss_fix->satellites < 4) {
        canvas.setTextColor(fair_color);
    } else {
        canvas.setTextColor(good_color);
    }
    canvas.print(gnss_fix->satellites);

    // Position fix
    canvas.setCursor(TFT_WIDTH - 18 * 3.5, 0 + 24 / 2);
    if (gnss_fix->quality == 0) {
        canvas.setTextColor(bad_color);
        canvas.print("Bad");
    } else if (gnss_fix->quality >= 1) {
        canvas.setTextColor(good_color);
        canvas.print("Fix");
    }

    // Double buffer to avoid flicker
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}