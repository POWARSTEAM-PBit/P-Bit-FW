// tft_display.cpp
// UI router: decides which screen to draw and manages global overlays.

#include <math.h>
#include "tft_display.h"
#include "config.h"     
#include "alert_engine.h"
#include "ui_widgets.h" 
#include "layout.h"
#include "timer.h"      
#include "hw.h"
#include "fonts.h"
#include "languages.h"
#include "palette.h"
#include "runtime_events.h"
#include "led_control.h"
#include "demo_mode.h"
#include "sensor_visuals.h"
#include <stdio.h>
#include <string.h>

// --- Screen modules ---
#include "ui_boot.h"
#include "ui_ble_toggle.h"
#include "ui_temp.h"
#include "ui_humidity.h"
#include "ui_light.h"
#include "ui_sound.h"
#include "ui_soil.h"
#include "ui_ds18.h"
#include "ui_system.h"
#include "ui_timer.h"
#include "ui_graph.h"
#if PBIT_ENABLE_GRAPH_LAB
#include "sensor_zone.h"
#include "ui_lab_dash.h"
#include "ui_lab_focus.h"
#include "ui_lab_dual.h"
#include "ui_lab_icon_gallery.h"
#include "ui_lab_sensor_cards.h"
#include "ui_lab_sound_vu.h"
#include "ui_lab_widget_showcase.h"
#include "ui_lab_icon_sizes.h"
#include "ui_lab_home_cards.h"
#include "ui_lab_linear_dash.h"
#include "ui_lab_icon_test.h"
#endif
// ----------------------------------------------------

// --- Global TFT/UI state ---
volatile Screen active_screen;
Reading g_ui_readings_snapshot;
volatile UiOverlayState g_ui_overlay_state = UI_OVERLAY_NONE;
volatile bool g_ui_force_full_redraw = false;
volatile Screen g_last_active_screen_before_sleep = TEMP_SCREEN;

// --- External state ---
extern volatile bool g_sensor_data_ready;
extern bool userTimerRunning;
extern volatile bool g_timer_just_reset;
extern bool g_is_fahrenheit;

namespace {

constexpr uint32_t kUiSensorRefreshMs = 100;
constexpr uint32_t kUiGraphRefreshMs = 1000;
constexpr float kVisualSoundDeadband = 2.0f;

static bool is_sound_vu_screen(Screen screen) {
#if PBIT_ENABLE_GRAPH_LAB
    return screen == LAB_SOUND_VU_STACK_SCREEN || screen == LAB_SOUND_VU_WAVE_SCREEN;
#else
    (void)screen;
    return false;
#endif
}

static bool is_graph_rate_limited_screen(Screen screen) {
    if (screen == GRAPH_SCREEN) return true;
#if PBIT_ENABLE_GRAPH_LAB
    if (screen == LAB_SENSOR_FOCUS_SCREEN) return true;
    if (screen == SENSOR_ZONE_SCREEN) {
        const SzVizMode viz = sz_get_viz();
        return viz == SZ_VIZ_GRAPH || viz == SZ_VIZ_FOCUS;
    }
#else
    (void)screen;
#endif
    return false;
}

static void draw_demo_start_splash() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.drawString(L(ST_DEMO_MODE), 80, 51);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(L(ST_DEMO_START), 80, 78);
    tft.setTextFont(0);
}

static bool valid_number(float value) {
    return !isnan(value) && isfinite(value);
}

static void set_rgb565(uint16_t color) {
    uint8_t r = 0, g = 0, b = 0;
    pbit_rgb565_to_rgb888(color, r, g, b);
    set_rgb(r, g, b);
}

static void set_invalid_sensor_rgb() {
    set_rgb(90, 0, 0);
}

static void apply_temp_visual_rgb() {
    bool valid = false;
    const uint16_t color = pbit_sensor_visual_color(SZ_TEMP, g_ui_readings_snapshot, g_is_fahrenheit, &valid);
    if (!valid) {
        set_invalid_sensor_rgb();
        return;
    }
    set_rgb565(color);
}

