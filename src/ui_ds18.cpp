// ui_ds18.cpp
// Pantalla de sonda de temperatura externa DS18B20 (1-Wire, puerto J4/GPIO33).

#include "ui_ds18.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "hw.h"
#include "layout.h"
#include "led_control.h"
#include "alert_engine.h"
#include "runtime_events.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern bool g_is_fahrenheit;
extern uint16_t getTempColor(float temp);

// --- ESTADO DEL MENÚ DS18B20 ---
static Ds18MenuState g_ds18_menu_state = DS18_MODE_NORMAL;
static uint8_t g_ds18_menu_index = 0;
static int g_ds18_edit_off = 0;
static int g_ds18_edit_low = 0;
static int g_ds18_edit_high = 40;
static uint8_t g_ds18_edit_unit = 0;
static bool g_ds18_edit_alerts = false;
static uint8_t g_ds18_saved_kind = 0;
static bool g_ds18_save_ok = true;
static uint8_t g_ds18_reset_choice = 0;

namespace {

static const char* tr(const char* es, const char* cat, const char* en) {
    switch (g_language) {
        case LANG_CAT: return cat;
        case LANG_EN: return en;
        case LANG_ES:
        default: return es;
    }
}

static float to_display(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static float to_celsius(float display_temp) {
    return g_is_fahrenheit ? ((display_temp - 32.0f) / 1.8f) : display_temp;
}

static int to_display_int(int temp_c) {
    return (int)lroundf(to_display((float)temp_c));
}

static int to_celsius_int(int display_temp) {
    return (int)lroundf(to_celsius((float)display_temp));
}

static const char* unit_name() {
    return g_is_fahrenheit ? tr("Fahrenheit", "Fahrenheit", "Fahrenheit")
                            : tr("Celsius", "Celsius", "Celsius");
}

static const char* unit_short() {
    return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
}

static void apply_ds18_rgb(uint8_t alert_state) {
    switch (alert_state) {
        case ALERT_CODE_LOW:
            set_rgb(0, 90, 255);
            break;
        case ALERT_CODE_HIGH:
            set_rgb(255, 0, 0);
            break;
        case ALERT_CODE_OK:
        case ALERT_CODE_OFF:
        default:
            set_rgb(255, 255, 255);
            break;
    }
}
static void draw_ds18_alert_jewel(uint8_t alert_state, bool alerts_enabled, bool no_sensor) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_sensor || !alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
    } else if (alert_state == ALERT_CODE_LOW) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_BLUE;
    } else if (alert_state == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_RED;
    }

    drawAlertJewel(L_ALERT_JEWEL_X, L_ALERT_JEWEL_Y, jewel_state, jewel_color);
}

static void sync_edit_values_from_settings() {
    g_ds18_edit_off = get_ds18_offset_x10();
    g_ds18_edit_low = to_display_int(get_ds18_alarm_low());
    g_ds18_edit_high = to_display_int(get_ds18_alarm_high());
    g_ds18_edit_unit = g_is_fahrenheit ? 1 : 0;
    g_ds18_edit_alerts = get_ds18_alerts_enabled();
}

} // namespace

static void request_ds18_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

bool ds18_menu_is_active() { return g_ds18_menu_state != DS18_MODE_NORMAL; }

Ds18MenuState get_ds18_menu_state() { return g_ds18_menu_state; }

void start_ds18_menu() {
    g_ds18_menu_state = DS18_MODE_MENU;
    g_ds18_menu_index = 0;
    sync_edit_values_from_settings();
    g_ds18_saved_kind = 0;
    g_ds18_save_ok = true;
    g_ds18_reset_choice = 0;
    request_ds18_redraw(true);
}

int get_ds18_encoder_min() {
    switch (g_ds18_menu_state) {
        case DS18_MODE_MENU: return 0;
        case DS18_MODE_EDIT_OFFSET: return -50;
        case DS18_MODE_EDIT_LOW: return (int)lroundf(to_display(-55.0f));
        case DS18_MODE_EDIT_HIGH: return g_ds18_edit_low + 1;
        case DS18_MODE_EDIT_UNIT:
        case DS18_MODE_EDIT_ALERTS:
            return 0;
        case DS18_MODE_CONFIRM_RESET:
            return 0;
        default: return 0;
    }
}

