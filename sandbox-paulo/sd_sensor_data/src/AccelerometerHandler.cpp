#include "AccelerometerHandler.h"
#include <Arduino.h>

void setupAccelerometer(Adafruit_MPU6050 &mpu, float &baselineX, float &baselineY, float &baselineZ) {
  Serial.println("Calibrating accelerometer...");
  unsigned long startTime = millis();
  float accelXSum = 0.0, accelYSum = 0.0, accelZSum = 0.0;
  int count = 0;

  while (millis() - startTime < 2000) { // Read for 2 seconds
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    accelXSum += a.acceleration.x;
    accelYSum += a.acceleration.y;
    accelZSum += a.acceleration.z;
    count++;
    delay(10); // Adjust delay as needed to match sensor's read rate
  }

  baselineX = accelXSum / count;
  baselineY = accelYSum / count;
  baselineZ = accelZSum / count;

  Serial.print("Baseline Acceleration X: ");
  Serial.print(baselineX);
  Serial.println(" m/s^2");
  Serial.print("Baseline Acceleration Y: ");
  Serial.print(baselineY);
  Serial.println(" m/s^2");
  Serial.print("Baseline Acceleration Z: ");
  Serial.print(baselineZ);
  Serial.println(" m/s^2");
}
