// ui_lab_home_cards.cpp
// Layout A: 2x2 card grid showing temperature, humidity, light and sound.
//
// Each card follows the validated LC_MASTER_* card rule:
//   - small icon  (top-left corner, ~10px)
//   - sensor tag  (text, beside icon)
//   - large value (centre-right, FONT_BODY)
//   - unit        (small, after value)
//   - right mini tank reflecting sensor range

#include "ui_lab_home_cards.h"

#include "fonts.h"
#include "io.h"
#include "languages.h"
#include "layout.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <climits>
#include <math.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading  g_ui_readings_snapshot;
extern bool     g_is_fahrenheit;

namespace {

constexpr uint16_t kBg       = TFT_BLACK;
constexpr uint16_t kFrame    = 0x2945;   // dark blue-grey border
constexpr uint16_t kOrange   = TFT_ORANGE;
constexpr uint16_t kCyan     = TFT_CYAN;
constexpr uint16_t kYellow   = 0xFFE0;
constexpr uint16_t kMagenta  = TFT_MAGENTA;
constexpr uint16_t kGreen    = 0x07E0;
constexpr uint16_t kRed      = TFT_RED;

constexpr int kCardW = LC_MASTER_CARD_W;
constexpr int kCardH = LC_MASTER_CARD_H;
constexpr int kCol[2] = { LC_MASTER_CARD_X0, LC_MASTER_CARD_X1 };
constexpr int kRow[2] = { LC_MASTER_CARD_Y0, LC_MASTER_CARD_Y1 };
constexpr int kValueXPad = 26;
constexpr int kTankW = 8;

static void draw_home_temp_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 4, cy - 10, 8, 15, 4, color);
    tft.fillCircle(cx, cy + 5, 5, color);
    tft.fillRect(cx - 1, cy - 7, 2, 10, TFT_BLACK);
    tft.drawFastHLine(cx + 4, cy - 6, 2, color);
    tft.drawFastHLine(cx + 4, cy - 2, 2, color);
}

static void draw_home_humidity_icon(int cx, int cy, uint16_t color) {
    tft.fillTriangle(cx, cy - 11, cx - 6, cy - 1, cx + 6, cy - 1, color);
    tft.fillCircle(cx, cy + 3, 6, color);
    tft.fillCircle(cx, cy + 4, 3, TFT_BLACK);
}

static void draw_home_light_icon(int cx, int cy, uint16_t color) {
    tft.fillCircle(cx, cy, 5, color);
    tft.drawFastHLine(cx - 9, cy, 3, color);
    tft.drawFastHLine(cx + 7, cy, 3, color);
    tft.drawFastVLine(cx, cy - 9, 3, color);
    tft.drawFastVLine(cx, cy + 7, 3, color);
    tft.drawLine(cx - 6, cy - 6, cx - 8, cy - 8, color);
    tft.drawLine(cx + 6, cy - 6, cx + 8, cy - 8, color);
    tft.drawLine(cx - 6, cy + 6, cx - 8, cy + 8, color);
    tft.drawLine(cx + 6, cy + 6, cx + 8, cy + 8, color);
}

static void draw_home_sound_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 4, cy - 10, 8, 13, 4, color);
    tft.drawFastVLine(cx, cy + 3, 4, color);
    tft.drawFastHLine(cx - 5, cy + 6, 11, color);
}

// ── State colour helpers ──────────────────────────────────────────────────

static uint16_t temp_state_color(float t) {
    if (t < 18.0f) return kCyan;
    if (t < 27.0f) return kGreen;
    if (t < 35.0f) return kYellow;
    return kRed;
}

static uint16_t hum_state_color(float h) {
    if (h < 30.0f) return kYellow;
    if (h < 70.0f) return kGreen;
    return kCyan;
}

static uint16_t light_state_color(float l) {
    if (l < 100.0f) return 0x2104;
    if (l < 600.0f) return kYellow;
    return 0xFFFF;
}

static uint16_t sound_state_color(float s) {
    if (s < 40.0f) return kGreen;
    if (s < 70.0f) return kYellow;
    return kRed;
}

// ── Card drawing ──────────────────────────────────────────────────────────

// Draw the static card shell (border + background).
static void draw_card_shell(int col, int row, uint16_t accent) {
    const int x = kCol[col];
    const int y = kRow[row];
    tft.fillRoundRect(x, y, kCardW, kCardH, LC_MASTER_CARD_RADIUS, 0x0841);
    tft.drawRoundRect(x, y, kCardW, kCardH, LC_MASTER_CARD_RADIUS, accent);
}

struct CardData {
    int     col, row;
    uint16_t accent;
    void (*icon_fn)(int, int, uint16_t);
    const char* tag;
    bool    valid;
    float   value;
    const char* fmt;    // "%.1f" or "%.0f"
    const char* unit;
    float   min_value;
    float   max_value;
    bool    mic_extra_drop;
};

struct HomeCardsCache {
    bool valid = false;
    bool fahrenheit = false;
    bool card_valid[4] = { false, false, false, false };
    int  value_key[4]  = { INT_MIN, INT_MIN, INT_MIN, INT_MIN };
};

static HomeCardsCache g_last;

