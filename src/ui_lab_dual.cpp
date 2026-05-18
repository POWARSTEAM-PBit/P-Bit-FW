// Temporary lab screen: dual Temp + Humidity.
// Designed as a safe, compact, professional-looking experiment that can be
// integrated later into the carousel without affecting the existing screens.

#include "ui_lab_dual.h"

#include "ui_widgets.h"
#include "io.h"
#include "hw.h"
#include "languages.h"
#include "fonts.h"
#include "layout.h"
#include "ui_icons.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern bool g_is_fahrenheit;

namespace {

constexpr int kHeaderClearH = L_HEADER_LINE + 4;
constexpr int kPanelY = LC_CARD_TOP;
constexpr int kPanelH = 73;
constexpr int kPanelW = LC_MASTER_CARD_W;
constexpr int kPanelGap = 4;
constexpr int kLeftX = LC_MASTER_CARD_X0;
constexpr int kRightX = kLeftX + kPanelW + kPanelGap;
constexpr int kCenterX = 80;
constexpr int kFooterX = LC_SCREEN_X;
constexpr int kFooterY = 104;
constexpr int kFooterW = LC_SCREEN_W;
constexpr int kFooterH = 23;
constexpr int kFooterTextCx = kFooterX + (kFooterW / 2);
constexpr int kFooterTextCy = kFooterY + (kFooterH / 2) - 1;

constexpr int kTitleY = 32;
constexpr int kValueY = 52;
constexpr int kStatusY = 80;
constexpr int kBarY = 85;
constexpr int kBarH = 9;
constexpr int kValueClearY = kValueY - kPanelY - 4;
constexpr int kValueClearH = 28;
constexpr int kTempValueClearX = 8;
constexpr int kTempValueClearW = kPanelW - 16;
constexpr int kHumValueClearX = 8;
constexpr int kHumValueClearW = kPanelW - 16;
constexpr int kBarClearY = kBarY - kPanelY - 2;
constexpr int kBarClearH = 13;

static int g_last_temp_key = INT_MIN;
static int g_last_hum_key = INT_MIN;
static bool g_last_temp_valid = false;
static bool g_last_hum_valid = false;
static uint16_t g_last_temp_panel_color = TFT_DARKGREY;
static uint16_t g_last_hum_panel_color = TFT_DARKGREY;
static bool g_last_unit_mode = false;

static float to_display_temp(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static const char* temp_unit_short() {
    return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
}

static const char* short_panel_title(bool humidity) {
    return humidity ? L(LAB_HUM_SHORT) : L(LAB_TEMP_SHORT);
}

static void clear_panel_interior(int x, int y, int w, int h) {
    tft.fillRect(x + 1, y + 1, w - 2, h - 2, TFT_BLACK);
}

static uint16_t humidity_color(float hum) {
    if (isnan(hum)) return TFT_DARKGREY;
    if (hum < 35.0f) return tft.color565(180, 96, 255);
    if (hum < 70.0f) return tft.color565(0, 230, 255);
    return tft.color565(96, 170, 255);
}

static uint16_t temp_shell_color(float temp_c, bool valid) {
    if (!valid) return TFT_DARKGREY;
    if (temp_c <= 15.0f) return tft.color565(0, 180, 255);
    if (temp_c <= 22.0f) return tft.color565(80, 255, 188);
    if (temp_c <= 27.0f) return tft.color565(255, 232, 0);
    if (temp_c <= 32.0f) return tft.color565(255, 144, 0);
    return tft.color565(255, 48, 168);
}

static uint16_t temp_value_color(float temp_c, bool valid) {
    if (!valid) return TFT_DARKGREY;
    if (temp_c <= 22.0f) return tft.color565(255, 220, 96);
    if (temp_c <= 32.0f) return tft.color565(255, 216, 72);
    return tft.color565(255, 216, 72);
}

static uint16_t temp_unit_color(bool valid) {
    return valid ? tft.color565(255, 216, 72) : TFT_DARKGREY;
}

static uint16_t hum_value_color(bool valid) {
    return valid ? tft.color565(176, 255, 255) : TFT_DARKGREY;
}

static uint16_t hum_unit_color(bool valid) {
    return valid ? tft.color565(176, 255, 255) : TFT_DARKGREY;
}

static uint16_t footer_bg_color() {
    return tft.color565(14, 6, 24);
}

static void draw_climate_footer(float temp_c, bool no_temp, float hum, bool no_hum) {
    const char* footer_text = L(ST_NO_SENSOR);
    uint16_t footer_color = TFT_DARKGREY;

    if (!no_temp && !no_hum) {
        const int hum_dry_max = get_humidity_threshold_dry();
        const int hum_comfort_max = get_humidity_threshold_comfort();
        const int temp_low = get_temp_alarm_low();
        const int temp_high = get_temp_alarm_high();

        if (hum > (float)hum_comfort_max) {
            footer_text = L(ST_MOLD_RISK);
            footer_color = TFT_RED;
        } else if (hum < (float)hum_dry_max) {
            footer_text = L(ST_TOO_DRY);
            footer_color = TFT_ORANGE;
        } else if (temp_c < (float)temp_low) {
            footer_text = L(ST_CLIMATE_FRESH);
            footer_color = tft.color565(72, 212, 255);
        } else if (temp_c > (float)temp_high) {
            footer_text = L(ST_CLIMATE_WARM);
            footer_color = tft.color565(255, 182, 64);
        } else {
            footer_text = L(ST_OPTIMAL);
            footer_color = TFT_GREEN;
        }
    }

    const uint16_t bg = footer_bg_color();
    const uint16_t card_border = tft.color565(160, 0, 255);
    tft.fillRect(kFooterX - 1, kFooterY - 1, kFooterW + 2, kFooterH + 2, TFT_BLACK);
    tft.fillRoundRect(kFooterX, kFooterY, kFooterW, kFooterH, 4, bg);
    tft.drawRoundRect(kFooterX, kFooterY, kFooterW, kFooterH, 4, card_border);
    tft.fillCircle(kFooterX + 8, kFooterY + (kFooterH / 2), 2, footer_color);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(footer_color, bg);
    tft.drawString(footer_text, kFooterTextCx, kFooterTextCy);
    tft.setTextFont(0);
}

static void draw_panel_frame(int x, int y, int w, int h, uint16_t color) {
    drawCard(x, y, w, h, color);
}

static void draw_panel_title_centered(int x, int y, int w, const char* title, uint16_t color) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(title, x + (w / 2), y + 4);
    tft.setTextFont(0);
}

static void draw_panel_unit(int x, int y, int w, const char* unit_text, uint16_t unit_color) {
    if (!unit_text || !unit_text[0]) return;
    tft.setTextDatum(TR_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(unit_color, TFT_BLACK);
    tft.drawString(unit_text, x + w - 8, y + 3);
    tft.setTextFont(0);
}

static void draw_center_value(int x,
                              int y,
                              int w,
                              const char* main_text,
                              uint16_t main_color) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_TIMER);
    tft.setTextColor(main_color, TFT_BLACK);
    tft.drawString(main_text, x + (w / 2), y);
    tft.setTextFont(0);
}

