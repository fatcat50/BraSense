#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "WLAN-Kornfeind";
const char *password = "Vbk70Mfk75Kvh96Mfk00";

extern bool ledState;
extern AsyncWebSocket ws;

void setupWebSocket(AsyncWebServer &server);

#endif // WEBSOCKET_HANDLER_H