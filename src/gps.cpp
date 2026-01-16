#include "gps.h"

volatile bool PPS_FLAG = false;
constexpr int GPS_MESSAGE_BUFFER_MAX_LENGTH = 100;
uint8_t gps_message_buffer[GPS_MESSAGE_BUFFER_MAX_LENGTH];

void PPS_Interrupt() {
    PPS_FLAG = true;
}

void send_UBX_frame(uint8_t* message, size_t len) {
    // Frame sync chars
    Serial2.write((uint8_t)0xB5);
    Serial2.write((uint8_t)0x62);

    // Message and payload
    uint8_t CK_A = 0, CK_B = 0;
    for (size_t i = 0; i < len; i++) {
        Serial2.write((uint8_t)message[i]);
        CK_A += message[i];
        CK_B += CK_A;
    }

    // Checksum
    Serial2.write((uint8_t)CK_A);
    Serial2.write((uint8_t)CK_B);
}

void send_UBX_CFG_MSG(uint8_t NMEA_message_type, uint8_t send_rate) {
    // Set message rate
    uint8_t message[] = {
        0x06,  // UBX message class - UBX_CFG
        0x01,  // UBX message ID - MSG
        0x03,  // 2 byte length
        0x00,  // of UBX message payload
        0xF0,  // Message class - NMEA
        0x00,  // Message identifier - ?
        0x00   // Send rate on current port - per second
    };

    message[5] = NMEA_message_type;
    message[6] = send_rate;

    send_UBX_frame(message, sizeof(message));
}

void initGPS() {
    Serial2.begin(9600);

    // Disable messages via UBX protocol
    send_UBX_CFG_MSG(1, 0);  // GLL
    send_UBX_CFG_MSG(2, 0);  // GSA
    send_UBX_CFG_MSG(3, 0);  // GSV
    send_UBX_CFG_MSG(4, 0);  // RMC
    send_UBX_CFG_MSG(5, 0);  // VTG

    pinMode(PIN_NFC1, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_NFC1), PPS_Interrupt, RISING);
}

bool readNMEA() {
    if (Serial2.available() == 0) {
        return false;
    }

    // Grab full line from serial
    int bytesRead = Serial2.readBytesUntil('\n', gps_message_buffer, GPS_MESSAGE_BUFFER_MAX_LENGTH - 2);

    // Fix newline and null terminate
    gps_message_buffer[bytesRead] = '\n';
    gps_message_buffer[bytesRead + 1] = '\0';
    return true;
}

bool parseNMEA(uint8_t* message, size_t message_length, gnss_data* gnss_data_out) {
    char* token = strtok((char*)message, "\r\n");
    while (token != NULL) {
        char time_string[16] = {0};
        char latitude_string[16] = {0};
        char longitude_string[16] = {0};

        char latitude_dir = 0, longitude_dir = 0;
        int quality = 0, satellites = 0;

        int variables_read = sscanf(token, "$GNGGA,%15[^,],%15[^,],%c,%15[^,],%c,%d,%d",
                                    time_string, latitude_string, &latitude_dir, longitude_string, &longitude_dir, &quality, &satellites);

        double nmea_time = atof(time_string);
        double latitude = atof(latitude_string);
        double longitude = atof(longitude_string);

        if (variables_read > 0) {
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

void getGNSSData(gnss_data* gnss_fix) {
    // if (PPS_FLAG) {
    if (true) {
        bool sentence_available = readNMEA();
        while (sentence_available) {
            bool sentence_correct = parseNMEA(gps_message_buffer,
                                              GPS_MESSAGE_BUFFER_MAX_LENGTH,
                                              gnss_fix);

            if (sentence_correct) return;

            sentence_available = readNMEA();
        }
    }
}

float calculateBearing(coords start, coords end) {
    double latitudeStart = start.latitude_dec * DEG_TO_RAD;
    double latitudeEnd = end.latitude_dec * DEG_TO_RAD;
    double longitudeStart = start.longitude_dec * DEG_TO_RAD;
    double longitudeEnd = end.longitude_dec * DEG_TO_RAD;

    // Apply haversine (for great circle bearing) formula
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

float calculateDistance(coords start, coords end) {
    const double R = 6371000.0;  // Earth radius in metres

    double latitudeStart = start.latitude_dec * DEG_TO_RAD;
    double latitudeEnd = end.latitude_dec * DEG_TO_RAD;
    double longitudeStart = start.longitude_dec * DEG_TO_RAD;
    double longitudeEnd = end.longitude_dec * DEG_TO_RAD;

    double deltaLatitude = latitudeEnd - latitudeStart;
    double deltaLongitude = longitudeEnd - longitudeStart;

    // Apply haversine (for great circle distances) formula
    double a = sin(deltaLatitude / 2) * sin(deltaLatitude / 2) +
               cos(latitudeStart) * cos(latitudeEnd) *
                   sin(deltaLongitude / 2) * sin(deltaLongitude / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    double distance = R * c;  // distance in metres

    return distance;
}