static void draw_temp_shell(int x, int y, int w, int h, uint16_t panel_color, uint16_t title_color, uint16_t unit_color, bool no_sensor) {
    clear_panel_interior(x, y, w, h);
    draw_panel_frame(x, y, w, h, panel_color);
    pbit_draw_temp_icon(x + 12, y + 13, unit_color);
    draw_panel_title_centered(x, y, w, short_panel_title(false), title_color);
    draw_panel_unit(x, y, w, no_sensor ? "" : temp_unit_short(), unit_color);
}

static void draw_temp_content(int x, int y, int w, int h, float temp_display, bool no_sensor, uint16_t value_color, uint16_t bar_color) {
    tft.fillRect(x + kTempValueClearX, y + kValueClearY, kTempValueClearW, kValueClearH, TFT_BLACK);
    char value_buf[16];
    if (no_sensor) {
        snprintf(value_buf, sizeof(value_buf), "---");
    } else {
        snprintf(value_buf, sizeof(value_buf), "%.1f", temp_display);
    }
    draw_center_value(x, kValueY, w, value_buf, value_color);

    if (no_sensor) {
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(L(ST_NO_SENSOR), x + (w / 2), kStatusY);
        tft.setTextFont(0);
    }

    const float bar_max = g_is_fahrenheit ? 122.0f : 50.0f;
    const float bar_min = g_is_fahrenheit ? 32.0f : 0.0f;
    const float bar_value = no_sensor ? bar_min : temp_display;
    tft.fillRect(x + 6, y + kBarClearY, w - 12, kBarClearH, TFT_BLACK);
    drawBarGraph(x + 6, kBarY, w - 12, kBarH, bar_color, bar_value, bar_min, bar_max);
}

static void draw_temp_panel(bool redraw_shell, bool redraw_content, bool no_sensor, float temp_display, uint16_t panel_color, uint16_t title_color, uint16_t value_color, uint16_t unit_color) {
    const int x = kLeftX;
    const int y = kPanelY;
    if (redraw_shell) {
        draw_temp_shell(x, y, kPanelW, kPanelH, panel_color, title_color, unit_color, no_sensor);
    }
    if (redraw_content) {
        draw_temp_content(x, y, kPanelW, kPanelH, temp_display, no_sensor, value_color, panel_color);
    }
}

static void draw_humidity_shell(int x, int y, int w, int h, uint16_t panel_color, uint16_t title_color, uint16_t unit_color, bool no_sensor) {
    clear_panel_interior(x, y, w, h);
    draw_panel_frame(x, y, w, h, panel_color);
    pbit_draw_humidity_icon(x + 12, y + 14, unit_color);
    draw_panel_title_centered(x, y, w, short_panel_title(true), title_color);
    draw_panel_unit(x, y, w, no_sensor ? "" : "%", unit_color);
}

