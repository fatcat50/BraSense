#include "sd_handler.h"

uint16_t fileCounter = 0;
String currentFileName;
File file;

bool initSDCard() {
    EEPROM.begin(128);

    if (!SD.begin()) {
        Serial.println("Failed to initialize SD card.");
        return false;
    }
    Serial.println("SD card successfully initialized.");
    return true;
}

void loadFileCounter() {
    EEPROM.get(0, fileCounter);
    // Serial.print("Current file counter: ");
    // Serial.println(fileCounter);
}

void saveFileCounter() {
    EEPROM.put(0, fileCounter);
    EEPROM.commit();
    // Serial.print("New file counter saved: ");
    // Serial.println(fileCounter);
}

void createNewMeasurementFile() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Error retrieving time! Using default filename.");
        char filename[32];
        sprintf(filename, "/messung_%04u.csv",
                fileCounter);  // Fallback, if time is not available
        currentFileName = String(filename);
    } else {
        char filename[32];
        sprintf(filename, "/%02d.%02d.%04d_%02d-%02d-%02d.csv",
                timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        currentFileName = String(filename);
    }

    file = SD.open(currentFileName, FILE_WRITE);
    if (!file) {
        Serial.println("Error opening file in write mode");
    } else {
        Serial.println("New file created: " + currentFileName);
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