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
#include "light_display.h"
#include "layout.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <climits>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern TFT_eSPI tft;
extern Reading  g_ui_readings_snapshot;
extern bool     g_is_fahrenheit;

namespace {

constexpr uint16_t kBg       = TFT_BLACK;
constexpr uint16_t kCardBg   = 0x0841;    // interior de card (navy oscuro, ver DESIGN_SYSTEM.md)
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
    pbit_draw_humidity_icon(cx, cy, color);
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
    tft.fillRoundRect(x, y, kCardW, kCardH, LC_MASTER_CARD_RADIUS, kCardBg);
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
    bool    compact_large_lux;
};

struct HomeCardsCache {
    bool valid = false;
    bool fahrenheit = false;
    uint8_t light_mode = 0;
    bool card_valid[4] = { false, false, false, false };
    int  value_key[4]  = { INT_MIN, INT_MIN, INT_MIN, INT_MIN };
};

static HomeCardsCache g_last;

// Draws the static chrome of a card: full interior clear, icon, tag label,
// and tank border only. Call when valid-state changes or on screen_changed.
static void draw_card_chrome(const CardData& d) {
    const int x = kCol[d.col];
    const int y = kRow[d.row];

    // clear full card interior
    tft.fillRoundRect(x + 1, y + 1, kCardW - 2, kCardH - 2, 3, kCardBg);

    // icon
    const int icon_x = x + 15;
    const int icon_y = y + 23 + (d.mic_extra_drop ? 1 : 0);
    d.icon_fn(icon_x, icon_y, d.valid ? d.accent : TFT_DARKGREY);

    // tag label
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, kCardBg);
    tft.drawString(d.tag, x + 29, y + 6);
    tft.setTextFont(0);

    // tank border only (fill is drawn by draw_card_value_and_tank)
    const int tank_x = x + kCardW - 12;
    const int tank_y = y + 6;
    const int tank_h = kCardH - 12;
    tft.drawRoundRect(tank_x, tank_y, kTankW, tank_h, 1, 0x2104);
}

// Draws only the dynamic parts (value text and tank fill) with a targeted clear
// so the static chrome (icon, tag) is not disturbed every sensor update.
static void draw_card_value_and_tank(const CardData& d) {
    const int x = kCol[d.col];
    const int y = kRow[d.row];

    const int tank_x     = x + kCardW - 12;
    const int value_x    = x + kValueXPad + 3;
    const int value_max_w = tank_x - value_x - 3;

    // targeted clear of value text area only
    tft.setFreeFont(FONT_BODY);
    const int fh = tft.fontHeight();
    tft.fillRect(value_x, y + 22, value_max_w, fh + 4, kCardBg);

    // value
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_BODY);
    if (d.valid) {
        char buf[16];
        char full[20];
        bool hide_unit = false;
        if (d.compact_large_lux && d.value >= 1000.0f) {
            snprintf(buf, sizeof(buf), "%.1fk", d.value / 1000.0f);
            hide_unit = true;
        } else {
            snprintf(buf, sizeof(buf), d.fmt, d.value);
        }
        const bool no_unit = hide_unit || d.unit[0] == '\0';
        const bool tight_unit = no_unit
            || strcmp(d.unit, "%") == 0
            || strcmp(d.unit, L(ST_UNIT_C_SHORT)) == 0
            || strcmp(d.unit, L(ST_UNIT_F_SHORT)) == 0;
        snprintf(full, sizeof(full), "%s%s%s", buf, tight_unit ? "" : " ", no_unit ? "" : d.unit);
        tft.setTextColor(TFT_WHITE, kCardBg);
        if (tft.textWidth(full) > value_max_w) {
            tft.setFreeFont(FONT_SMALL);
        }
        tft.drawString(full, value_x, y + 24);
    } else {
        tft.setTextColor(TFT_DARKGREY, kCardBg);
        tft.drawString("--", value_x, y + 24);
    }
    tft.setTextFont(0);

    // tank fill
    const int tank_y = y + 6;
    const int tank_h = kCardH - 12;
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
    const bool s_ok  = !isnan(r.mic);
    const float t_c  = t_ok ? r.temperature : 0.0f;
    const float t_d  = g_is_fahrenheit ? (t_c * 1.8f + 32.0f) : t_c;
    const char* t_u  = g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
    const LightDisplayReading light = light_display_from_reading(r);
    const bool light_lux_mode = (light.mode == LIGHT_DISPLAY_LUX);

    switch (index) {
        case 0:
            return { 0, 0, kOrange,  draw_home_temp_icon,     L(LAB_TEMP_SHORT),  t_ok, t_d, "%.0f", t_u,  g_is_fahrenheit ? 32.0f : 0.0f, g_is_fahrenheit ? 122.0f : 50.0f, false, false };
        case 1:
            return { 1, 0, kCyan,    draw_home_humidity_icon, L(LAB_AIR_SHORT),   h_ok, h_ok ? r.humidity : 0.0f, "%.0f", "%", 0.0f, 100.0f, false, false };
        case 2:
            return { 0, 1, kYellow,  draw_home_light_icon,    light_lux_mode ? "Lux" : L(LAB_LIGHT_SHORT), light.valid, light.valid ? light.value : 0.0f, "%.0f", light_lux_mode ? "" : light.unit, 0.0f, light_display_max(light.mode), false, light_lux_mode };
        default:
            return { 1, 1, kMagenta, draw_home_sound_icon,    L(LAB_SOUND_SHORT), s_ok, s_ok ? r.mic : 0.0f, "%.0f", "%", 0.0f, 100.0f, true, false };
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
            return light_display_from_reading(r).key;
        default:
            return isnan(r.mic) ? INT_MIN : (int)lroundf(r.mic);
    }
}

