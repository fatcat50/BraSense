#include "tasks.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "measurement.h"
#include "sd_handler.h"
#include "websocket_handler.h"

#define SAMPLE_INTERVAL_US 2000   // 500 Hz = every 2000 µs

unsigned long lastWsTime = 0;
const unsigned int wsInterval = 50;   // WebSocket max. 20 Hz (enough for live view)

// ── SD Write Task (Core 0, Priority 2) ───────────────────────────────────────
// Receives full buffers from the queue and writes them in binary to the SD card.
void sdTask(void *pvParam) {
    static datapoint localBuf[ARR_SIZE];  // own stack buffer, not global

    while (1) {
        if (xQueueReceive(sdQueue, &localBuf, portMAX_DELAY)) {
            // Secure SPI bus before accessing SD
            if (xSemaphoreTake(spiMutex, portMAX_DELAY) == pdTRUE) {
                file.write((byte*)localBuf, sizeof(localBuf));
                file.flush();
                xSemaphoreGive(spiMutex);
            }
        }
        vTaskDelay(1);
    }
}

// ── WebSocket Send Task (Core 0, Priority 1) ─────────────────────────────────
// Sends live data at max. 20 Hz to all connected browsers.
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

// ── Sensor Measurement Task (Core 1, Priority 3) ─────────────────────────────
// Runs with a hard 500 Hz loop. Drift compensation prevents timing errors.
void sensorTask(void *pvParam) {
    int64_t nextWakeUs = esp_timer_get_time();

    while (1) {
        if (isMeasuring) {
            logMeasurementData();
        }

        // Precise timing: calculate next wake-up time
        nextWakeUs += SAMPLE_INTERVAL_US;
        int64_t now    = esp_timer_get_time();
        int64_t waitUs = nextWakeUs - now;

        if (waitUs > 0) {
            if (waitUs > 1000) {
                // Coarse sleep for the majority of the wait time
                vTaskDelay(pdMS_TO_TICKS(waitUs / 1000));
            }
            // Fine busy-wait for the remainder (< 1 ms)
            while (esp_timer_get_time() < nextWakeUs) {
                taskYIELD();
            }
        } else {
            // Cycle was too slow -> reset drift, do not catch up
            nextWakeUs = esp_timer_get_time();
        }
    }
}

// ── Create Tasks ─────────────────────────────────────────────────────────────
void createTasks() {
    xTaskCreatePinnedToCore(sdTask,     "SDTask",     4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(wsTask,     "WSTask",     6144, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "SensorTask", 4096, NULL, 3, NULL, 1);
}