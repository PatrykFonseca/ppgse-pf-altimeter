#include <Arduino.h>
#include <M5Stack.h>
#include <Adafruit_MPU6050.h>
#include "MS5611.h"
#include "MPU6050Handler.h"
#include "AltimeterHandler.h"
#include "AccelerometerHandler.h"

MS5611 ms5611(0x77);
Adafruit_MPU6050 mpu;

float baselinePressure = 0.0, baselineTemperature = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

void setup() {
  M5.begin();
  Serial.begin(115200);

  // Initialize Altimeter
  if (!ms5611.begin()) {
    Serial.println("MS5611 initialization failed!");
    while (1);
  }
  setupAltimeter(ms5611, baselinePressure, baselineTemperature);

  // Initialize Accelerometer
  if (!initializeMPU6050(mpu)) {
    while (1);
  }
  configureMPU6050(mpu);
  setupAccelerometer(mpu, baselineX, baselineY, baselineZ);
}


void loop() {
  uint32_t rawPressure = ms5611.readRawData(0x40); // Read raw pressure
  uint32_t rawTemperature = ms5611.readRawData(0x50); // Read raw temperature

  float temperature = ms5611.calculateTemperature(rawTemperature);
  float pressure = ms5611.calculatePressure(rawPressure, rawTemperature);

  float relativePressure = pressure - baselinePressure;
  float relativeHeight = (baselinePressure - pressure) * 8.43; // Approximate conversion factor

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Pressure: ");
  Serial.print(relativePressure);
  Serial.println(" mbar");

  Serial.print("Height: ");
  Serial.print(relativeHeight);
  Serial.println(" m");
  
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float accelX = a.acceleration.x - baselineX;
  float accelY = a.acceleration.y - baselineY;
  float accelZ = a.acceleration.z - baselineZ;

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(accelX);
  Serial.print(", Y: ");
  Serial.print(accelY);
  Serial.print(", Z: ");
  Serial.print(accelZ);
  Serial.println(" m/s^2");

  Serial.println("");

  delay(500);
}
