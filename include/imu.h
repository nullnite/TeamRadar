#include <ICM_20948.h>

void initIMU();
void readIMU();
float computeHeading(float mx_raw, float my_raw, float mz_raw,
                     float ax, float ay, float az);
int getHeading();