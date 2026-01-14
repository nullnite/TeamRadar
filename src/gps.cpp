#include "gps.h"

void initGPS() {
    Serial1.begin(9600);
}

bool parseNMEA(uint8_t* message, size_t message_length, gnss_data* gnss_data_out) {
    char* token = strtok((char*)message, "\r\n");
    while (token != NULL) {
        double nmea_time, latitude, longitude;
        char latitude_dir, longitude_dir;
        int quality, satellites;

        int variables_read = sscanf(token, "$GPGGA,%lf,%lf,%c,%lf,%c,%d,%d,%*s",
                                    &nmea_time, &latitude, &latitude_dir, &longitude, &longitude_dir, &quality, &satellites);

        if (variables_read == 7) {
            // Convert NMEA time to seconds since epoch (and microseconds)
            // From hhmmss.ss to ss.ssssss
            struct tm time = {0};
            time.tm_hour = (int)(nmea_time / 10000);
            time.tm_min = (int)(nmea_time / 100) % 100;
            time.tm_sec = (int)nmea_time % 100;

            setenv("TZ", "UTC0", 1);
            gnss_data_out->utc_time.tv_sec = mktime(&time);

            double microseconds = modf(nmea_time, NULL);
            gnss_data_out->utc_time.tv_usec = (suseconds_t)(microseconds * 1000000);

            // Convert latitude to decimal degrees
            // From DDmm.mm to DD.DDDDD
            double degrees, minutes;
            minutes = modf(latitude / 100.0, &degrees) * 100.0;  // DD.mmmm
            gnss_data_out->coordinates.latitude_dec = degrees + minutes / 60.0;

            // In southern hemisphere latitude is negative
            if (latitude_dir == 'S') {
                gnss_data_out->coordinates.latitude_dec *= -1.0f;
            }

            // Convert longitude to decimal degrees
            // From DDDmm.mm to DDD.DDDD
            minutes = modf(longitude / 100.0, &degrees) * 100.0;  // DDD.mmmm
            gnss_data_out->coordinates.longitude_dec = degrees + minutes / 60.0;

            // In western hemisphere longitude is negative
            if (longitude_dir == 'W') {
                gnss_data_out->coordinates.longitude_dec *= -1.0f;
            }

            gnss_data_out->quality = quality;

            gnss_data_out->satellites = satellites;

            return true;
        }
        token = strtok(NULL, "\r\n");
    }

    return false;
}

coords getLocation() {
}

float calculateBearing(coords start, coords end) {
    double latitudeStart = start.latitude_dec * DEG_TO_RAD;
    double latitudeEnd = end.latitude_dec * DEG_TO_RAD;
    double longitudeStart = start.longitude_dec * DEG_TO_RAD;
    double longitudeEnd = end.longitude_dec * DEG_TO_RAD;

    double deltaLongitude = longitudeEnd - longitudeStart;

    double y = sin(deltaLongitude) * cos(latitudeEnd);
    double x = cos(latitudeStart) * sin(latitudeEnd) -
               sin(latitudeStart) * cos(latitudeEnd) * cos(deltaLongitude);

    double bearing = atan2(y, x) * RAD_TO_DEG;
    if (bearing < 0) {
        bearing += 360;
    }

    return bearing;
}
