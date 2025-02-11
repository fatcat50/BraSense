#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>

#include "MTi.h"
#include "SD.h"

extern bool isMeasuring;
extern uint32_t measurementCounter;
extern uint16_t recordCounter;
extern uint16_t fileCounter;
extern String currentFileName;
extern MTi *MyMTi;

void print();
void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();
void initMeasurement();
void handleButtonPress();

#endif  // MEASUREMENT_H