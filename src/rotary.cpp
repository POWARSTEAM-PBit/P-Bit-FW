// rotary.cpp
// Rotary encoder navigation, button state machine, and context switching.
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include <esp_system.h>
#include <esp_timer.h>
#include "rotary.h"
#include "config.h"
#include "tft_display.h"
#include "timer.h"
#include "led_control.h"
#include "hw.h"
#include "io.h"
#include "ui_soil.h"
#include "ui_temp.h"
#include "ui_humidity.h"
#include "ui_ds18.h"
#include "ui_sound.h"
#include "ui_light.h"
#include "ui_system.h"
#include "ui_graph.h"
#if PBIT_ENABLE_GRAPH_LAB
#include "ui_lab_focus.h"
#endif
#include "runtime_events.h"

// External state shared with the rest of the firmware.
extern volatile unsigned long g_last_activity_ms;
extern bool g_is_fahrenheit;
extern bool g_sound_enabled;

constexpr uint8_t ENCODER_STEPS_PER_DETENT = 2;
constexpr unsigned long BUTTON_DEBOUNCE_MS = 30;
constexpr unsigned long MENU_LONG_PRESS_MS = 1200;
constexpr unsigned long TIMER_RESET_LONG_PRESS_MS = 1000;

extern RotaryEncoder rotaryEncoder;

namespace {

bool g_button_raw_pressed = false;
bool g_button_debounced_pressed = false;
unsigned long g_button_last_change_ms = 0;
unsigned long g_button_press_start_ms = 0;
bool g_button_long_press_handled = false;

bool g_deferred_beep_active = false;
unsigned long g_deferred_beep_due_ms = 0;
int g_deferred_beep_freq_hz = 0;
int g_deferred_beep_duration_ms = 0;

#if PBIT_ENABLE_GRAPH_LAB
static bool isHiddenLabScreen(Screen screen) {
    switch (screen) {
        case LAB_ICON_SET_A_SCREEN:
        case LAB_ICON_SET_B_SCREEN:
        case LAB_ICON_SET_C_SCREEN:
        case LAB_ICON_SIZES_ENV_SCREEN:
        case LAB_ICON_SIZES_EXT_SCREEN:
            return true;
        default:
            return false;
    }
}

static Screen stepAppScreen(Screen screen, int direction) {
    int next = (int)screen + direction;
    if (next > (int)LAST_APP_SCREEN) next = (int)TEMP_SCREEN;
    if (next < (int)TEMP_SCREEN) next = (int)LAST_APP_SCREEN;
    return static_cast<Screen>(next);
}

static Screen resolveVisibleAppScreen(Screen screen, int direction) {
    Screen candidate = screen;
    while (isHiddenLabScreen(candidate)) {
        candidate = stepAppScreen(candidate, direction);
    }
    return candidate;
}
#endif

static bool is_button_pressed_raw() {
    return digitalRead((uint8_t)DI_ENCODER_SW) == LOW;
}

static void schedule_deferred_beep(unsigned long due_ms, int freq_hz, int duration_ms) {
    g_deferred_beep_active = true;
    g_deferred_beep_due_ms = due_ms;
    g_deferred_beep_freq_hz = freq_hz;
    g_deferred_beep_duration_ms = duration_ms;
}

static void service_deferred_beep(unsigned long now_ms_value) {
    if (!g_deferred_beep_active) return;
    if (now_ms_value < g_deferred_beep_due_ms) return;

    g_deferred_beep_active = false;
    if (g_sound_enabled) {
        beep(g_deferred_beep_freq_hz, g_deferred_beep_duration_ms);
    }
}

} // namespace

static void configure_app_rotary_bounds() {
    rotaryEncoder.setBoundaries(TEMP_SCREEN, LAST_APP_SCREEN, true);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue((int)active_screen);
}

