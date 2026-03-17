#include "tasks.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "measurement.h"
#include "sd_handler.h"
#include "websocket_handler.h"

#define SAMPLE_INTERVAL_US 2000   // 500 Hz = alle 2000 µs

unsigned long lastWsTime = 0;
const unsigned int wsInterval = 50;   // WebSocket max. 20 Hz (reicht für Live-Ansicht)

// ── SD-Schreibtask (Core 0, Priorität 2) ─────────────────────────────────────
// Empfängt volle Buffer aus der Queue und schreibt sie binär auf die SD-Karte.
void sdTask(void *pvParam) {
    static datapoint localBuf[ARR_SIZE];  // eigener Stack-Buffer, kein globaler

    while (1) {
        if (xQueueReceive(sdQueue, &localBuf, portMAX_DELAY)) {
            // SPI-Bus sichern, bevor auf SD zugegriffen wird
            if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE) {
                file.write((byte*)localBuf, sizeof(localBuf));
                file.flush();
                xSemaphoreGive(spiMutex);
            }
        }
        vTaskDelay(1);
    }
}

// ── WebSocket-Sendetask (Core 0, Priorität 1) ────────────────────────────────
// Sendet Livedata mit max. 20 Hz an alle verbundenen Browser.
void wsTask(void *pvParam) {
    while (1) {
        ws.cleanupClients();
        if (isMeasuring && millis() - lastWsTime >= wsInterval) {
            sendSensorData();
            lastWsTime = millis();
        }
        vTaskDelay(1);
    }
}

// ── Sensor-Messtask (Core 1, Priorität 3) ────────────────────────────────────
// Läuft mit harter 500-Hz-Schleife. Drift-Kompensation verhindert Zeitfehler.
void sensorTask(void *pvParam) {
    int64_t nextWakeUs = esp_timer_get_time();

    while (1) {
        if (isMeasuring) {
            logMeasurementData();
        }

        // Präzises Timing: nächsten Weckzeitpunkt berechnen
        nextWakeUs += SAMPLE_INTERVAL_US;
        int64_t now    = esp_timer_get_time();
        int64_t waitUs = nextWakeUs - now;

        if (waitUs > 0) {
            if (waitUs > 1000) {
                // Grobes Schlafen für den Großteil der Wartezeit
                vTaskDelay(pdMS_TO_TICKS(waitUs / 1000));
            }
            // Feines Busy-Wait für den Rest (< 1 ms)
            while (esp_timer_get_time() < nextWakeUs) {
                taskYIELD();
            }
        } else {
            // Zyklus war zu langsam → Drift zurücksetzen, nicht aufholen
            nextWakeUs = esp_timer_get_time();
        }
    }
}

// ── Tasks anlegen ─────────────────────────────────────────────────────────────
void createTasks() {
    xTaskCreatePinnedToCore(sdTask,     "SDTask",     4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(wsTask,     "WSTask",     6144, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 3, NULL, 1);
}
