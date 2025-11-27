#include "BellManager.h"

BellManager::BellManager(int pin, NTPClient &timeClient)
    : _pin(pin), _timeClient(timeClient) {
  _isRinging = false;
  _lastMinuteChecked = -1;
}

void BellManager::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(
      _pin, LOW); // Assuming Active HIGH relay. If Active LOW, change to HIGH.
  loadSchedule();
}

void BellManager::loop() {
  unsigned long currentMillis = millis();

  // Handle Ringing Logic
  if (_isRinging) {
    if (currentMillis - _ringStartTime >= _ringDuration) {
      digitalWrite(_pin, LOW);
      _isRinging = false;
      Serial.println("Bell OFF");
    }
  }

  // Check Schedule
  // Only check once per minute
  int currentMinute = _timeClient.getMinutes();
  if (currentMinute != _lastMinuteChecked && _timeClient.isTimeSet()) {
    _lastMinuteChecked = currentMinute;
    int currentHour = _timeClient.getHours();

    for (const auto &schedule : _schedules) {
      if (schedule.enabled && schedule.hour == currentHour &&
          schedule.minute == currentMinute) {
        trigger(schedule.durationSec);
        break; // Trigger only once per minute match
      }
    }
  }
}

void BellManager::trigger(int durationSec) {
  Serial.printf("Triggering Bell for %d seconds\n", durationSec);
  digitalWrite(_pin, HIGH);
  _isRinging = true;
  _ringStartTime = millis();
  _ringDuration = durationSec * 1000;
}

bool BellManager::addSchedule(int hour, int minute, int durationSec) {
  if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || durationSec <= 0) {
    Serial.println("Invalid schedule parameters");
    return false;
  }

  BellSchedule newSchedule = {hour, minute, durationSec, true};
  _schedules.push_back(newSchedule);
  saveSchedule();
  return true;
}

bool BellManager::removeSchedule(int index) {
  if (index >= 0 && index < _schedules.size()) {
    _schedules.erase(_schedules.begin() + index);
    saveSchedule();
    return true;
  }
  return false;
}

void BellManager::clearSchedule() {
  _schedules.clear();
  saveSchedule();
}

String BellManager::getScheduleJson() {
  DynamicJsonDocument doc(4096);
  JsonArray array = doc.to<JsonArray>();

  for (const auto &schedule : _schedules) {
    JsonObject obj = array.createNestedObject();
    obj["h"] = schedule.hour;
    obj["m"] = schedule.minute;
    obj["d"] = schedule.durationSec;
    obj["e"] = schedule.enabled;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

void BellManager::loadSchedule() {
  if (!LittleFS.exists("/config.json")) {
    Serial.println("No config file found.");
    return;
  }

  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("Failed to open config file");
    return;
  }

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("Failed to parse config file");
    return;
  }

  _schedules.clear();
  JsonArray array = doc.as<JsonArray>();
  for (JsonObject obj : array) {
    BellSchedule s;
    s.hour = obj["h"];
    s.minute = obj["m"];
    s.durationSec = obj["d"];
    s.enabled = obj["e"];

    // Filter out invalid entries
    if (s.durationSec <= 0 || s.hour < 0 || s.hour > 23 || s.minute < 0 ||
        s.minute > 59) {
      Serial.println("Skipping invalid schedule entry");
      continue;
    }

    _schedules.push_back(s);
  }
  Serial.println("Schedule loaded.");
}

void BellManager::saveSchedule() {
  DynamicJsonDocument doc(4096);
  JsonArray array = doc.to<JsonArray>();

  for (const auto &schedule : _schedules) {
    JsonObject obj = array.createNestedObject();
    obj["h"] = schedule.hour;
    obj["m"] = schedule.minute;
    obj["d"] = schedule.durationSec;
    obj["e"] = schedule.enabled;
  }

  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("Failed to open config file for writing");
    return;
  }

  serializeJson(doc, file);
  file.close();
  Serial.println("Schedule saved.");
}