static void configure_soil_ui_rotary_bounds() {
    SoilCalibrationState state = getSoilCalibrationState();
    bool circular = (state == SOIL_CAL_MENU || state == SOIL_CAL_EDIT_ALERTS || state == SOIL_CAL_RESET_CONFIRM);
    rotaryEncoder.setBoundaries(getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(getSoilCalibrationEncoderValue());
}

static void configure_humidity_ui_rotary_bounds() {
    HumidityMenuState state = get_humidity_menu_state();
    bool circular = (state == HUM_MODE_MENU
                  || state == HUM_MODE_EDIT_ALERTS
                  || state == HUM_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_humidity_encoder_min(), get_humidity_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_humidity_encoder_value());
}

static void configure_temp_ui_rotary_bounds() {
    TempMenuState state = get_temp_menu_state();
    bool circular = (state == TEMP_MODE_MENU
                  || state == TEMP_MODE_EDIT_UNIT
                  || state == TEMP_MODE_EDIT_ALERTS
                  || state == TEMP_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_temp_encoder_min(), get_temp_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_temp_encoder_value());
}

static void configure_ds18_ui_rotary_bounds() {
    Ds18MenuState state = get_ds18_menu_state();
    bool circular = (state == DS18_MODE_MENU
                  || state == DS18_MODE_EDIT_UNIT
                  || state == DS18_MODE_EDIT_ALERTS
                  || state == DS18_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_ds18_encoder_min(), get_ds18_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_ds18_encoder_value());
}

static void configure_sound_ui_rotary_bounds() {
    SoundMenuState state = get_sound_menu_state();
    bool circular = (state == SOUND_MODE_MENU
                  || state == SOUND_MODE_EDIT_ALERTS
                  || state == SOUND_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_sound_encoder_min(), get_sound_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_sound_encoder_value());
}

static void configure_light_ui_rotary_bounds() {
    LightMenuState state = get_light_menu_state();
    bool circular = (state == LIGHT_MODE_MENU
                  || state == LIGHT_MODE_EDIT_DISPLAY
                  || state == LIGHT_MODE_EDIT_ALERTS
                  || state == LIGHT_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_light_encoder_min(), get_light_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_light_encoder_value());
}

static void configure_system_ui_rotary_bounds() {
    SysMenuState state = get_system_menu_state();
    bool circular = (state == SYS_MODE_MENU
                  || state == SYS_MODE_EDIT_SOUND
                  || state == SYS_MODE_EDIT_SLEEP
                  || state == SYS_MODE_EDIT_LANG
                  || state == SYS_MODE_CONFIRM_RESET);
    rotaryEncoder.setBoundaries(get_system_encoder_min(), get_system_encoder_max(), circular);
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(get_system_encoder_value());
}

static void configure_timer_ui_rotary_bounds() {
    rotaryEncoder.setBoundaries(getTimerMenuEncoderMin(), getTimerMenuEncoderMax(), !timer_menu_is_editing());
    rotaryEncoder.setStepValue(1);
    rotaryEncoder.setEncoderValue(getTimerMenuEncoderValue());
}

static void play_double_beep(int first_hz, int second_hz) {
    if (!g_sound_enabled) return;
    beep(first_hz, 45);
    schedule_deferred_beep(millis() + 80UL, second_hz, 55);
}

static void play_soil_nav_beep() {
    if (!g_sound_enabled) return;
    beep(950, 12);
}

static void play_soil_confirm_beep() {
    if (!g_sound_enabled) return;
    beep(1250, 28);
}

static bool isRestorableScreen(Screen screen) {
    return screen >= TEMP_SCREEN && screen <= LAST_APP_SCREEN;
}

static unsigned long now_ms() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}


RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, -1, DO_ENCODER_VCC, ENCODER_STEPS_PER_DETENT);

/**
 * Restore the visible app context when the user wakes the device from IDLE.
 */
static void exitIdleModeIfNeeded() {
    if (g_power_mode == POWER_IDLE) {
        Serial.println("[Power] Leaving IDLE mode.");

        g_power_mode = POWER_ACTIVE;
        runtime_set_ui_overlay(UI_OVERLAY_NONE);

        Screen restored_screen = runtime_get_last_active_screen_before_sleep();
        if (isRestorableScreen(restored_screen)) {
            active_screen = restored_screen;
        } else {
            active_screen = TEMP_SCREEN;
        }

        runtime_mark_sensor_data_ready();
        runtime_request_ui_full_redraw();
        g_last_activity_ms = now_ms();
    }
}


/**
 * Handle rotary turns. Outside menus it switches screens; inside menus it edits
 * the active setting and emits a light click only when the value actually changes.
 */
