// ui_lab_linear_dash.cpp
// SENSOR LIST: compact row-based dashboard with cached visible state.

#include "ui_lab_linear_dash.h"

#include "fonts.h"
#include "io.h"
#include "languages.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern TFT_eSPI tft;
extern Reading  g_ui_readings_snapshot;
extern bool     g_is_fahrenheit;

namespace {

constexpr uint16_t kBg      = TFT_BLACK;
constexpr uint16_t kOrange  = TFT_ORANGE;
constexpr uint16_t kCyan    = TFT_CYAN;
constexpr uint16_t kYellow  = 0xFFE0;
constexpr uint16_t kMagenta = TFT_MAGENTA;
constexpr uint16_t kGreen   = TFT_GREEN;
constexpr uint16_t kGrey    = TFT_DARKGREY;

constexpr int kRowTop[5] = { 34, 50, 66, 82, 98 };
constexpr int kRowH = 15;
constexpr int kIconCX = 11;
constexpr int kLabelX = 23;
constexpr int kValueRightX = 107;
constexpr int kDividerX = 111;
constexpr int kBarX = 116;
constexpr int kBarW = 32;
constexpr int kBarH = 5;
constexpr int kBarSegments = 8;

struct RowData {
    void (*icon_fn)(int, int, uint16_t);
    const char* label;
    uint16_t color;
    bool valid;
    float value;
    float v_min;
    float v_max;
    const char* fmt;
    const char* unit;
};

struct RowCache {
    bool ready = false;
    bool valid = false;
    uint16_t color = 0;
    uint8_t bar_on = 0;
    char value[20] = {0};
};

static RowCache g_last_rows[5];

static int row_center_y(int row) {
    return kRowTop[row] + (kRowH / 2);
}

static void build_value_text(const RowData& d, char* out, size_t out_sz) {
    if (!d.valid) {
        snprintf(out, out_sz, "--");
        return;
    }

    char value_buf[16];
    snprintf(value_buf, sizeof(value_buf), d.fmt, d.value);
    if (strcmp(d.unit, "%") == 0 || strcmp(d.unit, L(ST_UNIT_C_SHORT)) == 0 || strcmp(d.unit, L(ST_UNIT_F_SHORT)) == 0) {
        snprintf(out, out_sz, "%s%s", value_buf, d.unit);
    } else {
        snprintf(out, out_sz, "%s %s", value_buf, d.unit);
    }
}

static uint8_t compute_bar_on(float value, float v_min, float v_max, bool valid) {
    if (!valid || v_max <= v_min) return 0;
    const float ratio = constrain((value - v_min) / (v_max - v_min), 0.0f, 1.0f);
    return (uint8_t)lroundf(ratio * (float)kBarSegments);
}

static void draw_bar(int row, uint8_t on, uint16_t color) {
    constexpr int gap = 1;
    const int sw = (kBarW - (kBarSegments - 1) * gap) / kBarSegments;
    const int cy = row_center_y(row);
    for (int i = 0; i < kBarSegments; ++i) {
        const int sx = kBarX + i * (sw + gap);
        const uint16_t c = (i < on) ? color : tft.color565(24, 28, 36);
        tft.fillRoundRect(sx, cy - (kBarH / 2), sw, kBarH, 1, c);
    }
}

static void draw_row_shell() {
    for (int i = 0; i < 5; ++i) {
        const int top = kRowTop[i];
        tft.drawFastHLine(0, top, tft.width(), tft.color565(32, 36, 44));
        tft.drawFastHLine(0, top + kRowH - 1, tft.width(), tft.color565(26, 30, 38));
    }
    tft.drawFastVLine(kDividerX, kRowTop[0], (kRowTop[4] + kRowH - kRowTop[0]), tft.color565(40, 44, 54));
}

static void clear_row_interior(int row) {
    const int top = kRowTop[row];
    tft.fillRect(0, top + 1, kDividerX, kRowH - 2, kBg);
    tft.fillRect(kBarX - 1, top + 1, tft.width() - (kBarX - 1), kRowH - 2, kBg);
}

static void draw_row_content(int row, const RowData& d, const char* value_text, uint8_t bar_on, uint16_t visible_color) {
    const int cy = row_center_y(row);

    d.icon_fn(kIconCX, cy, visible_color);

    tft.setTextFont(2);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(TFT_WHITE, kBg);
    tft.drawString(d.label, kLabelX, cy);

    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(visible_color, kBg);
    tft.drawString(value_text, kValueRightX, cy);
    tft.setTextFont(0);

    draw_bar(row, bar_on, visible_color);
}

static void draw_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_LINEAR_DASH), TFT_WHITE);
    draw_row_shell();

    tft.setTextDatum(TC_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(TFT_DARKGREY, kBg);
    tft.drawString(L(LAB_EXPERIMENT_HINT), 80, 119);
    tft.setTextFont(0);
}