static void apply_humidity_visual_rgb() {
    bool valid = false;
    const uint16_t color = pbit_sensor_visual_color(SZ_HUM, g_ui_readings_snapshot, g_is_fahrenheit, &valid);
    if (!valid) {
        set_invalid_sensor_rgb();
        return;
    }
    set_rgb565(color);
}

static void apply_sound_visual_rgb() {
    bool valid = false;
    const uint16_t color = pbit_sensor_visual_color(SZ_SOUND, g_ui_readings_snapshot, g_is_fahrenheit, &valid);
    if (!valid) {
        set_invalid_sensor_rgb();
        return;
    }
    set_rgb565(color);
}

static void apply_soil_visual_rgb() {
    bool valid = false;
    const uint16_t color = pbit_sensor_visual_color(SZ_SOIL, g_ui_readings_snapshot, g_is_fahrenheit, &valid);
    if (!valid) {
        set_invalid_sensor_rgb();
        return;
    }
    set_rgb565(color);
}

static void apply_ds18_visual_rgb() {
    bool valid = false;
    const uint16_t color = pbit_sensor_visual_color(SZ_DS18, g_ui_readings_snapshot, g_is_fahrenheit, &valid);
    if (!valid) {
        set_invalid_sensor_rgb();
        return;
    }
    set_rgb565(color);
}

static void apply_timer_visual_rgb() {
    const bool flash = ((millis() / 250UL) & 0x01UL) == 0;
    set_rgb565(pbit_timer_visual_color(g_timer_finished_active, userTimerRunning, userTimerElapsed, flash));
}

#if PBIT_ENABLE_GRAPH_LAB
static void apply_sensor_visual_rgb(SzSensorId sensor) {
    switch (sensor) {
        case SZ_TEMP:
            apply_temp_visual_rgb();
            break;
        case SZ_HUM:
            apply_humidity_visual_rgb();
            break;
        case SZ_LIGHT:
            // Keep the RGB LED off while measuring light; otherwise it pollutes the LDR.
            set_rgb(0, 0, 0);
            break;
        case SZ_SOUND:
            apply_sound_visual_rgb();
            break;
        case SZ_SOIL:
            apply_soil_visual_rgb();
            break;
        case SZ_DS18:
            apply_ds18_visual_rgb();
            break;
        default:
            set_rgb(0, 0, 0);
            break;
    }
}

static void apply_sensor_zone_visual_rgb() {
    apply_sensor_visual_rgb(sz_get_sensor());
}

static SzSensorId focus_sensor_to_sz(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:     return SZ_TEMP;
        case LAB_FOCUS_HUMIDITY: return SZ_HUM;
        case LAB_FOCUS_LIGHT:    return SZ_LIGHT;
        case LAB_FOCUS_SOUND:    return SZ_SOUND;
        case LAB_FOCUS_SOIL:     return SZ_SOIL;
        case LAB_FOCUS_DS18:     return SZ_DS18;
        default:                 return SZ_TEMP;
    }
}
#endif

static void copy_readings_for_ui(bool force, bool raw_mic) {
    Reading next;
    portENTER_CRITICAL(&readings_mux);
    next = global_readings;
    portEXIT_CRITICAL(&readings_mux);

    static bool initialized = false;
    if (force || !initialized || raw_mic) {
        demo_mode_apply_simulated_readings(next);
        g_ui_readings_snapshot = next;
        initialized = true;
        return;
    }

    if (valid_number(next.mic) && valid_number(g_ui_readings_snapshot.mic)) {
        const float delta = fabsf(next.mic - g_ui_readings_snapshot.mic);
        if (delta < kVisualSoundDeadband) {
            next.mic = g_ui_readings_snapshot.mic;
        }
    }

    demo_mode_apply_simulated_readings(next);
    g_ui_readings_snapshot = next;
    initialized = true;
}

