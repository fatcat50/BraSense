#include "websocket_handler.h"

#include "index.h"
#include "measurement.h"

const char *ssid     = "iPhone 13";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

const char *ntpServer        = "pool.ntp.org";
const long  gmtOffset_sec    = 3600;
const int   daylightOffset_sec = 3600;

AsyncWebSocket  ws("/ws");
AsyncWebServer  server(80);

// 6 Floats + JSON-Overhead: {"x1":XX.XX,"y1":XX.XX,"z1":XX.XX,"x2":XX.XX,"y2":XX.XX,"z2":XX.XX}
char json[128];

void initWiFi() {
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    Serial.print("Verbinde mit WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
    }
    Serial.println("\nIP: " + WiFi.localIP().toString());
}

void initTime() {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void printLocalTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Zeitabfrage fehlgeschlagen.");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setupWebSocket() {
    ws.onEvent(eventHandler);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html, processor);
    });

    server.on("/downloadBin", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (isMeasuring) {
            request->send(403, "text/plain", "Messung zuerst stoppen!");
            return;
        }
        if (!SD.exists(currentFileName)) {
            request->send(404, "text/plain", "Datei nicht gefunden.");
            return;
        }
        auto *response = request->beginResponse(
            SD, currentFileName, "application/octet-stream");
        response->addHeader("Content-Disposition",
                            "attachment; filename=\"log.bin\"");
        request->send(response);
    });

    server.begin();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len &&
        info->opcode == WS_TEXT) {
        data[len] = 0;
        if (strcmp((char *)data, "toggle") == 0) {
            isMeasuring ? stopMeasurement() : startMeasurement();
        }
    }
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client,
                  AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WS Client #%u verbunden (%s)\n",
                          client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WS Client #%u getrennt\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        default:
            break;
    }
}

String processor(const String &var) {
    if (var == "STATE") return isMeasuring ? "Measuring..." : "Standby";
    if (var == "CHECK") return isMeasuring ? "checked" : "";
    return String();
}

void sendSensorData() {
    snprintf(json, sizeof(json),
             "{\"x1\":%.2f,\"y1\":%.2f,\"z1\":%.2f,"
             "\"x2\":%.2f,\"y2\":%.2f,\"z2\":%.2f}",
             currentX1, currentY1, currentZ1,
             currentX2, currentY2, currentZ2);
    ws.textAll(json);
}
