#pragma once
#include <Arduino.h>
#include "io.h"

// --- Screen enum ---
// Keep this in sync with the router and the screen modules.
enum Screen {
    BOOT_SCREEN,
    TEMP_SCREEN,
    HUMIDITY_SCREEN,
    LIGHT_SCREEN,
    SOUND_SCREEN,
    SOIL_SCREEN,
    DS18B20_SCREEN,
    SYSTEM_SCREEN,
    TIMER_SCREEN
};

enum UiOverlayState {
    UI_OVERLAY_NONE,
    UI_OVERLAY_SLEEP_WARNING,
    UI_OVERLAY_RESTARTING,
    UI_OVERLAY_BLACKOUT
};

// POWER_IDLE is the visible idle state: the UI is frozen on "ZZZ" while
// sensors and BLE remain available.
enum PowerMode {
    POWER_ACTIVE,
    POWER_IDLE
};

extern Screen active_screen;
extern Reading g_ui_readings_snapshot;
extern volatile UiOverlayState g_ui_overlay_state;
extern volatile bool g_ui_force_full_redraw;
extern volatile Screen g_last_active_screen_before_sleep;
extern volatile PowerMode g_power_mode;

// --- Main UI task ---
void init_tft_display();
void switch_screen(void *param);
