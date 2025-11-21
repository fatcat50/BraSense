#include "tasks.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <queue.h>

#include "measurement.h"
#include "sd_handler.h"
#include "websocket_handler.h"

unsigned long lastTime = 0;
unsigned int interval = 50;

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
        ws.cleanupClients(); // nach vorne
        if (isMeasuring && millis() - lastTime >= interval) {
            sendSensorData();
            lastTime = millis();
        }
        vTaskDelay(1);
    }
}

void createTasks() {
    xTaskCreatePinnedToCore(sdTask, "SDTask", 4096, NULL,
                            2,  // Priority
                            NULL,
                            0  // Core
    );

    xTaskCreatePinnedToCore(wsTask, "WebSocket", 6144, NULL,
                            1,  // Priority
                            NULL,
                            1  // Core
    );
}