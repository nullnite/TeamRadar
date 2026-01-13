#include "display.h"

constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;

constexpr int8_t TFT_CS = PIN_QSPI_CS;
constexpr int8_t TFT_RST = PIN_LED1;
constexpr int8_t TFT_DC = PIN_NFC2;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas = GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);

void initDisplay() {
    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(2);
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

    canvas.setCursor(center_x-18, TFT_HEIGHT-24);
    canvas.setTextColor(color);
    canvas.setTextSize(3);
    canvas.setTextWrap(true);
    canvas.print(roundUp(heading, 10));

    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());
}