// ui_sound.cpp
// Pantalla de nivel de sonido (microfono integrado).

#include "ui_sound.h"
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

namespace {

const char* tr(const char* es, const char* cat, const char* en) {
    switch (g_language) {
        case LANG_CAT: return cat;
        case LANG_EN: return en;
        case LANG_ES:
        default: return es;
    }
}

void apply_sound_rgb(uint8_t alert_state) {
    switch (alert_state) {
        case ALERT_CODE_HIGH:
            set_rgb(255, 140, 0);
            break;
        case ALERT_CODE_CRITICAL:
            set_rgb(255, 0, 0);
            break;
        case ALERT_CODE_OK:
        case ALERT_CODE_OFF:
        default:
            set_rgb(255, 0, 255);
            break;
    }
}

static void draw_sound_alert_jewel(uint8_t alert_state, bool alerts_enabled) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (!alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
    } else if (alert_state == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_ORANGE;
    } else if (alert_state == ALERT_CODE_CRITICAL) {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_RED;
    }

    drawAlertJewel(L_ALERT_JEWEL_X, L_ALERT_JEWEL_Y, jewel_state, jewel_color);
}

void draw_sound_header(const char* title, uint16_t color) {
    drawHeader(title, color);
}

} // namespace

static SoundMenuState g_sound_menu_state = SOUND_MODE_NORMAL;
static uint8_t g_sound_menu_index = 0;
static uint8_t g_sound_last_saved_menu_index = 0;
static int g_sound_quiet_max = 20;
static int g_sound_normal_max = 60;
static int g_sound_loud_max = 85;
static bool g_sound_alerts_enabled = false;
static uint8_t g_sound_reset_choice = 0;

static void request_sound_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

void start_sound_menu() {
    g_sound_menu_state = SOUND_MODE_MENU;
    g_sound_menu_index = 0;
    g_sound_quiet_max = get_sound_threshold_quiet();
    g_sound_normal_max = get_sound_threshold_normal();
    g_sound_loud_max = get_sound_threshold_loud();
    g_sound_alerts_enabled = get_sound_alerts_enabled();
    g_sound_reset_choice = 0;
    request_sound_redraw(true);
}

bool sound_menu_is_active() {
    return g_sound_menu_state != SOUND_MODE_NORMAL;
}

SoundMenuState get_sound_menu_state() {
    return g_sound_menu_state;
}

int get_sound_encoder_min() {
    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU: return 0;
        case SOUND_MODE_EDIT_QUIET: return 0;
        case SOUND_MODE_EDIT_NORMAL: return g_sound_quiet_max + 1;
        case SOUND_MODE_EDIT_LOUD: return g_sound_normal_max + 1;
        case SOUND_MODE_EDIT_ALERTS: return 0;
        case SOUND_MODE_CONFIRM_RESET: return 0;
        default: return 0;
    }
}

int get_sound_encoder_max() {
    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU: return 3; // calibracion, alertas, reset, salir
        case SOUND_MODE_EDIT_QUIET: return g_sound_normal_max - 1;
        case SOUND_MODE_EDIT_NORMAL: return g_sound_loud_max - 1;
        case SOUND_MODE_EDIT_LOUD: return 100;
        case SOUND_MODE_EDIT_ALERTS: return 1;
        case SOUND_MODE_CONFIRM_RESET: return 1;
        default: return 0;
    }
}

int get_sound_encoder_value() {
    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU: return g_sound_menu_index;
        case SOUND_MODE_EDIT_QUIET: return g_sound_quiet_max;
        case SOUND_MODE_EDIT_NORMAL: return g_sound_normal_max;
        case SOUND_MODE_EDIT_LOUD: return g_sound_loud_max;
        case SOUND_MODE_EDIT_ALERTS: return g_sound_alerts_enabled ? 1 : 0;
        case SOUND_MODE_CONFIRM_RESET: return g_sound_reset_choice;
        default: return 0;
    }
}

