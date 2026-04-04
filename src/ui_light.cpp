#include "ui_light.h"
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
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

namespace {

const char* tr(const char* es, const char* cat, const char* en) {
    switch (g_language) {
        case LANG_CAT: return cat;
        case LANG_EN: return en;
        case LANG_ES:
        default: return es;
    }
}

void format_light_threshold_value(int value, char* out, size_t out_size) {
    if (value >= 10000) {
        snprintf(out, out_size, "< %dk", value / 1000);
        return;
    }
    snprintf(out, out_size, "< %d %s", value, L(ST_LUX_UNIT));
}

// Draw the tiny runtime alert indicator used by the light screen.
void draw_light_alert_jewel(uint8_t alert_state, bool alerts_enabled) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (!alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
    } else if (alert_state == ALERT_CODE_LOW) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_CYAN;
    } else if (alert_state == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_ORANGE;
    }

    drawAlertJewel(L_ALERT_JEWEL_X, L_ALERT_JEWEL_Y, jewel_state, jewel_color);
}

}

static LightMenuState g_light_menu_state = LIGHT_MODE_NORMAL;
static uint8_t g_light_menu_index = 0;
static uint8_t g_light_last_saved_menu_index = 0;
static int g_light_dim_max = 100;
static int g_light_indoor_max = 500;
static int g_light_bright_max = 2000;
static uint8_t g_light_display_mode = 0;
static bool g_light_alerts_enabled = true;
static uint8_t g_light_reset_choice = 0;

static void request_light_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

static const char* get_light_display_mode_name(uint8_t mode) {
    switch (mode) {
        case 1: return L(ST_LOG_PCT);
        case 2: return L(ST_RAW_ADC);
        case 0:
        default:
            return L(ST_LUX_UNIT);
    }
}

void start_light_menu() {
    // Snapshot persisted values so the menu opens from the active config.
    g_light_menu_state = LIGHT_MODE_MENU;
    g_light_menu_index = 0;
    g_light_dim_max = get_light_threshold_dim();
    g_light_indoor_max = get_light_threshold_indoor();
    g_light_bright_max = get_light_threshold_bright();
    g_light_display_mode = get_light_display_mode();
    g_light_alerts_enabled = get_light_alerts_enabled();
    g_light_reset_choice = 0;
    request_light_redraw(true);
}

bool light_menu_is_active() {
    return g_light_menu_state != LIGHT_MODE_NORMAL;
}

LightMenuState get_light_menu_state() {
    return g_light_menu_state;
}

int get_light_encoder_min() {
    switch (g_light_menu_state) {
        case LIGHT_MODE_MENU: return 0;
        case LIGHT_MODE_EDIT_DIM: return 10;
        case LIGHT_MODE_EDIT_INDOOR: return g_light_dim_max + 1;
        case LIGHT_MODE_EDIT_BRIGHT: return g_light_indoor_max + 1;
        case LIGHT_MODE_EDIT_DISPLAY: return 0;
        case LIGHT_MODE_EDIT_ALERTS: return 0;
        case LIGHT_MODE_CONFIRM_RESET: return 0;
        default: return 0;
    }
}

int get_light_encoder_max() {
    switch (g_light_menu_state) {
        case LIGHT_MODE_MENU: return 4; // calibracion, modo display, alertas, reset, salir
        case LIGHT_MODE_EDIT_DIM: return g_light_indoor_max - 1;
        case LIGHT_MODE_EDIT_INDOOR: return g_light_bright_max - 1;
        case LIGHT_MODE_EDIT_BRIGHT: return 10000;
        case LIGHT_MODE_EDIT_DISPLAY: return 2; // lux, %, raw
        case LIGHT_MODE_EDIT_ALERTS: return 1;
        case LIGHT_MODE_CONFIRM_RESET: return 1;
        default: return 0;
    }
}

int get_light_encoder_value() {
    switch (g_light_menu_state) {
        case LIGHT_MODE_MENU: return g_light_menu_index;
        case LIGHT_MODE_EDIT_DIM: return g_light_dim_max;
        case LIGHT_MODE_EDIT_INDOOR: return g_light_indoor_max;
        case LIGHT_MODE_EDIT_BRIGHT: return g_light_bright_max;
        case LIGHT_MODE_EDIT_DISPLAY: return g_light_display_mode;
        case LIGHT_MODE_EDIT_ALERTS: return g_light_alerts_enabled ? 1 : 0;
        case LIGHT_MODE_CONFIRM_RESET: return g_light_reset_choice;
        default: return 0;
    }
}