int get_ds18_encoder_max() {
    switch (g_ds18_menu_state) {
        case DS18_MODE_MENU: return 4;
        case DS18_MODE_EDIT_OFFSET: return 50;
        case DS18_MODE_EDIT_LOW: return g_ds18_edit_high - 1;
        case DS18_MODE_EDIT_HIGH: return (int)lroundf(to_display(125.0f));
        case DS18_MODE_EDIT_UNIT:
        case DS18_MODE_EDIT_ALERTS:
            return 1;
        case DS18_MODE_CONFIRM_RESET:
            return 1;
        default: return 0;
    }
}

int get_ds18_encoder_value() {
    switch (g_ds18_menu_state) {
        case DS18_MODE_MENU: return (int)g_ds18_menu_index;
        case DS18_MODE_EDIT_OFFSET: return g_ds18_edit_off;
        case DS18_MODE_EDIT_LOW: return g_ds18_edit_low;
        case DS18_MODE_EDIT_HIGH: return g_ds18_edit_high;
        case DS18_MODE_EDIT_UNIT: return g_ds18_edit_unit;
        case DS18_MODE_EDIT_ALERTS: return g_ds18_edit_alerts ? 1 : 0;
        case DS18_MODE_CONFIRM_RESET: return g_ds18_reset_choice;
        default: return 0;
    }
}

void set_ds18_input_value(int value) {
    int next = constrain(value, get_ds18_encoder_min(), get_ds18_encoder_max());
    switch (g_ds18_menu_state) {
        case DS18_MODE_MENU:
            if ((uint8_t)next != g_ds18_menu_index) {
                g_ds18_menu_index = (uint8_t)next;
                request_ds18_redraw(false);
            }
            break;
        case DS18_MODE_EDIT_OFFSET:
            if (next != g_ds18_edit_off) { g_ds18_edit_off = next; request_ds18_redraw(false); }
            break;
        case DS18_MODE_EDIT_LOW:
            if (next != g_ds18_edit_low) { g_ds18_edit_low = next; request_ds18_redraw(false); }
            break;
        case DS18_MODE_EDIT_HIGH:
            if (next != g_ds18_edit_high) { g_ds18_edit_high = next; request_ds18_redraw(false); }
            break;
        case DS18_MODE_EDIT_UNIT:
            if ((uint8_t)next != g_ds18_edit_unit) {
                g_ds18_edit_unit = (uint8_t)next;
                request_ds18_redraw(false);
            }
            break;
        case DS18_MODE_EDIT_ALERTS:
            if ((uint8_t)next != (g_ds18_edit_alerts ? 1 : 0)) {
                g_ds18_edit_alerts = (next == 1);
                request_ds18_redraw(false);
            }
            break;
        case DS18_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_ds18_reset_choice) {
                g_ds18_reset_choice = (uint8_t)next;
                request_ds18_redraw(false);
            }
            break;
        default: break;
    }
}

uint8_t handle_ds18_button() {
    switch (g_ds18_menu_state) {
        case DS18_MODE_MENU:
            if (g_ds18_menu_index == 0)      g_ds18_menu_state = DS18_MODE_EDIT_OFFSET;
            else if (g_ds18_menu_index == 1) g_ds18_menu_state = DS18_MODE_EDIT_UNIT;
            else if (g_ds18_menu_index == 2) g_ds18_menu_state = DS18_MODE_EDIT_ALERTS;
            else if (g_ds18_menu_index == 3) {
                g_ds18_reset_choice = 0;
                g_ds18_menu_state = DS18_MODE_CONFIRM_RESET;
            }
            else                             g_ds18_menu_state = DS18_MODE_NORMAL;
            break;
        case DS18_MODE_EDIT_OFFSET: g_ds18_menu_state = DS18_MODE_EDIT_LOW; break;
        case DS18_MODE_EDIT_LOW:    g_ds18_menu_state = DS18_MODE_EDIT_HIGH; break;
        case DS18_MODE_EDIT_HIGH:
            g_ds18_save_ok = save_ds18_settings(g_ds18_edit_off, to_celsius_int(g_ds18_edit_low), to_celsius_int(g_ds18_edit_high));
            g_ds18_saved_kind = 0;
            g_ds18_menu_state = DS18_MODE_SAVED;
            break;
        case DS18_MODE_EDIT_UNIT:
            g_is_fahrenheit = (g_ds18_edit_unit != 0);
            sync_edit_values_from_settings();
            g_ds18_save_ok = true;
            g_ds18_saved_kind = 1;
            g_ds18_menu_state = DS18_MODE_SAVED;
            break;
        case DS18_MODE_EDIT_ALERTS:
            set_ds18_alerts_enabled(g_ds18_edit_alerts);
            g_ds18_save_ok = true;
            g_ds18_saved_kind = 2;
            g_ds18_menu_state = DS18_MODE_SAVED;
            break;
        case DS18_MODE_CONFIRM_RESET:
            if (g_ds18_reset_choice == 1) {
                reset_ds18_settings();
                g_is_fahrenheit = false;
                sync_edit_values_from_settings();
                g_ds18_save_ok = true;
                g_ds18_saved_kind = 3;
                g_ds18_reset_choice = 0;
                g_ds18_menu_state = DS18_MODE_SAVED;
            } else {
                g_ds18_menu_state = DS18_MODE_MENU;
            }
            break;
        case DS18_MODE_SAVED:       g_ds18_menu_state = DS18_MODE_MENU; break;
        default: return (uint8_t)g_ds18_menu_state;
    }
    request_ds18_redraw(g_ds18_menu_state == DS18_MODE_NORMAL);
    return (uint8_t)g_ds18_menu_state;
}

