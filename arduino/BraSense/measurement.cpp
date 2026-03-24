#include "measurement.h"

#include <SPI.h>
#include "esp_timer.h"
#include "sd_handler.h"
#include "websocket_handler.h"

#define DEBOUNCE_DELAY 50

bool isMeasuring       = false;
bool firstMeasurement  = false;
uint16_t recordCounter     = 0;
uint32_t measurementCounter = 0;
unsigned long measurementStartTime = 0;

datapoint buffer [ARR_SIZE];
datapoint buffer1[ARR_SIZE];
datapoint buffer2[ARR_SIZE];
datapoint* currentBuffer = buffer1;
datapoint* writeBuffer   = buffer2;
size_t bufferIndex = 0;

QueueHandle_t    sdQueue   = xQueueCreate(10, sizeof(datapoint[ARR_SIZE]));
SemaphoreHandle_t spiMutex = NULL;

bool buttonState     = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;

MTi* MyMTi1 = NULL;
MTi* MyMTi2 = NULL;

float currentX1 = 0.0f, currentY1 = 0.0f, currentZ1 = 0.0f;
float currentX2 = 0.0f, currentY2 = 0.0f, currentZ2 = 0.0f;

static void configureSensor(MTi* sensor) {
    sensor->goToConfig();
    sensor->requestDeviceInfo();
    sensor->configureOutputs();
    sensor->goToMeasurement();
}

void initMTi() {
    // Mutex
    spiMutex = xSemaphoreCreateMutex();

    ledcSetup(0, 500, 8);
    ledcAttachPin(SYNC_PIN, 0);
    ledcWrite(0, 127);

    pinMode(MTI1_CS,   OUTPUT); digitalWrite(MTI1_CS,   HIGH);
    pinMode(MTI2_CS,   OUTPUT); digitalWrite(MTI2_CS,   HIGH);
    pinMode(PIN_CS_SD, OUTPUT); digitalWrite(PIN_CS_SD, HIGH);

    pinMode(MTI1_DRDY, INPUT);
    pinMode(MTI2_DRDY, INPUT);

    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI);

    delay(500);

    // Sensor 1
    MyMTi1 = new MTi(MTI1_CS, MTI1_DRDY);
    if (!MyMTi1->detect(1000)) {
        Serial.println("Error: Sensor 1 not found");
        while (1);  
    }
    configureSensor(MyMTi1);
    Serial.println("Sensor 1 found");

    // Sensor 2
    MyMTi2 = new MTi(MTI2_CS, MTI2_DRDY);
    if (!MyMTi2->detect(1000)) {
        Serial.println("Info: Sensor 2 not found - Single-Sensor-Mode");
        delete MyMTi2;
        MyMTi2 = NULL;
    } else {
        configureSensor(MyMTi2);
        Serial.println("Sensor 2 found - Dual-Sensor-Mode");
    }
}

void startMeasurement() {
    isMeasuring      = true;
    firstMeasurement = true;
    recordCounter++;
    measurementCounter = 0;

    openFile();

    while (digitalRead(MTI1_DRDY)) MyMTi1->readMessages();
    if (MyMTi2 != NULL) {
        while (digitalRead(MTI2_DRDY)) MyMTi2->readMessages();
    }

    ws.textAll("1");
}

void stopMeasurement() {
    isMeasuring = false;
    file.close();
    ws.textAll("0");
}

void logMeasurementData() {
    if (firstMeasurement) {
        measurementStartTime = esp_timer_get_time();
        firstMeasurement = false;
    }
    float ts = (esp_timer_get_time() - measurementStartTime) / 1e6f;

    if (xSemaphoreTake(spiMutex, pdMS_TO_TICKS(5)) == pdTRUE) {

        // Sensor 1
        if (digitalRead(MTI1_DRDY)) {
            MyMTi1->readMessages();
            float* a = MyMTi1->getAcceleration();
            currentX1 = a[0]; currentY1 = a[1]; currentZ1 = a[2];
        }

        // Sensor 2
        if (MyMTi2 != NULL && digitalRead(MTI2_DRDY)) {
            MyMTi2->readMessages();
            float* a = MyMTi2->getAcceleration();
            currentX2 = a[0]; currentY2 = a[1]; currentZ2 = a[2];
        } else if (MyMTi2 == NULL) {
            currentX2 = 0.0f; currentY2 = 0.0f; currentZ2 = 0.0f;
        }

        xSemaphoreGive(spiMutex);
    }

    currentBuffer[bufferIndex++] = {
        ts,
        currentX1, currentY1, currentZ1,
        currentX2, currentY2, currentZ2
    };

    if (bufferIndex >= ARR_SIZE) {
        datapoint* tmp = currentBuffer;
        currentBuffer  = writeBuffer;
        writeBuffer    = tmp;
        xQueueSend(sdQueue, writeBuffer, pdMS_TO_TICKS(100));
        bufferIndex = 0;
    }
}

void initMeasurement() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void handleButtonPress() {
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState)
        lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                isMeasuring ? stopMeasurement() : startMeasurement();
            }
        }
    }
    lastButtonState = reading;
}
