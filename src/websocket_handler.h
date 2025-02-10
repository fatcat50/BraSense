#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebSocket ws;

void initWiFi();
void setupWebSocket();
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
String processor(const String& var);
void sendSensorData();
void initTime();
void printLocalTime();

#endif // WEBSOCKET_HANDLER_H