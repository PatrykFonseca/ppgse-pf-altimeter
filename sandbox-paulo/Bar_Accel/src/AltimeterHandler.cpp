#include "AltimeterHandler.h"
#include <Arduino.h>

void setupAltimeter(MS5611 &ms5611, float &baselinePressure, float &baselineTemperature) {
  Serial.println("Calibrating altimeter...");
  unsigned long startTime = millis();
  float pressureSum = 0.0;
  float temperatureSum = 0.0;
  int count = 0;

  while (millis() - startTime < 2000) { // Read for 2 seconds
    uint32_t rawPressure = ms5611.readRawData(0x40);
    uint32_t rawTemperature = ms5611.readRawData(0x50);
    pressureSum += ms5611.calculatePressure(rawPressure, rawTemperature);
    temperatureSum += ms5611.calculateTemperature(rawTemperature);
    count++;
    delay(10); // Adjust delay as needed to avoid spamming the bus
  }

  baselinePressure = pressureSum / count;
  baselineTemperature = temperatureSum / count;
  Serial.print("Baseline Pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" mbar");
  Serial.print("Baseline Temperature: ");
  Serial.print(baselineTemperature);
  Serial.println(" °C");
}
