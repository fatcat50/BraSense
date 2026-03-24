#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_system.h>

#include "Arduino.h"
#include "index.h"
#include "measurement.h"
#include "sd_handler.h"
#include "tasks.h"
#include "time.h"
#include "websocket_handler.h"

void setup() {
    Serial.begin(115200);

    // 1. SPI bus + sensors (SPI.begin() happens here)
    initMTi();

    // 2. SD card (uses the same SPI bus)
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();

    // 3. WiFi + time
    initWiFi();
    initTime();

    // 4. WebSocket server
    setupWebSocket();

    // 5. Configure button pin
    initMeasurement();

    // 6. Start FreeRTOS tasks (sensorTask takes over measurement from here)
    createTasks();
}

void loop() {
    // Sensor measurement runs in sensorTask (Core 1) -> only button here
    handleButtonPress();
    vTaskDelay(pdMS_TO_TICKS(10));  // Free up Core 1 for sensorTask
}