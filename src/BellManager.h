#ifndef BELLMANAGER_H
#define BELLMANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <NTPClient.h>

struct BellSchedule {
    int hour;
    int minute;
    int durationSec; // Duration to ring in seconds
    bool enabled;
};

class BellManager {
public:
    BellManager(int pin, NTPClient& timeClient);
    void begin();
    void loop();
    void trigger(int durationSec = 5);
    
    // Schedule management
    bool addSchedule(int hour, int minute, int durationSec);
    bool removeSchedule(int index);
    void clearSchedule();
    String getScheduleJson();
    void loadSchedule();
    void saveSchedule();

private:
    int _pin;
    NTPClient& _timeClient;
    std::vector<BellSchedule> _schedules;
    
    bool _isRinging;
    unsigned long _ringStartTime;
    unsigned long _ringDuration;
    
    int _lastMinuteChecked; // To avoid multiple triggers in the same minute
};

#endif
