#include "Arduino.h"
#include "SD.h"
#include "SPI.h"
#include "MTi.h"
#include <Wire.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include "index.h"
#include "websocket_handler.h"
#include "sd_handler.h"
#include "measurement.h"

#define DRDY 3        // Arduino Digital IO pin for MTi-DRDY
#define ADDRESS 0x6B  // MTi I2C address (default for MTi 1-series)
MTi *MyMTi = NULL;

const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

const byte buttonPin = 2;         // Taster-Pin
bool isMeasuring = false;         // Zustandsvariable für Messung
bool lastButtonState = HIGH;      // Letzter Tasterzustand
unsigned long debounceTime = 50;  // Entprellzeit in ms
unsigned long lastDebounceTime = 0;
volatile bool buttonPressed = false;     // Zustandsvariable für Taster
const unsigned long debounceDelay = 50;  // Entprellzeit in Millisekunden
int count = 0;

const int EEPROM_ADDRESS = 0;
uint16_t fileCounter = 0;    // Datei-Zähler (aus dem EEPROM)
uint16_t recordCounter = 0;  // Aufnahme-Zähler (nur im RAM, reset bei jedem Neustart)
String currentFileName;      // Aktueller Dateiname

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  //Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    //Serial.println("Message appended");
  } else {
    //Serial.println("Append failed");
  }
  file.close();
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
  digitalWrite(LED_BUILTIN, HIGH);
}

void stopMeasurement() {
  if (isMeasuring) {
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
  digitalWrite(LED_BUILTIN, LOW);
}

void logMeasurementData() {
  char cnt[32];
  dtostrf(count, 8, 2, cnt);

  File file = SD.open(currentFileName, FILE_APPEND);
  if (file) {
    file.print(cnt);
    file.print(" ; Acceleration [m/s^2]: ");

    for (int i = 0; i < 3; ++i) {
      char str[32];
      dtostrf(MyMTi->getAcceleration()[i], 8, 2, str);
      file.print(str);
      if (i < 2) file.print(" ");
    }
    file.println();
    file.close();
  } else {
    Serial.println("Fehler beim Oeffnen der Datei (logData).");
  }
}

void IRAM_ATTR handleButtonPress() {
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {  // Entprellung
    buttonPressed = true;
    lastDebounceTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(128);  // Bereich reservieren
  Wire.begin();
  pinMode(DRDY, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  MyMTi = new MTi(ADDRESS, DRDY);

  if (!MyMTi->detect(1000)) {
    Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
    while (1)
      ;
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

  //EEPROM.get(EEPROM_ADDRESS, fileCounter);
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
  //EEPROM.put(EEPROM_ADDRESS, fileCounter);
  //EEPROM.commit();
  Serial.print("Neuer Datei-Zaehler: ");
  Serial.println(fileCounter);
}

void loop() {
  if (buttonPressed) {
    buttonPressed = false;
    if (!isMeasuring) {
      startMeasurement();
    } else {
      stopMeasurement();
    }
  }
  if (isMeasuring && digitalRead(MyMTi->drdy)) {
    MyMTi->readMessages();
    //MyMTi->printData();

    if (!isnan(MyMTi->getAcceleration()[0])) {
      count++;
      logMeasurementData();
    }
  }
  /*if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.print(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }*/
  ws.cleanupClients();
  delay(10);
}