#pragma once

struct TimeHMS {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
};

extern bool userTimerRunning;
extern unsigned long userTimerStart;
extern unsigned long userTimerElapsed;


/**
 * @brief Start the user timer.
 */
void startUserTimer();

/**
 * @brief Stop the user timer and record the elapsed time.
 */
void stopUserTimer();


void getTimeHMS(TimeHMS &t);