#include "imu.h"

constexpr bool AD0_VAL = 0;

ICM_20948_I2C myICM;

void initIMU() {
    Wire.begin();
    Wire.setClock(400000);

    bool initialized = false;
    while (!initialized) {
        myICM.begin(Wire, AD0_VAL);

        Serial.print(F(" Initialization of IMU returned: "));
        Serial.println(myICM.statusString());
        if (myICM.status != ICM_20948_Stat_Ok) {
            Serial.println("Trying again...");
            delay(500);
        } else {
            initialized = true;
        }
    }

    /*
    bool success = true;
    success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
    success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0) == ICM_20948_Stat_Ok);
    success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
    success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
    success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

    if (success) {
        Serial.print(F("DMP enabled successfully."));
    } else {
        Serial.print(F("Enable DMP failed!"));
    }
    */
}

void readIMU() {
    if (myICM.dataReady()) {
        myICM.getAGMT();
    }

    /*
    icm_20948_DMP_data_t data;
    myICM.readDMPdataFromFIFO(&data);

    if ((myICM.status == ICM_20948_Stat_Ok) || (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail))  // Was valid data available?
    {
        if ((data.header & DMP_header_bitmap_Quat9) > 0)  // We have asked for orientation data so we should receive Quat9
        {
            // Q0 value is computed from this equation: Q0^2 + Q1^2 + Q2^2 + Q3^2 = 1.
            // In case of drift, the sum will not add to 1, therefore, quaternion data need to be corrected with right bias values.
            // The quaternion data is scaled by 2^30.

            // Scale to +/- 1
            double q1 = ((double)data.Quat9.Data.Q1) / 1073741824.0;  // x
            double q2 = ((double)data.Quat9.Data.Q2) / 1073741824.0;  // y
            double q3 = ((double)data.Quat9.Data.Q3) / 1073741824.0;  // z

            // Calculate w
            double q0 = sqrt(1.0 - ((q1 * q1) + (q2 * q2) + (q3 * q3)));

            // Compute yaw (heading)
            double yaw = atan2(2.0 * (q0 * q3 + q1 * q2),
                               1.0 - 2.0 * (q2 * q2 + q3 * q3));

            double yaw_deg = yaw * 180.0 / PI;

            if (yaw_deg < 0) yaw_deg += 360.0;

            Serial.print("Yaw (deg): ");
            Serial.println(yaw_deg, 2);

            //Serial.print(F("Q1:"));
            //Serial.print(q1, 3);
            //Serial.print(F(" Q2:"));
            //Serial.print(q2, 3);
            //Serial.print(F(" Q3:"));
            //Serial.print(q3, 3);
            //Serial.print(F(" Accuracy:"));
            //Serial.println(data.Quat9.Data.Accuracy);
        }
    }

    if (myICM.status != ICM_20948_Stat_FIFOMoreDataAvail)  // If more data is available then we should read it right away - and not delay
    {
        delay(10);
    }
    */
}

float computeHeading(float mx_raw, float my_raw, float mz_raw,
                     float ax, float ay, float az) {
    // Calibration coefficients from MotionCal
    // Hard iron offsets
    constexpr float offX = -16.90f;
    constexpr float offY = 8.35f;
    constexpr float offZ = 11.46f;
    // Soft iron matrix
    constexpr float M00 = 1.020f, M01 = -0.047f, M02 = -0.048f;
    constexpr float M10 = -0.047f, M11 = 1.078f, M12 = -0.032f;
    constexpr float M20 = -0.048f, M21 = -0.032f, M22 = 0.915f;

    // Corect hard iron
    float mx_corr = mx_raw - offX;
    float my_corr = my_raw - offY;
    float mz_corr = mz_raw - offZ;
    // Correct soft iron
    float mx = M00 * mx_corr + M01 * my_corr + M02 * mz_corr;
    float my = M10 * mx_corr + M11 * my_corr + M12 * mz_corr;
    float mz = M20 * mx_corr + M21 * my_corr + M22 * mz_corr;

    // Compute roll/pitch from accelerometer
    float normA = sqrtf(ax * ax + ay * ay + az * az);
    if (normA < 1e-6f) return NAN;
    float axn = ax / normA, ayn = ay / normA, azn = az / normA;

    float roll = atan2f(ayn, azn);
    float pitch = asinf(-axn);

    // Compensate for tilt
    float mxh = mx * cosf(pitch) + mz * sinf(pitch);
    float myh = mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);

    float heading_rad = atan2f(myh, mxh);
    float heading_deg = heading_rad * 180.0f / M_PI;

    heading_deg = fmodf(heading_deg + 360.0f, 360.0f);
    if (heading_deg < 0.0f) heading_deg += 360.0f;

    return heading_deg;
}

int getHeading() {
    // Magnetic declination for Kaunas, Lithuania
    // Formula: (deg + (min / 60.0)) / (180 / PI);
    constexpr int declinationAngle = (8.0 + (19.0 / 60.0)) / (180 / PI);

    readIMU();

    float heading = computeHeading(myICM.agmt.mag.axes.x, myICM.agmt.mag.axes.y, myICM.agmt.mag.axes.z,
                                   myICM.accX(), myICM.accY(), myICM.accZ()) +
                    declinationAngle;

    return heading;
}