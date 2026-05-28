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

namespace {

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

static bool sound_level_valid(float level) {
    return !isnan(level) && level >= 0.0f && level <= 100.0f;
}

static void draw_sound_alert_jewel(uint8_t alert_state, bool alerts_enabled, bool no_sensor) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_sensor || !alerts_enabled) {
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

void draw_sound_header(const char* title) {
    drawHeader(title);
}

void draw_sound_value_with_unit(const char* value,
                                const char* unit,
                                uint16_t value_color,
                                uint16_t background_color,
                                bool screen_changed) {
    static int last_clear_x = 0;
    static int last_clear_w = 0;
    static int last_clear_h = 0;
    static bool has_last = false;

    const int cx = tft.width() / 2;
    tft.setFreeFont(FONT_VALUE);
    const int value_line_h = tft.fontHeight();
    const int value_w = tft.textWidth(value);
    tft.setFreeFont(FONT_BODY);
    const int unit_line_h = tft.fontHeight();
    const int unit_w = tft.textWidth(unit);

    const int clear_h = max(value_line_h, unit_line_h) + 4;
    const int clear_y = LB_VALUE_TOP - 2;
    const int start_x = cx - ((value_w + unit_w) / 2);
    const int clear_x = max(0, start_x - 2);
    const int clear_w = min(tft.width() - clear_x, value_w + unit_w + 4);

    if (!screen_changed) {
        int union_x = clear_x;
        int union_w = clear_w;
        int union_h = clear_h;
        if (has_last) {
            const int union_x2 = max(clear_x + clear_w, last_clear_x + last_clear_w);
            union_x = min(clear_x, last_clear_x);
            union_w = union_x2 - union_x;
            union_h = max(clear_h, last_clear_h);
        }
        tft.fillRect(union_x, clear_y, union_w, union_h, background_color);
    }

    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_VALUE);
    tft.setTextColor(value_color, background_color);
    tft.drawString(value, start_x, LB_VALUE_TOP);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_DARKGREY, background_color);
    tft.drawString(unit, start_x + value_w, LB_VALUE_TOP);

    last_clear_x = clear_x;
    last_clear_w = clear_w;
    last_clear_h = clear_h;
    has_last = true;
}

} // namespace

static SoundMenuState g_sound_menu_state = SOUND_MODE_NORMAL;
static uint8_t g_sound_menu_index = 0;
static uint8_t g_sound_last_saved_menu_index = 0;
static int g_sound_quiet_max = 20;
static int g_sound_normal_max = 60;
static int g_sound_loud_max = 85;
static bool g_sound_alerts_enabled = false;
static bool g_sound_marks_visible = false;
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
    g_sound_marks_visible = get_sound_range_marks_visible();
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
        case SOUND_MODE_EDIT_MARKS: return 0;
        case SOUND_MODE_CONFIRM_RESET: return 0;
        default: return 0;
    }
}

