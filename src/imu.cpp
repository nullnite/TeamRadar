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

ICM_20948_AGMT_t readIMU() {
    if (myICM.dataReady()) {
        return myICM.getAGMT();
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

            double yaw_deg = yaw * RAD_TO_DEG;

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
                     float ax_raw, float ay_raw, float az_raw) {
    // Magnetometer calibration coefficients from MotionCal
    constexpr float hard_iron[3] =
        {-17.12, -5.76, 11.15};
    constexpr float soft_iron[3][3] = {
        {1.006, 0.000, 0.009},
        {0.000, 0.998, -0.001},
        {0.009, -0.001, 0.996}};

    float mx_hi = mx_raw - hard_iron[0];
    float my_hi = my_raw - hard_iron[1];
    float mz_hi = mz_raw - hard_iron[2];

    float mx_cal = soft_iron[0][0] * mx_hi + soft_iron[0][1] * my_hi + soft_iron[0][2] * mz_hi;
    float my_cal = soft_iron[1][0] * mx_hi + soft_iron[1][1] * my_hi + soft_iron[1][2] * mz_hi;
    float mz_cal = soft_iron[2][0] * mx_hi + soft_iron[2][1] * my_hi + soft_iron[2][2] * mz_hi;

    // Remap axis to common reference frame
    // Magnetometer
    // +X back -> forward = -X
    // +Y left -> left    = +Y
    // +Z down -> up      = -Z
    float mx = -mx_cal;
    float my = my_cal;
    float mz = -mz_cal;

    // Accelerometer
    // +X back  -> forward = -X
    // +Y right -> left    = -Y
    // +Z up    -> up      = +Z
    float ax = -ax_raw;
    float ay = -ay_raw;
    float az = az_raw;
    Serial.printf("%f %f %f\n", ax, ay, az);

    // Compute roll/pitch from accelerometer
    float normA = sqrtf(ax * ax + ay * ay + az * az);
    if (normA < 1e-6f) return NAN;
    ax /= normA;
    ay /= normA;
    az /= normA;

    // Serial.print("Roll:");
    float roll = atan2f(ay, az);
    // Serial.println(roll);
    // Serial.print("Pitch:");
    float pitch = atan2f(-ax, sqrtf(ay * ay + az * az));
    // Serial.println(pitch);

    // Compensate for tilt
    float mxh = mx * cosf(pitch) + mz * sinf(pitch);
    float myh = mx * sinf(roll) * sinf(pitch) + my * cosf(roll) - mz * sinf(roll) * cosf(pitch);

    float heading = atan2f(myh, mxh) * RAD_TO_DEG;

    if (heading < 0) heading += 360;
    if (heading >= 360) heading -= 360;

    return heading;
}

float getHeading() {
    ICM_20948_AGMT_t agmt = readIMU();

    ICM_20948_axis3named_t acc = agmt.acc;
    ICM_20948_axis3named_t mag = agmt.mag;

    float heading = computeHeading(mag.axes.x * 0.1, mag.axes.y * 0.1, mag.axes.z * 0.1,
                                   acc.axes.x, acc.axes.y, acc.axes.z);

    // Magnetic declination for Kaunas, Lithuania
    constexpr int magneticDeclination = 8.0 + (20.0 / 60.0);

    return heading + magneticDeclination;
}