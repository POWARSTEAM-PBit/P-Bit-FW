// ui_lab_dash.cpp
// Temporary lab dashboard: compact sensor summary rows with icon primitives.

#include "ui_lab_dash.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"
#include "layout.h"
#include "ui_icons.h"
#include "hw.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <climits>
#include <math.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern bool g_is_fahrenheit;

namespace {

constexpr int kBodyX = 8;
constexpr int kBodyY = 33;
constexpr int kBodyW = 144;
constexpr int kBodyH = 84;

constexpr int kRowH = 18;
constexpr int kRowTop0 = kBodyY + 1;
constexpr int kRowGap = 20;

constexpr int kAccentX = kBodyX + 4;
constexpr int kAccentW = 3;
constexpr int kIconX = kBodyX + 11;
constexpr int kLabelX = kBodyX + 27;
constexpr int kValueX = kBodyX + kBodyW - 5;

constexpr uint16_t kPanelBorder = 0x39CC;
constexpr uint16_t kRowSep = 0x2104; // subtle dark grey tone.

struct LabDashCache {
    bool valid = false;
    bool temp_valid = false;
    int temp10 = 0;
    bool hum_valid = false;
    int hum = 0;
    int light = 0;
    int sound = 0;
    bool fahrenheit = false;
};

static LabDashCache g_last;
static const char* kSensorLabels[4] = {
    L(LAB_TEMP_SHORT),
    L(LAB_HUM_SHORT),
    L(LAB_LIGHT_SHORT),
    L(LAB_SOUND_SHORT),
};

static int row_top(int row_index);
static int row_center(int row_index);
static void draw_row_icon(int row_index, uint16_t color);

static uint16_t row_accent_color(int row_index) {
    switch (row_index) {
        case 0: return tft.color565(255, 66, 214);   // magenta
        case 1: return tft.color565(84, 255, 255);   // cyan
        case 2: return tft.color565(255, 255, 72);   // fluorescent yellow
        case 3: return tft.color565(180, 100, 255);  // purple
        default: return TFT_DARKGREY;
    }
}

static uint16_t row_title_color(int row_index) {
    (void)row_index;
    return TFT_WHITE;
}

static uint16_t row_value_color(int row_index) {
    return row_accent_color(row_index);
}

static uint16_t row_icon_color(int row_index) {
    return row_accent_color(row_index);
}

static uint16_t value_color_for_row(int row_index, bool valid) {
    return valid ? row_value_color(row_index) : TFT_DARKGREY;
}

static uint16_t accent_color_for_row(int row_index) {
    return row_accent_color(row_index);
}

static uint16_t icon_color_for_row(int row_index) {
    return row_icon_color(row_index);
}

static uint16_t title_color_for_row(int row_index) {
    return row_title_color(row_index);
}

static bool row_value_valid(int row_index, bool temp_valid, bool hum_valid, bool lux_valid, bool sound_valid) {
    switch (row_index) {
        case 0: return temp_valid;
        case 1: return hum_valid;
        case 2: return lux_valid;
        case 3: return sound_valid;
        default: return false;
    }
}

static void clear_value_area(int row_index) {
    const int row_y = row_top(row_index);
    tft.fillRect(kBodyX + 78, row_y + 2, 64, kRowH - 3, TFT_BLACK);
}

static void draw_row_value(int row_index, const char* value, uint16_t color) {
    const int cy = row_center(row_index);
    clear_value_area(row_index);
    tft.setTextDatum(MR_DATUM);
    tft.setFreeFont(FONT_MENU);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(value, kValueX, cy - 2);
    tft.setTextFont(0);
}

static void draw_row_static(int row_index) {
    const int row_y = row_top(row_index);
    const int cy = row_center(row_index);
    const uint16_t accent = accent_color_for_row(row_index);
    const uint16_t title = title_color_for_row(row_index);
    const uint16_t icon = icon_color_for_row(row_index);

    tft.fillRect(kBodyX + 1, row_y, kBodyW - 2, kRowH - 1, TFT_BLACK);
    tft.fillRect(kAccentX, row_y, kAccentW, kRowH - 1, accent);
    if (row_index < 3) {
        tft.drawFastHLine(kBodyX + 1, row_y + kRowH - 1, kBodyW - 2, kRowSep);
    }

    draw_row_icon(row_index, icon);
    tft.setTextDatum(ML_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(title, TFT_BLACK);
    tft.drawString(kSensorLabels[row_index], kLabelX, cy - 1);
    tft.setTextFont(0);
}

static int row_top(int row_index) {
    return kRowTop0 + (row_index * kRowGap);
}

static int row_center(int row_index) {
    return row_top(row_index) + (kRowH / 2);
}

static void format_temp_value(char* out, size_t out_size, float temp_c, bool valid) {
    if (!valid) {
        snprintf(out, out_size, "---");
        return;
    }
    const float displayed = g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
    const char* unit = g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
    snprintf(out, out_size, "%.1f%s", displayed, unit);
}

static void format_humidity_value(char* out, size_t out_size, float hum, bool valid) {
    if (!valid) {
        snprintf(out, out_size, "---");
        return;
    }
    snprintf(out, out_size, "%.0f%%", hum);
}

static void format_light_value(char* out, size_t out_size, float lux, bool valid) {
    if (!valid) {
        snprintf(out, out_size, "---");
        return;
    }
    if (lux >= 10000.0f) {
        snprintf(out, out_size, "%.0fk %s", lux / 1000.0f, L(ST_LUX_UNIT));
    } else {
        snprintf(out, out_size, "%.0f %s", lux, L(ST_LUX_UNIT));
    }
}

static void format_sound_value(char* out, size_t out_size, float level, bool valid) {
    if (!valid) {
        snprintf(out, out_size, "---");
        return;
    }
    if (level < 0.0f) level = 0.0f;
    if (level > 100.0f) level = 100.0f;
    snprintf(out, out_size, "%.0f%%", level);
}

static void draw_row_shell(int row_index, uint16_t accent, bool draw_separator) {
    const int row_y = row_top(row_index);
    tft.fillRect(kAccentX, row_y, kAccentW, kRowH - 1, accent);
    if (draw_separator) {
        tft.drawFastHLine(kBodyX + 1, row_y + kRowH - 1, kBodyW - 2, kRowSep);
    }
}

static void draw_row_text(int row_index, const char* title, const char* value, uint16_t color) {
    const int cy = row_center(row_index);
    tft.setTextDatum(ML_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(title, kLabelX, cy - 1);

    tft.setTextDatum(MR_DATUM);
    tft.setFreeFont(FONT_MENU);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(value, kValueX, cy - 2);
    tft.setTextFont(0);
}

static void draw_row_icon(int row_index, uint16_t color) {
    const int cy = row_center(row_index);
    const int cx = kIconX + 5;
    switch (row_index) {
        case 0: pbit_draw_temp_icon(cx, cy, color); break;
        case 1: pbit_draw_humidity_icon(cx, cy, color); break;
        case 2: pbit_draw_light_icon(cx, cy + 1, color); break;
        case 3: pbit_draw_sound_icon(cx, cy + 1, color); break;
        default: break;
    }
}

static void draw_full_panel() {
    tft.fillScreen(TFT_BLACK);
    drawHeader(L(TIT_LAB_DASH), TFT_WHITE);
    tft.drawRoundRect(kBodyX, kBodyY, kBodyW, kBodyH, 4, kPanelBorder);
    for (int i = 0; i < 4; ++i) {
        draw_row_static(i);
    }
}

static void draw_body_row(int row_index) {
    const float temp_c = g_ui_readings_snapshot.temperature;
    const bool temp_valid = !isnan(temp_c);
    const float hum = g_ui_readings_snapshot.humidity;
    const bool hum_valid = !isnan(hum);
    const float lux = g_ui_readings_snapshot.ldr;
    const bool lux_valid = isfinite(lux);
    const float sound = g_ui_readings_snapshot.mic;
    const bool sound_valid = isfinite(sound);

    char temp_buf[16];
    char hum_buf[12];
    char lux_buf[16];
    char sound_buf[12];

    format_temp_value(temp_buf, sizeof(temp_buf), temp_c, temp_valid);
    format_humidity_value(hum_buf, sizeof(hum_buf), hum, hum_valid);
    format_light_value(lux_buf, sizeof(lux_buf), lux, lux_valid);
    format_sound_value(sound_buf, sizeof(sound_buf), sound, sound_valid);

    const char* values[4] = { temp_buf, hum_buf, lux_buf, sound_buf };
    const bool row_valid = row_value_valid(row_index, temp_valid, hum_valid, lux_valid, sound_valid);
    draw_row_value(row_index, values[row_index], value_color_for_row(row_index, row_valid));
}

} // namespace

void draw_lab_dash_screen(bool screen_changed, bool sensor_data_changed) {
    const float temp_c = g_ui_readings_snapshot.temperature;
    const bool temp_valid = !isnan(temp_c);
    const int temp10 = temp_valid ? (int)lroundf(temp_c * 10.0f) : INT_MIN;
    const float hum = g_ui_readings_snapshot.humidity;
    const bool hum_valid = !isnan(hum);
    const int hum_i = hum_valid ? (int)lroundf(hum) : INT_MIN;
    const int light_i = (int)lroundf(g_ui_readings_snapshot.ldr);
    const int sound_i = (int)lroundf(g_ui_readings_snapshot.mic);
    const bool fahrenheit = g_is_fahrenheit;
    const bool temp_dirty = !g_last.valid
        || g_last.temp_valid != temp_valid
        || g_last.temp10 != temp10
        || g_last.fahrenheit != fahrenheit;
    const bool hum_dirty = !g_last.valid
        || g_last.hum_valid != hum_valid
        || g_last.hum != hum_i;
    const bool light_dirty = !g_last.valid
        || g_last.light != light_i;
    const bool sound_dirty = !g_last.valid
        || g_last.sound != sound_i;
    const bool need_redraw = screen_changed || temp_dirty || hum_dirty || light_dirty || sound_dirty;

    if (!need_redraw) {
        return;
    }

    if (screen_changed || !g_last.valid) {
        draw_full_panel();
        draw_body_row(0);
        draw_body_row(1);
        draw_body_row(2);
        draw_body_row(3);
    } else {
        if (temp_dirty) draw_body_row(0);
        if (hum_dirty) draw_body_row(1);
        if (light_dirty) draw_body_row(2);
        if (sound_dirty) draw_body_row(3);
    }

    g_last.valid = true;
    g_last.temp_valid = temp_valid;
    g_last.temp10 = temp10;
    g_last.hum_valid = hum_valid;
    g_last.hum = hum_i;
    g_last.light = light_i;
    g_last.sound = sound_i;
    g_last.fahrenheit = fahrenheit;
}
