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
    EEPROM.begin(128);
    EEPROM.get(0, fileCounter);

    if (fileCounter == 0xFFFF || fileCounter == 0) {  
        Serial.println("EEPROM leer, setze fileCounter auf 1");
        fileCounter = 1;
        saveFileCounter();
    }
    Serial.print("Geladener Datei-Zähler: ");
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