#include "display.h"

constexpr uint16_t TFT_WIDTH = 240;
constexpr uint16_t TFT_HEIGHT = 280;
constexpr uint8_t TFT_ROTATION = 2;

constexpr int8_t TFT_CS = PIN_QSPI_CS;
constexpr int8_t TFT_RST = PIN_LED1;
constexpr int8_t TFT_DC = PIN_NFC2;

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
GFXcanvas16 canvas = GFXcanvas16(TFT_WIDTH, TFT_HEIGHT);

constexpr uint8_t button1Pin = WB_SW1;
constexpr uint8_t button2Pin = WB_IO2;
constexpr uint8_t button3Pin = WB_IO1;
constexpr uint8_t backlightPWMPin = WB_A1;
constexpr uint8_t batteryVoltagePin = PIN_A0;

volatile int backlightBrightness = 255;

void button1Pressed() {
    backlightBrightness = backlightBrightness + 50;
    if (backlightBrightness > 255) {
        backlightBrightness = 255;
    }
}

void button2Pressed() {
    /*
    if (backlightBrightness > 127) {
        backlightBrightness = 0;
    } else {
        backlightBrightness = 255;
    }
    */
}

void button3Pressed() {
    backlightBrightness = backlightBrightness - 50;
    if (backlightBrightness < 0) {
        backlightBrightness = 0;
    }
}

void initDisplay() {
    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(TFT_ROTATION);

    // Buttons
    pinMode(button1Pin, INPUT_PULLUP);
    pinMode(button2Pin, INPUT_PULLUP);
    pinMode(button3Pin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(button1Pin), button1Pressed, FALLING);
    attachInterrupt(digitalPinToInterrupt(button2Pin), button2Pressed, FALLING);
    attachInterrupt(digitalPinToInterrupt(button3Pin), button3Pressed, FALLING);

    // Backlight control
    pinMode(backlightPWMPin, OUTPUT);

    // Battery voltage
    pinMode(batteryVoltagePin, INPUT);
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

void drawCompass(int heading, int bearings[], int distances[], gnss_data* gnss_fix) {
    constexpr int16_t radius = 100;
    constexpr uint16_t background_color = ST77XX_BLACK;
    constexpr uint16_t color = ST77XX_GREEN;

    constexpr int16_t center_x = TFT_WIDTH / 2;
    constexpr int16_t center_y = TFT_HEIGHT / 2;

    constexpr int16_t char_width = 18;
    constexpr int16_t char_height = 24;

    // Outer circle
    canvas.fillScreen(background_color);
    canvas.drawCircle(center_x, center_y, radius, color);

    /*
    // Heading marker
    const int rounded_heading = roundUp(heading, 30);

    const float heading_radians1 = (rounded_heading - 15) * DEG_TO_RAD;
    const int16_t heading_x1 = center_x + radius * sin(heading_radians1);
    const int16_t heading_y1 = center_y - radius * cos(heading_radians1);

    const float heading_radians2 = (rounded_heading + 15) * DEG_TO_RAD;
    const int16_t heading_x2 = center_x + radius * sin(heading_radians2);
    const int16_t heading_y2 = center_y - radius * cos(heading_radians2);

    canvas.fillTriangle(center_x, center_y, heading_x1, heading_y1, heading_x2, heading_y2, color);
    */

    // Heading indicator
    canvas.setCursor(center_x - char_width, TFT_HEIGHT - char_height);
    canvas.setTextColor(color);
    canvas.setTextSize(3);
    canvas.print(heading);

    // Team indicators
    for (int i = 0; i < teamSize; i++) {
        if (bearings[i] != -1) {
            const char ID = i;

            // Calculate relative angle
            const int angle_rounded = roundUp(bearings[i] - heading, 10);
            const float angle_radians = angle_rounded * DEG_TO_RAD;

            // Clamp distance to 0–500 m
            float distance = distances[i];
            if (distance < 0) distance = 0;
            if (distance > 500) distance = 500;

            // Scale and apply distance
            constexpr float deadzone = 10.0f;
            const float usable_radius = radius - deadzone;
            const float scaled_radius = deadzone + (distance / 500.0f) * usable_radius;

            const int16_t x = center_x + scaled_radius * sin(angle_radians);
            const int16_t y = center_y - scaled_radius * cos(angle_radians);

            canvas.setCursor(x - char_width / 2, y - char_height / 2);
            canvas.print(ID);
        }
    }

    // GNSS data indicators
    constexpr uint16_t bad_color = ST77XX_RED;
    constexpr uint16_t fair_color = ST77XX_YELLOW;
    constexpr uint16_t good_color = ST77XX_GREEN;

    // Number of GNSS satellites
    canvas.setCursor(0 + char_width, 0 + char_height / 2);
    if (gnss_fix->satellites == 0) {
        canvas.setTextColor(bad_color);
    } else if (gnss_fix->satellites < 4) {
        canvas.setTextColor(fair_color);
    } else {
        canvas.setTextColor(good_color);
    }
    canvas.print(gnss_fix->satellites);

    // Position fix
    canvas.setCursor(TFT_WIDTH - char_width * 3.5, 0 + char_height / 2);
    if (gnss_fix->quality == 0) {
        canvas.setTextColor(bad_color);
        canvas.print("Bad");
    } else if (gnss_fix->quality >= 1) {
        canvas.setTextColor(good_color);
        canvas.print("Fix");
    }

    // Batery voltage indicator
    constexpr float R7 = 0.998e6f;      // ohms
    constexpr float R8 = 1.502e6f;      // ohms
    constexpr float ADC_MAX = 1023.0f;  // ADC
    constexpr float VREF = 3.7f;        // VDD reference

    uint32_t adc_raw = analogRead(batteryVoltagePin);

    float Vout = (float)adc_raw * (VREF / ADC_MAX);
    float Vbat = Vout * ((R7 + R8) / R8);

    if (Vbat > 4.0) {
        canvas.setTextColor(good_color);
    } else if (Vbat > 3.7) {
        canvas.setTextColor(fair_color);
    } else {
        canvas.setTextColor(bad_color);
    }

    canvas.setCursor(center_x - char_width * 2, 0 + char_height / 2);
    canvas.print(Vbat);

    // Double buffer to avoid flicker
    tft.drawRGBBitmap(0, 0, canvas.getBuffer(), canvas.width(), canvas.height());

    analogWrite(backlightPWMPin, backlightBrightness);
}