static bool card_valid_now(int index) {
    const Reading& r = g_ui_readings_snapshot;
    switch (index) {
        case 0: return !isnan(r.temperature);
        case 1: return !isnan(r.humidity);
        case 2: return light_display_from_reading(r).valid;
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
            const CardData d = build_card_data(i);
            draw_card_chrome(d);
            draw_card_value_and_tank(d);
            g_last.card_valid[i] = card_valid_now(i);
            g_last.value_key[i] = card_value_key(i);
        }
        g_last.fahrenheit = g_is_fahrenheit;
        g_last.light_mode = light_display_mode();
        g_last.valid = true;
        return;
    }
    if (!sensor_data_changed) return;

    const uint8_t current_light_mode = light_display_mode();
    bool any_dirty = !g_last.valid
        || (g_last.fahrenheit != g_is_fahrenheit)
        || (g_last.light_mode != current_light_mode);
    bool dirty[4] = { false, false, false, false };
    for (int i = 0; i < 4; ++i) {
        dirty[i] = !g_last.valid
            || (g_last.card_valid[i] != card_valid_now(i))
            || (g_last.value_key[i] != card_value_key(i))
            || (i == 0 && g_last.fahrenheit != g_is_fahrenheit)
            || (i == 2 && g_last.light_mode != current_light_mode);
        any_dirty = any_dirty || dirty[i];
    }

    if (!any_dirty) return;

    for (int i = 0; i < 4; ++i) {
        if (!dirty[i]) continue;
        const CardData d = build_card_data(i);
        // Redraw chrome only when the valid state changes (icon colour flips accent↔grey)
        const bool meta = !g_last.valid || (g_last.card_valid[i] != card_valid_now(i));
        if (meta) draw_card_chrome(d);
        draw_card_value_and_tank(d);
        g_last.card_valid[i] = card_valid_now(i);
        g_last.value_key[i] = card_value_key(i);
    }
    g_last.fahrenheit = g_is_fahrenheit;
    g_last.light_mode = current_light_mode;
    g_last.valid = true;
}