void knobCallback(long value) {
    g_last_activity_ms = now_ms(); 
    exitIdleModeIfNeeded();

    if (active_screen == SOIL_SCREEN && soilCalibrationIsActive()) {
        int previous = getSoilCalibrationEncoderValue();
        setSoilCalibrationInputValue((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }

    if (active_screen == TEMP_SCREEN && temp_menu_is_active()) {
        int previous = get_temp_encoder_value();
        set_temp_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }
    
    if (active_screen == HUMIDITY_SCREEN && humidity_menu_is_active()) {
        int previous = get_humidity_encoder_value();
        set_humidity_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep(); // Reuse the lightweight navigation tick.
        }
        return;
    }
    
    if (active_screen == DS18B20_SCREEN && ds18_menu_is_active()) {
        int previous = get_ds18_encoder_value();
        set_ds18_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }

    if (active_screen == SOUND_SCREEN && sound_menu_is_active()) {
        int previous = get_sound_encoder_value();
        set_sound_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }

    if (active_screen == LIGHT_SCREEN && light_menu_is_active()) {
        int previous = get_light_encoder_value();
        set_light_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }

    if (active_screen == SYSTEM_SCREEN && system_menu_is_active()) {
        int previous = get_system_encoder_value();
        set_system_input_value((int)value);
        if ((int)value != previous) {
            play_soil_nav_beep();
        }
        return;
    }

    if (active_screen == TIMER_SCREEN && timer_menu_is_active()) {
        int previous = getTimerMenuEncoderValue();
        setTimerMenuEncoderValue((int)value);
        if (getTimerMenuEncoderValue() != previous) {
            runtime_request_ui_full_redraw();
            play_soil_nav_beep();
        }
        return;
    }

    if (value < TEMP_SCREEN || value > LAST_APP_SCREEN) return;

    Screen requested_screen = static_cast<Screen>(value);
#if PBIT_ENABLE_GRAPH_LAB
    int direction = 1;
    if (active_screen == LAST_APP_SCREEN && requested_screen == TEMP_SCREEN) {
        direction = 1;
    } else if (active_screen == TEMP_SCREEN && requested_screen == LAST_APP_SCREEN) {
        direction = -1;
    } else if ((int)requested_screen < (int)active_screen) {
        direction = -1;
    }

    requested_screen = resolveVisibleAppScreen(requested_screen, direction);
    if (requested_screen != static_cast<Screen>(value)) {
        rotaryEncoder.setEncoderValue((int)requested_screen);
    }
#endif

    active_screen = requested_screen;
    DPRINT("[Rotary] Switched to screen %d\n", (int)active_screen);

    // Gentle click when changing the active screen.
    if (g_sound_enabled) {
        beep(800, 15); 
    }

    // Screen-level RGB feedback. Light keeps the LED off to avoid polluting the LDR.
    switch (active_screen) {
        case TEMP_SCREEN:
            set_rgb(255, 69, 0); 
            break;
        case HUMIDITY_SCREEN:
            set_rgb(0, 0, 255); 
            break;
        case LIGHT_SCREEN:
            set_rgb(0, 0, 0); // LED apagado: evita incidencia en el LDR
            break;
        case SOUND_SCREEN:
            set_rgb(255, 0, 255); 
            break;
        case SOIL_SCREEN:
            set_rgb(180, 80, 0); 
            break;
        case DS18B20_SCREEN:
            set_rgb(255, 255, 255); 
            break;
        case SYSTEM_SCREEN:
            set_rgb(0, 255, 0); 
            break;
        case TIMER_SCREEN:
            if (userTimerRunning) {
                set_rgb(0, 255, 0);
            } else if (userTimerElapsed > 0) {
                set_rgb(255, 200, 0);
            } else {
                set_rgb(0, 0, 255);
            }
            break;
        case GRAPH_SCREEN:
            set_rgb(0, 80, 80); // Teal neutro para la pantalla de gráfica
            break;
#if PBIT_ENABLE_GRAPH_LAB
        case LAB_DASH_OVERVIEW_SCREEN:
            set_rgb(90, 90, 140);
            break;
        case LAB_SENSOR_FOCUS_SCREEN:
            set_rgb(0, 110, 130);
            break;
        case LAB_DUAL_TH_SCREEN:
            set_rgb(0, 140, 180);
            break;
        case LAB_ICON_SET_A_SCREEN:
            set_rgb(180, 80, 255);
            break;
        case LAB_ICON_SET_B_SCREEN:
            set_rgb(80, 180, 255);
            break;
        case LAB_ICON_SET_C_SCREEN:
            set_rgb(255, 120, 80);
            break;
        case LAB_GAUGE_TEMP_SCREEN:
            set_rgb(255, 140, 0);
            break;
        case LAB_VALUE_MODERN_SCREEN:
            set_rgb(255, 0, 180);
            break;
        case LAB_WIDGET_MIX_SCREEN:
            set_rgb(0, 200, 255);
            break;
#endif
        default:
            set_rgb(0, 0, 0);
            break;
    }
}

static bool handle_button_long_press() {
    if (active_screen == SYSTEM_SCREEN && !system_menu_is_active()) {
        start_system_menu();
        configure_system_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == TEMP_SCREEN && !temp_menu_is_active()) {
        start_temp_menu();
        configure_temp_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == SOIL_SCREEN && !soilCalibrationIsActive()) {
        startSoilCalibration();
        configure_soil_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == HUMIDITY_SCREEN && !humidity_menu_is_active()) {
        start_humidity_menu();
        configure_humidity_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == DS18B20_SCREEN && !ds18_menu_is_active()) {
        start_ds18_menu();
        configure_ds18_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == SOUND_SCREEN && !sound_menu_is_active()) {
        start_sound_menu();
        configure_sound_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == LIGHT_SCREEN && !light_menu_is_active()) {
        start_light_menu();
        configure_light_ui_rotary_bounds();
        play_double_beep(1200, 1600);
        return true;
    }

    if (active_screen == TIMER_SCREEN) {
        if (timer_menu_is_active()) {
            if (!timer_menu_is_editing()) {
                confirmTimerMenu();
                runtime_request_ui_full_redraw();
                if (g_sound_enabled) {
                    play_double_beep(1300, 1700);
                }
                configure_app_rotary_bounds();
            }
        } else if (userTimerRunning || userTimerElapsed > 0) {
            if (g_sound_enabled) {
                beep(2000, 100);
            }
            resetUserTimer();
            set_rgb(0, 0, 255);
        } else {
            startTimerMenu();
            configure_timer_ui_rotary_bounds();
            runtime_request_ui_full_redraw();
            play_double_beep(1200, 1600);
        }
        return true;
    }

    return false;
}

/**
 * Handle button release for short presses and menu confirmation.
 * Long-press actions are handled while the button is still held down.
 */
void buttonCallback(unsigned long duration) {
    g_last_activity_ms = now_ms(); 
    exitIdleModeIfNeeded();

    if (active_screen == SOIL_SCREEN && soilCalibrationIsActive()) {
        SoilCalibrationState previous_state = getSoilCalibrationState();
        uint8_t next_state = handleSoilCalibrationButton();
        if ((next_state == SOIL_CAL_WAIT_DRY || next_state == SOIL_CAL_THRESH_DRY) && g_sound_enabled) {
            play_soil_confirm_beep();
        }
        if (next_state == SOIL_CAL_WAIT_WET && g_sound_enabled) {
            beep(1350, 45);
        }
        if ((next_state == SOIL_CAL_THRESH_OPTIMAL || next_state == SOIL_CAL_THRESH_MOIST || next_state == SOIL_CAL_EDIT_ALERTS) && g_sound_enabled) {
            beep(1450, 35);
        }
        if (next_state == SOIL_CAL_DONE || next_state == SOIL_CAL_THRESH_DONE || next_state == SOIL_CAL_ALERTS_DONE || next_state == SOIL_CAL_ERROR) {
            play_double_beep(1300, 1700);
        }
        if (next_state == SOIL_CAL_IDLE && previous_state == SOIL_CAL_MENU && g_sound_enabled) {
            beep(900, 22);
        }
        if (next_state == SOIL_CAL_IDLE) {
            configure_app_rotary_bounds();
        } else {
            configure_soil_ui_rotary_bounds();
        }
        return;
    }

    if (active_screen == TEMP_SCREEN && temp_menu_is_active()) {
        uint8_t next_state = handle_temp_button();
        if (next_state >= TEMP_MODE_EDIT_LOW && next_state <= TEMP_MODE_EDIT_ALERTS) {
            if (g_sound_enabled) play_soil_confirm_beep();
        }
        if (next_state == TEMP_MODE_SAVED) {
            play_double_beep(1300, 1700);
        }
        if (next_state == TEMP_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_temp_ui_rotary_bounds();
        }
        return;
    }
    
    if (active_screen == HUMIDITY_SCREEN && humidity_menu_is_active()) {
        uint8_t next_state = handle_humidity_button();
        if (next_state == HUM_MODE_EDIT_DRY || next_state == HUM_MODE_EDIT_COMFORT) {
            if (g_sound_enabled) play_soil_confirm_beep();
        }
        if (next_state == HUM_MODE_SAVED) {
            play_double_beep(1300, 1700);
        }
        if (next_state == HUM_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_humidity_ui_rotary_bounds();
        }
        return;
    }
    
    if (active_screen == DS18B20_SCREEN && ds18_menu_is_active()) {
        uint8_t next_state = handle_ds18_button();
        if (next_state >= DS18_MODE_EDIT_OFFSET && next_state <= DS18_MODE_EDIT_ALERTS) {
            if (g_sound_enabled) play_soil_confirm_beep();
        }
        if (next_state == DS18_MODE_SAVED) {
            play_double_beep(1300, 1700);
        }
        if (next_state == DS18_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_ds18_ui_rotary_bounds();
        }
        return;
    }

    if (active_screen == SOUND_SCREEN && sound_menu_is_active()) {
        uint8_t next_state = handle_sound_button();
        if (next_state >= SOUND_MODE_EDIT_QUIET && next_state <= SOUND_MODE_EDIT_ALERTS) {
            if (g_sound_enabled) play_soil_confirm_beep();
        }
        if (next_state == SOUND_MODE_SAVED) {
            play_double_beep(1300, 1700);
        }
        if (next_state == SOUND_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_sound_ui_rotary_bounds();
        }
        return;
    }

    if (active_screen == LIGHT_SCREEN && light_menu_is_active()) {
        uint8_t next_state = handle_light_button();
        if (next_state >= LIGHT_MODE_EDIT_DIM && next_state <= LIGHT_MODE_EDIT_ALERTS) {
            if (g_sound_enabled) play_soil_confirm_beep();
        }
        if (next_state == LIGHT_MODE_SAVED) {
            play_double_beep(1300, 1700);
        }
        if (next_state == LIGHT_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_light_ui_rotary_bounds();
        }
        return;
    }

    if (active_screen == SYSTEM_SCREEN && system_menu_is_active()) {
        uint8_t next_state = handle_system_button();
        if (next_state != SYS_MODE_NORMAL && g_sound_enabled) {
            play_soil_confirm_beep();
        }
        if (next_state == SYS_MODE_NORMAL) {
            if (g_sound_enabled) beep(900, 22);
            configure_app_rotary_bounds();
        } else {
            configure_system_ui_rotary_bounds();
        }
        return;
    }

    if (active_screen == TIMER_SCREEN && timer_menu_is_active()) {
        handleTimerMenuButton();
        runtime_request_ui_full_redraw();
        if (g_sound_enabled) {
            play_soil_confirm_beep();
        }
        configure_timer_ui_rotary_bounds();
        return;
    }

    // -----------------------------------------------------------------
    // Short-press behavior on the System screen: toggle the global sound flag.
    // -----------------------------------------------------------------
    if (active_screen == SYSTEM_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_system_menu();
            configure_system_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        } else {
            g_sound_enabled = !g_sound_enabled;
            save_sound_enabled(g_sound_enabled);
            runtime_mark_sensor_data_ready();
            if (g_sound_enabled) {
                beep(1200, 70);
            } else {
                beep(800, 70);
            }
        }
        return;
    }

    if (active_screen == TEMP_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_temp_menu();
            configure_temp_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        } else {
            g_is_fahrenheit = !g_is_fahrenheit;
            runtime_mark_sensor_data_ready();
        }
        return;
    }

    if (active_screen == SOIL_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            startSoilCalibration();
            configure_soil_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        }
        return;
    }
    
    if (active_screen == HUMIDITY_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_humidity_menu();
            configure_humidity_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        }
        return;
    }
    
    if (active_screen == DS18B20_SCREEN && !ds18_menu_is_active()) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_ds18_menu();
            configure_ds18_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        } else {
            g_is_fahrenheit = !g_is_fahrenheit;
            runtime_mark_sensor_data_ready();
        }
        return;
    }

    if (active_screen == SOUND_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_sound_menu();
            configure_sound_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        }
        return;
    }

    if (active_screen == LIGHT_SCREEN) {
        if (duration >= MENU_LONG_PRESS_MS) {
            start_light_menu();
            configure_light_ui_rotary_bounds();
            play_double_beep(1200, 1600);
        }
        return;
    }

    // -----------------------------------------------------------------
    // Timer screen behavior: short press toggles run/pause. Long press is handled
    // while the button is held so idle opens the minute selector and active time resets.
    // -----------------------------------------------------------------

    if (active_screen == TIMER_SCREEN) {
        if (g_sound_enabled) {
            beep(2000, 100); // Timer action tone.
        }

        if (!userTimerRunning) {
            startUserTimer();
            set_rgb(0, 255, 0); 
        } else {
            stopUserTimer();
            set_rgb(255, 200, 0); 
        }
        return;
    }

    // -----------------------------------------------------------------
    // Graph screen behavior: short press cycles between sensors.
    // -----------------------------------------------------------------

    if (active_screen == GRAPH_SCREEN) {
        if (duration < MENU_LONG_PRESS_MS) {
            graph_cycle_sensor();
            if (g_sound_enabled) beep(800, 15);
        }
        return;
    }