static void draw_humidity_content(int x, int y, int w, int h, float hum, bool no_sensor, uint16_t value_color, uint16_t bar_color) {
    tft.fillRect(x + kHumValueClearX, y + kValueClearY, kHumValueClearW, kValueClearH, TFT_BLACK);
    char value_buf[16];
    if (no_sensor) {
        snprintf(value_buf, sizeof(value_buf), "---");
    } else {
        snprintf(value_buf, sizeof(value_buf), "%.0f", hum);
    }
    draw_center_value(x, kValueY, w, value_buf, value_color);

    if (no_sensor) {
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(L(ST_NO_SENSOR), x + (w / 2), kStatusY);
        tft.setTextFont(0);
    }

    tft.fillRect(x + 6, y + kBarClearY, w - 12, kBarClearH, TFT_BLACK);
    drawBarGraph(x + 6, kBarY, w - 12, kBarH, bar_color, no_sensor ? 0.0f : hum, 0.0f, 100.0f);
}

static void draw_humidity_panel(bool redraw_shell, bool redraw_content, bool no_sensor, float hum_value, uint16_t panel_color, uint16_t title_color, uint16_t value_color, uint16_t unit_color) {
    const int x = kRightX;
    const int y = kPanelY;
    if (redraw_shell) {
        draw_humidity_shell(x, y, kPanelW, kPanelH, panel_color, title_color, unit_color, no_sensor);
    }
    if (redraw_content) {
        draw_humidity_content(x, y, kPanelW, kPanelH, hum_value, no_sensor, value_color, panel_color);
    }
}

} // namespace

void draw_lab_dual_th_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    const float temp_c = g_ui_readings_snapshot.temperature;
    const float hum = g_ui_readings_snapshot.humidity;
    const bool no_temp = isnan(temp_c);
    const bool no_hum = isnan(hum);
    const int temp_key = no_temp ? INT_MIN : (int)lroundf(to_display_temp(temp_c) * 10.0f);
    const int hum_key = no_hum ? INT_MIN : (int)lroundf(hum * 10.0f);
    const uint16_t temp_panel_color = temp_shell_color(temp_c, !no_temp);
    const uint16_t hum_panel_color = humidity_color(hum);
    const uint16_t temp_title_color = no_temp ? TFT_DARKGREY : temp_panel_color;
    const uint16_t temp_value_col = temp_value_color(temp_c, !no_temp);
    const uint16_t temp_unit_col = temp_unit_color(!no_temp);
    const uint16_t hum_title_color = no_hum ? TFT_DARKGREY : hum_panel_color;
    const uint16_t hum_value_col = hum_value_color(!no_hum);
    const uint16_t hum_unit_col = hum_unit_color(!no_hum);
    const bool unit_changed = (g_last_unit_mode != g_is_fahrenheit);
    const bool temp_shell_changed = screen_changed || unit_changed || (temp_panel_color != g_last_temp_panel_color) || (no_temp != g_last_temp_valid);
    const bool hum_shell_changed = screen_changed || (hum_panel_color != g_last_hum_panel_color) || (no_hum != g_last_hum_valid);
    const bool temp_changed = screen_changed || unit_changed || (temp_key != g_last_temp_key) || (no_temp != g_last_temp_valid);
    const bool hum_changed = screen_changed || (hum_key != g_last_hum_key) || (no_hum != g_last_hum_valid);

    if (!screen_changed && !temp_changed && !hum_changed) {
        return;
    }

    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_LAB_DUAL_TH));
        tft.drawFastVLine(kCenterX, kPanelY, kPanelH, TFT_DARKGREY);
    }

    if (temp_shell_changed) {
        draw_temp_panel(true, true, no_temp, no_temp ? 0.0f : to_display_temp(temp_c), temp_panel_color, temp_title_color, temp_value_col, temp_unit_col);
    } else if (temp_changed) {
        draw_temp_panel(false, true, no_temp, no_temp ? 0.0f : to_display_temp(temp_c), temp_panel_color, temp_title_color, temp_value_col, temp_unit_col);
    }

    if (hum_shell_changed) {
        draw_humidity_panel(true, true, no_hum, hum, hum_panel_color, hum_title_color, hum_value_col, hum_unit_col);
    } else if (hum_changed) {
        draw_humidity_panel(false, true, no_hum, hum, hum_panel_color, hum_title_color, hum_value_col, hum_unit_col);
    }

    if (screen_changed || temp_changed || hum_changed) {
        draw_climate_footer(temp_c, no_temp, hum, no_hum);
    }

    g_last_temp_key = temp_key;
    g_last_hum_key = hum_key;
    g_last_temp_panel_color = temp_panel_color;
    g_last_hum_panel_color = hum_panel_color;
    g_last_temp_valid = no_temp;
    g_last_hum_valid = no_hum;
    g_last_unit_mode = g_is_fahrenheit;
}