static void apply_global_alert_rgb(const GlobalAlertSummary& summary) {
    // On sensor/timer screens the LED mirrors the visible UI state instead of
    // a hidden global alert, so the physical RGB tells the same story as the TFT.
    switch (active_screen) {
        case TEMP_SCREEN:
            apply_temp_visual_rgb();
            return;
        case HUMIDITY_SCREEN:
            apply_humidity_visual_rgb();
            return;
        case LIGHT_SCREEN:
            set_rgb(0, 0, 0);
            return;
        case SOUND_SCREEN:
            apply_sound_visual_rgb();
            return;
        case SOIL_SCREEN:
            apply_soil_visual_rgb();
            return;
        case DS18B20_SCREEN:
            apply_ds18_visual_rgb();
            return;
        case TIMER_SCREEN:
            apply_timer_visual_rgb();
            return;
        case GRAPH_SCREEN:
#if PBIT_ENABLE_GRAPH_LAB
            apply_sensor_visual_rgb((SzSensorId)graph_get_sensor());
#else
            set_rgb(0, 80, 80);
#endif
            return;
#if PBIT_ENABLE_GRAPH_LAB
        case LAB_SENSOR_FOCUS_SCREEN:
            apply_sensor_visual_rgb(focus_sensor_to_sz(lab_focus_get_sensor()));
            return;
        case LAB_GAUGE_TEMP_SCREEN:
            apply_sensor_visual_rgb((SzSensorId)lab_gauge_get_sensor());
            return;
        case LAB_VALUE_MODERN_SCREEN:
            apply_sensor_visual_rgb((SzSensorId)lab_value_get_sensor());
            return;
        case LAB_SENSOR_CARD_SCREEN:
            apply_sensor_visual_rgb((SzSensorId)lab_sensor_card_get_sz_sensor());
            return;
        case LAB_TEMP_CARD_SCREEN:
            apply_sensor_visual_rgb(SZ_TEMP);
            return;
        case LAB_DS18_CARD_SCREEN:
            apply_sensor_visual_rgb(SZ_DS18);
            return;
        case LAB_SOUND_VU_STACK_SCREEN:
        case LAB_SOUND_VU_WAVE_SCREEN:
            apply_sensor_visual_rgb(SZ_SOUND);
            return;
        case SENSOR_ZONE_SCREEN:
            apply_sensor_zone_visual_rgb();
            return;
#endif
        default:
            break;
    }

    // Keep the RGB LED off on the light screen so the LED does not skew the LDR.
    if (active_screen == LIGHT_SCREEN) {
        set_rgb(0, 0, 0);
        return;
    }

    if (!summary.active) {
        switch (active_screen) {
            case SYSTEM_SCREEN:
                set_rgb(0, 255, 0);
                break;
            case BLE_TOGGLE_SCREEN:
                set_rgb(0, 80, 255);
                break;
            case GRAPH_SCREEN:
                set_rgb(0, 80, 80); // Teal neutro para la pantalla de gráfica
                break;
#if PBIT_ENABLE_GRAPH_LAB
            case LAB_HOME_CARDS_SCREEN:
                set_rgb(0, 150, 210);
                break;
            case LAB_LINEAR_DASH_SCREEN:
                set_rgb(0, 170, 100);
                break;
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
            case LAB_SENSOR_CARD_SCREEN:
                set_rgb(255, 130, 0);
                break;
            case LAB_TEMP_CARD_SCREEN:
                set_rgb(255, 110, 0);
                break;
            case LAB_DS18_CARD_SCREEN:
                set_rgb(255, 255, 255);
                break;
            case LAB_WIDGET_MIX_SCREEN:
                set_rgb(255, 140, 60);
                break;
            case LAB_SOUND_VU_STACK_SCREEN:
                set_rgb(0, 220, 120);
                break;
            case LAB_SOUND_VU_WAVE_SCREEN:
                set_rgb(0, 160, 255);
                break;
            case LAB_ICON_TEST_SCREEN:
                set_rgb(255, 165, 0);
                break;
#endif
            default:
                break;
        }
        return;
    }

    switch (summary.primary_sensor) {
        case AlertSensor::Temp:
        case AlertSensor::Ds18:
            if (summary.primary_code == ALERT_CODE_LOW) set_rgb(0, 90, 255);
            else set_rgb(255, 0, 0);
            break;

        case AlertSensor::Humidity:
            if (summary.primary_code == ALERT_CODE_LOW) set_rgb(255, 120, 0);
            else set_rgb(255, 0, 0);
            break;

        case AlertSensor::Light:
            if (summary.primary_code == ALERT_CODE_LOW) set_rgb(0, 180, 255);
            else set_rgb(255, 180, 0);
            break;

        case AlertSensor::Sound:
            if (summary.primary_code == ALERT_CODE_CRITICAL) set_rgb(255, 0, 0);
            else set_rgb(255, 140, 0);
            break;

        case AlertSensor::Soil:
            if (summary.primary_code == ALERT_CODE_LOW) set_rgb(255, 0, 0);
            else set_rgb(0, 0, 200);
            break;

        default:
            break;
    }
}

