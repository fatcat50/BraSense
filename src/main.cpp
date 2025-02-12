#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <Wire.h>

#include "Arduino.h"
#include "index.h"
#include "measurement.h"
#include "sd_handler.h"
#include "time.h"
#include "websocket_handler.h"

unsigned long lastTime = 0;
unsigned long interval = 10;

void setup() {
    Serial.begin(115200);

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