#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>
#include <SPI.h>
#include "MTi.h"
#include "SD.h"

#define ARR_SIZE 1000

// ── SPI Pins (Arduino Nano ESP32 / S3) ───────────────────────────────────────
// WARNING: GPIO48 = SCK = LED_BUILTIN → LED cannot be used
#define PIN_MOSI    11   // COPI (~D11)
#define PIN_MISO    12   // CIPO (~D12)
#define PIN_SCK     13   // SCK  (~D13)

// PIN_CS_SD
#define PIN_CS_SD   10

// MTi Sensor 1
#define MTI1_CS      9   // (~D2 left side)
#define MTI1_DRDY    4   // (~D3)

// MTi Sensor 2 (optional, Auto-Detect)
#define MTI2_CS      5
#define MTI2_DRDY    6

// Button → GPIO0 = B1, has internal pull-up
#define BUTTON_PIN   8
#define SYNC_PIN     17

// ── External Variables ────────────────────────────────────────────────────────
extern bool isMeasuring;
extern uint32_t measurementCounter;
extern uint16_t recordCounter;
extern uint16_t fileCounter;
extern String currentFileName;

extern MTi *MyMTi1;
extern MTi *MyMTi2;   // NULL if Sensor 2 is not found

extern QueueHandle_t sdQueue;
extern SemaphoreHandle_t spiMutex;  // Protects the shared SPI bus

extern float currentX1, currentY1, currentZ1;
extern float currentX2, currentY2, currentZ2;

// ── Data point: both sensors ──────────────────────────────────────────────────
typedef struct {
    float time;
    float x1, y1, z1;   // Sensor 1
    float x2, y2, z2;   // Sensor 2 (0.0 if not present)
} datapoint;

extern datapoint buffer[ARR_SIZE];

// ── Functions ─────────────────────────────────────────────────────────────────
void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();
void initMeasurement();
void handleButtonPress();

#endif  // MEASUREMENT_H