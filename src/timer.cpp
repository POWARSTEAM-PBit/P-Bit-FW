#include <Arduino.h>
#include "timer.h"

bool userTimerRunning = false;
unsigned long userTimerStart = 0;
unsigned long userTimerElapsed = 0;

volatile bool g_timer_just_reset = false;

// Formato M:SS:CC — máximo "9999:59:99" = 10 chars + '\0' = 11 bytes. 12 con margen.
constexpr size_t TIME_STR_MAX = 12;

void startUserTimer() {
    if (userTimerElapsed > 0) {
        userTimerStart = millis() - userTimerElapsed; 
    } else {
        userTimerStart = millis();
    }
    userTimerElapsed = 0;
    userTimerRunning = true;
    g_timer_just_reset = false; 
    Serial.println("[Timer] Started");
}

void stopUserTimer() {
    if (userTimerRunning) {
        userTimerElapsed = millis() - userTimerStart;
        userTimerRunning = false;
        Serial.printf("[Timer] Stopped. Elapsed: %.2f s\n", userTimerElapsed / 1000.0);
    }
}

/**
 * @brief Resets the user timer to zero.
 */
void resetUserTimer() {
    userTimerRunning = false;
    userTimerStart = 0;
    userTimerElapsed = 0;
    
    g_timer_just_reset = true;
    
    Serial.println("[Timer] Reset");
}


/**
 * @brief Returns the current timer value as a formatted string (M:SS:CC).
 */
char * getTimeHMS() {
    static char timeStr[TIME_STR_MAX];
    unsigned long totalMilliseconds;

    if (userTimerRunning) {
        totalMilliseconds = millis() - userTimerStart;
    } else {
        totalMilliseconds = userTimerElapsed;
    }
    
    // 1. Calcular Minutos, Segundos, Centésimas
    unsigned int minutes = (totalMilliseconds / 60000);
    unsigned int seconds = (totalMilliseconds / 1000) % 60;
    unsigned int centiseconds = (totalMilliseconds % 1000) / 10; 

    // 2. Formatear la cadena: M:SS:CC
    snprintf(timeStr, TIME_STR_MAX, "%u:%.02u:%.02u", minutes, seconds, centiseconds);
    
    return timeStr;
}