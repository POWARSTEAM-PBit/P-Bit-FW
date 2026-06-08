// ui_soil.cpp
// Pantalla de humedad del suelo (sensor capacitivo externo, referencia PCB IO35).

#include "ui_soil.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include "palette.h"
#include "hw.h"
#include "led_control.h"
#include "alert_engine.h"
#include "external_sensor_state.h"
#include "runtime_events.h"
#include "sensor_visuals.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

static SoilCalibrationState g_soil_cal_state = SOIL_CAL_IDLE;
static uint8_t g_soil_menu_index = 0;
static int g_soil_cal_dry_raw = 0;
static int g_soil_cal_wet_raw = 0;
static int g_soil_cal_live_raw = 0;
static int g_soil_thr_dry_pct = 20;
static int g_soil_thr_optimal_pct = 50;
static int g_soil_thr_moist_pct = 80;
static bool g_soil_alerts_enabled = true;
static uint8_t g_soil_reset_choice = 0;
static uint8_t g_soil_save_choice = 0;
static uint8_t g_soil_capture_choice = 1;

constexpr int SOIL_CAL_TITLE_Y = 44;
constexpr int SOIL_CAL_VALUE_Y = 78;
constexpr int SOIL_CAL_VALUE_BOX_Y = 64;
constexpr int SOIL_CAL_VALUE_BOX_H = 30;
constexpr int SOIL_CAL_SUMMARY_Y1 = 62;
constexpr int SOIL_CAL_SUMMARY_Y2 = 78;
constexpr int SOIL_CAL_SUMMARY_Y3 = 94;
constexpr int SOIL_CAL_SUMMARY_BOX_Y = 54;
constexpr int SOIL_CAL_SUMMARY_BOX_H = 52;
constexpr unsigned long SOIL_CAL_LIVE_SAMPLE_MS = 250UL;
constexpr int SOIL_CAL_LIVE_RAW_DEADBAND = 3;
constexpr int SOIL_CAL_CAPTURE_VALUE_Y = 68;
constexpr int SOIL_CAL_CAPTURE_BUTTON_H = 23;

static void request_soil_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

static int soil_auto_optimal_threshold(int dry_thr, int moist_thr) {
    return constrain((dry_thr + moist_thr) / 2, dry_thr + 1, moist_thr - 1);
}

static void reload_soil_threshold_drafts() {
    g_soil_thr_dry_pct = get_soil_threshold_dry();
    g_soil_thr_moist_pct = get_soil_threshold_moist();
    g_soil_thr_optimal_pct = soil_auto_optimal_threshold(g_soil_thr_dry_pct, g_soil_thr_moist_pct);
}

static void clear_soil_calibration_drafts() {
    g_soil_cal_dry_raw = 0;
    g_soil_cal_wet_raw = 0;
    g_soil_cal_live_raw = 0;
    g_soil_save_choice = 0;
    g_soil_capture_choice = 1;
}

static bool soil_sensor_available_for_calibration() {
    return !pbit_external_sensor_missing(SZ_SOIL, g_ui_readings_snapshot);
}

static void draw_soil_choice_button(int x, int y, int w, int h,
                                    const char* label,
                                    bool selected,
                                    uint16_t accent,
                                    uint16_t idle_text);

static void draw_soil_cal_capture_prompt(const char* title,
                                         const char* value,
                                         uint16_t value_color,
                                         uint8_t selected_choice) {
    constexpr int card_x = 20;
    constexpr int card_y = 54;
    constexpr int card_w = 120;
    constexpr int card_h = 34;
    constexpr int button_y = 98;
    constexpr int button_w = 54;
    constexpr int button_h = SOIL_CAL_CAPTURE_BUTTON_H;
    const int cx = tft.width() / 2;
    const uint16_t card_bg = tft.color565(4, 8, 18);
    const uint16_t card_shadow = tft.color565(18, 12, 34);

    clearMenuBands();

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(value_color, TFT_BLACK);
    tft.drawString(title, cx, 40);
    tft.setTextFont(0);

    tft.fillRoundRect(card_x, card_y, card_w, card_h, 5, card_bg);
    tft.drawRoundRect(card_x, card_y, card_w, card_h, 5, value_color);
    tft.drawRoundRect(card_x + 1, card_y + 1, card_w - 2, card_h - 2, 4, card_shadow);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_TIMER);
    if (tft.textWidth(value) > card_w - 12) {
        tft.setFreeFont(FONT_SMALL);
    }
    if (tft.textWidth(value) > card_w - 12) {
        tft.setTextFont(1);
    }
    tft.setTextColor(value_color, card_bg);
    tft.drawString(value, cx, SOIL_CAL_CAPTURE_VALUE_Y);
    tft.setTextFont(0);

    tft.drawFastHLine(34, 92, 92, tft.color565(18, 36, 58));
    draw_soil_choice_button(20, button_y, button_w, button_h, L(MENU_EXIT), selected_choice == 0, PB_SOUND_P3, PB_SOUND_P3);
    draw_soil_choice_button(86, button_y, button_w, button_h, L(MENU_CAPTURE), selected_choice == 1, value_color, PB_HUM_P3);
    tft.setTextDatum(MC_DATUM);
}