static void render_global_alert_badge() {
    const GlobalAlertSummary summary = alert_engine_get_global_summary();

    apply_global_alert_rgb(summary);
}

} // namespace

// Animated sleep screen — "Respira" design.
// Breathing orb at center + ZZZ centered above it, appearing left-to-right.
// The orb shifts from green when small to blue as it grows.
// Called every ~10 ms frame; first_frame=true resets all animation state.
static void draw_sleep_warning_overlay(bool first_frame) {
    static uint32_t t_start    = 0;
    static int      last_r     = -1;
    static uint8_t  z_phase    = 0;   // 0=pause, 1=z1 visible, 2=+z2, 3=+z3 hold
    static uint32_t z_phase_ms = 0;

    // The middle Z is exactly on the orb centerline (x=80).
    static constexpr int ORB_CX = 80;
    static constexpr int ORB_CY = 66;
    static constexpr int ORB_R_MIN = 7;
    static constexpr int ORB_R_RANGE = 11;
    static constexpr int ORB_R_CLEAR = ORB_R_MIN + ORB_R_RANGE + 2;
    static constexpr int ZY   = 29;         // shared vertical center for all three glyphs
    static constexpr int ZX1  = 62;
    static constexpr int ZX2  = ORB_CX;
    static constexpr int ZX3  = 100;
    static constexpr int ZCX  = 46;         // clear rect left
    static constexpr int ZCYW = 13;         // clear rect top
    static constexpr int ZCWW = 70;         // clear rect width  (46..115)
    static constexpr int ZCHH = 34;         // clear rect height (17..50)

    if (first_frame) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        // "Durmiendo" — primary status label, dim white
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(tft.color565(160, 160, 160), TFT_BLACK);
        tft.drawString(L(ST_SLEEPING), 80, 96);
        // "Pulsa para despertar" — secondary hint, very dim
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(tft.color565(55, 55, 55), TFT_BLACK);
        tft.drawString(L(ST_PUSH_TO_WAKE), 80, 113);
        tft.setTextFont(0);
        t_start    = millis();
        last_r     = -1;
        z_phase    = 0;
        z_phase_ms = millis();
    }

    const uint32_t now = millis();

    // --- Breathing orb (center 80,66, r = 7..18, period 4 s) ---
    const float t_sec = (float)(now - t_start) / 4000.0f;
    const int r = ORB_R_MIN + (int)((float)ORB_R_RANGE * 0.5f * (1.0f + sinf(t_sec * 6.2832f - 1.5708f)));
    if (r != last_r) {
        const int grow = constrain(r - ORB_R_MIN, 0, ORB_R_RANGE);
        const uint8_t red   = (uint8_t)(28 - (grow * 2));
        const uint8_t green = (uint8_t)(230 - (grow * 12));
        const uint8_t blue  = (uint8_t)(50 + (grow * 18));
        tft.fillCircle(ORB_CX, ORB_CY, ORB_R_CLEAR, TFT_BLACK);                // erase previous
        tft.fillCircle(ORB_CX, ORB_CY, r, tft.color565(red, green, blue));
        if (r >= 5) {
            tft.fillCircle(ORB_CX, ORB_CY, 3, tft.color565(80 - (grow * 5), 255 - (grow * 8), 90 + (grow * 14)));
        }
        last_r = r;
    }

    // --- ZZZ: three centered glyphs; the middle one is directly above the orb ---
    const uint32_t age = now - z_phase_ms;
    tft.setTextDatum(MC_DATUM);
    switch (z_phase) {
        case 0:  // pause between cycles
            if (age >= 500) {
                tft.fillRect(ZCX, ZCYW, ZCWW, ZCHH, TFT_BLACK);
                tft.setFreeFont(FONT_SMALL);
                tft.setTextColor(tft.color565(0, 100, 220), TFT_BLACK);  // brand blue
                tft.drawString("Z", ZX1, ZY);
                tft.setTextFont(0);
                z_phase = 1; z_phase_ms = now;
            }
            break;
        case 1:  // z1 visible — wait 700 ms then add z2
            if (age >= 700) {
                tft.setFreeFont(FONT_BODY);
                tft.setTextColor(tft.color565(0, 175, 125), TFT_BLACK);  // teal transition
                tft.drawString("Z", ZX2, ZY);
                tft.setTextFont(0);
                z_phase = 2; z_phase_ms = now;
            }
            break;
        case 2:  // z1+z2 visible — wait 700 ms then add z3
            if (age >= 700) {
                tft.setFreeFont(FONT_TIMER);
                tft.setTextColor(tft.color565(30, 220, 50), TFT_BLACK);  // vivid green
                tft.drawString("Z", ZX3, ZY);
                tft.setTextFont(0);
                z_phase = 3; z_phase_ms = now;
            }
            break;
        case 3:  // all visible — hold 1200 ms then clear and pause
            if (age >= 1200) {
                tft.fillRect(ZCX, ZCYW, ZCWW, ZCHH, TFT_BLACK);
                z_phase = 0; z_phase_ms = now;
            }
            break;
    }
}

