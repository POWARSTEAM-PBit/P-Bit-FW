// ui_humidity.cpp
#include "ui_humidity.h"
#include "tft_display.h"
#include "io.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include "hw.h"         // Para get_humidity_threshold_... y save_humidity_...
#include "led_control.h"
#include "alert_engine.h"
#include "ui_widgets.h"
#include "runtime_events.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

namespace {

constexpr int HUM_INFO_CARD_X = 10;
constexpr int HUM_INFO_CARD_Y = 36;
constexpr int HUM_INFO_CARD_W = 96;
constexpr int HUM_INFO_CARD_H = 62;

static uint16_t hum_info_card_bg() {
    return tft.color565(8, 12, 18);
}

static void draw_hum_info_card(uint16_t border_color) {
    const uint16_t bg = hum_info_card_bg();
    tft.fillRoundRect(HUM_INFO_CARD_X, HUM_INFO_CARD_Y, HUM_INFO_CARD_W, HUM_INFO_CARD_H, 5, bg);
    tft.drawRoundRect(HUM_INFO_CARD_X, HUM_INFO_CARD_Y, HUM_INFO_CARD_W, HUM_INFO_CARD_H, 5, border_color);
}

static void draw_hum_card_title(const char* text) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_DARKGREY, hum_info_card_bg());
    tft.drawString(text, LA_LEFT_CX, HUM_INFO_CARD_Y + 12);
    tft.setTextFont(0);
}

static void draw_hum_card_value(bool valid, const char* value_text, const char* unit_text, uint16_t value_color) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_TIMER);
    tft.setTextColor(valid ? value_color : TFT_DARKGREY, hum_info_card_bg());
    tft.drawString(value_text, LA_LEFT_CX, HUM_INFO_CARD_Y + 26);
    tft.setTextFont(0);

    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(value_color, hum_info_card_bg());
    tft.drawString(unit_text, LA_LEFT_CX, HUM_INFO_CARD_Y + 42);
    tft.setTextFont(0);
}

static void draw_hum_card_status(const char* text, uint16_t color) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, hum_info_card_bg());
    tft.drawString(text, LA_LEFT_CX, HUM_INFO_CARD_Y + 56);
    tft.setTextFont(0);
}

void apply_humidity_rgb(uint8_t alert_state) {
    switch (alert_state) {
        case ALERT_CODE_LOW:
            set_rgb(255, 120, 0);
            break;
        case ALERT_CODE_HIGH:
            set_rgb(255, 0, 0);
            break;
        case ALERT_CODE_OK:
        case ALERT_CODE_OFF:
        default:
            set_rgb(0, 0, 255);
            break;
    }
}
static void draw_humidity_alert_jewel(uint8_t alert_state, bool alerts_enabled, bool no_sensor) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_sensor || !alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
    } else if (alert_state == ALERT_CODE_LOW) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_ORANGE;
    } else if (alert_state == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_RED;
    }

    drawAlertJewel(L_ALERT_JEWEL_X, L_ALERT_JEWEL_Y, jewel_state, jewel_color);
}

} // namespace

// --- ESTADO DEL MENÚ DE HUMEDAD ---
static HumidityMenuState g_hum_menu_state = HUM_MODE_NORMAL;
static uint8_t g_hum_menu_index = 0;
static int g_hum_edit_dry = 30;
static int g_hum_edit_comf = 70;
static bool g_hum_edit_alerts = true;
static uint8_t g_hum_saved_kind = 0;
static uint8_t g_hum_reset_choice = 0;

static void request_humidity_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

bool humidity_menu_is_active() {
    return g_hum_menu_state != HUM_MODE_NORMAL;
}

HumidityMenuState get_humidity_menu_state() {
    return g_hum_menu_state;
}

void start_humidity_menu() {
    g_hum_menu_state = HUM_MODE_MENU;
    g_hum_menu_index = 0;
    g_hum_edit_dry = get_humidity_threshold_dry();
    g_hum_edit_comf = get_humidity_threshold_comfort();
    g_hum_edit_alerts = get_humidity_alerts_enabled();
    g_hum_saved_kind = 0;
    g_hum_reset_choice = 0;
    request_humidity_redraw(true);
}