static void draw_soil_cal_capture_value_only(const char* value, uint16_t value_color) {
    constexpr int card_x = 20;
    constexpr int card_y = 54;
    constexpr int card_w = 120;
    constexpr int card_h = 34;
    const int cx = tft.width() / 2;
    const uint16_t card_bg = tft.color565(4, 8, 18);

    tft.fillRect(card_x + 3, card_y + 3, card_w - 6, card_h - 6, card_bg);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_TIMER);
    if (tft.textWidth(value) > card_w - 12) {
        tft.setFreeFont(FONT_SMALL);
    }
    if (tft.textWidth(value) > card_w - 12) {
        tft.setTextFont(1);
    }
    tft.setTextColor(value_color, card_bg);
    tft.drawString(value, cx, SOIL_CAL_CAPTURE_VALUE_Y);
    tft.setTextFont(0);
}

static void draw_soil_choice_button(int x, int y, int w, int h,
                                    const char* label,
                                    bool selected,
                                    uint16_t accent,
                                    uint16_t idle_text) {
    const uint16_t panel_bg = tft.color565(4, 8, 18);
    const uint16_t selected_bg = tft.color565(18, 36, 18);
    const uint16_t bg = selected ? selected_bg : panel_bg;
    tft.fillRoundRect(x, y, w, h, 4, bg);
    tft.drawRoundRect(x, y, w, h, 4, selected ? accent : tft.color565(28, 52, 70));
    if (selected) {
        tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 3, TFT_WHITE);
    }
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(selected ? TFT_WHITE : idle_text, bg);
    if (tft.textWidth(label) > w - 8) {
        tft.setTextFont(1);
    }
    tft.drawString(label, x + w / 2, y + h / 2 - 1);
    tft.setTextFont(0);
}

static void draw_soil_cal_review_prompt(uint8_t selected_choice) {
    const int cx = tft.width() / 2;
    char line_buf_1[24];
    char line_buf_2[24];
    const char* lines[2];
    const uint16_t colors[2] = { TFT_RED, TFT_CYAN };

    drawCenteredMenuFrame(L(ST_SOIL_MENU_CAL), PB_SOIL_P1, L(ST_TURN_PUSH));
    snprintf(line_buf_1, sizeof(line_buf_1), "%s %d", L(ST_SOIL_DRY_REF), g_soil_cal_dry_raw);
    snprintf(line_buf_2, sizeof(line_buf_2), "%s %d", L(ST_SOIL_WET_REF), g_soil_cal_wet_raw);
    lines[0] = line_buf_1;
    lines[1] = line_buf_2;
    drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_SMALL, 55, 13);

    tft.drawFastHLine(34, 76, 92, tft.color565(18, 36, 58));
    draw_soil_choice_button(20, 83, 54, 20, L(MENU_EXIT), selected_choice == 0, PB_SOUND_P3, PB_SOUND_P3);
    draw_soil_choice_button(86, 83, 54, 20, L(MENU_SAVE), selected_choice == 1, PB_SOIL_P1, PB_HUM_P3);
    tft.setTextDatum(MC_DATUM);
}

static void draw_soil_cal_no_sensor_prompt() {
    const char* lines[2] = { L(ST_SOIL_CONNECT_SENSOR), L(ST_CHECK_SOIL) };
    const uint16_t colors[2] = { PB_SOIL_P1, pbit_external_dim_primary(SZ_SOIL) };
    drawCenteredMenuFrame(L(ST_NO_SENSOR), pbit_external_dim_primary(SZ_SOIL), L(ST_PUSH_MENU));
    drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_BODY, 62, 17);
}

static int soil_very_dry_limit(int dry_thr) {
    return constrain(dry_thr / 2, 0, dry_thr);
}

static int soil_very_moist_limit(int moist_thr) {
    return constrain(moist_thr + ((100 - moist_thr) / 2), moist_thr, 100);
}

static int soil_category_id(float soil, bool no_sensor, int dry_thr, int moist_thr) {
    if (no_sensor) return -1;
    if (soil < (float)soil_very_dry_limit(dry_thr)) return 0;
    if (soil < (float)dry_thr) return 1;
    if (soil < (float)moist_thr) return 2;
    if (soil < (float)soil_very_moist_limit(moist_thr)) return 3;
    return 4;
}

