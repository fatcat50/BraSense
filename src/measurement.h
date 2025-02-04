#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>
#include "MTi.h"

extern bool isMeasuring;
extern uint16_t recordCounter;
extern MTi *MyMTi;

void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();

#endif // MEASUREMENT_H