#if PBIT_ENABLE_GRAPH_LAB
    if (active_screen == LAB_SENSOR_FOCUS_SCREEN) {
        if (duration < MENU_LONG_PRESS_MS) {
            lab_focus_cycle_sensor();
            if (g_sound_enabled) beep(800, 15);
        }
        return;
    }
#endif
}

void init_rotary() {
    // The encoder switch uses an external pull-up, so we keep the library in
    // floating mode and manage the switch state ourselves in polling.
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    pinMode((uint8_t)DI_ENCODER_SW, INPUT_PULLUP);
    
    // Screen boundaries start at TEMP_SCREEN and exclude BOOT_SCREEN.
    configure_app_rotary_bounds();
    
    rotaryEncoder.onTurned(&knobCallback);
    
    // Manual polling avoids racing the library's internal timer and reduces
    // missed or double-consumed button events.
    rotaryEncoder.begin(false);
    const unsigned long now = now_ms();
    g_button_raw_pressed = is_button_pressed_raw();
    g_button_debounced_pressed = g_button_raw_pressed;
    g_button_last_change_ms = now;
    g_button_press_start_ms = g_button_raw_pressed ? now : 0;
    g_button_long_press_handled = false;
    g_deferred_beep_active = false;
    Serial.println("[Rotary] Initialized.");
}

