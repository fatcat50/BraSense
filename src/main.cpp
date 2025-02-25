#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <queue.h>

#include "Arduino.h"
#include "esp_timer.h"
#include "index.h"
#include "measurement.h"
#include "sd_handler.h"
#include "time.h"
#include "websocket_handler.h"
#include "tasks.h"

void setup() {
    Serial.begin(115200);
    esp_reset_reason_t reason = esp_reset_reason();

    EEPROM.begin(128);
    Wire.begin();
    Wire.setClock(400000);
    delay(500);  // Delay 0.5sec to allow I2C bus to stabilize

    initWiFi();
    initTime();
    initMeasurement();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
    setupWebSocket();
    createTasks();
    initMTi();

    //Serial.print("Reset reason: ");
    //Serial.println((int)reason);
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