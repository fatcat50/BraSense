#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>

#include "MTi.h"
#include "SD.h"
#define ARR_SIZE 1500

extern bool isMeasuring;
extern uint32_t measurementCounter;
extern uint16_t recordCounter;
extern uint16_t fileCounter;
extern String currentFileName;
extern MTi *MyMTi;

typedef struct {
    uint32_t time;
    uint32_t x1;
    uint32_t y1;
    uint32_t z1;
} datapoint;

extern datapoint logs[ARR_SIZE];

void print();
void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();
void initMeasurement();
void handleButtonPress();

#endif  // MEASUREMENT_H