void set_sound_input_value(int value) {
    int next = constrain(value, get_sound_encoder_min(), get_sound_encoder_max());

    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU:
            if (next != (int)g_sound_menu_index) {
                g_sound_menu_index = (uint8_t)next;
                request_sound_redraw();
            }
            break;
        case SOUND_MODE_EDIT_QUIET:
            if (next != g_sound_quiet_max) {
                g_sound_quiet_max = next;
                request_sound_redraw();
            }
            break;
        case SOUND_MODE_EDIT_NORMAL:
            if (next != g_sound_normal_max) {
                g_sound_normal_max = next;
                request_sound_redraw();
            }
            break;
        case SOUND_MODE_EDIT_LOUD:
            if (next != g_sound_loud_max) {
                g_sound_loud_max = next;
                request_sound_redraw();
            }
            break;
        case SOUND_MODE_EDIT_ALERTS:
            if (next != (g_sound_alerts_enabled ? 1 : 0)) {
                g_sound_alerts_enabled = (next == 1);
                request_sound_redraw();
            }
            break;
        case SOUND_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_sound_reset_choice) {
                g_sound_reset_choice = (uint8_t)next;
                request_sound_redraw();
            }
            break;
        default:
            break;
    }
}

uint8_t handle_sound_button() {
    bool force_full = false;

    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU:
            if (g_sound_menu_index == 0) {
                g_sound_menu_state = SOUND_MODE_EDIT_QUIET;
            } else if (g_sound_menu_index == 1) {
                g_sound_menu_state = SOUND_MODE_EDIT_ALERTS;
            } else if (g_sound_menu_index == 2) {
                g_sound_reset_choice = 0;
                g_sound_menu_state = SOUND_MODE_CONFIRM_RESET;
            } else {
                g_sound_menu_state = SOUND_MODE_NORMAL;
                force_full = true;
            }
            break;
        case SOUND_MODE_EDIT_QUIET:
            g_sound_menu_state = SOUND_MODE_EDIT_NORMAL;
            break;
        case SOUND_MODE_EDIT_NORMAL:
            g_sound_menu_state = SOUND_MODE_EDIT_LOUD;
            break;
        case SOUND_MODE_EDIT_LOUD:
            save_sound_settings(g_sound_quiet_max, g_sound_normal_max, g_sound_loud_max);
            g_sound_last_saved_menu_index = 0;
            g_sound_menu_state = SOUND_MODE_SAVED;
            break;
        case SOUND_MODE_EDIT_ALERTS:
            set_sound_alerts_enabled(g_sound_alerts_enabled);
            g_sound_last_saved_menu_index = 1;
            g_sound_menu_state = SOUND_MODE_SAVED;
            break;
        case SOUND_MODE_CONFIRM_RESET:
            if (g_sound_reset_choice == 1) {
                reset_sound_settings();
                g_sound_quiet_max = get_sound_threshold_quiet();
                g_sound_normal_max = get_sound_threshold_normal();
                g_sound_loud_max = get_sound_threshold_loud();
                g_sound_alerts_enabled = get_sound_alerts_enabled();
                g_sound_last_saved_menu_index = 2;
                g_sound_reset_choice = 0;
                g_sound_menu_state = SOUND_MODE_SAVED;
            } else {
                g_sound_menu_state = SOUND_MODE_MENU;
            }
            break;
        case SOUND_MODE_SAVED:
            g_sound_menu_state = SOUND_MODE_MENU;
            break;
        case SOUND_MODE_NORMAL:
        default:
            break;
    }

    request_sound_redraw(force_full || g_sound_menu_state == SOUND_MODE_NORMAL);
    return (uint8_t)g_sound_menu_state;
}

