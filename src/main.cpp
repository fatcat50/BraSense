#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include "MTi.h"
#include <Wire.h>

#define DRDY 3       // Data Ready Pin
#define ADDRESS 0x6B // I2C-Adresse des MTi-3

MTi *MyMTi = NULL;
bool recording = false; // Status der Aufnahme

// Netzwerk-Konfiguration
const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

// WebSocket-Server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Funktion, um Euler-Winkel und Beschleunigung zu extrahieren (durch Ausgabe von `printData`)
String getSensorReadings() {
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
  if (eulerStart != -1) {
    eulerData = output.substring(eulerStart + 6, output.indexOf("\n", eulerStart));
  }

  int accStart = output.indexOf("Accel:");
  if (accStart != -1) {
    accData = output.substring(accStart + 6, output.indexOf("\n", accStart));
  }

  // Daten in JSON formatieren
  JSONVar sensorData;
  sensorData["euler"] = JSONVar({eulerData.c_str()});
  sensorData["acceleration"] = JSONVar({accData.c_str()});

  return JSON.stringify(sensorData);
}

// WebSocket-Clients benachrichtigen
void notifyClients(String message) {
  ws.textAll(message);
}

// WebSocket-Nachrichten behandeln
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    String message = String((char *)data).substring(0, len);

    if (message == "start") {
      recording = true;
      notifyClients("{\"status\":\"recording\"}");
    } else if (message == "stop") {
      recording = false;
      notifyClients("{\"status\":\"stopped\"}");
    } else if (message == "getStatus") {
      String status = recording ? "{\"status\":\"recording\"}" : "{\"status\":\"stopped\"}";
      notifyClients(status);
    }
  }
}

// WebSocket-Ereignisse
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
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

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(DRDY, INPUT);

  MyMTi = new MTi(ADDRESS, DRDY);
  if (!MyMTi->detect(1000)) {       // Check if MTi is detected before moving on
    Serial.println("Please check your hardware connections.");
    while (1) {
      // Cannot continue because no MTi was detected.
    }
  } else {
    MyMTi->goToConfig();            // Switch device to Config mode
    MyMTi->requestDeviceInfo();     // Request the device's product code and firmware version
    MyMTi->configureOutputs();      // Configure the device's outputs based on its functionality
    MyMTi->goToMeasurement();       // Switch device to Measurement mode
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS konnte nicht gemountet werden.");
    return;
  }

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

void loop() {
  if (digitalRead(MyMTi->drdy)) {   // MTi reports that new data/notifications are available
    MyMTi->readMessages();          // Read new data messages

    if (recording) {
      String sensorData = getSensorReadings();
      notifyClients(sensorData);    // Send sensor data to all WebSocket clients
    }
  }
  ws.cleanupClients();
}
