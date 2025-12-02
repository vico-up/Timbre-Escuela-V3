#include "BellManager.h"
#include "WebServer.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // Include mDNS
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>

const int BELL_PIN = D1; // D1 = GPIO5 nuevo pin para activar rele
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

  WiFiManager wifiManager;
  // wifiManager.resetSettings(); // Uncomment to reset credentials

  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Timbre-Config")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize mDNS
  if (MDNS.begin("timbre")) {
    Serial.println("mDNS responder started: http://timbre.local");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  // Initialize NTP
  timeClient.begin();

  // Initialize Bell Manager
  bellManager.begin();

  // Initialize Web Server
  webServer.begin();
}

void loop() {
  MDNS.update();
  timeClient.update();
  bellManager.loop();
  webServer.handleClient();

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    lastPrint = millis();
    Serial.print("Time Status: ");
    if (timeClient.isTimeSet()) {
      Serial.println(timeClient.getFormattedTime());
    } else {
      Serial.println("Not Synced");
    }
  }
}
