#include "measurement.h"

#include "sd_handler.h"
#include "websocket_handler.h"

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 50

bool isMeasuring = false;
bool firstMeasurement = false;
uint16_t recordCounter = 0;
uint32_t measurementCounter = 0;
unsigned long measurementStartTime = 0;
const uint32_t FLUSH_INTERVAL = 1000;
datapoint buffer[ARR_SIZE];
size_t bufferIndex = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
MTi *MyMTi = NULL;

float currentX = 0.0;
float currentY = 0.0;
float currentZ = 0.0;

void print() {
    Serial.print("Measurement " + String(recordCounter) + " : ");
    Serial.println(isMeasuring ? "STARTED" : "STOPPED");
    // digitalWrite(LED_BUILTIN, isMeasuring);
}

void initMTi() {
    pinMode(3, INPUT);

    MyMTi = new MTi(0x6B, 3);
    if (!MyMTi->detect(1000)) {
        Serial.println("MTi not detected. Check connections.");
        while (1);
    }
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
}

void startMeasurement() {
    isMeasuring = true;
    firstMeasurement = true;
    recordCounter++;
    measurementCounter = 0;

    openFile();
    //file.println(">>>> Measurement #" + String(recordCounter) + " START <<<<");
    //file.println("Time [s];Data Point;X [deg];Y [deg];Z [deg]");
    //print();
    while (digitalRead(MyMTi->drdy)) {
        MyMTi->readMessages();
    }
    ws.textAll(String(isMeasuring));
    // MyMTi->goToMeasurement();
}

void stopMeasurement() {
    // MyMTi->goToConfig();
    isMeasuring = false;

    //file.println(">>>> Measurement #" + String(recordCounter) + " STOP <<<<");
    file.close();
    //print();
    ws.textAll(String(isMeasuring));
}

// Logs the measurement data to the SD card and WebSocket
void logMeasurementData() {
    if (firstMeasurement) {
        measurementStartTime = esp_timer_get_time();
        firstMeasurement = false;
    }
    float timestamp = (esp_timer_get_time() - measurementStartTime) / 1e6;;

    float* angles = MyMTi->getEulerAngles();
    currentX = angles[0];
    currentY = angles[1];
    currentZ = angles[2];

    buffer[bufferIndex] = {timestamp, currentX, currentY, currentZ};
    bufferIndex++;

    if (bufferIndex >= ARR_SIZE) {
        file.write((byte*)buffer, sizeof(buffer));
        bufferIndex = 0;
    }

    /*char timeStr[10];
    measurementCounter++;
    dtostrf(timestamp, 7, 3, timeStr);
    file.print(timeStr);
    file.print(";");

    char cnt[32];
    dtostrf(measurementCounter, 8, 2, cnt);

    file.print(cnt);
    file.print(";");
    if (!isnan(MyMTi->getEulerAngles()[0])) {
        for (int i = 0; i < 3; ++i) {
            char str[32];
            dtostrf(MyMTi->getEulerAngles()[i], 8, 2, str);
            file.print(str);
            if (i < 2) file.print(";");
        }
        file.println();

        /*if (measurementCounter % FLUSH_INTERVAL == 0) {
            file.flush();  // SD-Puffer leeren
            // Serial.println("FLUSHED");
            //delay(1);  // Watchdog-Reset ermöglichen
        }*/
    //}
}

void initMeasurement() {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void handleButtonPress() {
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                isMeasuring = !isMeasuring;

                if (isMeasuring) {
                    startMeasurement();
                } else {
                    stopMeasurement();
                }
            }
        }
    }
    lastButtonState = reading;
}