void set_light_input_value(int value) {
    int next = constrain(value, get_light_encoder_min(), get_light_encoder_max());

    switch (g_light_menu_state) {
        case LIGHT_MODE_MENU:
            if (next != (int)g_light_menu_index) {
                g_light_menu_index = (uint8_t)next;
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_EDIT_DIM:
            if (next != g_light_dim_max) {
                g_light_dim_max = next;
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_EDIT_INDOOR:
            if (next != g_light_indoor_max) {
                g_light_indoor_max = next;
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_EDIT_BRIGHT:
            if (next != g_light_bright_max) {
                g_light_bright_max = next;
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_EDIT_DISPLAY:
            if (next != (int)g_light_display_mode) {
                g_light_display_mode = (uint8_t)next;
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_EDIT_ALERTS:
            if ((next != 0) != g_light_alerts_enabled) {
                g_light_alerts_enabled = (next != 0);
                request_light_redraw();
            }
            break;
        case LIGHT_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_light_reset_choice) {
                g_light_reset_choice = (uint8_t)next;
                request_light_redraw();
            }
            break;
        default:
            break;
    }
}

uint8_t handle_light_button() {
    bool force_full = false;

    switch (g_light_menu_state) {
        case LIGHT_MODE_MENU:
            if (g_light_menu_index == 0) {
                g_light_menu_state = LIGHT_MODE_EDIT_DIM;
            } else if (g_light_menu_index == 1) {
                g_light_menu_state = LIGHT_MODE_EDIT_DISPLAY;
            } else if (g_light_menu_index == 2) {
                g_light_menu_state = LIGHT_MODE_EDIT_ALERTS;
            } else if (g_light_menu_index == 3) {
                g_light_reset_choice = 0;
                g_light_menu_state = LIGHT_MODE_CONFIRM_RESET;
            } else {
                g_light_menu_state = LIGHT_MODE_NORMAL;
                force_full = true;
            }
            break;
        case LIGHT_MODE_EDIT_DIM:
            g_light_menu_state = LIGHT_MODE_EDIT_INDOOR;
            break;
        case LIGHT_MODE_EDIT_INDOOR:
            g_light_menu_state = LIGHT_MODE_EDIT_BRIGHT;
            break;
        case LIGHT_MODE_EDIT_BRIGHT:
            save_light_settings(g_light_dim_max, g_light_indoor_max, g_light_bright_max);
            g_light_last_saved_menu_index = 0;
            g_light_menu_state = LIGHT_MODE_SAVED;
            break;
        case LIGHT_MODE_EDIT_DISPLAY:
            set_light_display_mode(g_light_display_mode);
            g_light_last_saved_menu_index = 1;
            g_light_menu_state = LIGHT_MODE_SAVED;
            break;
        case LIGHT_MODE_EDIT_ALERTS:
            set_light_alerts_enabled(g_light_alerts_enabled);
            g_light_last_saved_menu_index = 2;
            g_light_menu_state = LIGHT_MODE_SAVED;
            break;
        case LIGHT_MODE_CONFIRM_RESET:
            if (g_light_reset_choice == 1) {
                reset_light_settings();
                g_light_dim_max = get_light_threshold_dim();
                g_light_indoor_max = get_light_threshold_indoor();
                g_light_bright_max = get_light_threshold_bright();
                g_light_display_mode = get_light_display_mode();
                g_light_alerts_enabled = get_light_alerts_enabled();
                g_light_last_saved_menu_index = 3;
                g_light_reset_choice = 0;
                g_light_menu_state = LIGHT_MODE_SAVED;
            } else {
                g_light_menu_state = LIGHT_MODE_MENU;
            }
            break;
        case LIGHT_MODE_SAVED:
            g_light_menu_state = LIGHT_MODE_MENU;
            break;
        case LIGHT_MODE_NORMAL:
        default:
            break;
    }

    request_light_redraw(force_full || g_light_menu_state == LIGHT_MODE_NORMAL);
    return (uint8_t)g_light_menu_state;
}

static void draw_light_menu_screen(bool screen_changed) {
    const int cx = tft.width() / 2;
    static LightMenuState last_drawn_state = LIGHT_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_edit_value = -1;
    static int last_display_mode = -1;
    static int last_reset_choice = -1;
    static int last_saved_kind = -1;

    bool state_changed = screen_changed || (g_light_menu_state != last_drawn_state);
    bool needs_redraw = state_changed;

    if (g_light_menu_state == LIGHT_MODE_MENU) {
        needs_redraw = needs_redraw || (last_menu_index != (int)g_light_menu_index);
    } else if (g_light_menu_state >= LIGHT_MODE_EDIT_DIM && g_light_menu_state <= LIGHT_MODE_EDIT_BRIGHT) {
        needs_redraw = needs_redraw || (last_edit_value != get_light_encoder_value());
    } else if (g_light_menu_state == LIGHT_MODE_EDIT_DISPLAY) {
        needs_redraw = needs_redraw || (last_display_mode != (int)g_light_display_mode);
    } else if (g_light_menu_state == LIGHT_MODE_EDIT_ALERTS) {
        needs_redraw = needs_redraw || (last_display_mode != (int)g_light_alerts_enabled);
    } else if (g_light_menu_state == LIGHT_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_light_reset_choice);
    } else if (g_light_menu_state == LIGHT_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != (int)g_light_last_saved_menu_index);
    }

    if (!needs_redraw) return;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_LIGHT), TFT_YELLOW);
        last_menu_index = -1;
        last_edit_value = -1;
        last_display_mode = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
    }

    if (g_light_menu_state == LIGHT_MODE_MENU) {
        // Root menu: calibration, display mode, alerts, reset, exit.
        const char* items[] = {
            tr("Calibración", "Calibració", "Calibration"),
            tr("Modo display", "Mode display", "Display mode"),
            tr("Alertas", "Alertes", "Alerts"),
            tr("Reset", "Reset", "Reset"),
            tr("Salir", "Sortir", "Exit")
        };
        drawCenteredMenuList(items, 5, g_light_menu_index, LM_MENU5_Y0, LM_MENU5_GAP);
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_light_menu_index;
    } else if (g_light_menu_state >= LIGHT_MODE_EDIT_DIM && g_light_menu_state <= LIGHT_MODE_EDIT_BRIGHT) {
        // Threshold editing keeps the current value centered and compact.
        const char* title = tr("Max penumbra", "Max penombra", "Max dim");
        int value = g_light_dim_max;
        if (g_light_menu_state == LIGHT_MODE_EDIT_INDOOR) {
            title = tr("Max interior", "Max interior", "Max indoor");
            value = g_light_indoor_max;
        } else if (g_light_menu_state == LIGHT_MODE_EDIT_BRIGHT) {
            title = tr("Max brillante", "Max brillant", "Max bright");
            value = g_light_bright_max;
        }

        char value_buf[16];
        format_light_threshold_value(value, value_buf, sizeof(value_buf));
        drawCenteredMenuValueScreen(title, value_buf, TFT_WHITE, MENU_VALUE_FONT_TIMER, L(ST_TURN_PUSH));
        last_edit_value = value;
    } else if (g_light_menu_state == LIGHT_MODE_EDIT_DISPLAY) {
        // Display mode changes how the large value is presented.
        drawCenteredMenuValueScreen(tr("Modo display", "Mode display", "Display mode"),
                                    get_light_display_mode_name(g_light_display_mode),
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_display_mode = (int)g_light_display_mode;
    } else if (g_light_menu_state == LIGHT_MODE_EDIT_ALERTS) {
        // Alert toggle mirrors the same ON/OFF interaction used elsewhere.
        drawCenteredMenuValueScreen(tr("Alertas", "Alertes", "Alerts"),
                                    g_light_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_light_alerts_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_display_mode = (int)g_light_alerts_enabled;
    } else if (g_light_menu_state == LIGHT_MODE_CONFIRM_RESET) {
        drawResetChoicePrompt(tr("Reset", "Reset", "Reset"),
                              tr("Valores por defecto", "Valors per defecte", "Default values"),
                              tr("de luz", "de llum", "for light"),
                              tr("NO", "NO", "NO"),
                              tr("SÍ", "SÍ", "YES"),
                              g_light_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_light_reset_choice;
    } else if (g_light_menu_state == LIGHT_MODE_SAVED) {
        // Saved state previews the updated setting before returning.
        const char* saved_title = tr("Guardado", "Desat", "Saved");

        if (g_light_last_saved_menu_index == 0) {
            char line_buf_1[18];
            char line_buf_2[18];
            char line_buf_3[18];
            const char* lines[3];
            const uint16_t colors[3] = { TFT_CYAN, TFT_GREEN, TFT_YELLOW };
            snprintf(line_buf_1, sizeof(line_buf_1), "%s < %d", tr("Pen", "Pen", "Dim"), g_light_dim_max);
            snprintf(line_buf_2, sizeof(line_buf_2), "%s < %d", tr("Int", "Int", "In"), g_light_indoor_max);
            snprintf(line_buf_3, sizeof(line_buf_3), "%s < %d", tr("Bri", "Bri", "Bri"), g_light_bright_max);
            lines[0] = line_buf_1;
            lines[1] = line_buf_2;
            lines[2] = line_buf_3;
            drawCenteredMenuFrame(saved_title, TFT_GREEN, L(ST_PUSH_MENU), TFT_CYAN);
            drawCenteredMenuBodyLines(lines, colors, 3, MENU_TEXT_FONT_SMALL, LM_SUMMARY3_Y0, LM_SUMMARY3_GAP);
        } else if (g_light_last_saved_menu_index == 1) {
            drawCenteredMenuSavedScreen(saved_title,
                                        get_light_display_mode_name(g_light_display_mode),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_light_last_saved_menu_index == 2) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_light_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                        g_light_alerts_enabled ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else {
            drawCenteredMenuSavedScreen(saved_title,
                                        tr("Valores por defecto", "Valors per defecte", "Default values"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
        last_saved_kind = (int)g_light_last_saved_menu_index;
    }

    last_drawn_state = g_light_menu_state;
}

void draw_light_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (light_menu_is_active()) {
        draw_light_menu_screen(screen_changed);
        return;
    }

    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    float lux = g_ui_readings_snapshot.ldr;
    float raw = g_ui_readings_snapshot.ldr_raw;

    int dim_max = get_light_threshold_dim();
    int indoor_max = get_light_threshold_indoor();
    int bright_max = get_light_threshold_bright();
    uint8_t display_mode = get_light_display_mode();
    bool alerts_enabled = get_light_alerts_enabled();
    uint8_t alert_state = alert_engine_get_code(AlertSensor::Light);

    const char* categoryText;
    uint16_t categoryColor;
    int category_id = 0;
    if (lux < 10.0f) {
        categoryText = L(ST_DARK);
        categoryColor = TFT_DARKGREY;
        category_id = 0;
    } else if (lux < dim_max) {
        categoryText = L(ST_DIM);
        categoryColor = TFT_CYAN;
        category_id = 1;
    } else if (lux < indoor_max) {
        categoryText = L(ST_INDOOR);
        categoryColor = TFT_GREEN;
        category_id = 2;
    } else if (lux < bright_max) {
        categoryText = L(ST_BRIGHT);
        categoryColor = TFT_YELLOW;
        category_id = 3;
    } else {
        categoryText = L(ST_SUNLIGHT);
        categoryColor = TFT_ORANGE;
        category_id = 4;
    }

    float log_pct = (lux < 1.0f) ? 0.0f : (log10f(lux) / log10f(20000.0f)) * 100.0f;
    log_pct = constrain(log_pct, 0.0f, 100.0f);

    const int cx = tft.width() / 2;
    char value_str[10];
    const char* unit_str = "lux";
    float display_val = lux;

    if (display_mode == 1) {
        display_val = log_pct;
        unit_str = "%";
    } else if (display_mode == 2) {
        display_val = raw;
        unit_str = "raw";
    }

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_LIGHT), TFT_YELLOW);
    }

    static int last_display_cache = -1;
    static int last_category_id = -1;
    static int last_display_mode = -1;
    static uint8_t last_alert_state = ALERT_CODE_OFF;

    int display_cache = (int)roundf(display_val);
    if (!screen_changed
        && display_cache == last_display_cache
        && category_id == last_category_id
        && display_mode == (uint8_t)last_display_mode
        && alert_state == last_alert_state) {
        return;
    }

    last_display_cache = display_cache;
    last_category_id = category_id;
    last_display_mode = (int)display_mode;
    last_alert_state = alert_state;

    if (display_mode == 0 && display_val >= 10000.0f) {
        snprintf(value_str, sizeof(value_str), "%.0fk", display_val / 1000.0f);
        unit_str = "";
    } else {
        snprintf(value_str, sizeof(value_str), "%.0f", display_val);
    }

    tft.fillRect(0, LB_VALUE_TOP - 4, tft.width(), 52, BACKGROUND_COLOR);
    tft.setFreeFont(FONT_VALUE);
    int numW = tft.textWidth(value_str);
    tft.setFreeFont(FONT_BODY);
    int unitW = tft.textWidth(unit_str);
    int startX = cx - (numW + unitW) / 2;
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_VALUE);
    tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
    tft.drawString(value_str, startX, LB_VALUE_TOP);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
    tft.drawString(unit_str, startX + numW, LB_VALUE_TOP);

    drawBarGraph(LB_BAR_X, LB_BAR_Y, LB_BAR_W, LB_BAR_H, categoryColor, log_pct, 0.0f, 100.0f);

    tft.fillRect(0, LB_CATEGORY_Y - 8, tft.width(), 16, BACKGROUND_COLOR);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(categoryColor, BACKGROUND_COLOR);
    tft.drawString(categoryText, cx, LB_CATEGORY_Y);
    tft.setTextFont(0);

    draw_light_alert_jewel(alert_state, alerts_enabled);
}

