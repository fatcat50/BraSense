#include "websocket_handler.h"

#include "index.h"
#include "measurement.h"

const char *ssid = "iPhone 13";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

//const char* ssid     = "ESP32";
//const char* password = "12345678";

AsyncWebSocket ws("/ws");
AsyncWebServer server(80);

char json[64];

void initWiFi() {

  //ignore for client mode
  //-- 
  /*WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());
  */
  //--

  WiFi.begin(ssid, password); // ignore for AP Mode
  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi .."); //ignore for AP Mode
  while (WiFi.status() != WL_CONNECTED) { //ignore for AP Mode
    Serial.print('.');  //ignore for AP Mode
    delay(1000);  //ignore for AP Mode
  } //ignore for AP Mode
  Serial.println(WiFi.localIP()); //ignore for AP Mode
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  /*Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour,3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay,10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
    */
}

void initTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // printLocalTime();
}

void setupWebSocket() {
  ws.onEvent(eventHandler);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/downloadBin", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isMeasuring) {
      request->send(403, "text/plain", "Stop measurement first!");
      return;
    }
    if (!SD.exists(currentFileName)) {
      request->send(404, "text/plain", "Binary file not found");
      return;
    }

    auto *response = request->beginResponse(
      SD, currentFileName, "application/octet-stream");
    response->addHeader("Content-Disposition", "attachment; filename=\"log.bin\"");
    request->send(response);
  });

  server.begin();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0) {
      isMeasuring = !isMeasuring;

      if (isMeasuring) {
        startMeasurement();
      } else {
        stopMeasurement();
      }
    }
  }
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n",
                    client->id(), client->remoteIP().toString().c_str());
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

String processor(const String &var) {
  if (var == "STATE") {
    return isMeasuring ? "Measuring..." : "Standby";
  }
  if (var == "CHECK") {
    return isMeasuring ? "checked" : "";
  }
  return String();
}

void sendSensorData() {
  snprintf(json, sizeof(json),
           "{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f}",
           currentX, currentY, currentZ);

  ws.textAll(json);
}