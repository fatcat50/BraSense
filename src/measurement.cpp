#include "measurement.h"

bool isMeasuring = false;
uint16_t recordCounter = 0;
MTi *MyMTi = NULL;

void initMTi() {
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
    Serial.println("Messung gestartet.");
}

void stopMeasurement() {
    isMeasuring = false;
    Serial.println("Messung gestoppt.");
}

void logMeasurementData() {
    if (MyMTi == NULL) return;
    Serial.println("Messdaten werden geloggt...");
}