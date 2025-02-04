#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern bool ledState;
extern AsyncWebSocket ws;

void setupWebSocket(AsyncWebServer &server);

#endif // WEBSOCKET_HANDLER_H