static void draw_ds18_menu_screen(bool screen_changed) {
    static Ds18MenuState last_drawn_state = DS18_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_edit_value = -9999;
    static int last_unit_value = -1;
    static int last_alert_value = -1;
    static int last_reset_choice = -1;
    static uint8_t last_saved_kind = 255;
    static bool last_save_ok = true;

    bool state_changed = screen_changed || (g_ds18_menu_state != last_drawn_state);
    bool needs_redraw = state_changed;

    if (g_ds18_menu_state == DS18_MODE_MENU) {
        needs_redraw = needs_redraw || (last_menu_index != (int)g_ds18_menu_index);
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_OFFSET
            || g_ds18_menu_state == DS18_MODE_EDIT_LOW
            || g_ds18_menu_state == DS18_MODE_EDIT_HIGH) {
        needs_redraw = needs_redraw || (last_edit_value != get_ds18_encoder_value());
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_UNIT) {
        needs_redraw = needs_redraw || (last_unit_value != (int)g_ds18_edit_unit);
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_ALERTS) {
        needs_redraw = needs_redraw || (last_alert_value != (g_ds18_edit_alerts ? 1 : 0));
    } else if (g_ds18_menu_state == DS18_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_ds18_reset_choice);
    } else if (g_ds18_menu_state == DS18_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != g_ds18_saved_kind) || (last_save_ok != g_ds18_save_ok);
    }

    if (!needs_redraw) return;

    const int cx = tft.width() / 2;
    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_THERM), TFT_ORANGE);
        last_menu_index = -1;
        last_edit_value = -9999;
        last_unit_value = -1;
        last_alert_value = -1;
        last_reset_choice = -1;
        last_saved_kind = 255;
        last_save_ok = true;
    }

    if (g_ds18_menu_state == DS18_MODE_MENU) {
        const char* items[5] = {
            tr("Calibración", "Calibració", "Calibration"),
            tr("Unidad", "Unitat", "Unit"),
            tr("Alertas", "Alertes", "Alerts"),
            tr("Reset", "Reset", "Reset"),
            tr("Salir", "Sortir", "Exit")
        };
        drawCenteredMenuList(items, 5, g_ds18_menu_index, LM_MENU5_Y0, LM_MENU5_GAP);
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_ds18_menu_index;
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_OFFSET) {
        char value_buf[16];
        snprintf(value_buf, sizeof(value_buf), "%s%d.%d", g_ds18_edit_off >= 0 ? "+" : "-", abs(g_ds18_edit_off) / 10, abs(g_ds18_edit_off) % 10);
        drawCenteredMenuValueScreen(tr("Offset", "Offset", "Offset"),
                                    value_buf,
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_edit_value = g_ds18_edit_off;
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_LOW || g_ds18_menu_state == DS18_MODE_EDIT_HIGH) {
        const bool low_mode = (g_ds18_menu_state == DS18_MODE_EDIT_LOW);
        char value_buf[20];
        snprintf(value_buf, sizeof(value_buf), "%d %s", get_ds18_encoder_value(), unit_short());
        drawCenteredMenuValueScreen(low_mode ? tr("Límite bajo", "Límit baix", "Low limit")
                                             : tr("Límite alto", "Límit alt", "High limit"),
                                    value_buf,
                                    low_mode ? TFT_CYAN : TFT_ORANGE,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_edit_value = get_ds18_encoder_value();
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_UNIT) {
        drawCenteredMenuValueScreen(tr("Unidad", "Unitat", "Unit"),
                                    g_ds18_edit_unit ? tr("Fahrenheit", "Fahrenheit", "Fahrenheit")
                                                     : tr("Celsius", "Celsius", "Celsius"),
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_unit_value = (int)g_ds18_edit_unit;
    } else if (g_ds18_menu_state == DS18_MODE_EDIT_ALERTS) {
        drawCenteredMenuValueScreen(tr("Alertas", "Alertes", "Alerts"),
                                    g_ds18_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                    g_ds18_edit_alerts ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_alert_value = g_ds18_edit_alerts ? 1 : 0;
    } else if (g_ds18_menu_state == DS18_MODE_CONFIRM_RESET) {
        drawResetChoicePrompt(tr("Reset", "Reset", "Reset"),
                              tr("Valores por defecto", "Valors per defecte", "Default values"),
                              tr("del termómetro", "del termòmetre", "for probe menu"),
                              tr("NO", "NO", "NO"),
                              tr("SÍ", "SÍ", "YES"),
                              g_ds18_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_ds18_reset_choice;
    } else if (g_ds18_menu_state == DS18_MODE_SAVED) {
        const char* saved_title = g_ds18_save_ok ? tr("Guardado", "Desat", "Saved")
                                                 : tr("Error", "Error", "Error");
        const uint16_t saved_title_color = g_ds18_save_ok ? TFT_GREEN : TFT_RED;
        if (g_ds18_saved_kind == 0 && g_ds18_save_ok) {
            char line1[24];
            char line2[24];
            char line3[24];
            const char* lines[3];
            const uint16_t colors[3] = { TFT_CYAN, TFT_GREEN, TFT_ORANGE };
            snprintf(line1, sizeof(line1), "%s %s%d.%d", tr("Offset", "Offset", "Offset"), g_ds18_edit_off >= 0 ? "+" : "-", abs(g_ds18_edit_off) / 10, abs(g_ds18_edit_off) % 10);
            snprintf(line2, sizeof(line2), "%s %d %s", tr("Bajo", "Baix", "Low"), g_ds18_edit_low, unit_short());
            snprintf(line3, sizeof(line3), "%s %d %s", tr("Alto", "Alt", "High"), g_ds18_edit_high, unit_short());
            lines[0] = line1;
            lines[1] = line2;
            lines[2] = line3;
            drawCenteredMenuFrame(saved_title, saved_title_color, L(ST_PUSH_MENU));
            drawCenteredMenuBodyLines(lines, colors, 3, MENU_TEXT_FONT_SMALL, LM_SUMMARY3_Y0, LM_SUMMARY3_GAP);
        } else if (g_ds18_saved_kind == 1) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_ds18_edit_unit ? tr("Fahrenheit", "Fahrenheit", "Fahrenheit")
                                                         : tr("Celsius", "Celsius", "Celsius"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_ds18_saved_kind == 2) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_ds18_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                        g_ds18_edit_alerts ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_ds18_saved_kind == 3) {
            drawCenteredMenuSavedScreen(saved_title,
                                        tr("Valores por defecto", "Valors per defecte", "Default values"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else {
            drawCenteredMenuSavedScreen(saved_title,
                                        tr("Guardado", "Desat", "Saved"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
        last_saved_kind = g_ds18_saved_kind;
        last_save_ok = g_ds18_save_ok;
    }

    last_drawn_state = g_ds18_menu_state;
}

// =============================================================
// DS18B20_SCREEN — sonda externa 1-Wire
// =============================================================
void draw_ds18_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (ds18_menu_is_active()) {
        draw_ds18_menu_screen(screen_changed);
        return;
    }

    const uint16_t TITLE_COLOR = TFT_ORANGE;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    const int LEFT_PANEL_W = LA_TANK_X - 2;
    float temp_c = g_ui_readings_snapshot.temp_ds18b20;
    bool no_sensor = (temp_c < -100.0f);
    const bool alerts_enabled = get_ds18_alerts_enabled();
    uint8_t alert_state = alert_engine_get_code(AlertSensor::Ds18);

    uint16_t value_color = TFT_DARKGREY;
    uint16_t tank_color = TFT_DARKGREY;
    uint16_t label_color = TFT_DARKGREY;

    if (!no_sensor) {
        value_color = (temp_c < 0.0f) ? TFT_CYAN : getTempColor(temp_c);
        tank_color = value_color;
        label_color = TFT_LIGHTGREY;

        if (alert_state == ALERT_CODE_LOW) {
            value_color = TFT_BLUE;
            tank_color = TFT_BLUE;
            label_color = TFT_BLUE;
        } else if (alert_state == ALERT_CODE_HIGH) {
            value_color = TFT_RED;
            tank_color = TFT_RED;
            label_color = TFT_RED;
        }
    }

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_THERM), TITLE_COLOR);
    }

    static int last_display_cache = INT16_MIN;
    static uint8_t last_alert_state = ALERT_CODE_OFF;
    static bool last_no_sensor = false;
    static bool last_unit_mode = false;
    static bool last_alerts_enabled = false;
    int display_cache = no_sensor ? INT16_MIN : (int)lroundf(to_display(temp_c) * 10.0f);
    if (!screen_changed
        && display_cache == last_display_cache
        && alert_state == last_alert_state
        && no_sensor == last_no_sensor
        && g_is_fahrenheit == last_unit_mode
        && alerts_enabled == last_alerts_enabled) {
        return;
    }

    if (screen_changed || alert_state != last_alert_state) {
        apply_ds18_rgb(alert_state);
    }

    tft.fillRect(0, LA_HINT_Y - 4, LEFT_PANEL_W, 18, BACKGROUND_COLOR);
    tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 46, BACKGROUND_COLOR);
    tft.fillRect(0, LA_CATEGORY_Y - 10, LEFT_PANEL_W, 28, BACKGROUND_COLOR);

    if (no_sensor) {
        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_RED, BACKGROUND_COLOR);
        tft.drawString(L(ST_NO_SENSOR), LA_LEFT_CX, LA_HINT_Y);
        tft.setTextFont(0);

        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_VALUE);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString("---", LA_LEFT_CX, LA_VALUE_TOP);
        tft.setTextFont(0);

        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString(L(ST_CHECK_DS18), LA_LEFT_CX, LA_CATEGORY_Y);
        tft.setTextFont(0);
    } else {
        const char* instruction = g_is_fahrenheit ? L(INSTR_C) : L(INSTR_F);
        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString(instruction, LA_LEFT_CX, LA_HINT_Y);
        tft.setTextFont(0);

        drawSplitDecimalValue(to_display(temp_c), LA_LEFT_CX, LA_VALUE_TOP, value_color, BACKGROUND_COLOR);

        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(label_color, BACKGROUND_COLOR);
        tft.drawString(unit_name(), LA_LEFT_CX, LA_CATEGORY_Y);
        tft.setTextFont(0);
    }

    draw_ds18_alert_jewel(alert_state, alerts_enabled, no_sensor);

    {
        const int inner_h = LA_TANK_H - 2;
        const int zero_fill = (int)(55.0f / 180.0f * inner_h);
        const int zero_y = LA_TANK_Y + 1 + (inner_h - zero_fill);

        tft.fillRect(LA_TANK_X + 1, LA_TANK_Y + 1, LA_TANK_W - 2, LA_TANK_H - 2, TFT_BLACK);

        if (no_sensor) {
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, TFT_DARKGREY, 0.0f, 0.0f, 50.0f);
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
        } else {
            float norm = constrain((temp_c + 55.0f) / 180.0f, 0.0f, 1.0f);
            int fill_px = (int)(norm * inner_h);
            int empty_px = inner_h - fill_px;

            if (empty_px > 0) {
                tft.fillRect(LA_TANK_X + 1, LA_TANK_Y + 1, LA_TANK_W - 2, empty_px, TFT_BLACK);
            }

            if (temp_c >= 0.0f) {
                int pos_fill = fill_px - zero_fill;
                if (pos_fill > 0) {
                    tft.fillRect(LA_TANK_X + 1, LA_TANK_Y + 1 + empty_px, LA_TANK_W - 2, pos_fill, tank_color);
                }
                tft.fillRect(LA_TANK_X + 1, zero_y, LA_TANK_W - 2, zero_fill, TFT_BLUE);
            } else if (fill_px > 0) {
                tft.fillRect(LA_TANK_X + 1, LA_TANK_Y + 1 + empty_px, LA_TANK_W - 2, fill_px, TFT_BLUE);
            }

            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, tank_color);
        }

        tft.drawFastHLine(LA_TANK_X + 1, zero_y, LA_TANK_W - 2, TFT_WHITE);
        tft.drawFastHLine(LA_TANK_X + LA_TANK_W + 1, zero_y, 2, TFT_WHITE);
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.setFreeFont(FONT_SMALL);
        tft.drawString("0", LA_TANK_X + LA_TANK_W + 3, zero_y - 4);
        tft.setTextFont(0);
    }

    last_display_cache = display_cache;
    last_alert_state = alert_state;
    last_no_sensor = no_sensor;
    last_unit_mode = g_is_fahrenheit;
    last_alerts_enabled = alerts_enabled;
}
