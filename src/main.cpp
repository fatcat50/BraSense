#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_system.h>

#include "Arduino.h"
#include "index.h"
#include "measurement.h"
#include "sd_handler.h"
#include "time.h"
#include "websocket_handler.h"

unsigned long lastTime = 0;
unsigned long interval = 50;

void setup() {
    Serial.begin(115200);
    esp_reset_reason_t reason = esp_reset_reason();
    
    EEPROM.begin(128);
    Wire.begin();
    Wire.setClock(400000UL);
    initWiFi();
    initTime();
    initMTi();
    initMeasurement();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
    setupWebSocket();
    Serial.print("Reset reason: ");
    Serial.println((int)reason);
}

void loop() {
    handleButtonPress();

    if (isMeasuring && digitalRead(MyMTi->drdy)) {
        MyMTi->readMessages();

        if (!isnan(MyMTi->getAcceleration()[0])) {
            if (millis() - lastTime >= interval) {
                logMeasurementData();
                sendSensorData();

                lastTime = millis();
            }
        }
    }
    ws.cleanupClients();
}