#include <Arduino.h>
#include "timer.h"
#include "led_control.h"

extern bool g_sound_enabled;

bool userTimerRunning = false;
unsigned long userTimerStart = 0;
unsigned long userTimerElapsed = 0;
volatile bool g_timer_just_finished = false;

volatile bool g_timer_just_reset = false;

// Format either MM:SS:CC or HH:MM:SS. The longer variant still fits comfortably.
constexpr size_t TIME_STR_MAX = 12;
constexpr int TIMER_MAX_HOURS = 23;
constexpr int TIMER_MAX_MINUTES = 59;
constexpr int TIMER_MAX_SECONDS = 59;

static unsigned long g_timer_duration_seconds = 0;
static bool g_timer_menu_active = false;
static bool g_timer_menu_editing = false;
static TimerMenuField g_timer_menu_selected_field = TIMER_FIELD_MINUTES;
static int g_timer_menu_hours = 0;
static int g_timer_menu_minutes = 0;
static int g_timer_menu_seconds = 0;

static int clamp_timer_field(int value, int max_value) {
    if (value < 0) return 0;
    if (value > max_value) return max_value;
    return value;
}

static void load_duration_into_menu_fields() {
    unsigned long total_seconds = g_timer_duration_seconds;
    g_timer_menu_hours = (int)(total_seconds / 3600UL);
    total_seconds %= 3600UL;
    g_timer_menu_minutes = (int)(total_seconds / 60UL);
    g_timer_menu_seconds = (int)(total_seconds % 60UL);
}

static unsigned long pack_menu_duration_seconds() {
    return ((unsigned long)g_timer_menu_hours * 3600UL)
         + ((unsigned long)g_timer_menu_minutes * 60UL)
         + (unsigned long)g_timer_menu_seconds;
}

static bool is_countdown_mode() {
    return getTimerPresetMs() > 0;
}

static unsigned long get_visible_timer_ms() {
    if (is_countdown_mode()) {
        const unsigned long preset_ms = getTimerPresetMs();
        const unsigned long elapsed_ms = userTimerRunning ? (millis() - userTimerStart) : userTimerElapsed;
        return (elapsed_ms >= preset_ms) ? 0 : (preset_ms - elapsed_ms);
    }

    if (userTimerRunning) {
        return millis() - userTimerStart;
    }

    return userTimerElapsed;
}

static void format_time_value(unsigned long totalMilliseconds, char* out, size_t out_size) {
    if (totalMilliseconds >= 3600000UL) {
        const unsigned long totalSeconds = totalMilliseconds / 1000UL;
        const unsigned int hours = (unsigned int)(totalSeconds / 3600UL);
        const unsigned int minutes = (unsigned int)((totalSeconds / 60UL) % 60UL);
        const unsigned int seconds = (unsigned int)(totalSeconds % 60UL);
        snprintf(out, out_size, "%02u:%02u:%02u", hours, minutes, seconds);
        return;
    }

    const unsigned long totalSeconds = totalMilliseconds / 1000UL;
    const unsigned int minutes = (unsigned int)(totalSeconds / 60UL);
    const unsigned int seconds = (unsigned int)(totalSeconds % 60UL);
    const unsigned int centiseconds = (unsigned int)((totalMilliseconds % 1000UL) / 10UL);
    snprintf(out, out_size, "%02u:%02u:%02u", minutes, seconds, centiseconds);
}

bool timer_menu_is_active() {
    return g_timer_menu_active;
}

bool timer_menu_is_editing() {
    return g_timer_menu_editing;
}

void startTimerMenu() {
    if (userTimerRunning || userTimerElapsed > 0) {
        return;
    }
    g_timer_menu_active = true;
    g_timer_menu_editing = false;
    g_timer_menu_selected_field = TIMER_FIELD_MINUTES;
    load_duration_into_menu_fields();
}

void cancelTimerMenu() {
    g_timer_menu_active = false;
    g_timer_menu_editing = false;
}

void confirmTimerMenu() {
    g_timer_duration_seconds = pack_menu_duration_seconds();
    g_timer_menu_active = false;
    g_timer_menu_editing = false;
    g_timer_just_reset = true;
    g_timer_just_finished = false;
    Serial.printf("[Timer] Duration -> %lus\n", g_timer_duration_seconds);
}

void handleTimerMenuButton() {
    if (!g_timer_menu_active) {
        return;
    }
    g_timer_menu_editing = !g_timer_menu_editing;
}

int getTimerMenuEncoderMin() {
    if (!g_timer_menu_editing) {
        return TIMER_FIELD_HOURS;
    }
    switch (g_timer_menu_selected_field) {
        case TIMER_FIELD_HOURS: return 0;
        case TIMER_FIELD_MINUTES: return 0;
        case TIMER_FIELD_SECONDS: return 0;
        default: return 0;
    }
}