int get_humidity_encoder_min() {
    switch (g_hum_menu_state) {
        case HUM_MODE_MENU: return 0;
        case HUM_MODE_EDIT_DRY: return 0;
        case HUM_MODE_EDIT_COMFORT: return g_hum_edit_dry + 1; // Comfort siempre > Dry
        case HUM_MODE_EDIT_ALERTS: return 0;
        case HUM_MODE_CONFIRM_RESET: return 0;
        default: return 0;
    }
}

int get_humidity_encoder_max() {
    switch (g_hum_menu_state) {
        case HUM_MODE_MENU: return 3; // 4 opciones (Umbrales, Alertas, Reset, Salir)
        case HUM_MODE_EDIT_DRY: return g_hum_edit_comf - 1; // Dry siempre < Comfort
        case HUM_MODE_EDIT_COMFORT: return 100;
        case HUM_MODE_EDIT_ALERTS: return 1;
        case HUM_MODE_CONFIRM_RESET: return 1;
        default: return 0;
    }
}

int get_humidity_encoder_value() {
    switch (g_hum_menu_state) {
        case HUM_MODE_MENU: return (int)g_hum_menu_index;
        case HUM_MODE_EDIT_DRY: return g_hum_edit_dry;
        case HUM_MODE_EDIT_COMFORT: return g_hum_edit_comf;
        case HUM_MODE_EDIT_ALERTS: return g_hum_edit_alerts ? 1 : 0;
        case HUM_MODE_CONFIRM_RESET: return (int)g_hum_reset_choice;
        default: return 0;
    }
}

void set_humidity_input_value(int value) {
    int next = constrain(value, get_humidity_encoder_min(), get_humidity_encoder_max());
    switch (g_hum_menu_state) {
        case HUM_MODE_MENU:
            if ((uint8_t)next != g_hum_menu_index) {
                g_hum_menu_index = (uint8_t)next;
                request_humidity_redraw(false);
            }
            break;
        case HUM_MODE_EDIT_DRY:
            if (next != g_hum_edit_dry) {
                g_hum_edit_dry = next;
                request_humidity_redraw(false);
            }
            break;
        case HUM_MODE_EDIT_COMFORT:
            if (next != g_hum_edit_comf) {
                g_hum_edit_comf = next;
                request_humidity_redraw(false);
            }
            break;
        case HUM_MODE_EDIT_ALERTS:
            if ((next == 1) != g_hum_edit_alerts) {
                g_hum_edit_alerts = (next == 1);
                request_humidity_redraw(false);
            }
            break;
        case HUM_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_hum_reset_choice) {
                g_hum_reset_choice = (uint8_t)next;
                request_humidity_redraw(false);
            }
            break;
        default: break;
    }
}

uint8_t handle_humidity_button() {
    switch (g_hum_menu_state) {
        case HUM_MODE_MENU:
            if (g_hum_menu_index == 0) {
                g_hum_menu_state = HUM_MODE_EDIT_DRY;
            } else if (g_hum_menu_index == 1) {
                g_hum_menu_state = HUM_MODE_EDIT_ALERTS;
            } else if (g_hum_menu_index == 2) {
                g_hum_reset_choice = 0;
                g_hum_menu_state = HUM_MODE_CONFIRM_RESET;
            } else {
                g_hum_menu_state = HUM_MODE_NORMAL;
            }
            break;
        case HUM_MODE_EDIT_DRY:
            g_hum_menu_state = HUM_MODE_EDIT_COMFORT;
            break;
        case HUM_MODE_EDIT_COMFORT:
            save_humidity_thresholds(g_hum_edit_dry, g_hum_edit_comf);
            g_hum_saved_kind = 0;
            g_hum_menu_state = HUM_MODE_SAVED;
            break;
        case HUM_MODE_EDIT_ALERTS:
            set_humidity_alerts_enabled(g_hum_edit_alerts);
            g_hum_saved_kind = 1;
            g_hum_menu_state = HUM_MODE_SAVED;
            break;
        case HUM_MODE_CONFIRM_RESET:
            if (g_hum_reset_choice == 1) {
                reset_humidity_settings();
                g_hum_edit_dry = get_humidity_threshold_dry();
                g_hum_edit_comf = get_humidity_threshold_comfort();
                g_hum_edit_alerts = get_humidity_alerts_enabled();
                g_hum_saved_kind = 2;
                g_hum_reset_choice = 0;
                g_hum_menu_state = HUM_MODE_SAVED;
            } else {
                g_hum_menu_state = HUM_MODE_MENU;
            }
            break;
        case HUM_MODE_SAVED:
            g_hum_menu_state = HUM_MODE_MENU;
            break;
        default:
            return (uint8_t)g_hum_menu_state;
    }
    request_humidity_redraw(g_hum_menu_state == HUM_MODE_NORMAL);
    return (uint8_t)g_hum_menu_state;
}

