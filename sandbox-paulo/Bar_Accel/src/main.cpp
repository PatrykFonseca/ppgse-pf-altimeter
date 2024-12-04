#include <Arduino.h>
#include <M5Stack.h>
#include <Adafruit_MPU6050.h>
#include "MS5611.h"
#include "MPU6050Handler.h"
#include "AltimeterHandler.h"
#include "AccelerometerHandler.h"
#include "Constants.h"

MS5611 ms5611(0x77);
Adafruit_MPU6050 mpu;

float baselinePressure = 0.0, baselineHeight = 0.0;
float baselineX = 0.0, baselineY = 0.0, baselineZ = 0.0;

void setup() {
  M5.begin();
  Serial.begin(115200);

  // Initialize Altimeter
  if (!ms5611.begin()) {
    Serial.println("MS5611 initialization failed!");
    while (1);
  }
  Serial.println("Setting up altimeter");
  setupAltimeter(ms5611, baselinePressure, baselineHeight);

  // Initialize Accelerometer
  if (!initializeMPU6050(mpu)) {
    while (1);
  }
  configureMPU6050(mpu);
  Serial.println("Setting up accelerometer");
  setupAccelerometer(mpu, baselineX, baselineY, baselineZ);
}


void loop() {
  // Read raw data from the MS5611 sensor
  uint32_t rawPressure = ms5611.readRawData(0x40); // Read raw pressure
  uint32_t rawTemperature = ms5611.readRawData(0x50); // Read raw temperature

  // Calculate temperature and absolute pressure
  ms5611.calculateTemperature(rawTemperature);
  float pressure = ms5611.calculatePressure(rawPressure, rawTemperature);

  // Calculate relative pressure (difference from baseline)
  float relativePressure = pressure - baselinePressure;

  // Calculate absolute and relative heights
  float absoluteHeight = (T0 / g) * R * log(P0 / pressure); // Absolute height
  float relativeHeight = absoluteHeight - baselineHeight;   // Relative height

  Serial.print("Absolute Pressure: ");
  Serial.print(pressure);
  Serial.println(" mbar");

  Serial.print("Relative Pressure: ");
  Serial.print(relativePressure);
  Serial.println(" mbar");

  Serial.print("Absolute Height: ");
  Serial.print(absoluteHeight);
  Serial.println(" m");

  Serial.print("Relative Height: ");
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

