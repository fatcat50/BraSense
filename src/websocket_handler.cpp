#include "websocket_handler.h"
#include "index.h"
#include "measurement.h"

const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

bool ledState = 0;
AsyncWebSocket ws("/ws");
AsyncWebServer server(80);

void initWiFi()
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}

void setupWebSocket()
{
    ws.onEvent(eventHandler);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html, processor); });
    server.begin();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "toggle") == 0)
        {
            isMeasuring = !isMeasuring; // Messstatus togglen

            if (isMeasuring)
            {
                startMeasurement();
            }
            else
            {
                stopMeasurement();
            }
            ws.textAll(String(isMeasuring));
        }
    }
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
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
        digitalWrite(LED_BUILTIN, ledState);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

String processor(const String &var)
{
    if (var == "STATE")
    {
        return ledState ? "ON" : "OFF";
    }
    if (var == "CHECK")
    {
        return ledState ? "checked" : "";
    }
    return String();
}

void sendSensorData()
{
    float x = MyMTi->getAcceleration()[0];
    float y = MyMTi->getAcceleration()[1];
    float z = MyMTi->getAcceleration()[2];

    String json = "{\"x\": " + String(x, 2) + ", \"y\": " + String(y, 2) + ", \"z\": " + String(z, 2) + "}";
    ws.textAll(json);
}