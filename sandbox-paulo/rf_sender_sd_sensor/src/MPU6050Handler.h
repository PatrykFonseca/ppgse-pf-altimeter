#ifndef MPU6050_HANDLER_H
#define MPU6050_HANDLER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// Function to initialize the MPU6050
bool initializeMPU6050(Adafruit_MPU6050 &mpu);

// Function to configure the MPU6050
void configureMPU6050(Adafruit_MPU6050 &mpu);

#endif // MPU6050_HANDLER_H
