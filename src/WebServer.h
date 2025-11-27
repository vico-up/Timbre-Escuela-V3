#ifndef APPWEBSERVER_H
#define APPWEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "BellManager.h"
#include <NTPClient.h>

class AppWebServer {
public:
    AppWebServer(BellManager& bellManager, NTPClient& timeClient);
    void begin();

private:
    AsyncWebServer _server;
    BellManager& _bellManager;
    NTPClient& _timeClient;

    void setupRoutes();
};

#endif
