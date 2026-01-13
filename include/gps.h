#pragma once
#include <Arduino.h>
#include <time.h>

typedef struct {
    float latitude_dec;
    float longitude_dec;
} coords;

typedef struct {
    struct timeval utc_time;
    coords coordinates;
    int quality;
    int satellites;
} gnss_data;

void initGPS();
bool parseNMEA(uint8_t* message, size_t message_length, gnss_data* gnss_data_out);
bool getCoordinates();