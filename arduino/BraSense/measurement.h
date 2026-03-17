#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <Arduino.h>
#include <SPI.h>
#include "MTi.h"
#include "SD.h"

#define ARR_SIZE 1000

// ── SPI-Pins (Arduino Nano ESP32 / S3) ───────────────────────────────────────
// ACHTUNG: GPIO48 = SCK = LED_BUILTIN → LED nicht nutzbar
#define PIN_MOSI    38   // COPI (~D11)
#define PIN_MISO    47   // CIPO (~D12)
#define PIN_SCK     48   // SCK  (~D13)

// ⚠️ PIN_CS_SD: Trag hier deinen echten SD-CS-Pin ein!
#define PIN_CS_SD   21

// MTi Sensor 1
#define MTI1_CS      5   // (~D2 linke Seite)
#define MTI1_DRDY    6   // (~D3)

// MTi Sensor 2 (optional, Auto-Detect)
#define MTI2_CS      7
#define MTI2_DRDY    8

// Button → GPIO0 = B1, hat internen Pullup
#define BUTTON_PIN   0

// ── Externe Variablen ─────────────────────────────────────────────────────────
extern bool isMeasuring;
extern uint32_t measurementCounter;
extern uint16_t recordCounter;
extern uint16_t fileCounter;
extern String currentFileName;

extern MTi *MyMTi1;
extern MTi *MyMTi2;   // NULL wenn Sensor 2 nicht gefunden

extern QueueHandle_t sdQueue;
extern SemaphoreHandle_t spiMutex;  // Schutz des gemeinsamen SPI-Bus

extern float currentX1, currentY1, currentZ1;
extern float currentX2, currentY2, currentZ2;

// ── Datenpunkt: beide Sensoren ────────────────────────────────────────────────
typedef struct {
    float time;
    float x1, y1, z1;   // Sensor 1
    float x2, y2, z2;   // Sensor 2 (0.0 wenn nicht vorhanden)
} datapoint;

extern datapoint buffer[ARR_SIZE];

// ── Funktionen ────────────────────────────────────────────────────────────────
void initMTi();
void startMeasurement();
void stopMeasurement();
void logMeasurementData();
void initMeasurement();
void handleButtonPress();

#endif  // MEASUREMENT_H
