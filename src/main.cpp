#include "BellManager.h"
#include "WebServer.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// --- CONFIGURATION ---
const char *ssid = "Martinez-2.4G";
const char *password = "Guadalupe";
const int BELL_PIN = 2; // Confirmed by user
const long UTC_OFFSET_IN_SECONDS =
    -14400; // Example: UTC-4 (Bolivia/Venezuela/East US). Adjust as needed.

// --- OBJECTS ---
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC_OFFSET_IN_SECONDS);
BellManager bellManager(BELL_PIN, timeClient);
AppWebServer webServer(bellManager, timeClient);

void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize NTP
  timeClient.begin();

  // Initialize Bell Manager
  bellManager.begin();

  // Initialize Web Server
  webServer.begin();
}

void loop() {
  timeClient.update();
  bellManager.loop();
}
