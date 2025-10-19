#include <Arduino.h>
#include "timer.h"

bool userTimerRunning = false;
unsigned long userTimerStart = 0;
unsigned long userTimerElapsed = 0;

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


void getTimeHMS(TimeHMS &t) {
    unsigned long totalSeconds;
    if (userTimerRunning) {
        totalSeconds = (millis() - userTimerStart) / 1000;
    } else {
        totalSeconds = userTimerElapsed / 1000;
    }

    t.hours = totalSeconds / 3600;
    t.minutes = (totalSeconds % 3600) / 60;
    t.seconds = totalSeconds % 60;
}