static void apply_soil_rgb(float soil, bool no_sensor) {
    if (no_sensor) {
        set_rgb(0, 36, 0);
        return;
    }

    uint8_t r = 0, g = 0, b = 0;
    pbit_rgb565_to_rgb888(pbit_soil_visual_color(soil), r, g, b);
    set_rgb(r, g, b);
}

// Draw the tiny runtime state indicator used by the soil screen.
static void draw_soil_alert_jewel(uint8_t alert_code, bool no_sensor) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_sensor || !g_soil_alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = no_sensor ? pbit_external_dim_primary(SZ_SOIL) : TFT_DARKGREY;
    } else if (alert_code == ALERT_CODE_LOW) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_ORANGE;
    } else if (alert_code == ALERT_CODE_OK) {
        jewel_state = ALERT_JEWEL_OK;
        jewel_color = TFT_GREEN;
    } else if (alert_code == ALERT_CODE_MOIST) {
        jewel_state = ALERT_JEWEL_OK;
        jewel_color = TFT_CYAN;
    } else {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_BLUE;
    }

    drawAlertJewel(L_ALERT_JEWEL_X, L_ALERT_JEWEL_Y, jewel_state, jewel_color);
}

bool soilCalibrationIsActive() {
    return g_soil_cal_state != SOIL_CAL_IDLE;
}

SoilCalibrationState getSoilCalibrationState() {
    return g_soil_cal_state;
}

void startSoilCalibration() {
    // Cache persisted values before entering the calibration workflow.
    g_soil_cal_state = SOIL_CAL_MENU;
    g_soil_menu_index = 0;
    clear_soil_calibration_drafts();
    g_soil_alerts_enabled = get_soil_alerts_enabled();
    g_soil_reset_choice = 0;
    g_soil_save_choice = 0;
    g_soil_capture_choice = 1;
    set_rgb(255, 180, 0);
    request_soil_redraw(true);
}

bool cancelSoilCalibration() {
    if (g_soil_cal_state == SOIL_CAL_IDLE) {
        return false;
    }

    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU:
            g_soil_cal_state = SOIL_CAL_IDLE;
            clear_soil_calibration_drafts();
            g_soil_reset_choice = 0;
            set_rgb(180, 80, 0);
            break;
        case SOIL_CAL_NO_SENSOR:
            g_soil_menu_index = 0;
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_WAIT_DRY:
        case SOIL_CAL_WAIT_WET:
        case SOIL_CAL_REVIEW_SAVE:
            clear_soil_calibration_drafts();
            g_soil_menu_index = 0;
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_THRESH_DRY:
        case SOIL_CAL_THRESH_MOIST:
            reload_soil_threshold_drafts();
            g_soil_menu_index = 1;
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_EDIT_ALERTS:
            g_soil_alerts_enabled = get_soil_alerts_enabled();
            g_soil_menu_index = 2;
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_RESET_CONFIRM:
            g_soil_reset_choice = 0;
            g_soil_menu_index = 3;
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_DONE:
        case SOIL_CAL_RESET_DONE:
        case SOIL_CAL_THRESH_DONE:
        case SOIL_CAL_ALERTS_DONE:
        case SOIL_CAL_ERROR:
        default:
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
    }

    request_soil_redraw(true);
    return true;
}

int getSoilCalibrationEncoderMin() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return 0;
        case SOIL_CAL_REVIEW_SAVE: return 0;
        case SOIL_CAL_WAIT_DRY:
        case SOIL_CAL_WAIT_WET:
            return 0;
        case SOIL_CAL_THRESH_DRY: return 1;
        case SOIL_CAL_THRESH_MOIST: return g_soil_thr_dry_pct + 2;
        case SOIL_CAL_EDIT_ALERTS: return 0;
        case SOIL_CAL_RESET_CONFIRM: return 0;
        default: return 0;
    }
}

int getSoilCalibrationEncoderMax() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return 4;
        case SOIL_CAL_REVIEW_SAVE: return 1;
        case SOIL_CAL_WAIT_DRY:
        case SOIL_CAL_WAIT_WET:
            return 1;
        case SOIL_CAL_THRESH_DRY: return g_soil_thr_moist_pct - 2;
        case SOIL_CAL_THRESH_MOIST: return 100;
        case SOIL_CAL_EDIT_ALERTS: return 1;
        case SOIL_CAL_RESET_CONFIRM: return 1;
        default: return 0;
    }
}

