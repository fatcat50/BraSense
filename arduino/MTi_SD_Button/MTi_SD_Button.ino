#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "MTi.h"
#include <Wire.h>
#include <EEPROM.h>

#define DRDY 3        // Arduino Digital IO pin for MTi-DRDY
#define ADDRESS 0x6B  // MTi I2C address (default for MTi 1-series)
MTi *MyMTi = NULL;

const byte buttonPin = 2; // Taster-Pin
bool isMeasuring = false; // Zustandsvariable für Messung
bool lastButtonState = HIGH; // Letzter Tasterzustand
unsigned long debounceTime = 50; // Entprellzeit in ms
unsigned long lastDebounceTime = 0;
volatile bool buttonPressed = false; // Zustandsvariable für Taster
const unsigned long debounceDelay = 50; // Entprellzeit in Millisekunden
int count = 0;

const int EEPROM_ADDRESS = 0;
uint16_t fileCounter = 0;             // Datei-Zähler (aus dem EEPROM)
uint16_t recordCounter = 0;           // Aufnahme-Zähler (nur im RAM, reset bei jedem Neustart)
String currentFileName;               // Aktueller Dateiname 

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        //Serial.println("Message appended");
    } else {
        //Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void startNewMeasurement() {
  // Dateiname erzeugen, z. B. "messung_0000.txt"
  // Die 4 steht für 4-stellige Darstellung (z. B. 0001, 0002 usw.)
  char filename[20];
  sprintf(filename, "/messung_%04u.txt", fileCounter);
  
  // Datei anlegen
  File file = SD.open(filename, FILE_WRITE);
  if (file) {
    file.println("Neue Messung gestartet!");
    file.close();
    Serial.print("Neue Datei erstellt: ");
    Serial.println(filename);
    
    // Counter erhöhen und im EEPROM speichern
    fileCounter++;
    EEPROM.put(EEPROM_ADDRESS, fileCounter);
    Serial.print("Neuer Zählerstand: ");
    Serial.println(fileCounter);
  } else {
    Serial.print("Fehler beim Erstellen der Datei: ");
    Serial.println(filename);
  }
}

void startMeasurement() {
  isMeasuring = true;
  recordCounter++;

  // Ins File schreiben, dass eine neue Aufnahme gestartet wurde
  File file = SD.open(currentFileName, FILE_APPEND);
  if (file) {
    file.println(">>>> Aufnahme " + String(recordCounter) + " START <<<<");
    file.close();
    Serial.println("Aufnahme " + String(recordCounter) + " gestartet.");
  } else {
    Serial.println("Fehler beim Oeffnen der Datei (START).");
  }
  
  // LED als Status
  digitalWrite(LED_BUILTIN, HIGH);
}

/**
 * Beendet die aktuelle Aufnahme.
 */
void stopMeasurement() {
  if (isMeasuring) {
    // Logge STOPP ins File
    File file = SD.open(currentFileName, FILE_APPEND);
    if (file) {
      file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
      file.close();
      Serial.println("Aufnahme " + String(recordCounter) + " gestoppt.");
    } else {
      Serial.println("Fehler beim Oeffnen der Datei (STOPP).");
    }
    isMeasuring = false;
  }

  // LED aus
  digitalWrite(LED_BUILTIN, LOW);
}

/**
 * Schreibt die aktuellen Sensordaten in die Datei, 
 * solange isMeasuring = true.
 */
void logMeasurementData() {
  // Hier kannst du beliebig viele Sensorwerte einfügen
  char cnt[32];
  dtostrf(count, 8, 2, cnt);

  // An die Datei anhängen
  File file = SD.open(currentFileName, FILE_APPEND);
  if (file) {
    // Beispiel: Zähler + Beschleunigung in x, y, z
    file.print(cnt);
    file.print(" ; Acceleration [m/s^2]: ");

    for (int i = 0; i < 3; ++i) {
      char str[32];
      dtostrf(MyMTi->getAcceleration()[i], 8, 2, str);
      file.print(str);
      if (i < 2) file.print(" ");
    }
    file.println(); // Zeilenumbruch

    file.close();
    //Serial.println("Daten geloggt"); // optional
  } else {
    Serial.println("Fehler beim Oeffnen der Datei (logData).");
  }
}

void IRAM_ATTR handleButtonPress() {
  // Interrupt Service Routine für den Taster
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) { // Entprellung
    buttonPressed = true; // Signal an die Hauptschleife
    lastDebounceTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  EEPROM.begin(512); // Bereich reservieren (z.B. 512 Bytes)
  Wire.begin();          // Initialize Wire library for I2C communication
  pinMode(DRDY, INPUT);  // Data Ready pin
  pinMode(buttonPin, INPUT_PULLUP); // Taster mit Pullup
  pinMode(LED_BUILTIN, OUTPUT); // LED für Statusanzeige

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  MyMTi = new MTi(ADDRESS, DRDY); // MTi-Objekt erstellen

  if (!MyMTi->detect(1000)) {
    Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
    while (1);
  } else {
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
  }  
  
  if (!SD.begin()) {
    Serial.println("SD-Karte konnte nicht initialisiert werden.");
    return;
  }

  EEPROM.get(EEPROM_ADDRESS, fileCounter);
  Serial.print("Aktueller Zählerstand: ");
  Serial.println(fileCounter);
  
  char filename[32];
  sprintf(filename, "/messung_%04u.txt", fileCounter);
  currentFileName = String(filename);

   // Datei erstellen
  File file = SD.open(currentFileName, FILE_WRITE);
  if (!file) {
    Serial.println("Fehler beim Erstellen der neuen Datei!");
  } else {
    file.close();

    Serial.println("Neue Datei erstellt: " + currentFileName);
  }

  fileCounter++;
  EEPROM.put(EEPROM_ADDRESS, fileCounter);
  EEPROM.commit();
  Serial.print("Neuer Datei-Zaehler: ");
  Serial.println(fileCounter);
}

void loop() {
  // Tasterzustand auslesen und entprellen
  if (buttonPressed) { // Nur reagieren, wenn der Button über den Interrupt gesetzt wurde
    buttonPressed = false; // Zustand zurücksetzen
    if (!isMeasuring) {
      startMeasurement(); 
    } else {
      stopMeasurement();
    }
  }

  // Messung ausführen, wenn aktiv
  if (isMeasuring && digitalRead(MyMTi->drdy)) {
    MyMTi->readMessages();
    //MyMTi->printData();

    if (!isnan(MyMTi->getAcceleration()[0])) {
      count++;
      logMeasurementData();
    }
  }
  delay(10);
}