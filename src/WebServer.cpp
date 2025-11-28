#include "WebServer.h"
#include <Arduino.h>
#include <LittleFS.h>

AppWebServer::AppWebServer(BellManager &bellManager, NTPClient &timeClient)
    : _server(80), _bellManager(bellManager), _timeClient(timeClient) {}

void AppWebServer::begin() {
  setupRoutes();
  _server.begin();
}

void AppWebServer::handleClient() { _server.handleClient(); }

void AppWebServer::setupRoutes() {
  // Serve Static Files
  _server.serveStatic("/", LittleFS, "/index.html");
  _server.serveStatic("/style.css", LittleFS, "/style.css");
  _server.serveStatic("/script.js", LittleFS, "/script.js");

  // API: Get Status
  _server.on("/api/status", HTTP_GET, [this]() {
    DynamicJsonDocument doc(512);
    doc["time"] = _timeClient.getFormattedTime();
    doc["epoch"] = _timeClient.getEpochTime();

    String response;
    serializeJson(doc, response);
    _server.send(200, "application/json", response);
  });

  // API: Get Schedule
  _server.on("/api/schedule", HTTP_GET, [this]() {
    _server.send(200, "application/json", _bellManager.getScheduleJson());
  });

  // API: Add Schedule (POST)
  _server.on("/api/schedule", HTTP_POST, [this]() {
    if (!_server.hasArg("plain")) {
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Body missing\"}");
      return;
    }

    String body = _server.arg("plain");
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      return;
    }

    int h = doc["h"];
    int m = doc["m"];
    int d = doc["d"];

    Serial.printf("Add Schedule: %02d:%02d (%ds)\n", h, m, d);

    if (_bellManager.addSchedule(h, m, d)) {
      _server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Invalid Schedule\"}");
    }
  });

  // API: Remove Schedule (POST)
  _server.on("/api/remove_schedule", HTTP_POST, [this]() {
    if (!_server.hasArg("plain")) {
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Body missing\"}");
      return;
    }

    String body = _server.arg("plain");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
      return;
    }

    int idx = doc["index"];
    Serial.printf("Delete Schedule Index: %d\n", idx);

    if (_bellManager.removeSchedule(idx)) {
      _server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      _server.send(400, "application/json",
                   "{\"status\":\"error\",\"message\":\"Invalid Index\"}");
    }
  });

  // API: Trigger Bell
  _server.on("/api/trigger", HTTP_POST, [this]() {
    int duration = 5;
    // Check if duration param exists (query param)
    if (_server.hasArg("duration")) {
      duration = _server.arg("duration").toInt();
    }
    _bellManager.trigger(duration);
    _server.send(200, "application/json", "{\"status\":\"triggered\"}");
  });
}
