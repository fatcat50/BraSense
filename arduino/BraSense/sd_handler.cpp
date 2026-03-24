#include "sd_handler.h"
#include "measurement.h"

uint16_t fileCounter = 0;
String currentFileName;
File file;

bool initSDCard() {
    EEPROM.begin(128);

    if (!SD.begin(PIN_CS_SD, SPI)) {
        Serial.println("SD card not found. Check CS pin.");
        return false;
    }
    Serial.println("SD card OK");
    return true;
}

void loadFileCounter() {
    EEPROM.get(0, fileCounter);
}

void saveFileCounter() {
    EEPROM.put(0, fileCounter);
    EEPROM.commit();
}

void createNewMeasurementFile() {
    struct tm timeinfo;
    char filename[32];

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Time error - using counter as filename.");
        // "messung" wurde zu "measurement" geändert
        sprintf(filename, "/measurement_%04u.bin", fileCounter); 
    } else {
        sprintf(filename, "/%02d.%02d.%04d_%02d-%02d-%02d.bin",
                timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    currentFileName = String(filename);

    file = SD.open(currentFileName, FILE_WRITE);
    if (!file) {
        Serial.println("Error creating file: " + currentFileName);
    } else {
        Serial.println("New file: " + currentFileName);
    }

    fileCounter++;
    saveFileCounter();
}

void openFile() {
    if (!file) {
        file = SD.open(currentFileName, FILE_APPEND);
    }
}

void closeFile() {
    if (file) {
        file.close();
    }
}