static void draw_card_dynamic(const CardData& d) {
    const int x = kCol[d.col];
    const int y = kRow[d.row];

    // clear dynamic area
    tft.fillRoundRect(x + 1, y + 1, kCardW - 2, kCardH - 2, 3, 0x0841);

    // icon
    const int icon_x = x + 15;
    const int icon_y = y + 23 + (d.mic_extra_drop ? 1 : 0);
    d.icon_fn(icon_x, icon_y, d.valid ? d.accent : TFT_DARKGREY);

    // tag label
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, 0x0841);
    tft.drawString(d.tag, x + 29, y + 6);
    tft.setTextFont(0);

    const int tank_x = x + kCardW - 12;
    const int value_x = x + kValueXPad + 3;
    const int value_max_w = tank_x - value_x - 3;

    // value
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_BODY);
    if (d.valid) {
        char buf[16];
        char full[20];
        snprintf(buf, sizeof(buf), d.fmt, d.value);
        snprintf(full, sizeof(full), "%s%s", buf, d.unit);
        tft.setTextColor(TFT_WHITE, 0x0841);
        if (tft.textWidth(full) > value_max_w) {
            tft.setFreeFont(FONT_SMALL);
        }
        tft.drawString(full, value_x, y + 24);
    } else {
        tft.setTextColor(TFT_DARKGREY, 0x0841);
        tft.drawString("--", value_x, y + 24);
    }
    tft.setTextFont(0);

    // right vertical tank reflects the logical range of each sensor.
    const int tank_y = y + 6;
    const int tank_h = kCardH - 12;
    tft.drawRoundRect(tank_x, tank_y, kTankW, tank_h, 1, 0x2104);
    if (d.valid) {
        drawFillTank(tank_x, tank_y, kTankW, tank_h, d.accent, d.value, d.min_value, d.max_value, 1);
    } else {
        tft.fillRect(tank_x + 1, tank_y + 1, kTankW - 2, tank_h - 2, TFT_BLACK);
    }
}

static CardData build_card_data(int index) {
    const Reading& r = g_ui_readings_snapshot;
    const bool t_ok  = !isnan(r.temperature);
    const bool h_ok  = !isnan(r.humidity);
    const bool l_ok  = !isnan(r.ldr);
    const bool s_ok  = !isnan(r.mic);
    const float t_c  = t_ok ? r.temperature : 0.0f;
    const float t_d  = g_is_fahrenheit ? (t_c * 1.8f + 32.0f) : t_c;
    const char* t_u  = g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);

    switch (index) {
        case 0:
            return { 0, 0, kOrange,  draw_home_temp_icon,     "TEMP", t_ok, t_d, "%.0f", t_u,  g_is_fahrenheit ? 32.0f : 0.0f, g_is_fahrenheit ? 122.0f : 50.0f, false };
        case 1:
            return { 1, 0, kCyan,    draw_home_humidity_icon, "HUM",  h_ok, h_ok ? r.humidity : 0.0f, "%.0f", "%", 0.0f, 100.0f, false };
        case 2:
            return { 0, 1, kYellow,  draw_home_light_icon,    "LUZ",  l_ok, l_ok ? r.ldr : 0.0f, "%.0f", "lx", 0.0f, 1023.0f, false };
        default:
            return { 1, 1, kMagenta, draw_home_sound_icon,    "MIC",  s_ok, s_ok ? r.mic : 0.0f, "%.0f", "%", 0.0f, 100.0f, true };
    }
}

static int card_value_key(int index) {
    const Reading& r = g_ui_readings_snapshot;
    switch (index) {
        case 0:
            if (isnan(r.temperature)) return INT_MIN;
            return (int)lroundf((g_is_fahrenheit ? (r.temperature * 1.8f + 32.0f) : r.temperature));
        case 1:
            return isnan(r.humidity) ? INT_MIN : (int)lroundf(r.humidity);
        case 2:
            return isnan(r.ldr) ? INT_MIN : (int)lroundf(r.ldr);
        default:
            return isnan(r.mic) ? INT_MIN : (int)lroundf(r.mic);
    }
}

static bool card_valid_now(int index) {
    const Reading& r = g_ui_readings_snapshot;
    switch (index) {
        case 0: return !isnan(r.temperature);
        case 1: return !isnan(r.humidity);
        case 2: return !isnan(r.ldr);
        default: return !isnan(r.mic);
    }
}

static void draw_shell() {
    tft.fillScreen(kBg);
    drawMasterCardHeader(L(TIT_LAB_HOME_CARDS));
    draw_card_shell(0, 0, kOrange);
    draw_card_shell(1, 0, kCyan);
    draw_card_shell(0, 1, kYellow);
    draw_card_shell(1, 1, kMagenta);
}

} // namespace

void draw_lab_home_cards_screen(bool screen_changed, bool sensor_data_changed) {
    if (screen_changed) {
        draw_shell();
        for (int i = 0; i < 4; ++i) {
            draw_card_dynamic(build_card_data(i));
            g_last.card_valid[i] = card_valid_now(i);
            g_last.value_key[i] = card_value_key(i);
        }
        g_last.fahrenheit = g_is_fahrenheit;
        g_last.valid = true;
        return;
    }
    if (!sensor_data_changed) return;

    bool any_dirty = !g_last.valid || (g_last.fahrenheit != g_is_fahrenheit);
    bool dirty[4] = { false, false, false, false };
    for (int i = 0; i < 4; ++i) {
        dirty[i] = !g_last.valid
            || (g_last.card_valid[i] != card_valid_now(i))
            || (g_last.value_key[i] != card_value_key(i))
            || (i == 0 && g_last.fahrenheit != g_is_fahrenheit);
        any_dirty = any_dirty || dirty[i];
    }

    if (!any_dirty) return;

    for (int i = 0; i < 4; ++i) {
        if (!dirty[i]) continue;
        draw_card_dynamic(build_card_data(i));
        g_last.card_valid[i] = card_valid_now(i);
        g_last.value_key[i] = card_value_key(i);
    }
    g_last.fahrenheit = g_is_fahrenheit;
    g_last.valid = true;
}
