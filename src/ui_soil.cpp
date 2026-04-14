// ui_soil.cpp
// Pantalla de humedad del suelo (sensor capacitivo externo J6).

#include "ui_soil.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
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

static SoilCalibrationState g_soil_cal_state = SOIL_CAL_IDLE;
static uint8_t g_soil_menu_index = 0;
static int g_soil_cal_dry_raw = 0;
static int g_soil_cal_wet_raw = 0;
static int g_soil_cal_live_raw = 0;
static int g_soil_thr_dry_pct = 20;
static int g_soil_thr_optimal_pct = 55;
static int g_soil_thr_moist_pct = 80;
static bool g_soil_alerts_enabled = true;
static uint8_t g_soil_reset_choice = 0;

constexpr int SOIL_CAL_TITLE_Y = 44;
constexpr int SOIL_CAL_VALUE_Y = 78;
constexpr int SOIL_CAL_VALUE_BOX_Y = 64;
constexpr int SOIL_CAL_VALUE_BOX_H = 30;
constexpr int SOIL_CAL_SUMMARY_Y1 = 62;
constexpr int SOIL_CAL_SUMMARY_Y2 = 78;
constexpr int SOIL_CAL_SUMMARY_Y3 = 94;
constexpr int SOIL_CAL_SUMMARY_BOX_Y = 54;
constexpr int SOIL_CAL_SUMMARY_BOX_H = 52;

static void request_soil_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

static int soil_category_id(float soil, bool no_sensor, int dry_thr, int optimal_thr, int moist_thr) {
    if (no_sensor) return -1;
    if (soil < (float)dry_thr) return 0;
    if (soil < (float)optimal_thr) return 1;
    if (soil < (float)moist_thr) return 2;
    return 3;
}

// Soil colors stay intentionally strong so the screen works as a quick
// educational status indicator, not just a raw sensor readout.
static void apply_soil_rgb_for_category(int category_id, bool no_sensor) {
    if (no_sensor) {
        set_rgb(120, 0, 0); // Mantener rojo oscuro para error
        return;
    }

    switch (category_id) {
        case 0: set_rgb(255, 0, 0); break;   // Seco = Rojo
        case 1: set_rgb(0, 255, 0); break;   // Óptimo = Verde
        case 2: set_rgb(0, 0, 200); break;   // Húmedo = Azul oscuro
        default: set_rgb(0, 0, 200); break; // Muy húmedo = Azul oscuro
    }
}

// Draw the tiny runtime state indicator used by the soil screen.
static void draw_soil_alert_jewel(uint8_t alert_code, bool no_sensor) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (no_sensor || !g_soil_alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
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
    g_soil_cal_dry_raw = 0;
    g_soil_cal_wet_raw = 0;
    g_soil_alerts_enabled = get_soil_alerts_enabled();
    g_soil_reset_choice = 0;
    set_rgb(255, 180, 0);
    request_soil_redraw(false);
}

int getSoilCalibrationEncoderMin() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return 0;
        case SOIL_CAL_THRESH_DRY: return 0;
        case SOIL_CAL_THRESH_OPTIMAL: return g_soil_thr_dry_pct + 1;
        case SOIL_CAL_THRESH_MOIST: return g_soil_thr_optimal_pct + 1;
        case SOIL_CAL_EDIT_ALERTS: return 0;
        case SOIL_CAL_RESET_CONFIRM: return 0;
        default: return 0;
    }
}

int getSoilCalibrationEncoderMax() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return 4;
        case SOIL_CAL_THRESH_DRY: return g_soil_thr_optimal_pct - 1;
        case SOIL_CAL_THRESH_OPTIMAL: return g_soil_thr_moist_pct - 1;
        case SOIL_CAL_THRESH_MOIST: return 100;
        case SOIL_CAL_EDIT_ALERTS: return 1;
        case SOIL_CAL_RESET_CONFIRM: return 1;
        default: return 0;
    }
}

