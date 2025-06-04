#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
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
    esp_reset_reason_t reason = esp_reset_reason();

    initWiFi();
    initTime();
    initMeasurement();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
    setupWebSocket();
    createTasks();
    initMTi();

    // Serial.print("Reset reason: ");
    // Serial.println((int)reason);
}

void loop() {
    if (digitalRead(MyMTi->drdy)) {
        MyMTi->readMessages();
        if (isMeasuring) {
            logMeasurementData();
        }
    }
    handleButtonPress();
}