static void draw_humidity_menu_screen(bool screen_changed) {
    static HumidityMenuState last_drawn_state = HUM_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_edit_val = -1;
    static int last_alert_value = -1;
    static int last_reset_choice = -1;

    const int cx = tft.width() / 2;
    const bool state_changed = (g_hum_menu_state != last_drawn_state) || screen_changed;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_HUM));
        last_menu_index = -1;
        last_edit_val = -1;
        last_alert_value = -1;
        last_reset_choice = -1;
    }

    if (g_hum_menu_state == HUM_MODE_MENU) {
        if (state_changed || last_menu_index != (int)g_hum_menu_index) {
            const char* items[4] = {
                L(MENU_LIMITS),
                L(MENU_ALERTS),
                L(MENU_RESET),
                L(MENU_EXIT)
            };
            drawCenteredMenuList(items, 4, g_hum_menu_index, LM_MENU4_Y0, LM_MENU4_GAP);
            drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
            last_menu_index = (int)g_hum_menu_index;
        }
        last_drawn_state = g_hum_menu_state;
        return;
    }

    if (g_hum_menu_state == HUM_MODE_EDIT_DRY || g_hum_menu_state == HUM_MODE_EDIT_COMFORT) {
        int current_val = (g_hum_menu_state == HUM_MODE_EDIT_DRY) ? g_hum_edit_dry : g_hum_edit_comf;
        uint16_t color = (g_hum_menu_state == HUM_MODE_EDIT_DRY) ? TFT_ORANGE : TFT_GREEN;

        if (state_changed || current_val != last_edit_val) {
            char value_buf[12];
            snprintf(value_buf, sizeof(value_buf), "%d%%", current_val);
            drawCenteredMenuValueScreen((g_hum_menu_state == HUM_MODE_EDIT_DRY)
                                            ? L(ST_DRY)
                                            : L(ST_SATURATED),
                                        value_buf,
                                        color,
                                        MENU_VALUE_FONT_TIMER,
                                        L(ST_TURN_PUSH));
            last_edit_val = current_val;
        }
    }

    if (g_hum_menu_state == HUM_MODE_EDIT_ALERTS) {
        int alert_value = g_hum_edit_alerts ? 1 : 0;
        if (state_changed || alert_value != last_alert_value) {
            drawCenteredMenuValueScreen(L(MENU_ALERTS),
                                        g_hum_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                        g_hum_edit_alerts ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_TIMER,
                                        L(ST_TURN_PUSH));
            last_alert_value = alert_value;
        }
    }

    if (g_hum_menu_state == HUM_MODE_CONFIRM_RESET && (state_changed || last_reset_choice != (int)g_hum_reset_choice)) {
        drawResetChoicePrompt(L(MENU_RESET),
                              L(MENU_DEFAULTS),
                              L(MENU_RESET_SUB_HUM),
                              L(MENU_NO),
                              L(MENU_YES),
                              g_hum_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_hum_reset_choice;
    }

    if (g_hum_menu_state == HUM_MODE_SAVED && state_changed) {
        if (g_hum_saved_kind == 0) {
            char line_buf_1[24];
            char line_buf_2[24];
            const char* lines[2];
            const uint16_t colors[2] = { TFT_ORANGE, TFT_GREEN };
            snprintf(line_buf_1, sizeof(line_buf_1), "%s %d%%", L(ST_DRY), g_hum_edit_dry);
            snprintf(line_buf_2, sizeof(line_buf_2), "%s %d%%", L(ST_SATURATED), g_hum_edit_comf);
            lines[0] = line_buf_1;
            lines[1] = line_buf_2;
            drawCenteredMenuFrame(L(MENU_SAVED), TFT_GREEN, L(ST_PUSH_MENU));
            drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_SMALL, LM_SUMMARY2_Y0, LM_SUMMARY2_GAP);
        } else if (g_hum_saved_kind == 1) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        g_hum_edit_alerts ? L(ST_ON) : L(ST_OFF),
                                        g_hum_edit_alerts ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_TIMER,
                                        L(ST_PUSH_MENU));
        } else if (g_hum_saved_kind == 2) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        L(MENU_DEFAULTS),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
    }

    last_drawn_state = g_hum_menu_state;
}