void poll_rotary_aux() {
    const unsigned long now = now_ms();
    service_deferred_beep(now);
    serviceUserTimer();

    const bool raw_pressed = is_button_pressed_raw();
    if (raw_pressed != g_button_raw_pressed) {
        g_button_raw_pressed = raw_pressed;
        g_button_last_change_ms = now;
    }

    if ((now - g_button_last_change_ms) >= BUTTON_DEBOUNCE_MS
        && g_button_debounced_pressed != g_button_raw_pressed) {
        g_button_debounced_pressed = g_button_raw_pressed;

        if (g_button_debounced_pressed) {
            g_button_press_start_ms = now;
            g_button_long_press_handled = false;
            g_last_activity_ms = now;
            exitIdleModeIfNeeded();
        } else {
            g_last_activity_ms = now;
            if (!g_button_long_press_handled) {
                buttonCallback(now - g_button_press_start_ms);
            }
        }
    }

    if (!g_button_debounced_pressed) {
        return;
    }

    g_last_activity_ms = now;
    exitIdleModeIfNeeded();

    if (g_button_long_press_handled) {
        return;
    }

    unsigned long threshold_ms = 0;
    if (active_screen == TIMER_SCREEN) {
        if (timer_menu_is_active()) {
            threshold_ms = timer_menu_is_editing() ? 0 : MENU_LONG_PRESS_MS;
        } else {
            threshold_ms = TIMER_RESET_LONG_PRESS_MS;
        }
    } else if ((active_screen == TEMP_SCREEN && !temp_menu_is_active())
            || (active_screen == HUMIDITY_SCREEN && !humidity_menu_is_active())
            || (active_screen == LIGHT_SCREEN && !light_menu_is_active())
            || (active_screen == SOUND_SCREEN && !sound_menu_is_active())
            || (active_screen == SOIL_SCREEN && !soilCalibrationIsActive())
            || (active_screen == DS18B20_SCREEN && !ds18_menu_is_active())
            || (active_screen == SYSTEM_SCREEN && !system_menu_is_active())) {
        threshold_ms = MENU_LONG_PRESS_MS;
    }

    if (threshold_ms == 0 || (now - g_button_press_start_ms) < threshold_ms) {
        return;
    }

    if (handle_button_long_press()) {
        g_button_long_press_handled = true;
    }
}
