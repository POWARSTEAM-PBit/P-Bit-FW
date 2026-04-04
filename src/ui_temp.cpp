// ui_temp.cpp
// Pantalla y menu para temperatura del DHT.

#include "ui_temp.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"
#include "layout.h"
#include "hw.h"
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

static TempMenuState g_temp_menu_state = TEMP_MODE_NORMAL;
static uint8_t g_temp_menu_index = 0;
static int g_temp_edit_low = 18;
static int g_temp_edit_high = 28;
static uint8_t g_temp_edit_unit = 0;
static bool g_temp_edit_alerts = false;
static uint8_t g_temp_saved_kind = 0;
static bool g_temp_save_ok = true;
static uint8_t g_temp_reset_choice = 0;

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

// Temperature alert colors are intentionally strong so they read instantly.
static void apply_temp_rgb(uint8_t alert_state) {
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
            set_rgb(255, 69, 0);
            break;
    }
}

static void sync_edit_values_from_settings() {
    // Keep the editor aligned with the values currently stored in NVS.
    g_temp_edit_low = to_display_int(get_temp_alarm_low());
    g_temp_edit_high = to_display_int(get_temp_alarm_high());
    g_temp_edit_unit = g_is_fahrenheit ? 1 : 0;
    g_temp_edit_alerts = get_temp_alerts_enabled();
}

} // namespace

static void request_temp_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

static void draw_temp_alert_jewel(uint8_t alert_state, bool alerts_enabled, bool no_dht) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_dht || !alerts_enabled) {
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

void start_temp_menu() {
    g_temp_menu_state = TEMP_MODE_MENU;
    g_temp_menu_index = 0;
    g_temp_saved_kind = 0;
    g_temp_save_ok = true;
    g_temp_reset_choice = 0;
    sync_edit_values_from_settings();
    request_temp_redraw(true);
}

bool temp_menu_is_active() {
    return g_temp_menu_state != TEMP_MODE_NORMAL;
}

TempMenuState get_temp_menu_state() {
    return g_temp_menu_state;
}

int get_temp_encoder_min() {
    switch (g_temp_menu_state) {
        case TEMP_MODE_MENU: return 0;
        case TEMP_MODE_EDIT_LOW: return (int)lroundf(to_display(0.0f));
        case TEMP_MODE_EDIT_HIGH: return g_temp_edit_low + 1;
        case TEMP_MODE_EDIT_UNIT:
        case TEMP_MODE_EDIT_ALERTS:
            return 0;
        case TEMP_MODE_CONFIRM_RESET:
            return 0;
        default:
            return 0;
    }
}

int get_temp_encoder_max() {
    switch (g_temp_menu_state) {
        case TEMP_MODE_MENU: return 4;
        case TEMP_MODE_EDIT_LOW: return g_temp_edit_high - 1;
        case TEMP_MODE_EDIT_HIGH: return (int)lroundf(to_display(50.0f));
        case TEMP_MODE_EDIT_UNIT:
        case TEMP_MODE_EDIT_ALERTS:
            return 1;
        case TEMP_MODE_CONFIRM_RESET:
            return 1;
        default:
            return 0;
    }
}

int get_temp_encoder_value() {
    switch (g_temp_menu_state) {
        case TEMP_MODE_MENU: return (int)g_temp_menu_index;
        case TEMP_MODE_EDIT_LOW: return g_temp_edit_low;
        case TEMP_MODE_EDIT_HIGH: return g_temp_edit_high;
        case TEMP_MODE_EDIT_UNIT: return (int)g_temp_edit_unit;
        case TEMP_MODE_EDIT_ALERTS: return g_temp_edit_alerts ? 1 : 0;
        case TEMP_MODE_CONFIRM_RESET: return (int)g_temp_reset_choice;
        default:
            return 0;
    }
}

void set_temp_input_value(int value) {
    int next = constrain(value, get_temp_encoder_min(), get_temp_encoder_max());

    switch (g_temp_menu_state) {
        case TEMP_MODE_MENU:
            if ((uint8_t)next != g_temp_menu_index) {
                g_temp_menu_index = (uint8_t)next;
                request_temp_redraw();
            }
            break;
        case TEMP_MODE_EDIT_LOW:
            if (next != g_temp_edit_low) {
                g_temp_edit_low = next;
                request_temp_redraw();
            }
            break;
        case TEMP_MODE_EDIT_HIGH:
            if (next != g_temp_edit_high) {
                g_temp_edit_high = next;
                request_temp_redraw();
            }
            break;
        case TEMP_MODE_EDIT_UNIT:
            if ((uint8_t)next != g_temp_edit_unit) {
                g_temp_edit_unit = (uint8_t)next;
                request_temp_redraw();
            }
            break;
        case TEMP_MODE_EDIT_ALERTS:
            if ((next == 1) != g_temp_edit_alerts) {
                g_temp_edit_alerts = (next == 1);
                request_temp_redraw();
            }
            break;
        case TEMP_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_temp_reset_choice) {
                g_temp_reset_choice = (uint8_t)next;
                request_temp_redraw();
            }
            break;
        default:
            break;
    }
}

