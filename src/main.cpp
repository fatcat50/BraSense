#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "index.h"
#include "websocket_handler.h"
#include "sd_handler.h"
#include "measurement.h"

<<<<<<< Updated upstream
#define BUTTON_PIN 2

const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

volatile bool buttonPressed = false;
AsyncWebServer server(80);

void IRAM_ATTR handleButtonPress() {
    buttonPressed = true;
=======
unsigned long lastLogTime = 0;
const unsigned long logInterval = 100;

void setup()
{
    Serial.begin(115200);

    EEPROM.begin(128);
    Wire.begin();
    Wire.setClock(400000UL); //Set I2C Clock to 400kHz Fast Mode.
    initMTi();
    initMeasurement();
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();
    initWiFi();
    setupWebSocket();
>>>>>>> Stashed changes
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING);

    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

<<<<<<< Updated upstream
    setupWebSocket(server);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        //request->send_P(200, "text/html", index_html, processor);
    });
    server.begin();

    if (!initSDCard()) return;
    loadFileCounter();
    createNewMeasurementFile();

    initMTi();
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        if (!isMeasuring) {
            startMeasurement();
        } else {
            stopMeasurement();
        }
    }
    if (isMeasuring) {
        logMeasurementData();
    }
=======
        if (!isnan(MyMTi->getAcceleration()[0]))
        {
            measurementCounter++;
            logMeasurementData();
            sendSensorData();
            ws.cleanupClients();
        }
    }
    // delay(50);
>>>>>>> Stashed changes
}