int getSoilCalibrationEncoderValue() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return (int)g_soil_menu_index;
        case SOIL_CAL_REVIEW_SAVE: return (int)g_soil_save_choice;
        case SOIL_CAL_WAIT_DRY:
        case SOIL_CAL_WAIT_WET:
            return (int)g_soil_capture_choice;
        case SOIL_CAL_THRESH_DRY: return g_soil_thr_dry_pct;
        case SOIL_CAL_THRESH_MOIST: return g_soil_thr_moist_pct;
        case SOIL_CAL_EDIT_ALERTS: return g_soil_alerts_enabled ? 1 : 0;
        case SOIL_CAL_RESET_CONFIRM: return (int)g_soil_reset_choice;
        default: return 0;
    }
}

void setSoilCalibrationInputValue(int value) {
    int next = value;
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU:
            next = constrain(next, 0, 4);
            if ((uint8_t)next != g_soil_menu_index) {
                g_soil_menu_index = (uint8_t)next;
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_REVIEW_SAVE:
            next = constrain(next, 0, 1);
            if ((uint8_t)next != g_soil_save_choice) {
                g_soil_save_choice = (uint8_t)next;
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_WAIT_DRY:
        case SOIL_CAL_WAIT_WET:
            next = constrain(next, 0, 1);
            if ((uint8_t)next != g_soil_capture_choice) {
                g_soil_capture_choice = (uint8_t)next;
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_THRESH_DRY:
            next = constrain(next, getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax());
            if (next != g_soil_thr_dry_pct) {
                g_soil_thr_dry_pct = next;
                g_soil_thr_optimal_pct = soil_auto_optimal_threshold(g_soil_thr_dry_pct, g_soil_thr_moist_pct);
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_THRESH_MOIST:
            next = constrain(next, getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax());
            if (next != g_soil_thr_moist_pct) {
                g_soil_thr_moist_pct = next;
                g_soil_thr_optimal_pct = soil_auto_optimal_threshold(g_soil_thr_dry_pct, g_soil_thr_moist_pct);
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_EDIT_ALERTS:
            next = constrain(next, 0, 1);
            if ((next != 0) != g_soil_alerts_enabled) {
                g_soil_alerts_enabled = (next != 0);
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_RESET_CONFIRM:
            next = constrain(next, 0, 1);
            if ((uint8_t)next != g_soil_reset_choice) {
                g_soil_reset_choice = (uint8_t)next;
                request_soil_redraw(false);
            }
            break;
        default:
            break;
    }
}

uint8_t handleSoilCalibrationButton() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU:
            if (g_soil_menu_index == 0) {
                if (soil_sensor_available_for_calibration()) {
                    clear_soil_calibration_drafts();
                    g_soil_cal_state = SOIL_CAL_WAIT_DRY;
                } else {
                    clear_soil_calibration_drafts();
                    g_soil_cal_state = SOIL_CAL_NO_SENSOR;
                }
            } else if (g_soil_menu_index == 1) {
                reload_soil_threshold_drafts();
                g_soil_cal_state = SOIL_CAL_THRESH_DRY;
            } else if (g_soil_menu_index == 2) {
                g_soil_alerts_enabled = get_soil_alerts_enabled();
                g_soil_cal_state = SOIL_CAL_EDIT_ALERTS;
            } else if (g_soil_menu_index == 3) {
                g_soil_reset_choice = 0;
                g_soil_cal_state = SOIL_CAL_RESET_CONFIRM;
            } else {
                g_soil_cal_state = SOIL_CAL_IDLE;
                set_rgb(180, 80, 0);
            }
            break;
        case SOIL_CAL_WAIT_DRY:
            if (g_soil_capture_choice == 0) {
                clear_soil_calibration_drafts();
                g_soil_cal_state = SOIL_CAL_MENU;
            } else {
                g_soil_cal_dry_raw = read_soil_raw_average();
                g_soil_capture_choice = 1;
                g_soil_cal_state = SOIL_CAL_WAIT_WET;
            }
            break;
        case SOIL_CAL_WAIT_WET:
            if (g_soil_capture_choice == 0) {
                clear_soil_calibration_drafts();
                g_soil_cal_state = SOIL_CAL_MENU;
            } else {
                g_soil_cal_wet_raw = read_soil_raw_average();
                g_soil_save_choice = 0;
                g_soil_cal_state = SOIL_CAL_REVIEW_SAVE;
            }
            break;
        case SOIL_CAL_NO_SENSOR:
            g_soil_cal_state = SOIL_CAL_MENU;
            break;
        case SOIL_CAL_REVIEW_SAVE:
            if (g_soil_save_choice == 1) {
                g_soil_cal_state = save_soil_calibration(g_soil_cal_dry_raw, g_soil_cal_wet_raw)
                    ? SOIL_CAL_DONE
                    : SOIL_CAL_ERROR;
            } else {
                clear_soil_calibration_drafts();
                g_soil_cal_state = SOIL_CAL_MENU;
            }
            break;
        case SOIL_CAL_THRESH_DRY:
            g_soil_cal_state = SOIL_CAL_THRESH_MOIST;
            break;
        case SOIL_CAL_THRESH_MOIST:
            g_soil_thr_optimal_pct = soil_auto_optimal_threshold(g_soil_thr_dry_pct, g_soil_thr_moist_pct);
            g_soil_cal_state = save_soil_thresholds(g_soil_thr_dry_pct, g_soil_thr_optimal_pct, g_soil_thr_moist_pct)
                ? SOIL_CAL_THRESH_DONE
                : SOIL_CAL_ERROR;
            break;
        case SOIL_CAL_EDIT_ALERTS:
            set_soil_alerts_enabled(g_soil_alerts_enabled);
            g_soil_cal_state = SOIL_CAL_ALERTS_DONE;
            break;
        case SOIL_CAL_RESET_CONFIRM:
            if (g_soil_reset_choice == 1) {
                reset_soil_settings();
                g_soil_thr_dry_pct = get_soil_threshold_dry();
                g_soil_thr_moist_pct = get_soil_threshold_moist();
                g_soil_thr_optimal_pct = soil_auto_optimal_threshold(g_soil_thr_dry_pct, g_soil_thr_moist_pct);
                g_soil_alerts_enabled = get_soil_alerts_enabled();
                g_soil_cal_state = SOIL_CAL_RESET_DONE;
            } else {
                g_soil_cal_state = SOIL_CAL_MENU;
            }
            break;
        case SOIL_CAL_DONE:
        case SOIL_CAL_RESET_DONE:
        case SOIL_CAL_THRESH_DONE:
        case SOIL_CAL_ALERTS_DONE:
        case SOIL_CAL_ERROR:
            g_soil_cal_state = SOIL_CAL_MENU;
            set_rgb(255, 180, 0);
            break;
        case SOIL_CAL_IDLE:
        default:
            return (uint8_t)g_soil_cal_state;
    }
    request_soil_redraw(g_soil_cal_state == SOIL_CAL_IDLE);
    return (uint8_t)g_soil_cal_state;
}

static void draw_soil_calibration_screen(bool screen_changed) {
    static SoilCalibrationState last_drawn_state = SOIL_CAL_IDLE;
    static int last_menu_index = -1;
    static int last_live_raw = -1;
    static int last_dry_raw = -1;
    static int last_wet_raw = -1;
    static int last_alerts_enabled = -1;
    static int last_reset_choice = -1;
    static int last_save_choice = -1;
    static int last_capture_choice = -1;
    static unsigned long last_live_sample_ms = 0;

    const int cx = tft.width() / 2;
    const bool state_changed = screen_changed || (g_soil_cal_state != last_drawn_state);

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_SOIL));
        last_menu_index = -1;
        last_live_raw = -1;
        last_dry_raw = -1;
        last_wet_raw = -1;
        last_alerts_enabled = -1;
        last_reset_choice = -1;
        last_save_choice = -1;
        last_capture_choice = -1;
        last_live_sample_ms = 0;
    }

    tft.setTextDatum(MC_DATUM);

    if (g_soil_cal_state == SOIL_CAL_MENU) {
        // Root calibration menu: capture, thresholds, alerts, reset, exit.
        if (state_changed || last_menu_index != (int)g_soil_menu_index) {
            const char* items[3] = {
                L(ST_SOIL_MENU_CAL),
                L(MENU_RANGES),
                L(MENU_ALERTS)
            };
            drawSettingsGridMenu(items, 3, g_soil_menu_index, L(MENU_RESET), L(MENU_EXIT));
            drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
            last_menu_index = (int)g_soil_menu_index;
        }
        last_drawn_state = g_soil_cal_state;
        return;
    }

    if (state_changed) {
        switch (g_soil_cal_state) {
            case SOIL_CAL_DONE:
                drawCenteredMenuFrame(L(ST_SOIL_CAL_SAVED), TFT_MAGENTA, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_NO_SENSOR:
                draw_soil_cal_no_sensor_prompt();
                break;
            case SOIL_CAL_RESET_DONE:
                drawCenteredMenuFrame(L(MENU_RESET_DONE), TFT_MAGENTA, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_THRESH_DONE:
                drawCenteredMenuFrame(L(ST_SOIL_THRESH_SAVED), TFT_MAGENTA, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_ALERTS_DONE:
                drawCenteredMenuFrame(L(MENU_SAVED), TFT_MAGENTA, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_ERROR:
                drawCenteredMenuFrame(L(ST_SOIL_CAL_ERROR), TFT_RED, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_RESET_CONFIRM:
            case SOIL_CAL_REVIEW_SAVE:
            case SOIL_CAL_IDLE:
            case SOIL_CAL_MENU:
            default:
                break;
        }
    }

    const bool wait_live_state = (g_soil_cal_state == SOIL_CAL_WAIT_DRY || g_soil_cal_state == SOIL_CAL_WAIT_WET);
    const bool capture_choice_changed = wait_live_state && (last_capture_choice != (int)g_soil_capture_choice);
    bool live_value_changed = false;
    if (wait_live_state) {
        const unsigned long now_ms = millis();
        if (state_changed || last_live_sample_ms == 0 || now_ms - last_live_sample_ms >= SOIL_CAL_LIVE_SAMPLE_MS) {
            const int sampled_raw = read_soil_raw_average();
            last_live_sample_ms = now_ms;
            if (state_changed || last_live_raw < 0 || abs(sampled_raw - last_live_raw) >= SOIL_CAL_LIVE_RAW_DEADBAND) {
                g_soil_cal_live_raw = sampled_raw;
                live_value_changed = true;
            }
        }
    }

    if (wait_live_state && (live_value_changed || capture_choice_changed)) {
        char value_buf[10];
        snprintf(value_buf, sizeof(value_buf), "%d", g_soil_cal_live_raw);
        const uint16_t value_color = g_soil_cal_state == SOIL_CAL_WAIT_DRY ? TFT_RED : TFT_CYAN;
        if (state_changed || capture_choice_changed) {
            draw_soil_cal_capture_prompt(g_soil_cal_state == SOIL_CAL_WAIT_DRY ? L(ST_SOIL_CAL_DRY) : L(ST_SOIL_CAL_WET),
                                         value_buf,
                                         value_color,
                                         g_soil_capture_choice);
            last_capture_choice = (int)g_soil_capture_choice;
        } else {
            draw_soil_cal_capture_value_only(value_buf, value_color);
        }
        last_live_raw = g_soil_cal_live_raw;
    }

    if (g_soil_cal_state == SOIL_CAL_REVIEW_SAVE
        && (state_changed
            || last_save_choice != (int)g_soil_save_choice
            || last_dry_raw != g_soil_cal_dry_raw
            || last_wet_raw != g_soil_cal_wet_raw)) {
        draw_soil_cal_review_prompt(g_soil_save_choice);
        last_save_choice = (int)g_soil_save_choice;
        last_dry_raw = g_soil_cal_dry_raw;
        last_wet_raw = g_soil_cal_wet_raw;
    }

    if (g_soil_cal_state == SOIL_CAL_RESET_CONFIRM && (state_changed || last_reset_choice != (int)g_soil_reset_choice)) {
        if (state_changed) {
            drawResetChoicePromptShell(L(MENU_RESET),
                                       L(MENU_SOIL_SENSOR_LIMITS),
                                       L(MENU_RESET_SUB_SOIL),
                                       L(ST_TURN_PUSH));
        }
        updateResetChoiceButtons(L(MENU_NO), L(MENU_YES), g_soil_reset_choice);
        last_reset_choice = (int)g_soil_reset_choice;
    }

    if (g_soil_cal_state == SOIL_CAL_EDIT_ALERTS && (state_changed || last_alerts_enabled != (int)g_soil_alerts_enabled)) {
        // Keep the alert toggle visually aligned with the other binary menus.
        drawCenteredMenuValueScreen(L(MENU_ALERTS),
                                    g_soil_alerts_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_soil_alerts_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
        last_alerts_enabled = (int)g_soil_alerts_enabled;
    }

    if ((g_soil_cal_state == SOIL_CAL_THRESH_DRY || g_soil_cal_state == SOIL_CAL_THRESH_MOIST)
        && (state_changed
            || (g_soil_cal_state == SOIL_CAL_THRESH_DRY && g_soil_thr_dry_pct != last_dry_raw)
            || (g_soil_cal_state == SOIL_CAL_THRESH_MOIST && g_soil_thr_moist_pct != last_live_raw))) {
        char value_buf[12];
        int value = 0;
        uint16_t value_color = TFT_WHITE;

        if (g_soil_cal_state == SOIL_CAL_THRESH_DRY) {
            value = g_soil_thr_dry_pct;
            value_color = TFT_ORANGE;
            last_dry_raw = value;
        } else {
            value = g_soil_thr_moist_pct;
            value_color = TFT_CYAN;
            last_live_raw = value;
        }

        snprintf(value_buf, sizeof(value_buf), "%d%%", value);
        drawCenteredMenuValueScreen(g_soil_cal_state == SOIL_CAL_THRESH_DRY
                                        ? L(ST_DRY)
                                        : L(ST_MOIST),
                                    value_buf,
                                    value_color,
                                    MENU_VALUE_FONT_TIMER,
                                    L(ST_TURN_PUSH));
    }

    if ((g_soil_cal_state == SOIL_CAL_DONE || g_soil_cal_state == SOIL_CAL_ERROR)
        && (state_changed || g_soil_cal_dry_raw != last_dry_raw || g_soil_cal_wet_raw != last_wet_raw)) {
        char line_buf_1[24];
        char line_buf_2[24];
        const char* lines[2];
        const uint16_t colors[2] = { TFT_RED, TFT_CYAN };
        snprintf(line_buf_1, sizeof(line_buf_1), "%s %d", L(ST_SOIL_DRY_REF), g_soil_cal_dry_raw);
        snprintf(line_buf_2, sizeof(line_buf_2), "%s %d", L(ST_SOIL_WET_REF), g_soil_cal_wet_raw);
        lines[0] = line_buf_1;
        lines[1] = line_buf_2;
        drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_SMALL, LM_SUMMARY2_Y0, LM_SUMMARY2_GAP);

        last_dry_raw = g_soil_cal_dry_raw;
        last_wet_raw = g_soil_cal_wet_raw;
    }

    if (g_soil_cal_state == SOIL_CAL_THRESH_DONE
        && (state_changed
            || g_soil_thr_dry_pct != last_dry_raw
            || g_soil_thr_moist_pct != last_wet_raw)) {
        char line_buf_1[24];
        char line_buf_2[24];
        const char* lines[2];
        const uint16_t colors[2] = { TFT_ORANGE, TFT_CYAN };
        snprintf(line_buf_1, sizeof(line_buf_1), "%s %d%%", L(ST_DRY), g_soil_thr_dry_pct);
        snprintf(line_buf_2, sizeof(line_buf_2), "%s %d%%", L(ST_MOIST), g_soil_thr_moist_pct);
        lines[0] = line_buf_1;
        lines[1] = line_buf_2;
        drawCenteredMenuBodyLines(lines, colors, 2, MENU_TEXT_FONT_SMALL, LM_SUMMARY2_Y0, LM_SUMMARY2_GAP);

        last_dry_raw = g_soil_thr_dry_pct;
        last_wet_raw = g_soil_thr_moist_pct;
    }

    if (g_soil_cal_state == SOIL_CAL_RESET_DONE && state_changed) {
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(L(MENU_SOIL_SENSOR_LIMITS), cx, 70);
        tft.drawString(L(MENU_RESTORED), cx, 88);
        tft.setTextFont(0);
    }

    if (g_soil_cal_state == SOIL_CAL_ALERTS_DONE && (state_changed || last_alerts_enabled != (int)g_soil_alerts_enabled)) {
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(g_soil_alerts_enabled ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString(L(MENU_ALERTS), cx, 70);
        tft.drawString(g_soil_alerts_enabled ? L(ST_ON) : L(ST_OFF), cx, 88);
        tft.setTextFont(0);
        last_alerts_enabled = (int)g_soil_alerts_enabled;
    }

    last_drawn_state = g_soil_cal_state;
}

// =============================================================
// SOIL_SCREEN — tanque vertical + valor
// =============================================================
void draw_soil_screen(bool screen_changed, bool data_changed) {
    if (soilCalibrationIsActive()) {
        draw_soil_calibration_screen(screen_changed);
        return;
    }

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_GREEN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    const int LEFT_PANEL_W = LA_TANK_X - 1;

    // Coordenadas definidas en layout.h (Familia A)

    float soil = (float)g_ui_readings_snapshot.soil_humidity;
    bool no_sensor = pbit_external_sensor_missing(SZ_SOIL, g_ui_readings_snapshot);
    const uint16_t missing_primary = pbit_external_dim_primary(SZ_SOIL);
    const uint16_t missing_secondary = pbit_external_dim_secondary(SZ_SOIL);
    const int dry_thr = get_soil_threshold_dry();
    const int moist_thr = get_soil_threshold_moist();
    g_soil_alerts_enabled = get_soil_alerts_enabled();

    const char*   categoryText;
    uint16_t      tankColor;
    uint16_t      categoryColor;
    int category_id = soil_category_id(soil, no_sensor, dry_thr, moist_thr);
    const uint16_t soil_visual = pbit_soil_visual_color(soil, !no_sensor);
    if (category_id < 0) {
        categoryText = L(ST_NO_SENSOR); tankColor = missing_primary; categoryColor = missing_secondary;
    } else if (category_id == 0) {
        categoryText = L(ST_TOO_DRY);   tankColor = soil_visual; categoryColor = soil_visual;
    } else if (category_id == 1) {
        categoryText = L(ST_DRY);       tankColor = soil_visual; categoryColor = soil_visual;
    } else if (category_id == 2) {
        categoryText = L(ST_OPTIMAL);   tankColor = soil_visual; categoryColor = soil_visual;
    } else if (category_id == 3) {
        categoryText = L(ST_MOIST);     tankColor = soil_visual; categoryColor = soil_visual;
    } else {
        categoryText = L(ST_SATURATED); tankColor = soil_visual; categoryColor = soil_visual;
    }

    char soilStr[6];
    snprintf(soilStr, sizeof(soilStr), "%.0f", soil);

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_SOIL));
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, no_sensor ? missing_secondary : TFT_DARKGREY);
    }

    static int last_soil_drawn = -1;
    static int last_category_id = -1;
    static uint8_t last_alert_code = ALERT_CODE_OFF;
    static bool last_alerts_enabled = false;
    int soil_rounded = no_sensor ? -9999 : (int)roundf(soil);
    bool value_changed = screen_changed || (soil_rounded != last_soil_drawn);
    bool category_changed = screen_changed || (category_id != last_category_id);
    uint8_t alert_code = alert_engine_get_code(AlertSensor::Soil);
    bool alert_changed = screen_changed
        || (alert_code != last_alert_code)
        || (g_soil_alerts_enabled != last_alerts_enabled);
    if (!value_changed && !category_changed && !alert_changed) return;

    last_soil_drawn = soil_rounded;
    last_category_id = category_id;
    last_alert_code = alert_code;
    last_alerts_enabled = g_soil_alerts_enabled;

    // --- Dinámicos ---
    if (data_changed || screen_changed || alert_changed) {
        if (screen_changed || value_changed) {
            tft.fillRect(0, LA_HINT_Y - 4, LEFT_PANEL_W, 18, BACKGROUND_COLOR);
            if (!no_sensor) {
                tft.setFreeFont(FONT_SMALL);
                tft.setTextDatum(TC_DATUM);
                tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
                tft.drawString(L(SUB_SOIL_MOIST), LA_LEFT_CX, LA_HINT_Y);
                tft.setTextFont(0);
            }
        }

        if (value_changed || category_changed) {
            // Refresh only the tank region when the reading changes.
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, tankColor, no_sensor ? 0.0f : soil, 0.0f, 100.0f, 3);
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, no_sensor ? missing_secondary : TFT_DARKGREY);
            apply_soil_rgb(soil, no_sensor);
        }

        const char* unitStr = "%";
        if (value_changed) {
            tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 50, BACKGROUND_COLOR);
            if (no_sensor) {
                tft.setTextDatum(MC_DATUM);
                tft.setFreeFont(FONT_BODY);
                tft.setTextColor(missing_primary, BACKGROUND_COLOR);
                tft.drawString(L(ST_NO_SENSOR), LA_LEFT_CX, LA_VALUE_TOP + 8);
                tft.setFreeFont(FONT_SMALL);
                tft.setTextColor(missing_primary, BACKGROUND_COLOR);
                tft.drawString(L(ST_CHECK_SOIL), LA_LEFT_CX, LA_VALUE_TOP + 24);
                tft.setTextFont(0);
            } else {
                tft.setFreeFont(FONT_VALUE);
                int intW  = tft.textWidth(soilStr);
                tft.setFreeFont(FONT_BODY);
                int unitW = tft.textWidth(unitStr);
                int startX = LA_LEFT_CX - (intW + unitW) / 2;
                tft.setTextDatum(TL_DATUM);
                tft.setFreeFont(FONT_VALUE);
                tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
                tft.drawString(soilStr, startX, LA_VALUE_TOP);
                tft.setFreeFont(FONT_BODY);
                tft.setTextColor(TITLE_COLOR, BACKGROUND_COLOR);
                tft.drawString(unitStr, startX + intW, LA_VALUE_TOP);
                tft.setTextFont(0); // liberar GFXfont
            }
        }

        // Categoría — centrada en panel izquierdo, debajo del número
        if (value_changed || category_changed) {
            tft.fillRect(0, LA_CATEGORY_Y - 10, LEFT_PANEL_W, 28, BACKGROUND_COLOR);
            if (!no_sensor) {
                tft.setFreeFont(FONT_BODY);
                tft.setTextDatum(TC_DATUM);
                tft.setTextColor(categoryColor, BACKGROUND_COLOR);
                tft.drawString(categoryText, LA_LEFT_CX, LA_CATEGORY_Y);
                tft.setTextFont(0); // liberar GFXfont
            }
        }

        if (value_changed || category_changed || alert_changed) {
            draw_soil_alert_jewel(alert_code, no_sensor);
        }
    }
}
