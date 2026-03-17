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

    // 1. SPI-Bus + Sensoren (SPI.begin() passiert hier)
    initMTi();

    // 2. SD-Karte (nutzt denselben SPI-Bus)
    initSDCard();
    loadFileCounter();
    createNewMeasurementFile();

    // 3. WiFi + Zeit
    initWiFi();
    initTime();

    // 4. WebSocket-Server
    setupWebSocket();

    // 5. Button-Pin konfigurieren
    initMeasurement();

    // 6. FreeRTOS-Tasks starten (sensorTask übernimmt ab jetzt die Messung)
    createTasks();
}

void loop() {
    // Sensor-Messung läuft in sensorTask (Core 1) → hier nur Button
    handleButtonPress();
    vTaskDelay(pdMS_TO_TICKS(10));  // Core 1 für sensorTask freigeben
}
