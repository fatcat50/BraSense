#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>

#include "MTi.h"
#include "SD.h"
#define ARR_SIZE 100

extern bool isMeasuring;
extern uint32_t measurementCounter;
extern uint16_t recordCounter;
extern uint16_t fileCounter;
extern String currentFileName;
extern MTi *MyMTi;
extern QueueHandle_t sdQueue;
extern bool bufferFull;
extern float currentX, currentY, currentZ;

typedef struct {
    float time;
    float x1;
    float y1;
    float z1;
} datapoint;

extern datapoint buffer[ARR_SIZE];

void print();
void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();
void initMeasurement();
void handleButtonPress();

#endif  // MEASUREMENT_H