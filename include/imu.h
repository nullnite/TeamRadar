#pragma once
#include <ICM_20948.h>

void initIMU();
ICM_20948_AGMT_t readIMU();
float computeHeading(float mx_raw, float my_raw, float mz_raw,
                     float ax, float ay, float az);
float getHeading();