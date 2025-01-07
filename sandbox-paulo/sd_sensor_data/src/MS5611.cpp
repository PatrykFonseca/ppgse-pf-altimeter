
#include "MS5611.h"
#include <Wire.h>

// Commands for MS5611
#define CMD_RESET 0x1E
#define CMD_READ_PROM 0xA0
#define CMD_CONVERT_D1 0x40
#define CMD_CONVERT_D2 0x50
#define CMD_READ_ADC 0x00

MS5611::MS5611(uint8_t address) {
  _address = address;
}

bool MS5611::begin() {
  Wire.begin();
  resetSensor();
  delay(10);
  return readCalibrationData();
}

void MS5611::resetSensor() {
  Wire.beginTransmission(_address);
  Wire.write(CMD_RESET);
  Wire.endTransmission();
  delay(3); // Wait for reset to complete
}

bool MS5611::readCalibrationData() {
  for (uint8_t i = 0; i < 6; i++) {
    prom[i] = readPromValue(i + 1);
    if (prom[i] == 0) {
      return false; // Failed to read calibration data
    }
  }
  return true;
}

uint16_t MS5611::readPromValue(uint8_t reg) {
  Wire.beginTransmission(_address);
  Wire.write(CMD_READ_PROM + (reg * 2));
  Wire.endTransmission();
  Wire.requestFrom(_address, (uint8_t)2);

  if (Wire.available() < 2) {
    return 0; // Error
  }

  uint16_t value = (Wire.read() << 8) | Wire.read();
  return value;
}

uint32_t MS5611::readRawData(uint8_t cmd) {
  Wire.beginTransmission(_address);
  Wire.write(cmd);
  Wire.endTransmission();

  // Wait for conversion (depends on resolution; adjust delay if necessary)
  delay(10);

  Wire.beginTransmission(_address);
  Wire.write(CMD_READ_ADC);
  Wire.endTransmission();
  Wire.requestFrom(_address, (uint8_t)3);

  if (Wire.available() < 3) {
    return 0; // Error
  }

  uint32_t value = (Wire.read() << 16) | (Wire.read() << 8) | Wire.read();
  return value;
}

float MS5611::calculateTemperature(uint32_t D2) {
  int32_t dT = D2 - (prom[4] * 256);
  float temperature = 2000 + ((dT * prom[5]) / 8388608.0);
  temperature = temperature / 100.0;

  // Serial.print("Temperature: ");
  // Serial.print(temperature);
  // Serial.println(" °C");

  return temperature; // Convert to Celsius
}

float MS5611::calculatePressure(uint32_t rawPressure, uint32_t rawTemperature) {
  int32_t dT = rawTemperature - (prom[4] * 256);
  int64_t OFF = (int64_t)prom[1] * 65536 + ((int64_t)dT * prom[3]) / 128;
  int64_t SENS = (int64_t)prom[0] * 32768 + ((int64_t)dT * prom[2]) / 256;
  int64_t P = ((rawPressure * SENS / 2097152) - OFF) / 32768;
  return P / 100.0; // Convert to mbar
}
