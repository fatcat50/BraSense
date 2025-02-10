#include "websocket_handler.h"
#include "index.h"
#include "measurement.h"

const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

// const char* ssid     = "ESP32";
// const char* password = "12345678";

AsyncWebSocket ws("/ws");
AsyncWebServer server(80);

void initWiFi()
{
    /*WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println(WiFi.softAPIP());
    */

    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(WiFi.localIP());
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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

void initTime()
{
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
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
            isMeasuring = !isMeasuring;

            if (isMeasuring)
            {
                startMeasurement();
            }
            else
            {
                stopMeasurement();
            }
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
        return isMeasuring ? "Measuring..." : "Standby";
    }
    if (var == "CHECK")
    {
        return isMeasuring ? "checked" : "";
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