static void draw_sound_menu_screen(bool screen_changed) {
    const int cx = tft.width() / 2;
    static SoundMenuState last_drawn_state = SOUND_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_edit_value = -1;
    static int last_alert_value = -1;
    static int last_reset_choice = -1;
    static int last_saved_kind = -1;

    bool state_changed = screen_changed || (g_sound_menu_state != last_drawn_state);
    bool needs_redraw = state_changed;

    if (g_sound_menu_state == SOUND_MODE_MENU) {
        needs_redraw = needs_redraw || (last_menu_index != (int)g_sound_menu_index);
    } else if (g_sound_menu_state >= SOUND_MODE_EDIT_QUIET && g_sound_menu_state <= SOUND_MODE_EDIT_LOUD) {
        needs_redraw = needs_redraw || (last_edit_value != get_sound_encoder_value());
    } else if (g_sound_menu_state == SOUND_MODE_EDIT_ALERTS) {
        needs_redraw = needs_redraw || (last_alert_value != (g_sound_alerts_enabled ? 1 : 0));
    } else if (g_sound_menu_state == SOUND_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_sound_reset_choice);
    } else if (g_sound_menu_state == SOUND_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != (int)g_sound_last_saved_menu_index);
    }

    if (!needs_redraw) return;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_sound_header(L(TIT_SOUND), TFT_MAGENTA);
        last_menu_index = -1;
        last_edit_value = -1;
        last_alert_value = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
    }

    if (g_sound_menu_state == SOUND_MODE_MENU) {
        const char* items[] = {
            tr("Calibración", "Calibració", "Calibration"),
            tr("Alertas", "Alertes", "Alerts"),
            tr("Reset", "Reset", "Reset"),
            tr("Salir", "Sortir", "Exit")
        };
        drawCenteredMenuList(items, 4, g_sound_menu_index, LM_MENU4_Y0, LM_MENU4_GAP);
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_sound_menu_index;
    } else if (g_sound_menu_state >= SOUND_MODE_EDIT_QUIET && g_sound_menu_state <= SOUND_MODE_EDIT_LOUD) {
        const char* title = tr("Max silencio", "Max silenci", "Max quiet");
        int value = g_sound_quiet_max;
        if (g_sound_menu_state == SOUND_MODE_EDIT_NORMAL) {
            title = tr("Max normal", "Max normal", "Max normal");
            value = g_sound_normal_max;
        } else if (g_sound_menu_state == SOUND_MODE_EDIT_LOUD) {
            title = tr("Max alto", "Max alt", "Max loud");
            value = g_sound_loud_max;
        }

        char value_buf[12];
        snprintf(value_buf, sizeof(value_buf), "< %d%%", value);
        drawCenteredMenuValueScreen(title, value_buf, TFT_WHITE, MENU_VALUE_FONT_TIMER, L(ST_TURN_PUSH));
        last_edit_value = value;
    } else if (g_sound_menu_state == SOUND_MODE_EDIT_ALERTS) {
        drawCenteredMenuValueScreen(tr("Alertas", "Alertes", "Alerts"),
                                    g_sound_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_sound_alerts_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_alert_value = g_sound_alerts_enabled ? 1 : 0;
    } else if (g_sound_menu_state == SOUND_MODE_CONFIRM_RESET) {
        drawResetChoicePrompt(tr("Reset", "Reset", "Reset"),
                              tr("Valores por defecto", "Valors per defecte", "Default values"),
                              tr("de sonido", "de so", "for sound"),
                              tr("NO", "NO", "NO"),
                              tr("SÍ", "SÍ", "YES"),
                              g_sound_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_sound_reset_choice;
    } else if (g_sound_menu_state == SOUND_MODE_SAVED) {
        const char* saved_title = tr("Guardado", "Desat", "Saved");

        if (g_sound_last_saved_menu_index == 0) {
            char line_buf_1[18];
            char line_buf_2[18];
            char line_buf_3[18];
            const char* lines[3];
            const uint16_t colors[3] = { TFT_GREEN, TFT_YELLOW, TFT_ORANGE };
            snprintf(line_buf_1, sizeof(line_buf_1), "%s < %d", tr("Sil", "Sil", "Qui"), g_sound_quiet_max);
            snprintf(line_buf_2, sizeof(line_buf_2), "%s < %d", tr("Nor", "Nor", "Nor"), g_sound_normal_max);
            snprintf(line_buf_3, sizeof(line_buf_3), "%s < %d", tr("Alt", "Alt", "Loud"), g_sound_loud_max);
            lines[0] = line_buf_1;
            lines[1] = line_buf_2;
            lines[2] = line_buf_3;
            drawCenteredMenuFrame(saved_title, TFT_GREEN, L(ST_PUSH_MENU), TFT_CYAN);
            drawCenteredMenuBodyLines(lines, colors, 3, MENU_TEXT_FONT_SMALL, LM_SUMMARY3_Y0, LM_SUMMARY3_GAP);
        } else if (g_sound_last_saved_menu_index == 1) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_sound_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                        g_sound_alerts_enabled ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_TIMER,
                                        L(ST_PUSH_MENU));
        } else {
            drawCenteredMenuSavedScreen(saved_title,
                                        tr("Valores por defecto", "Valors per defecte", "Default values"),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
        last_saved_kind = (int)g_sound_last_saved_menu_index;
    }

    last_drawn_state = g_sound_menu_state;
}

void draw_sound_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (sound_menu_is_active()) {
        draw_sound_menu_screen(screen_changed);
        return;
    }

    const uint16_t TITLE_COLOR = TFT_MAGENTA;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    float level = (float)g_ui_readings_snapshot.mic;

    int quiet_max = get_sound_threshold_quiet();
    int normal_max = get_sound_threshold_normal();
    int loud_max = get_sound_threshold_loud();
    bool alerts_enabled = get_sound_alerts_enabled();

    const char* categoryText;
    uint16_t categoryColor;
    int category_id = 0;
    if (level < quiet_max) {
        categoryText = L(ST_QUIET);
        categoryColor = TFT_GREEN;
        category_id = 0;
    } else if (level < normal_max) {
        categoryText = L(ST_NORMAL);
        categoryColor = TFT_YELLOW;
        category_id = 1;
    } else if (level < loud_max) {
        categoryText = L(ST_LOUD);
        categoryColor = TFT_ORANGE;
        category_id = 2;
    } else {
        categoryText = L(ST_VERY_LOUD);
        categoryColor = TFT_RED;
        category_id = 3;
    }

    uint8_t alert_state = alert_engine_get_code(AlertSensor::Sound);

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        draw_sound_header(L(TIT_SOUND), TITLE_COLOR);
    }

    static int last_sound_drawn = -1;
    static int last_category_id = -1;
    static uint8_t last_alert_state = ALERT_CODE_OFF;
    static bool last_alerts_enabled = false;
    int sound_cache = (int)roundf(level);
    if (!screen_changed
        && sound_cache == last_sound_drawn
        && category_id == last_category_id
        && alert_state == last_alert_state
        && alerts_enabled == last_alerts_enabled) {
        return;
    }

    last_sound_drawn = sound_cache;
    last_category_id = category_id;

    if (screen_changed || alert_state != last_alert_state) {
        apply_sound_rgb(alert_state);
        last_alert_state = alert_state;
    }

    const int cx = tft.width() / 2;
    char levelStr[5];
    snprintf(levelStr, sizeof(levelStr), "%.0f", level);

    tft.fillRect(0, LB_VALUE_TOP - 4, tft.width(), 52, BACKGROUND_COLOR);
    tft.setFreeFont(FONT_VALUE);
    int numW = tft.textWidth(levelStr);
    tft.setFreeFont(FONT_BODY);
    int unitW = tft.textWidth("%");
    int startX = cx - (numW + unitW) / 2;
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_VALUE);
    tft.setTextColor(categoryColor, BACKGROUND_COLOR);
    tft.drawString(levelStr, startX, LB_VALUE_TOP);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
    tft.drawString("%", startX + numW, LB_VALUE_TOP);

    drawBarGraph(LB_BAR_X, LB_BAR_Y, LB_BAR_W, LB_BAR_H, categoryColor, level, 0.0f, 100.0f);

    tft.fillRect(0, LB_CATEGORY_Y - 8, tft.width(), 18, BACKGROUND_COLOR);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(categoryColor, BACKGROUND_COLOR);
    tft.drawString(categoryText, cx, LB_CATEGORY_Y - 5);
    tft.setTextFont(0);

    draw_sound_alert_jewel(alert_state, alerts_enabled);

    last_alerts_enabled = alerts_enabled;
}