uint8_t handle_temp_button() {
    switch (g_temp_menu_state) {
        case TEMP_MODE_MENU:
            if (g_temp_menu_index == 0) {
                g_temp_menu_state = TEMP_MODE_EDIT_LOW;
            } else if (g_temp_menu_index == 1) {
                g_temp_menu_state = TEMP_MODE_EDIT_UNIT;
            } else if (g_temp_menu_index == 2) {
                g_temp_menu_state = TEMP_MODE_EDIT_ALERTS;
            } else if (g_temp_menu_index == 3) {
                g_temp_reset_choice = 0;
                g_temp_menu_state = TEMP_MODE_CONFIRM_RESET;
            } else {
                g_temp_menu_state = TEMP_MODE_NORMAL;
            }
            break;
        case TEMP_MODE_EDIT_LOW:
            g_temp_menu_state = TEMP_MODE_EDIT_HIGH;
            break;
        case TEMP_MODE_EDIT_HIGH:
            g_temp_save_ok = save_temp_settings(to_celsius_int(g_temp_edit_low),
                                               to_celsius_int(g_temp_edit_high));
            g_temp_saved_kind = 0;
            g_temp_menu_state = TEMP_MODE_SAVED;
            break;
        case TEMP_MODE_EDIT_UNIT:
            g_is_fahrenheit = (g_temp_edit_unit == 1);
            sync_edit_values_from_settings();
            g_temp_saved_kind = 1;
            g_temp_save_ok = true;
            g_temp_menu_state = TEMP_MODE_SAVED;
            break;
        case TEMP_MODE_EDIT_ALERTS:
            set_temp_alerts_enabled(g_temp_edit_alerts);
            g_temp_saved_kind = 2;
            g_temp_save_ok = true;
            g_temp_menu_state = TEMP_MODE_SAVED;
            break;
        case TEMP_MODE_CONFIRM_RESET:
            if (g_temp_reset_choice == 1) {
                reset_temp_settings();
                g_is_fahrenheit = false;
                sync_edit_values_from_settings();
                g_temp_saved_kind = 3;
                g_temp_save_ok = true;
                g_temp_reset_choice = 0;
                g_temp_menu_state = TEMP_MODE_SAVED;
            } else {
                g_temp_menu_state = TEMP_MODE_MENU;
            }
            break;
        case TEMP_MODE_SAVED:
            g_temp_menu_state = TEMP_MODE_MENU;
            break;
        default:
            break;
    }

    request_temp_redraw(g_temp_menu_state == TEMP_MODE_NORMAL || g_temp_menu_state == TEMP_MODE_SAVED);
    return (uint8_t)g_temp_menu_state;
}