int getTimerMenuEncoderMax() {
    if (!g_timer_menu_editing) {
        return TIMER_FIELD_SECONDS;
    }
    switch (g_timer_menu_selected_field) {
        case TIMER_FIELD_HOURS: return TIMER_MAX_HOURS;
        case TIMER_FIELD_MINUTES: return TIMER_MAX_MINUTES;
        case TIMER_FIELD_SECONDS: return TIMER_MAX_SECONDS;
        default: return TIMER_MAX_SECONDS;
    }
}

int getTimerMenuEncoderValue() {
    if (!g_timer_menu_editing) {
        return (int)g_timer_menu_selected_field;
    }
    switch (g_timer_menu_selected_field) {
        case TIMER_FIELD_HOURS: return g_timer_menu_hours;
        case TIMER_FIELD_MINUTES: return g_timer_menu_minutes;
        case TIMER_FIELD_SECONDS: return g_timer_menu_seconds;
        default: return 0;
    }
}

void setTimerMenuEncoderValue(int value) {
    if (!g_timer_menu_editing) {
        g_timer_menu_selected_field = (TimerMenuField)clamp_timer_field(value, TIMER_FIELD_SECONDS);
        return;
    }

    switch (g_timer_menu_selected_field) {
        case TIMER_FIELD_HOURS:
            g_timer_menu_hours = clamp_timer_field(value, TIMER_MAX_HOURS);
            break;
        case TIMER_FIELD_MINUTES:
            g_timer_menu_minutes = clamp_timer_field(value, TIMER_MAX_MINUTES);
            break;
        case TIMER_FIELD_SECONDS:
            g_timer_menu_seconds = clamp_timer_field(value, TIMER_MAX_SECONDS);
            break;
        default:
            break;
    }
}

TimerMenuField getTimerMenuSelectedField() {
    return g_timer_menu_selected_field;
}

int getTimerMenuHours() {
    return g_timer_menu_hours;
}

int getTimerMenuMinutes() {
    return g_timer_menu_minutes;
}

int getTimerMenuSeconds() {
    return g_timer_menu_seconds;
}

void startUserTimer() {
    if (is_countdown_mode() && userTimerElapsed >= getTimerPresetMs()) {
        userTimerElapsed = 0;
        userTimerStart = millis();
    } else if (userTimerElapsed > 0) {
        userTimerStart = millis() - userTimerElapsed; 
    } else {
        userTimerStart = millis();
    }
    if (!is_countdown_mode()) {
        userTimerElapsed = 0;
    }
    userTimerRunning = true;
    g_timer_just_reset = false; 
    g_timer_just_finished = false;
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
 * Reset the user timer to zero and mark the UI for a visual refresh.
 */
void resetUserTimer() {
    userTimerRunning = false;
    userTimerStart = 0;
    userTimerElapsed = 0;
    
    g_timer_just_reset = true;
    g_timer_just_finished = false;
    
    Serial.println("[Timer] Reset");
}

void serviceUserTimer() {
    if (!userTimerRunning || !is_countdown_mode()) {
        return;
    }

    const unsigned long preset_ms = getTimerPresetMs();
    const unsigned long elapsed_ms = millis() - userTimerStart;
    if (elapsed_ms < preset_ms) {
        return;
    }

    userTimerElapsed = preset_ms;
    userTimerRunning = false;
    g_timer_just_finished = true;
    g_timer_just_reset = false;
    if (preset_ms > 0 && g_sound_enabled) {
        static const ToneStep timer_alarm_steps[] = {
            { 2200, 180 }, { 0, 140 },
            { 2200, 180 }, { 0, 140 },
            { 1800, 180 }, { 0, 140 },
            { 1800, 180 }, { 0, 140 },
            { 2200, 180 }, { 0, 140 },
            { 1800, 180 }, { 0, 140 },
        };
        play_tone_sequence(timer_alarm_steps, sizeof(timer_alarm_steps) / sizeof(timer_alarm_steps[0]));
    }
    Serial.println("[Timer] Countdown finished");
}

unsigned long getTimerDurationSeconds() {
    return g_timer_duration_seconds;
}

const char* getTimerDurationLabel() {
    static char label[12];
    const unsigned long total_seconds = g_timer_duration_seconds;
    const unsigned int hours = (unsigned int)(total_seconds / 3600UL);
    const unsigned int minutes = (unsigned int)((total_seconds / 60UL) % 60UL);
    const unsigned int seconds = (unsigned int)(total_seconds % 60UL);
    snprintf(label, sizeof(label), "%02u:%02u:%02u", hours, minutes, seconds);
    return label;
}

unsigned long getTimerPresetMs() {
    return g_timer_duration_seconds * 1000UL;
}

bool timer_display_uses_centiseconds() {
    return get_visible_timer_ms() < 3600000UL;
}

/**
 * Return the current timer value in adaptive format.
 */
char * getTimeHMS() {
    static char timeStr[TIME_STR_MAX];
    format_time_value(get_visible_timer_ms(), timeStr, TIME_STR_MAX);
    return timeStr;
}
