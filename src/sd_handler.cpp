#include "sd_handler.h"

uint16_t fileCounter = 0;
String currentFileName;
File file;

bool initSDCard() {
    if (!SD.begin()) {
        Serial.println("SD-Karte konnte nicht initialisiert werden.");
        return false;
    }
    Serial.println("SD-Karte erfolgreich initialisiert.");
    return true;
}

void loadFileCounter() {
    EEPROM.get(0, fileCounter);
    Serial.print("Aktueller Datei-Zähler: ");
    Serial.println(fileCounter);
}

void saveFileCounter() {
    EEPROM.put(0, fileCounter);
    EEPROM.commit();
    Serial.print("Neuer Datei-Zähler gespeichert: ");
    Serial.println(fileCounter);
}

void createNewMeasurementFile() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println(
            "Fehler beim Abrufen der Zeit! Verwende Standard-Dateinamen.");
        char filename[32];
        sprintf(filename, "/messung_%04u.csv",
                fileCounter);  // Fallback, falls Zeit nicht verfügbar
        currentFileName = String(filename);
    } else {
        char filename[32];
        sprintf(filename, "/%02d.%02d.%04d_%02d-%02d-%02d.csv",
                timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        currentFileName = String(filename);
    }

    File file = SD.open(currentFileName, FILE_WRITE);
    if (!file) {
        Serial.println("Fehler beim Erstellen der neuen Datei!");
    } else {
        //file.close();
        Serial.println("Neue Datei erstellt: " + currentFileName);
    }

    fileCounter++;
    saveFileCounter();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.print(message);
    file.close();
    Serial.println("Message appended successfully");
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