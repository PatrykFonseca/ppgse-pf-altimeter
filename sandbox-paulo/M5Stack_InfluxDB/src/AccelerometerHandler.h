#ifndef ACCELEROMETER_HANDLER_H
#define ACCELEROMETER_HANDLER_H

#include <Adafruit_MPU6050.h>

// Function to calculate and store the baseline for the MPU6050
void setupAccelerometer(Adafruit_MPU6050 &mpu, float &baselineX, float &baselineY, float &baselineZ);

#endif
