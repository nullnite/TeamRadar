#pragma once
#include <Arduino.h>
#include <time.h>

constexpr int teamSize = 2;
constexpr int GPS_MESSAGE_BUFFER_MAX_LENGTH = 100;

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
void getGNSSData(gnss_data* gnss_fix);
float calculateBearing(coords start, coords end);