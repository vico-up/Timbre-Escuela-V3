#ifndef APPWEBSERVER_H
#define APPWEBSERVER_H

#include "BellManager.h"
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>


class AppWebServer {
public:
  AppWebServer(BellManager &bellManager, NTPClient &timeClient);
  void begin();
  void handleClient(); // Must be called in loop

private:
  ESP8266WebServer _server;
  BellManager &_bellManager;
  NTPClient &_timeClient;

  void setupRoutes();
};

#endif
