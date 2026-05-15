#pragma once
#include <Arduino.h>
#include "config.h"
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
    TIMER_SCREEN,
    GRAPH_SCREEN
#if PBIT_ENABLE_GRAPH_LAB
    ,
    LAB_DASH_OVERVIEW_SCREEN,
    LAB_SENSOR_FOCUS_SCREEN,
    LAB_DUAL_TH_SCREEN,
    LAB_ICON_SET_A_SCREEN,
    LAB_ICON_SET_B_SCREEN,
    LAB_ICON_SET_C_SCREEN,
    LAB_GAUGE_TEMP_SCREEN,
    LAB_VALUE_MODERN_SCREEN,
    LAB_SENSOR_CARD_SCREEN,
    LAB_TEMP_CARD_SCREEN,
    LAB_DS18_CARD_SCREEN,
    LAB_WIDGET_MIX_SCREEN,
    LAB_SOUND_VU_STACK_SCREEN,
    LAB_SOUND_VU_WAVE_SCREEN,
    LAB_ICON_SIZES_ENV_SCREEN,
    LAB_ICON_SIZES_EXT_SCREEN,
    LAB_HOME_CARDS_SCREEN,
    LAB_LINEAR_DASH_SCREEN,
    LAB_ICON_TEST_SCREEN
#endif
};

#if PBIT_ENABLE_GRAPH_LAB
constexpr Screen FIRST_APP_SCREEN = LAB_HOME_CARDS_SCREEN;
constexpr Screen LAST_APP_SCREEN = LAB_ICON_TEST_SCREEN;
#else
constexpr Screen FIRST_APP_SCREEN = TEMP_SCREEN;
constexpr Screen LAST_APP_SCREEN = GRAPH_SCREEN;
#endif

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
