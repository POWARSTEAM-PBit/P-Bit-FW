#include <Arduino.h>
#include "timer.h"

bool userTimerRunning = false;
unsigned long userTimerStart = 0;
unsigned long userTimerElapsed = 0;

constexpr size_t TIME_STR_MAX = 9;

void startUserTimer() {
    userTimerStart = millis();
    userTimerElapsed = 0;
    userTimerRunning = true;
    Serial.println("[Timer] Started");
}

void stopUserTimer() {
    if (userTimerRunning) {
        userTimerElapsed = millis() - userTimerStart;
        userTimerRunning = false;
        Serial.printf("[Timer] Stopped. Elapsed: %.2f s\n", userTimerElapsed / 1000.0);
    }
}

char * getTimeHMS() {
    static char timeStr[TIME_STR_MAX];
    uint32_t totalSeconds;

    if (userTimerRunning) {
        totalSeconds = (millis() - userTimerStart) / 1000;
    } else {
        totalSeconds = userTimerElapsed / 1000;
    }

    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);

    return timeStr;
}
