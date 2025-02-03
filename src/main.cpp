#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include "MTi.h"
#include <Wire.h>
#include <EEPROM.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define DRDY 3       // Data Ready Pin
#define ADDRESS 0x6B // I2C-Adresse des MTi-3

MTi *MyMTi = NULL;
bool recording = false; // Status der Aufnahme

// Netzwerk-Konfiguration
const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

const byte buttonPin = 2;        // Taster-Pin
bool isMeasuring = false;        // Zustandsvariable für Messung
bool lastButtonState = HIGH;     // Letzter Tasterzustand
unsigned long debounceTime = 50; // Entprellzeit in ms
unsigned long lastDebounceTime = 0;
volatile bool buttonPressed = false;    // Zustandsvariable für Taster
const unsigned long debounceDelay = 50; // Entprellzeit in Millisekunden
int count = 0;

const int EEPROM_ADDRESS = 0;
uint16_t fileCounter = 0;   // Datei-Zähler (aus dem EEPROM)
uint16_t recordCounter = 0; // Aufnahme-Zähler (nur im RAM, reset bei jedem Neustart)
String currentFileName;     // Aktueller Dateiname

// WebSocket-Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Funktion, um Euler-Winkel und Beschleunigung zu extrahieren (durch Ausgabe von `printData`)
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("File written");
  }
  else
  {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  // Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    // Serial.println("Message appended");
  }
  else
  {
    // Serial.println("Append failed");
  }
  file.close();
}

void startNewMeasurement()
{
  char filename[20];
  sprintf(filename, "/messung_%04u.txt", fileCounter);

  // Datei anlegen
  File file = SD.open(filename, FILE_WRITE);
  if (file)
  {
    file.println("Neue Messung gestartet!");
    file.close();
    Serial.print("Neue Datei erstellt: ");
    Serial.println(filename);

    // Counter erhöhen und im EEPROM speichern
    fileCounter++;
    EEPROM.put(EEPROM_ADDRESS, fileCounter);
    Serial.print("Neuer Zählerstand: ");
    Serial.println(fileCounter);
  }
  else
  {
    Serial.print("Fehler beim Erstellen der Datei: ");
    Serial.println(filename);
  }
}

void startMeasurement()
{
  isMeasuring = true;
  recordCounter++;

  // Ins File schreiben, dass eine neue Aufnahme gestartet wurde
  File file = SD.open(currentFileName, FILE_APPEND);
  if (file)
  {
    file.println(">>>> Aufnahme " + String(recordCounter) + " START <<<<");
    file.close();
    Serial.println("Aufnahme " + String(recordCounter) + " gestartet.");
  }
  else
  {
    Serial.println("Fehler beim Oeffnen der Datei (START).");
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

/**
 * Beendet die aktuelle Aufnahme.
 */
void stopMeasurement()
{
  if (isMeasuring)
  {
    File file = SD.open(currentFileName, FILE_APPEND);
    if (file)
    {
      file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
      file.close();
      Serial.println("Aufnahme " + String(recordCounter) + " gestoppt.");
    }
    else
    {
      Serial.println("Fehler beim Oeffnen der Datei (STOPP).");
    }
    isMeasuring = false;
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void logMeasurementData()
{
  char cnt[32];
  dtostrf(count, 8, 2, cnt);

  File file = SD.open(currentFileName, FILE_APPEND);
  if (file)
  {
    file.print(cnt);
    file.print(" ; Acceleration [m/s^2]: ");

    for (int i = 0; i < 3; ++i)
    {
      char str[32];
      dtostrf(MyMTi->getAcceleration()[i], 8, 2, str);
      file.print(str);
      if (i < 2)
        file.print(" ");
    }
    file.println();
    file.close();
  }
  else
  {
    Serial.println("Fehler beim Oeffnen der Datei (logData).");
  }
}

void IRAM_ATTR handleButtonPress()
{
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay)
  { // Entprellung
    buttonPressed = true;
    lastDebounceTime = currentTime;
  }
}

String getSensorReadings()
{
  String output = "";
  String eulerData = "";
  String accData = "";

  // Daten von MTi lesen
  MyMTi->readMessages();

  // Ausgabe von `printData` fangen (wir gehen davon aus, dass `printData()` Sensordaten ausgibt)
  MyMTi->printData();

  // Simulierte Extraktion der Daten, indem wir nach den erwarteten Teilen in der Ausgabe suchen
  // Beispiel: Wenn die `printData` Ausgabe "Euler:" und "Accel:" enthält:
  // Achte darauf, wie die Ausgabe von `printData` aussieht und parse die entsprechenden Werte.

  // Hier ein Beispiel, wie man die Daten extrahieren könnte:
  // Es wird erwartet, dass `printData` Zeilen wie "Euler: X Y Z" und "Accel: X Y Z" enthält
  int eulerStart = output.indexOf("Euler:");
  if (eulerStart != -1)
  {
    eulerData = output.substring(eulerStart + 6, output.indexOf("\n", eulerStart));
  }

  int accStart = output.indexOf("Accel:");
  if (accStart != -1)
  {
    accData = output.substring(accStart + 6, output.indexOf("\n", accStart));
  }

  // Daten in JSON formatieren
  JSONVar sensorData;
  sensorData["euler"] = JSONVar({eulerData.c_str()});
  sensorData["acceleration"] = JSONVar({accData.c_str()});

  return JSON.stringify(sensorData);
}

// WebSocket-Clients benachrichtigen
void notifyClients(String message)
{
  ws.textAll(message);
}

// WebSocket-Nachrichten behandeln
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    String message = String((char *)data).substring(0, len);

    if (message == "start")
    {
      recording = true;
      notifyClients("{\"status\":\"recording\"}");
    }
    else if (message == "stop")
    {
      recording = false;
      notifyClients("{\"status\":\"stopped\"}");
    }
    else if (message == "getStatus")
    {
      String status = recording ? "{\"status\":\"recording\"}" : "{\"status\":\"stopped\"}";
      notifyClients(status);
    }
  }
}

// WebSocket-Ereignisse
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512); // Bereich reservieren
  Wire.begin();
  pinMode(DRDY, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButtonPress, FALLING);

  MyMTi = new MTi(ADDRESS, DRDY);

  if (!MyMTi->detect(1000))
  {
    Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
    while (1)
      ;
  }
  else
  {
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
  }

  if (!SD.begin())
  {
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
  if (!file)
  {
    Serial.println("Fehler beim Erstellen der neuen Datei!");
  }
  else
  {
    file.close();

    Serial.println("Neue Datei erstellt: " + currentFileName);
  }

  fileCounter++;
  EEPROM.put(EEPROM_ADDRESS, fileCounter);
  EEPROM.commit();
  Serial.print("Neuer Datei-Zaehler: ");
  Serial.println(fileCounter);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  if (!LittleFS.begin(true))
  {
    Serial.println("LittleFS konnte nicht gemountet werden.");
    return;
  }

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

void loop()
{
  if (buttonPressed)
  {
    buttonPressed = false;
    if (!isMeasuring)
    {
      startMeasurement();
    }
    else
    {
      stopMeasurement();
    }
  }
  if (isMeasuring && digitalRead(MyMTi->drdy))
  {
    MyMTi->readMessages();
    // MyMTi->printData();

    if (!isnan(MyMTi->getAcceleration()[0]))
    {
      count++;
      logMeasurementData();
    }
    if (recording)
    {
      String sensorData = getSensorReadings();
      notifyClients(sensorData); // Send sensor data to all WebSocket clients
    }
  }
  ws.cleanupClients();
  delay(10);
}
