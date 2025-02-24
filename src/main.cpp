#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <SD.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Arduino.h"
#include "esp_timer.h"
#include "index.h"
#include "measurement.h"
#include "sd_handler.h"
#include "time.h"
#include "websocket_handler.h"

unsigned long lastTime = 0;
unsigned long interval = 100;

// Task-Handle für Core 0
TaskHandle_t wsTaskHandle = NULL;

// Task-Funktion für WebSocket-Daten (Core 0)
void wsTask(void *pvParam) {
    while (1) {
        if (isMeasuring && millis() - lastTime >= interval) {
            sendSensorData();
            lastTime = millis();
        }
        ws.cleanupClients(); 

        vTaskDelay(pdMS_TO_TICKS(10)); // Kurze Verzögerung
    }
}

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
    Serial.print("Reset reason: ");
    Serial.println((int)reason);
    initMTi();
    xTaskCreatePinnedToCore(
        wsTask,        // Task-Funktion
        "WebSocket",   // Name
        4096,          // Stack-Größe
        NULL,          // Parameter
        1,             // Priorität (niedriger als Sensor-Tasks)
        &wsTaskHandle, // Task-Handle
        0              // Core 0
    );
}

void loop() {
    if (digitalRead(MyMTi->drdy)) {
        MyMTi->readMessages();
        if (isMeasuring) {
            logMeasurementData();

            /*if (millis() - lastTime >= interval) {
                sendSensorData();

                lastTime = millis();
            }*/
        }
    }
    handleButtonPress();
    //ws.cleanupClients();
}