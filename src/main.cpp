#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include "index.h"
#include "websocket_handler.h"
#include "sd_handler.h"
#include "measurement.h"

#define BUTTON_PIN 2

volatile bool buttonPressed = false;

void IRAM_ATTR handleButtonPress()
{
    buttonPressed = true;
}

void setup()
{
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(3, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

    EEPROM.begin(128);
    Wire.begin();
    initWiFi();
    setupWebSocket();
    initMTi();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
}

void loop()
{
    if (buttonPressed)
    {
        buttonPressed = false;
        if (!isMeasuring)
        {
            startMeasurement();
        }
        else
        {
            stopMeasurement();
        }
    }
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