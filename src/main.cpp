#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "time.h"
#include "index.h"
#include "websocket_handler.h"
#include "sd_handler.h"
#include "measurement.h"

unsigned long lastTime = 0;
unsigned long interval = 10;

void setup()
{
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

void loop()
{
    handleButtonPress();

    if (isMeasuring && digitalRead(MyMTi->drdy))
    {
        MyMTi->readMessages();

        if (!isnan(MyMTi->getAcceleration()[0]))
        {
            if (millis() - lastTime >= interval)
            {
                measurementCounter++;
                logMeasurementData();
                sendSensorData();

                lastTime = millis();
            }
        }
    }
    ws.cleanupClients();
    // delay(10);
}