static void draw_temp_menu_screen(bool screen_changed) {
    const int cx = tft.width() / 2;
    static TempMenuState last_drawn_state = TEMP_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_edit_value = INT16_MIN;
    static int last_unit_value = -1;
    static int last_alert_value = -1;
    static int last_reset_choice = -1;
    static int last_saved_kind = -1;
    static bool last_save_ok = true;

    bool state_changed = screen_changed || (g_temp_menu_state != last_drawn_state);
    bool needs_redraw = state_changed;

    if (g_temp_menu_state == TEMP_MODE_MENU) {
        needs_redraw = needs_redraw || (last_menu_index != (int)g_temp_menu_index);
    } else if (g_temp_menu_state == TEMP_MODE_EDIT_UNIT) {
        needs_redraw = needs_redraw || (last_unit_value != (int)g_temp_edit_unit);
    } else if (g_temp_menu_state == TEMP_MODE_EDIT_ALERTS) {
        needs_redraw = needs_redraw || (last_alert_value != (g_temp_edit_alerts ? 1 : 0));
    } else if (g_temp_menu_state == TEMP_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_temp_reset_choice);
    } else if (g_temp_menu_state == TEMP_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != (int)g_temp_saved_kind) || (last_save_ok != g_temp_save_ok);
    } else {
        needs_redraw = needs_redraw || (last_edit_value != get_temp_encoder_value());
    }

    if (!needs_redraw) return;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_TEMP), TFT_RED);
        last_menu_index = -1;
        last_edit_value = INT16_MIN;
        last_unit_value = -1;
        last_alert_value = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
        last_save_ok = true;
    }

    if (g_temp_menu_state == TEMP_MODE_MENU) {
        // Root menu follows the same five-item structure as the other sensors.
        const char* items[] = {
            tr("Límites", "Límits", "Limits"),
            tr("Unidad", "Unitat", "Unit"),
            tr("Alertas", "Alertes", "Alerts"),
            tr("Reset", "Reset", "Reset"),
            tr("Salir", "Sortir", "Exit")
        };
        drawCenteredMenuList(items, 5, g_temp_menu_index, LM_MENU5_Y0, LM_MENU5_GAP);
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_temp_menu_index;
    } else if (g_temp_menu_state == TEMP_MODE_EDIT_LOW || g_temp_menu_state == TEMP_MODE_EDIT_HIGH) {
        // Limit editing uses the same centered numeric layout in both units.
        bool low_mode = (g_temp_menu_state == TEMP_MODE_EDIT_LOW);
        char value_buf[18];
        snprintf(value_buf, sizeof(value_buf), "%d %s", get_temp_encoder_value(), unit_short());
        drawCenteredMenuValueScreen(low_mode ? tr("Límite bajo", "Límit baix", "Low limit")
                                             : tr("Límite alto", "Límit alt", "High limit"),
                                    value_buf,
                                    low_mode ? TFT_CYAN : TFT_ORANGE,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_edit_value = get_temp_encoder_value();
    } else if (g_temp_menu_state == TEMP_MODE_EDIT_UNIT) {
        // Unit selection is a binary toggle between Celsius and Fahrenheit.
        drawCenteredMenuValueScreen(tr("Unidad", "Unitat", "Unit"),
                                    g_temp_edit_unit ? tr("Fahrenheit", "Fahrenheit", "Fahrenheit")
                                                     : tr("Celsius", "Celsius", "Celsius"),
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_unit_value = (int)g_temp_edit_unit;
    } else if (g_temp_menu_state == TEMP_MODE_EDIT_ALERTS) {
        // Alert enable/disable uses the same compact ON/OFF interaction as the
        // rest of the firmware.
        drawCenteredMenuValueScreen(tr("Alertas", "Alertes", "Alerts"),
                                    g_temp_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                    g_temp_edit_alerts ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_alert_value = g_temp_edit_alerts ? 1 : 0;
    } else if (g_temp_menu_state == TEMP_MODE_CONFIRM_RESET) {
        drawResetChoicePrompt(tr("Reset", "Reset", "Reset"),
                              tr("Valores por defecto", "Valors per defecte", "Default values"),
                              tr("de temperatura", "de temperatura", "for temperature"),
                              tr("NO", "NO", "NO"),
                              tr("SÍ", "SÍ", "YES"),
                              g_temp_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_temp_reset_choice;
    } else if (g_temp_menu_state == TEMP_MODE_SAVED) {
        // Saved state previews the updated setting before returning to menu.
        const char* saved_title = g_temp_save_ok ? tr("Guardado", "Desat", "Saved")
                                                 : tr("Error", "Error", "Error");
        const uint16_t saved_title_color = g_temp_save_ok ? TFT_GREEN : TFT_RED;
        if (g_temp_saved_kind == 0 && g_temp_save_ok) {
            char line_buf_1[24];
            char line_buf_2[24];
            const char* lines[2];
            const uint16_t colors[2] = { TFT_CYAN, TFT_ORANGE };
            snprintf(line_buf_1, sizeof(line_buf_1), "%s %d %s",
                     tr("Bajo", "Baix", "Low"), g_temp_edit_low, unit_short());
            snprintf(line_buf_2, sizeof(line_buf_2), "%s %d %s",
                     tr("Alto", "Alt", "High"), g_temp_edit_high, unit_short());
            lines[0] = line_buf_1;
            lines[1] = line_buf_2;
            drawCenteredMenuFrame(saved_title, saved_title_color, L(ST_PUSH_MENU));
            drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_SMALL, LM_SUMMARY2_Y0, LM_SUMMARY2_GAP);
        } else if (g_temp_saved_kind == 1) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_temp_edit_unit ? tr("Fahrenheit", "Fahrenheit", "Fahrenheit")
                                                         : tr("Celsius", "Celsius", "Celsius"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_temp_saved_kind == 2) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_temp_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                        g_temp_edit_alerts ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_temp_saved_kind == 3) {
            drawCenteredMenuSavedScreen(saved_title,
                                        tr("Valores por defecto", "Valors per defecte", "Default values"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
        last_saved_kind = (int)g_temp_saved_kind;
        last_save_ok = g_temp_save_ok;
    }

    last_drawn_state = g_temp_menu_state;
}

void draw_temp_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (temp_menu_is_active()) {
        draw_temp_menu_screen(screen_changed);
        return;
    }

    const uint16_t COLOR_TITLE = TFT_RED;
    const int LEFT_PANEL_W = LA_TANK_X - 1;
    float temp_c = g_ui_readings_snapshot.temperature;
    bool no_dht = isnan(temp_c);
    const bool alerts_enabled = get_temp_alerts_enabled();
    uint8_t alert_state = alert_engine_get_code(AlertSensor::Temp);

    uint16_t temp_color = no_dht ? TFT_DARKGREY : getTempColor(temp_c);
    uint16_t label_color = TFT_LIGHTGREY;
    if (alert_state == ALERT_CODE_LOW) {
        temp_color = TFT_BLUE;
        label_color = TFT_BLUE;
    } else if (alert_state == ALERT_CODE_HIGH) {
        temp_color = TFT_RED;
        label_color = TFT_RED;
    }

    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_TEMP), COLOR_TITLE);
    }

    static int last_temp_drawn = INT16_MIN;
    static uint8_t last_alert_state = ALERT_CODE_OFF;
    static bool last_no_dht = false;
    static bool last_unit_mode = false;
    static bool last_alerts_enabled = false;
    int temp_cache = no_dht ? INT16_MIN : (int)lroundf(to_display(temp_c) * 10.0f);
    if (!screen_changed
        && temp_cache == last_temp_drawn
        && alert_state == last_alert_state
        && no_dht == last_no_dht
        && g_is_fahrenheit == last_unit_mode
        && alerts_enabled == last_alerts_enabled) {
        return;
    }

    if (screen_changed || alert_state != last_alert_state) {
        apply_temp_rgb(alert_state);
    }

    // Clear only the text and value bands that actually change on screen.
    tft.fillRect(0, LA_HINT_Y - 4, LEFT_PANEL_W, 18, TFT_BLACK);
    tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 46, TFT_BLACK);
    tft.fillRect(0, LA_CATEGORY_Y - 10, LEFT_PANEL_W, 28, TFT_BLACK);

    if (no_dht) {
        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString(L(ST_NO_SENSOR), LA_LEFT_CX, LA_HINT_Y);
        tft.setTextFont(0);

        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_VALUE);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("---", LA_LEFT_CX, LA_VALUE_TOP);
        tft.setTextFont(0);

        drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, TFT_DARKGREY, 0.0f, 0.0f, 50.0f, 3);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
    } else {
        const char* instruction_text = g_is_fahrenheit ? L(INSTR_C) : L(INSTR_F);
        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(instruction_text, LA_LEFT_CX, LA_HINT_Y);
        tft.setTextFont(0);

        drawSplitDecimalValue(to_display(temp_c), LA_LEFT_CX, LA_VALUE_TOP, temp_color, TFT_BLACK);

        tft.setFreeFont(FONT_BODY);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(alert_state == ALERT_CODE_OK ? TFT_LIGHTGREY : label_color, TFT_BLACK);
        tft.drawString(unit_name(), LA_LEFT_CX, LA_CATEGORY_Y);
        tft.setTextFont(0);

        drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, temp_color, temp_c, 0.0f, 50.0f, 3);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, temp_color);
    }

    draw_temp_alert_jewel(alert_state, alerts_enabled, no_dht);

    last_temp_drawn = temp_cache;
    last_alert_state = alert_state;
    last_no_dht = no_dht;
    last_unit_mode = g_is_fahrenheit;
    last_alerts_enabled = alerts_enabled;
}
