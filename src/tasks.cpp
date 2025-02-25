#include "tasks.h"
#include "measurement.h"
#include "sd_handler.h"
#include "websocket_handler.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <queue.h>

unsigned long lastTime = 0;
unsigned long interval = 50;

QueueHandle_t sdQueue = xQueueCreate(10, sizeof(datapoint[ARR_SIZE]));

void sdTask(void *pvParam) {
    while (1) {
        if (xQueueReceive(sdQueue, &buffer, portMAX_DELAY)) {
            file.write((byte *)buffer, sizeof(buffer));
        }
        vTaskDelay(1);
    }
}

void wsTask(void *pvParam) {
    while (1) {
        if (isMeasuring && millis() - lastTime >= interval) {
            sendSensorData();
            lastTime = millis();
        }
        ws.cleanupClients();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void createTasks() {
    xTaskCreatePinnedToCore(
        sdTask,
        "SDTask",
        4096,
        NULL,
        2,  // Priority
        NULL,
        0   // Core
    );

    xTaskCreatePinnedToCore(
        wsTask,
        "WebSocket",
        4096,
        NULL,
        1,            // Priority
        NULL,
        0             // Core
    );
}