static void draw_dynamic(bool force_all = false) {
    const Reading& r = g_ui_readings_snapshot;

    const bool t_ok = !isnan(r.temperature);
    const bool h_ok = !isnan(r.humidity);
    const bool l_ok = !isnan(r.ldr);
    const bool s_ok = !isnan(r.mic);
    const bool o_ok = !isnan(r.soil_humidity);

    const float t_c = t_ok ? r.temperature : 0.0f;
    const float t_d = g_is_fahrenheit ? (t_c * 1.8f + 32.0f) : t_c;
    const char* t_u = g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
    const float t_min = g_is_fahrenheit ? 32.0f : 0.0f;
    const float t_max = g_is_fahrenheit ? 122.0f : 50.0f;

    const RowData rows[5] = {
        { pbit_draw_temp_icon,     "TEMP",  kOrange,  t_ok, t_d,                     t_min, t_max,   "%.1f", t_u  },
        { pbit_draw_humidity_icon, "HUM",   kCyan,    h_ok, h_ok ? r.humidity : 0.0f, 0.0f, 100.0f,  "%.0f", "%"  },
        { pbit_draw_light_icon,    "LUZ",   kYellow,  l_ok, l_ok ? r.ldr : 0.0f,      0.0f, 1023.0f, "%.0f", "lx" },
        { pbit_draw_sound_icon,    "MIC",   kMagenta, s_ok, s_ok ? r.mic : 0.0f,      0.0f, 100.0f,  "%.0f", "%"  },
        { pbit_draw_plant_icon,    "SUELO", kGreen,   o_ok, o_ok ? r.soil_humidity : 0.0f, 0.0f, 100.0f, "%.0f", "%" },
    };

    for (int i = 0; i < 5; ++i) {
        char value_text[20];
        build_value_text(rows[i], value_text, sizeof(value_text));
        const uint8_t bar_on = compute_bar_on(rows[i].value, rows[i].v_min, rows[i].v_max, rows[i].valid);
        const uint16_t visible_color = rows[i].valid ? rows[i].color : kGrey;

        RowCache& cache = g_last_rows[i];
        const bool changed =
            force_all ||
            !cache.ready ||
            cache.valid != rows[i].valid ||
            cache.color != visible_color ||
            cache.bar_on != bar_on ||
            strcmp(cache.value, value_text) != 0;

        if (!changed) {
            continue;
        }

        clear_row_interior(i);
        draw_row_content(i, rows[i], value_text, bar_on, visible_color);

        cache.ready = true;
        cache.valid = rows[i].valid;
        cache.color = visible_color;
        cache.bar_on = bar_on;
        strncpy(cache.value, value_text, sizeof(cache.value) - 1);
        cache.value[sizeof(cache.value) - 1] = '\0';
    }
}

} // namespace

void draw_lab_linear_dash_screen(bool screen_changed, bool sensor_data_changed) {
    if (screen_changed) {
        draw_shell();
        memset(g_last_rows, 0, sizeof(g_last_rows));
        draw_dynamic(true);
        return;
    }

    if (sensor_data_changed) {
        draw_dynamic(false);
    }
}