void draw_humidity_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (humidity_menu_is_active()) {
        draw_humidity_menu_screen(screen_changed);
        return;
    }

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_STATUS = 2;
    const uint16_t HUMIDITY_COLOR   = TFT_CYAN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    const int LEFT_PANEL_W = LA_TANK_X - 1;

    // Coordenadas definidas en layout.h (Familia A)

    float hum      = g_ui_readings_snapshot.humidity;
    bool  no_dht_h = isnan(hum);
    const int dry_max = get_humidity_threshold_dry();
    const int comfort_max = get_humidity_threshold_comfort();
    const bool alerts_enabled = get_humidity_alerts_enabled();

    const char* statusText = nullptr;
    uint16_t statusColor = TFT_DARKGREY;
    uint16_t valueColor = TFT_WHITE;
    uint16_t tankBorderColor = TFT_DARKGREY;
    uint16_t tankFillColor = HUMIDITY_COLOR;

    if (!no_dht_h) {
        if (hum > (float)comfort_max) {
            statusText = L(ST_MOLD_RISK);
            statusColor = TFT_RED;
        } else if (hum < (float)dry_max) {
            statusText = L(ST_TOO_DRY);
            statusColor = TFT_ORANGE;
        } else {
            statusText = L(ST_OPTIMAL);
            statusColor = TFT_GREEN;
        }
    }

    uint8_t alert_state = alert_engine_get_code(AlertSensor::Humidity);
    if (alert_state == ALERT_CODE_LOW) {
        valueColor = TFT_ORANGE;
        tankBorderColor = TFT_ORANGE;
        tankFillColor = TFT_ORANGE;
    } else if (alert_state == ALERT_CODE_HIGH) {
        valueColor = TFT_RED;
        tankBorderColor = TFT_RED;
        tankFillColor = TFT_RED;
    }

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_HUM));
    }

    // Salida temprana si el valor ni el estado cambiaron
    static int last_hum_drawn = -1;
    static int last_status_id = -1;
    static uint8_t last_alert_state = ALERT_CODE_OFF;
    static bool last_no_sensor = false;
    static bool last_alerts_enabled = false;
    int hum_cache = no_dht_h ? -9999 : (int)roundf(hum);
    int status_id = no_dht_h ? -1 : ((hum > (float)comfort_max) ? 2 : ((hum < (float)dry_max) ? 0 : 1));
    if (!screen_changed
        && hum_cache == last_hum_drawn
        && status_id == last_status_id
        && alert_state == last_alert_state
        && alerts_enabled == last_alerts_enabled) return;
    last_hum_drawn = hum_cache;
    last_status_id = status_id;

    if (screen_changed || alert_state != last_alert_state) {
        apply_humidity_rgb(alert_state);
        last_alert_state = alert_state;
    }

    // --- Dinámicos ---
    tft.fillRect(0, LA_HINT_Y - 4, LEFT_PANEL_W, 72, BACKGROUND_COLOR);
    draw_hum_info_card((alert_state == ALERT_CODE_HIGH) ? TFT_RED
                       : (alert_state == ALERT_CODE_LOW) ? TFT_ORANGE
                       : HUMIDITY_COLOR);
    draw_hum_card_title(L(SUB_AIR_REL));

    if (no_dht_h) {
        drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, HUMIDITY_COLOR, 0.0f, 0.0f, 100.0f, 3);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
        draw_hum_card_value(false, "---", "%", TFT_DARKGREY);
        draw_hum_card_status(L(ST_NO_SENSOR), TFT_RED);
    } else {
        drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, tankFillColor, hum, 0.0f, 100.0f, 3);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, tankBorderColor);

        char humStr[6];
        snprintf(humStr, sizeof(humStr), "%.0f", hum);
        draw_hum_card_value(true, humStr, "%", valueColor);
        draw_hum_card_status(statusText, statusColor);
    }

    draw_humidity_alert_jewel(alert_state, alerts_enabled, no_dht_h);

    last_no_sensor = no_dht_h;
    last_alerts_enabled = alerts_enabled;
}
