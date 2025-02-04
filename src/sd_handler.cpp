#include "sd_handler.h"

uint16_t fileCounter = 0;
String currentFileName;

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
    char filename[32];
    sprintf(filename, "/messung_%04u.txt", fileCounter);
    currentFileName = String(filename);

    File file = SD.open(currentFileName, FILE_WRITE);
    if (!file) {
        Serial.println("Fehler beim Erstellen der neuen Datei!");
    } else {
        file.close();
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