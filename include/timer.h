#pragma once

extern bool userTimerRunning;
extern unsigned long userTimerStart;
extern unsigned long userTimerElapsed;
extern volatile bool g_timer_just_finished;

/**
 * Volatile flag used by the UI to force a redraw right after a timer reset.
 */
extern volatile bool g_timer_just_reset;

/**
 * Fields used by the timer duration editor.
 */
enum TimerMenuField : uint8_t {
    TIMER_FIELD_HOURS = 0,
    TIMER_FIELD_MINUTES = 1,
    TIMER_FIELD_SECONDS = 2
};

/**
 * Timer duration editor state. A duration of 00:00:00 means stopwatch mode.
 */
bool timer_menu_is_active();
bool timer_menu_is_editing();
void startTimerMenu();
void cancelTimerMenu();
void confirmTimerMenu();
void handleTimerMenuButton();
int getTimerMenuEncoderMin();
int getTimerMenuEncoderMax();
int getTimerMenuEncoderValue();
void setTimerMenuEncoderValue(int value);
TimerMenuField getTimerMenuSelectedField();
int getTimerMenuHours();
int getTimerMenuMinutes();
int getTimerMenuSeconds();

/**
 * Start the user timer.
 */
void startUserTimer();

/**
 * Stop the user timer and preserve the elapsed time.
 */
void stopUserTimer();

/**
 * Reset the user timer to zero.
 */
void resetUserTimer();

/**
 * Return the active timer duration in whole seconds.
 */
unsigned long getTimerDurationSeconds();

/**
 * Run lightweight timer housekeeping from the main loop.
 */
void serviceUserTimer();

/**
 * Return the active timer duration as HH:MM:SS.
 */
const char* getTimerDurationLabel();

/**
 * Return the active timer duration in milliseconds.
 */
unsigned long getTimerPresetMs();

/**
 * Return whether the live timer value currently uses centiseconds.
 */
bool timer_display_uses_centiseconds();

/**
 * Return the elapsed time in adaptive format:
 * MM:SS:CC below 1 hour, HH:MM:SS at 1 hour or above.
 */
char * getTimeHMS();
