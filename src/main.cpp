#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "index.h"
#include "websocket_handler.h"
#include "sd_handler.h"
#include "measurement.h"

void setup()
{
    Serial.begin(115200);

    EEPROM.begin(128);
    Wire.begin();
    initMTi();
    initMeasurement();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
    initWiFi();
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
            measurementCounter++;
            logMeasurementData();
        }
    }
    ws.cleanupClients();
    delay(10);
}