// Overlay used while the firmware is restarting after a language change or reset.
static void draw_restarting_overlay() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont(FONT_BODY);
    tft.drawString(L(ST_RESTARTING), tft.width() / 2, tft.height() / 2);
    tft.setTextFont(0);
}

// Overlay used when the panel has been intentionally blanked.
static void draw_blackout_overlay() {
    tft.fillScreen(TFT_BLACK);
}


// --- Initialization and cleanup helpers ---

void init_tft_display() {
    tft.init();
    tft.setRotation(1); // Landscape
    tft.fillScreen(TFT_BLACK); 
}

// --- Main display task (FreeRTOS) ---

void switch_screen(void *param) {
    DPRINTLN("[Display] UI router task started on core 1.");
    
    bool screen_changed = true; 
    Screen last_drawn = BOOT_SCREEN; 
    
    unsigned long last_timer_update_ms = 0;
    unsigned long last_system_update_ms = 0;
    unsigned long last_soil_cal_update_ms = 0;
    UiOverlayState last_overlay_state = UI_OVERLAY_NONE;
    
    while (1) {
        bool timer_needs_update = false;
        bool system_needs_update = false;
        bool soil_cal_needs_update = false;
        UiOverlayState overlay_state = runtime_get_ui_overlay();

        if (overlay_state != UI_OVERLAY_NONE) {
            if (overlay_state == UI_OVERLAY_SLEEP_WARNING) {
                // Animated — called every frame; function self-limits redraws internally.
                draw_sleep_warning_overlay(last_overlay_state != UI_OVERLAY_SLEEP_WARNING);
            } else if (overlay_state != last_overlay_state) {
                switch (overlay_state) {
                    case UI_OVERLAY_RESTARTING:
                        draw_restarting_overlay();
                        break;
                    case UI_OVERLAY_BLACKOUT:
                        draw_blackout_overlay();
                        break;
                    default:
                        break;
                }
            }
            last_overlay_state = overlay_state;
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (last_overlay_state != UI_OVERLAY_NONE) {
            runtime_request_ui_full_redraw();
        }
        last_overlay_state = UI_OVERLAY_NONE;

        bool force_redraw = runtime_take_ui_full_redraw();
        bool sensor_data_changed = runtime_take_sensor_data_ready();
        const bool demo_splash_active = demo_mode_splash_active();
        const bool demo_values_active = demo_mode_is_active() && !demo_splash_active;
        static bool last_demo_splash_active = false;
        const bool raw_mic_screen = is_sound_vu_screen(active_screen);
        const bool graph_limited_screen = is_graph_rate_limited_screen(active_screen);
        static unsigned long last_ui_sensor_update_ms = 0;
        static unsigned long last_ui_graph_update_ms = 0;
        const unsigned long now_ms = millis();

        if (demo_values_active) {
            sensor_data_changed = true;
        }

        if (sensor_data_changed && (!raw_mic_screen || demo_values_active)) {
            const uint32_t interval_ms = graph_limited_screen ? kUiGraphRefreshMs : kUiSensorRefreshMs;
            unsigned long& last_update_ms = graph_limited_screen
                ? last_ui_graph_update_ms
                : last_ui_sensor_update_ms;
            if ((now_ms - last_update_ms) < interval_ms) {
                sensor_data_changed = false;
            } else {
                last_update_ms = now_ms;
            }
        }

        const unsigned long timer_refresh_ms = timer_display_uses_centiseconds() ? 40UL : 100UL;
        if (now_ms - last_timer_update_ms >= timer_refresh_ms) {
            timer_needs_update = true;
            last_timer_update_ms = now_ms;
        }

        if (active_screen == SYSTEM_SCREEN && now_ms - last_system_update_ms >= 100) {
            system_needs_update = true;
            last_system_update_ms = now_ms;
        }

        if (active_screen == SOIL_SCREEN && soilCalibrationIsActive() && now_ms - last_soil_cal_update_ms >= 180) {
            soil_cal_needs_update = true;
            last_soil_cal_update_ms = now_ms;
        }

        if (last_drawn != active_screen) {
            screen_changed = true;
            if (active_screen != BOOT_SCREEN) sensor_data_changed = true; 
        } else {
            screen_changed = false;
        }
        if (force_redraw) {
            screen_changed = true;
            sensor_data_changed = true;
        }
        if (demo_splash_active && !last_demo_splash_active) {
            screen_changed = true;
        }
        if (!demo_splash_active && last_demo_splash_active) {
            screen_changed = true;
            sensor_data_changed = true;
        }

        if (g_timer_just_reset) timer_needs_update = true;

        // ------------------------------------------------------------------
        // Main drawing logic.
        // ------------------------------------------------------------------
        
        if (screen_changed
            || sensor_data_changed
            || demo_splash_active
            || (active_screen == SOIL_SCREEN && soil_cal_needs_update)
            || (active_screen == TIMER_SCREEN && (timer_needs_update || g_timer_just_reset))
            || (active_screen == SYSTEM_SCREEN && system_needs_update)) {
            if (demo_splash_active) {
                if (screen_changed || !last_demo_splash_active) {
                    draw_demo_start_splash();
                }
                last_demo_splash_active = true;
                vTaskDelay(pdMS_TO_TICKS(5));
                continue;
            }
            last_demo_splash_active = false;

            if (screen_changed) {
                last_drawn = active_screen;
            }

            if (sensor_data_changed || screen_changed) {
                copy_readings_for_ui(screen_changed, raw_mic_screen);
            }
            
            // --- ENRUTADOR DE UI ---
            switch (active_screen) {
                
                case BOOT_SCREEN:
                    // (Esta pantalla solo se ejecuta en el setup)
                    break; 

                case TEMP_SCREEN: 
                    draw_temp_screen(screen_changed, sensor_data_changed); 
                    break;
                
                case HUMIDITY_SCREEN: 
                    draw_humidity_screen(screen_changed, sensor_data_changed); 
                    break;

                case LIGHT_SCREEN: 
                    draw_light_screen(screen_changed, sensor_data_changed); 
                    break;

                case SOUND_SCREEN:
                    draw_sound_screen(screen_changed, sensor_data_changed);
                    break;

                case SOIL_SCREEN:
                    draw_soil_screen(screen_changed, sensor_data_changed);
                    break;

                case DS18B20_SCREEN:
                    draw_ds18_screen(screen_changed, sensor_data_changed);
                    break;

                case SYSTEM_SCREEN: 
                    draw_system_screen(screen_changed, sensor_data_changed);
                    break;
                
                case TIMER_SCREEN:
                    draw_timer_screen(screen_changed, sensor_data_changed, timer_needs_update);
                    break;

                case GRAPH_SCREEN:
                    draw_graph_screen(screen_changed, sensor_data_changed);
                    break;

                case BLE_TOGGLE_SCREEN:
                    draw_ble_toggle_screen(screen_changed, sensor_data_changed);
                    break;

#if PBIT_ENABLE_GRAPH_LAB
                case LAB_DASH_OVERVIEW_SCREEN:
                    draw_lab_dash_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_SENSOR_FOCUS_SCREEN:
                    draw_lab_focus_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_DUAL_TH_SCREEN:
                    draw_lab_dual_th_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_SET_A_SCREEN:
                    draw_lab_icon_set_a_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_SET_B_SCREEN:
                    draw_lab_icon_set_b_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_SET_C_SCREEN:
                    draw_lab_icon_set_c_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_GAUGE_TEMP_SCREEN:
                    draw_lab_gauge_temp_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_VALUE_MODERN_SCREEN:
                    draw_lab_value_modern_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_SENSOR_CARD_SCREEN:
                    draw_lab_sensor_card_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_TEMP_CARD_SCREEN:
                    draw_lab_temp_card_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_DS18_CARD_SCREEN:
                    draw_lab_ds18_card_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_WIDGET_MIX_SCREEN:
                    draw_lab_widget_mix_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_SOUND_VU_STACK_SCREEN:
                    draw_lab_sound_vu_stack_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_SOUND_VU_WAVE_SCREEN:
                    draw_lab_sound_vu_wave_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_SIZES_ENV_SCREEN:
                    draw_lab_icon_sizes_env_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_SIZES_EXT_SCREEN:
                    draw_lab_icon_sizes_ext_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_HOME_CARDS_SCREEN:
                    draw_lab_home_cards_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_LINEAR_DASH_SCREEN:
                    draw_lab_linear_dash_screen(screen_changed, sensor_data_changed);
                    break;

                case LAB_ICON_TEST_SCREEN:
                    draw_lab_icon_test_screen(screen_changed, sensor_data_changed);
                    break;

                case SENSOR_ZONE_SCREEN:
                    sz_set_active(true);
                    sz_sync_renderer(screen_changed);
                    switch (sz_get_viz()) {
                        case SZ_VIZ_CARD:
                            draw_lab_sensor_card_screen(screen_changed, sensor_data_changed);
                            break;
                        case SZ_VIZ_VALOR:
                            draw_lab_value_modern_screen(screen_changed, sensor_data_changed);
                            break;
                        case SZ_VIZ_FOCUS:
                            draw_lab_focus_screen(screen_changed, sensor_data_changed);
                            break;
                        case SZ_VIZ_GRAPH:
                            draw_graph_screen(screen_changed, sensor_data_changed);
                            break;
                        case SZ_VIZ_GAUGE:
                            draw_lab_gauge_temp_screen(screen_changed, sensor_data_changed);
                            break;
                        default:
                            break;
                    }
                    if (screen_changed) drawHeader(sz_header_name());
                    sz_set_active(false);
                    break;
#endif

            } // fin del switch
            
            if (g_timer_just_reset) g_timer_just_reset = false;

        } // fin del if(screen_changed...)

        render_global_alert_badge();

#ifdef FIRMWARE_DEBUG
        static bool _hwm_reported = false;
        if (!_hwm_reported) { _hwm_reported = true; DPRINT("[Stack] DisplayTask HWM: %u words\n", uxTaskGetStackHighWaterMark(NULL)); }
#endif
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
