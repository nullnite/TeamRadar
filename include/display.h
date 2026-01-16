#pragma once
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <SPI.h>

#include "gps.h"

void initDisplay();
int roundUp(int numToRound, int multiple);
void drawCompass(int heading, int bearings[], int distances[], gnss_data* gnss_fix);