int getSoilCalibrationEncoderValue() {
    switch (g_soil_cal_state) {
        case SOIL_CAL_MENU: return (int)g_soil_menu_index;
        case SOIL_CAL_THRESH_DRY: return g_soil_thr_dry_pct;
        case SOIL_CAL_THRESH_OPTIMAL: return g_soil_thr_optimal_pct;
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
        case SOIL_CAL_THRESH_DRY:
            next = constrain(next, getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax());
            if (next != g_soil_thr_dry_pct) {
                g_soil_thr_dry_pct = next;
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_THRESH_OPTIMAL:
            next = constrain(next, getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax());
            if (next != g_soil_thr_optimal_pct) {
                g_soil_thr_optimal_pct = next;
                request_soil_redraw(false);
            }
            break;
        case SOIL_CAL_THRESH_MOIST:
            next = constrain(next, getSoilCalibrationEncoderMin(), getSoilCalibrationEncoderMax());
            if (next != g_soil_thr_moist_pct) {
                g_soil_thr_moist_pct = next;
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
                g_soil_cal_state = SOIL_CAL_WAIT_DRY;
            } else if (g_soil_menu_index == 1) {
                g_soil_thr_dry_pct = get_soil_threshold_dry();
                g_soil_thr_optimal_pct = get_soil_threshold_optimal();
                g_soil_thr_moist_pct = get_soil_threshold_moist();
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
            g_soil_cal_dry_raw = read_soil_raw_average();
            g_soil_cal_state = SOIL_CAL_WAIT_WET;
            break;
        case SOIL_CAL_WAIT_WET:
            g_soil_cal_wet_raw = read_soil_raw_average();
            g_soil_cal_state = save_soil_calibration(g_soil_cal_dry_raw, g_soil_cal_wet_raw)
                ? SOIL_CAL_DONE
                : SOIL_CAL_ERROR;
            break;
        case SOIL_CAL_THRESH_DRY:
            g_soil_cal_state = SOIL_CAL_THRESH_OPTIMAL;
            break;
        case SOIL_CAL_THRESH_OPTIMAL:
            g_soil_cal_state = SOIL_CAL_THRESH_MOIST;
            break;
        case SOIL_CAL_THRESH_MOIST:
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
                g_soil_thr_optimal_pct = get_soil_threshold_optimal();
                g_soil_thr_moist_pct = get_soil_threshold_moist();
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

static void draw_soil_calibration_screen() {
    static SoilCalibrationState last_drawn_state = SOIL_CAL_IDLE;
    static int last_menu_index = -1;
    static int last_live_raw = -1;
    static int last_dry_raw = -1;
    static int last_wet_raw = -1;
    static int last_alerts_enabled = -1;
    static int last_reset_choice = -1;

    const int cx = tft.width() / 2;
    const bool state_changed = (g_soil_cal_state != last_drawn_state);

    if (g_soil_cal_state == SOIL_CAL_WAIT_DRY || g_soil_cal_state == SOIL_CAL_WAIT_WET) {
        g_soil_cal_live_raw = read_soil_raw_average();
    }

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_SOIL), TFT_GREEN);
        last_menu_index = -1;
        last_live_raw = -1;
        last_dry_raw = -1;
        last_wet_raw = -1;
        last_alerts_enabled = -1;
        last_reset_choice = -1;
    }

    tft.setTextDatum(MC_DATUM);

    if (g_soil_cal_state == SOIL_CAL_MENU) {
        // Root calibration menu: capture, thresholds, alerts, reset, exit.
        if (state_changed || last_menu_index != (int)g_soil_menu_index) {
            const char* items[5] = {
                L(ST_SOIL_MENU_CAL),
                L(ST_SOIL_MENU_THRESH),
                L(MENU_ALERTS),
                L(MENU_RESET),
                L(ST_SOIL_MENU_BACK)
            };
            drawCenteredMenuList(items, 5, g_soil_menu_index, LM_MENU5_Y0, LM_MENU5_GAP);
            drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
            last_menu_index = (int)g_soil_menu_index;
        }
        last_drawn_state = g_soil_cal_state;
        return;
    }

    if (state_changed) {
        switch (g_soil_cal_state) {
            case SOIL_CAL_DONE:
                drawCenteredMenuFrame(L(ST_SOIL_CAL_SAVED), TFT_GREEN, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_RESET_DONE:
                drawCenteredMenuFrame(L(MENU_RESET_DONE), TFT_GREEN, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_THRESH_DONE:
                drawCenteredMenuFrame(L(ST_SOIL_THRESH_SAVED), TFT_GREEN, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_ALERTS_DONE:
                drawCenteredMenuFrame(L(MENU_SAVED), TFT_GREEN, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_ERROR:
                drawCenteredMenuFrame(L(ST_SOIL_CAL_ERROR), TFT_RED, L(ST_PUSH_MENU));
                break;
            case SOIL_CAL_RESET_CONFIRM:
            case SOIL_CAL_IDLE:
            case SOIL_CAL_MENU:
            default:
                break;
        }
    }

    if ((g_soil_cal_state == SOIL_CAL_WAIT_DRY || g_soil_cal_state == SOIL_CAL_WAIT_WET) && (state_changed || g_soil_cal_live_raw != last_live_raw)) {
        char value_buf[10];
        snprintf(value_buf, sizeof(value_buf), "%d", g_soil_cal_live_raw);
        drawCenteredMenuValueScreen(g_soil_cal_state == SOIL_CAL_WAIT_DRY ? L(ST_SOIL_CAL_DRY) : L(ST_SOIL_CAL_WET),
                                    value_buf,
                                    g_soil_cal_state == SOIL_CAL_WAIT_DRY ? TFT_RED : TFT_CYAN,
                                    MENU_VALUE_FONT_TIMER,
                                    L(MENU_PUSH_CAPTURE));
        last_live_raw = g_soil_cal_live_raw;
    }

    if (g_soil_cal_state == SOIL_CAL_RESET_CONFIRM && (state_changed || last_reset_choice != (int)g_soil_reset_choice)) {
        drawResetChoicePrompt(L(MENU_RESET),
                              L(MENU_SOIL_SENSOR_LIMITS),
                              L(MENU_RESET_SUB_SOIL),
                              L(MENU_NO),
                              L(MENU_YES),
                              g_soil_reset_choice,
                              L(ST_TURN_PUSH));
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

    if ((g_soil_cal_state == SOIL_CAL_THRESH_DRY || g_soil_cal_state == SOIL_CAL_THRESH_OPTIMAL || g_soil_cal_state == SOIL_CAL_THRESH_MOIST)
        && (state_changed
            || (g_soil_cal_state == SOIL_CAL_THRESH_DRY && g_soil_thr_dry_pct != last_dry_raw)
            || (g_soil_cal_state == SOIL_CAL_THRESH_OPTIMAL && g_soil_thr_optimal_pct != last_wet_raw)
            || (g_soil_cal_state == SOIL_CAL_THRESH_MOIST && g_soil_thr_moist_pct != last_live_raw))) {
        char value_buf[12];
        int value = 0;
        uint16_t value_color = TFT_WHITE;

        if (g_soil_cal_state == SOIL_CAL_THRESH_DRY) {
            value = g_soil_thr_dry_pct;
            value_color = TFT_ORANGE;
            last_dry_raw = value;
        } else if (g_soil_cal_state == SOIL_CAL_THRESH_OPTIMAL) {
            value = g_soil_thr_optimal_pct;
            value_color = TFT_GREEN;
            last_wet_raw = value;
        } else {
            value = g_soil_thr_moist_pct;
            value_color = TFT_CYAN;
            last_live_raw = value;
        }

        snprintf(value_buf, sizeof(value_buf), "%d%%", value);
        drawCenteredMenuValueScreen(g_soil_cal_state == SOIL_CAL_THRESH_DRY
                                        ? L(ST_DRY)
                                        : (g_soil_cal_state == SOIL_CAL_THRESH_OPTIMAL ? L(ST_OPTIMAL) : L(ST_MOIST)),
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
            || g_soil_thr_optimal_pct != last_live_raw
            || g_soil_thr_moist_pct != last_wet_raw)) {
        char line_buf_1[24];
        char line_buf_2[24];
        char line_buf_3[24];
        const char* lines[3];
        const uint16_t colors[3] = { TFT_ORANGE, TFT_GREEN, TFT_CYAN };
        snprintf(line_buf_1, sizeof(line_buf_1), "%s %d%%", L(ST_DRY), g_soil_thr_dry_pct);
        snprintf(line_buf_2, sizeof(line_buf_2), "%s %d%%", L(ST_OPTIMAL), g_soil_thr_optimal_pct);
        snprintf(line_buf_3, sizeof(line_buf_3), "%s %d%%", L(ST_MOIST), g_soil_thr_moist_pct);
        lines[0] = line_buf_1;
        lines[1] = line_buf_2;
        lines[2] = line_buf_3;
        drawCenteredMenuBodyLines(lines, colors, 3, MENU_TEXT_FONT_SMALL, LM_SUMMARY3_Y0, LM_SUMMARY3_GAP);

        last_dry_raw = g_soil_thr_dry_pct;
        last_live_raw = g_soil_thr_optimal_pct;
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

    if (state_changed && g_soil_cal_state == SOIL_CAL_RESET_CONFIRM) {
        drawFooterHint(
            L(ST_TURN_PUSH),
            cx,
            LM_MENU_FOOTER_Y,
            TFT_CYAN
        );
    }

    last_drawn_state = g_soil_cal_state;
}

// =============================================================
// SOIL_SCREEN — tanque vertical + valor
// =============================================================
void draw_soil_screen(bool screen_changed, bool data_changed) {
    if (soilCalibrationIsActive()) {
        draw_soil_calibration_screen();
        return;
    }

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_GREEN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    const int LEFT_PANEL_W = LA_TANK_X - 1;

    // Coordenadas definidas en layout.h (Familia A)

    float soil = (float)g_ui_readings_snapshot.soil_humidity;
    bool no_sensor = isnan(soil);
    const int dry_thr = get_soil_threshold_dry();
    const int optimal_thr = get_soil_threshold_optimal();
    const int moist_thr = get_soil_threshold_moist();
    g_soil_alerts_enabled = get_soil_alerts_enabled();

    const char*   categoryText;
    uint16_t      tankColor;
    uint16_t      categoryColor;
    int category_id = soil_category_id(soil, no_sensor, dry_thr, optimal_thr, moist_thr);
    if (category_id < 0) {
        categoryText = L(ST_NO_SENSOR); tankColor = TFT_DARKGREY; categoryColor = TFT_RED;
    } else if (category_id == 0) {
        categoryText = L(ST_DRY);       tankColor = TFT_ORANGE; categoryColor = TFT_ORANGE;
    } else if (category_id == 1) {
        categoryText = L(ST_OPTIMAL);   tankColor = TFT_GREEN;  categoryColor = TFT_GREEN;
    } else if (category_id == 2) {
        categoryText = L(ST_MOIST);     tankColor = TFT_CYAN;   categoryColor = TFT_CYAN;
    } else {
        categoryText = L(ST_SATURATED); tankColor = TFT_BLUE;   categoryColor = TFT_BLUE;
    }

    char soilStr[6];
    snprintf(soilStr, sizeof(soilStr), "%.0f", soil);

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_SOIL), TITLE_COLOR);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
    }

    static int last_soil_drawn = -1;
    static int last_category_id = -1;
    int soil_rounded = no_sensor ? -9999 : (int)roundf(soil);
    bool value_changed = screen_changed || (soil_rounded != last_soil_drawn);
    bool category_changed = screen_changed || (category_id != last_category_id);
    uint8_t alert_code = alert_engine_get_code(AlertSensor::Soil);
    if (!value_changed && !category_changed) return;

    last_soil_drawn = soil_rounded;
    last_category_id = category_id;

    // --- Dinámicos ---
    if (data_changed || screen_changed) {
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
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
            apply_soil_rgb_for_category(category_id, no_sensor);
        }

        const char* unitStr = "%";
        if (value_changed) {
            tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 50, BACKGROUND_COLOR);
            if (no_sensor) {
                tft.setTextDatum(MC_DATUM);
                tft.setFreeFont(FONT_BODY);
                tft.setTextColor(TFT_RED, BACKGROUND_COLOR);
                tft.drawString(L(ST_NO_SENSOR), LA_LEFT_CX, LA_VALUE_TOP + 8);
                tft.setFreeFont(FONT_SMALL);
                tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
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

        draw_soil_alert_jewel(alert_code, no_sensor);
    }
}
