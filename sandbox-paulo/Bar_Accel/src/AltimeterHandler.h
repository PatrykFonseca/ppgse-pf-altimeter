#ifndef ALTIMETER_HANDLER_H
#define ALTIMETER_HANDLER_H

#include "MS5611.h"

// Function to calculate and store the baseline pressure and height for the MS5611
void setupAltimeter(MS5611 &ms5611, float &baselinePressure, float &baselineHeight);

#endif