int get_sound_encoder_max() {
    switch (g_sound_menu_state) {
        case SOUND_MODE_MENU: return 4; // niveles, alertas, ver limites, reset, salir
        case SOUND_MODE_EDIT_QUIET: return g_sound_normal_max - 1;
        case SOUND_MODE_EDIT_NORMAL: return g_sound_loud_max - 1;
        case SOUND_MODE_EDIT_LOUD: return 100;
        case SOUND_MODE_EDIT_ALERTS: return 1;
        case SOUND_MODE_EDIT_MARKS: return 1;
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
        case SOUND_MODE_EDIT_MARKS: return g_sound_marks_visible ? 1 : 0;
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
        case SOUND_MODE_EDIT_MARKS:
            if (next != (g_sound_marks_visible ? 1 : 0)) {
                g_sound_marks_visible = (next == 1);
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
                g_sound_menu_state = SOUND_MODE_EDIT_MARKS;
            } else if (g_sound_menu_index == 3) {
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
        case SOUND_MODE_EDIT_MARKS:
            set_sound_range_marks_visible(g_sound_marks_visible);
            g_sound_last_saved_menu_index = 2;
            g_sound_menu_state = SOUND_MODE_SAVED;
            break;
        case SOUND_MODE_CONFIRM_RESET:
            if (g_sound_reset_choice == 1) {
                reset_sound_settings();
                g_sound_quiet_max = get_sound_threshold_quiet();
                g_sound_normal_max = get_sound_threshold_normal();
                g_sound_loud_max = get_sound_threshold_loud();
                g_sound_alerts_enabled = get_sound_alerts_enabled();
                g_sound_marks_visible = get_sound_range_marks_visible();
                g_sound_last_saved_menu_index = 3;
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
    static int last_marks_value = -1;
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
    } else if (g_sound_menu_state == SOUND_MODE_EDIT_MARKS) {
        needs_redraw = needs_redraw || (last_marks_value != (g_sound_marks_visible ? 1 : 0));
    } else if (g_sound_menu_state == SOUND_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_sound_reset_choice);
    } else if (g_sound_menu_state == SOUND_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != (int)g_sound_last_saved_menu_index);
    }

    if (!needs_redraw) return;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_sound_header(L(TIT_SOUND));
        last_menu_index = -1;
        last_edit_value = -1;
        last_alert_value = -1;
        last_marks_value = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
    }

    if (g_sound_menu_state == SOUND_MODE_MENU) {
        const char* items[] = {
            L(MENU_LEVELS),
            L(MENU_ALERTS),
            L(MENU_SHOW_LIMITS)
        };
        drawSettingsGridMenu(items, 3, g_sound_menu_index, L(MENU_RESET), L(MENU_EXIT));
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_sound_menu_index;
    } else if (g_sound_menu_state >= SOUND_MODE_EDIT_QUIET && g_sound_menu_state <= SOUND_MODE_EDIT_LOUD) {
        const char* title = L(MENU_SND_MAX_QUIET);
        int value = g_sound_quiet_max;
        if (g_sound_menu_state == SOUND_MODE_EDIT_NORMAL) {
            title = L(MENU_SND_MAX_NORMAL);
            value = g_sound_normal_max;
        } else if (g_sound_menu_state == SOUND_MODE_EDIT_LOUD) {
            title = L(MENU_SND_MAX_LOUD);
            value = g_sound_loud_max;
        }

        char value_buf[12];
        snprintf(value_buf, sizeof(value_buf), "< %d%%", value);
        drawCenteredMenuValueScreen(title, value_buf, TFT_WHITE, MENU_VALUE_FONT_TIMER, L(ST_TURN_PUSH));
        last_edit_value = value;
    } else if (g_sound_menu_state == SOUND_MODE_EDIT_ALERTS) {
        drawCenteredMenuValueScreen(L(MENU_ALERTS),
                                    g_sound_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_sound_alerts_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_alert_value = g_sound_alerts_enabled ? 1 : 0;
    } else if (g_sound_menu_state == SOUND_MODE_EDIT_MARKS) {
        drawCenteredMenuValueScreen(L(MENU_SHOW_LIMITS),
                                    g_sound_marks_visible ? L(ST_ON) : L(ST_OFF),
                                    g_sound_marks_visible ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_marks_value = g_sound_marks_visible ? 1 : 0;
    } else if (g_sound_menu_state == SOUND_MODE_CONFIRM_RESET) {
        drawResetChoicePrompt(L(MENU_RESET),
                              L(MENU_DEFAULTS),
                              L(MENU_RESET_SUB_SOUND),
                              L(MENU_NO),
                              L(MENU_YES),
                              g_sound_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_sound_reset_choice;
    } else if (g_sound_menu_state == SOUND_MODE_SAVED) {
        const char* saved_title = L(MENU_SAVED);

        if (g_sound_last_saved_menu_index == 0) {
            char line_buf_1[18];
            char line_buf_2[18];
            char line_buf_3[18];
            const char* lines[3];
            const uint16_t colors[3] = { TFT_GREEN, TFT_YELLOW, TFT_ORANGE };
            snprintf(line_buf_1, sizeof(line_buf_1), "%s < %d%%", L(MENU_SND_ABR_QUIET), g_sound_quiet_max);
            snprintf(line_buf_2, sizeof(line_buf_2), "%s < %d%%", L(MENU_SND_ABR_NORMAL), g_sound_normal_max);
            snprintf(line_buf_3, sizeof(line_buf_3), "%s < %d%%", L(MENU_SND_ABR_LOUD), g_sound_loud_max);
            lines[0] = line_buf_1;
            lines[1] = line_buf_2;
            lines[2] = line_buf_3;
            drawCenteredMenuFrame(saved_title, TFT_MAGENTA, L(ST_PUSH_MENU), TFT_CYAN);
            drawCenteredMenuBodyLines(lines, colors, 3, MENU_TEXT_FONT_SMALL, LM_SUMMARY3_Y0, LM_SUMMARY3_GAP);
        } else if (g_sound_last_saved_menu_index == 1) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_sound_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                        g_sound_alerts_enabled ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_TIMER,
                                        L(ST_PUSH_MENU));
        } else if (g_sound_last_saved_menu_index == 2) {
            drawCenteredMenuSavedScreen(saved_title,
                                        g_sound_marks_visible ? L(ST_ON) : L(ST_OFF),
                                        g_sound_marks_visible ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else {
            drawCenteredMenuSavedScreen(saved_title,
                                        L(MENU_DEFAULTS),
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
    const bool sound_valid = sound_level_valid(level);

    int quiet_max = get_sound_threshold_quiet();
    int normal_max = get_sound_threshold_normal();
    int loud_max = get_sound_threshold_loud();
    bool alerts_enabled = get_sound_alerts_enabled();

    const char* categoryText;
    uint16_t categoryColor;
    int category_id = 0;
    if (!sound_valid) {
        categoryText = L(ST_NO_SENSOR);
        categoryColor = TFT_RED;
        category_id = -1;
    } else if (level < quiet_max) {
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
        draw_sound_header(L(TIT_SOUND));
    }

    static int last_sound_drawn = -1;
    static int last_category_id = -1;
    static uint8_t last_alert_state = ALERT_CODE_OFF;
    static bool last_alerts_enabled = false;
    static uint8_t last_jewel_code = 255;
    static bool last_jewel_alerts_en = false;
    static bool last_jewel_no_sensor = false;
    int sound_cache = sound_valid ? (int)roundf(level) : -32768;
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
    const char* unitStr = "%";
    if (sound_valid) {
        snprintf(levelStr, sizeof(levelStr), "%.0f", level);
    } else {
        snprintf(levelStr, sizeof(levelStr), "---");
        unitStr = "";
    }

    draw_sound_value_with_unit(levelStr,
                               unitStr,
                               sound_valid ? categoryColor : TFT_DARKGREY,
                               BACKGROUND_COLOR,
                               screen_changed);

    drawCachedBarGraph(LB_BAR_X,
                       LB_BAR_Y,
                       LB_BAR_W,
                       LB_BAR_H,
                       sound_valid ? categoryColor : TFT_DARKGREY,
                       sound_valid ? level : 0.0f,
                       0.0f,
                       100.0f,
                       screen_changed);

    const int category_clear_x = L_ALERT_JEWEL_X + 10;
    tft.fillRect(category_clear_x, LB_CATEGORY_Y - 8, tft.width() - category_clear_x, 18, BACKGROUND_COLOR);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(categoryColor, BACKGROUND_COLOR);
    tft.drawString(categoryText, cx, LB_CATEGORY_Y - 5);
    tft.setTextFont(0);

    if (screen_changed
        || alert_state != last_jewel_code
        || alerts_enabled != last_jewel_alerts_en
        || (!sound_valid) != last_jewel_no_sensor) {
        draw_sound_alert_jewel(alert_state, alerts_enabled, !sound_valid);
        last_jewel_code = alert_state;
        last_jewel_alerts_en = alerts_enabled;
        last_jewel_no_sensor = !sound_valid;
    }

    last_alerts_enabled = alerts_enabled;
}
