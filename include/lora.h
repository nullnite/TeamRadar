#pragma once
#include <SPI.h>
#include <SX126x-RAK4630.h>

#include "gps.h"

typedef struct {
    uint8_t ID = 1;
    coords location;
} lora_packet;

void initLora();
void getTeamLocations(coords* locations);
void sendLocation(coords location);