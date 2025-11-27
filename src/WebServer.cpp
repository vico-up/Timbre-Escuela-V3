#include "WebServer.h"

AppWebServer::AppWebServer(BellManager &bellManager, NTPClient &timeClient)
    : _server(80), _bellManager(bellManager), _timeClient(timeClient) {}

void AppWebServer::begin() {
  setupRoutes();
  _server.begin();
}

void AppWebServer::setupRoutes() {
  // Serve Static Files
  _server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // API: Get Status
  _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(512);
    doc["time"] = _timeClient.getFormattedTime();
    doc["epoch"] = _timeClient.getEpochTime();

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  // API: Get Schedule
  _server.on("/api/schedule", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send(200, "application/json", _bellManager.getScheduleJson());
  });

  // API: Add Schedule (POST)
  _server.on(
      "/api/schedule", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index, size_t total) {
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          request->send(400, "application/json",
                        "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
          return;
        }

        int h = doc["h"];
        int m = doc["m"];
        int d = doc["d"];

        Serial.printf("Add Schedule: %02d:%02d (%ds)\n", h, m, d);

        if (_bellManager.addSchedule(h, m, d)) {
          request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          request->send(
              400, "application/json",
              "{\"status\":\"error\",\"message\":\"Invalid Schedule\"}");
        }
      });

  // API: Delete Schedule (DELETE)
  // Using POST with ?action=delete or just a query param for simplicity in
  // embedded But let's try to use proper DELETE or a POST to
  // /api/schedule/delete
  _server.on(
      "/api/remove_schedule", HTTP_POST, [](AsyncWebServerRequest *request) {},
      NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index, size_t total) {
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, data, len);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          request->send(400, "application/json",
                        "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
          return;
        }

        int idx = doc["index"];
        Serial.printf("Delete Schedule Index: %d\n", idx);

        if (_bellManager.removeSchedule(idx)) {
          request->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
          request->send(400, "application/json",
                        "{\"status\":\"error\",\"message\":\"Invalid Index\"}");
        }
      });

  // API: Trigger Bell
  _server.on("/api/trigger", HTTP_POST, [this](AsyncWebServerRequest *request) {
    int duration = 5;
    if (request->hasParam("duration")) {
      duration = request->getParam("duration")->value().toInt();
    }
    _bellManager.trigger(duration);
    request->send(200, "application/json", "{\"status\":\"triggered\"}");
  });
}
