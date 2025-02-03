#ifndef MS5611_H
#define MS5611_H

#include <Arduino.h>

class MS5611 {
public:
  MS5611(uint8_t address);
  bool begin();
  float calculateTemperature(uint32_t D2);
  float calculatePressure(uint32_t D1, uint32_t D2);
  uint32_t readRawData(uint8_t cmd);
  String getPromValues();

private:
  uint8_t _address;
  uint16_t prom[6];

  void resetSensor();
  bool readCalibrationData();
  uint16_t readPromValue(uint8_t reg);
};

#endif
