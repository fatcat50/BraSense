#include "measurement.h"

#include "sd_handler.h"
#include "websocket_handler.h"

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 50

bool isMeasuring = false;
uint16_t recordCounter = 0;
uint32_t measurementCounter = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
MTi *MyMTi = NULL;

void print() {
    Serial.print("Messung " + String(recordCounter) + " : ");
    Serial.println(isMeasuring ? "GESTARTET" : "GESTOPPT");
    digitalWrite(LED_BUILTIN, isMeasuring);
}

void initMTi() {
    pinMode(3, INPUT);

    MyMTi = new MTi(0x6B, 3);
    if (!MyMTi->detect(1000)) {
        Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
        while (1);
    }
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
}

void startMeasurement() {
    isMeasuring = true;
    recordCounter++;

    openFile();
    file.println(">>>> Aufnahme " + String(recordCounter) + " START <<<<");
    print();
    ws.textAll(String(isMeasuring));
}

void stopMeasurement() {
    isMeasuring = false;

    file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
    file.close();
    print();
    ws.textAll(String(isMeasuring));
}

void logMeasurementData() {
    char cnt[32];
    dtostrf(measurementCounter, 8, 2, cnt);

    file.print(cnt);
    file.print(" ; Acceleration [m/s^2]: ");

    for (int i = 0; i < 3; ++i) {
        char str[32];
        dtostrf(MyMTi->getAcceleration()[i], 8, 2, str);
        file.print(str);
        if (i < 2) file.print(" ");
    }
    file.println();
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