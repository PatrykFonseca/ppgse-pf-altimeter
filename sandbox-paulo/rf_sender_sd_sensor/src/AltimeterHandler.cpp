#include "AltimeterHandler.h"
#include <Arduino.h>
#include "Constants.h"

// Constants for height calculation (Earth's atmosphere assumptions)
// const float P0 = 1013.25; // Sea-level standard atmospheric pressure in mbar
// const float T0 = 288.15;  // Standard temperature at sea level in Kelvin
// const float g = 9.80665;  // Acceleration due to gravity (m/s^2)
// const float R = 287.05;   // Specific gas constant for dry air (J/(kg·K))

void setupAltimeter(MS5611 &ms5611, float &baselinePressure, float &baselineHeight)
{
  Serial.println("Calibrating altimeter...");
  unsigned long startTime = millis();
  float pressureSum = 0.0;
  float heightSum = 0.0;
  int count = 0;

  while (millis() - startTime < 2000)
  { // Read for 2 seconds
    uint32_t rawPressure = ms5611.readRawData(0x40);
    uint32_t rawTemperature = ms5611.readRawData(0x50);

    // Calculate compensated pressure
    float pressure = ms5611.calculatePressure(rawPressure, rawTemperature);
    pressureSum += pressure;

    // Calculate height from pressure using barometric formula
    float height = (T0 / g) * R * log(P0 / pressure);
    heightSum += height;

    count++;
    delay(10); // Adjust delay as needed to avoid spamming the bus
  }

  baselinePressure = pressureSum / count;
  baselineHeight = heightSum / count;

  Serial.print("Baseline Pressure: ");
  Serial.print(baselinePressure);
  Serial.println(" mbar");

  Serial.print("Baseline Height: ");
  Serial.print(baselineHeight);
  Serial.println(" m");
}
