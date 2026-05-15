// ui_lab_icon_sizes.cpp
// Icon size comparison gallery: shows 3 sensor icons at S / M / L side by side.
// ENV page: temperature, humidity, light
// EXT page: sound, soil, probe
//
// Layout (160x128 landscape):
//   Column centres: S=52  M=96  L=140
//   Row centres:    row1=50  row2=78  row3=106
//   Sensor labels: left-aligned at x=2, same y as row centres.

#include "ui_lab_icon_sizes.h"

#include "fonts.h"
#include "languages.h"
#include "tft_display.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

namespace {

// ── palette ──────────────────────────────────────────────────────────────────
constexpr uint16_t kBg      = TFT_BLACK;
constexpr uint16_t kOrange  = TFT_ORANGE;
constexpr uint16_t kCyan    = TFT_CYAN;
constexpr uint16_t kYellow  = 0xFFE0;
constexpr uint16_t kMagenta = TFT_MAGENTA;
constexpr uint16_t kGreen   = TFT_GREEN;
// color565(180, 100, 255): ((176<<8)|(100<<3)|31) = 0xB33F
constexpr uint16_t kPurple  = 0xB33F;

// ── column / row layout constants ────────────────────────────────────────────
constexpr int kColS  = 48;
constexpr int kColM  = 88;
constexpr int kColL  = 130;
constexpr int kRow1  = 48;
constexpr int kRow2  = 76;
constexpr int kRow3  = 104;
constexpr int kLabelX = 4;

// ── Small icons (~10px tall, solid flat) ─────────────────────────────────────

static void draw_s_temp(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 2, cy - 6, 5, 8, 2, c);
    tft.fillCircle(cx, cy + 3, 4, c);
    tft.fillRect(cx - 1, cy - 4, 2, 7, kBg);
}

static void draw_s_hum(int cx, int cy, uint16_t c) {
    tft.fillTriangle(cx, cy - 8, cx - 5, cy - 1, cx + 5, cy - 1, c);
    tft.fillCircle(cx, cy + 3, 5, c);
    tft.fillCircle(cx, cy + 4, 2, kBg);
}

static void draw_s_light(int cx, int cy, uint16_t c) {
    tft.fillCircle(cx, cy - 2, 3, c);
    tft.drawFastHLine(cx - 6, cy - 2, 3, c);
    tft.drawFastHLine(cx + 4, cy - 2, 3, c);
    tft.drawFastVLine(cx, cy - 8, 3, c);
    tft.drawFastVLine(cx, cy + 3, 3, c);
    tft.drawLine(cx - 3, cy - 5, cx - 5, cy - 7, c);
    tft.drawLine(cx + 3, cy - 5, cx + 5, cy - 7, c);
    tft.drawLine(cx - 3, cy + 1, cx - 5, cy + 3, c);
    tft.drawLine(cx + 3, cy + 1, cx + 5, cy + 3, c);
}

static void draw_s_sound(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 3, cy - 6, 6, 8, 2, c);
    tft.drawFastVLine(cx, cy + 2, 3, c);
    tft.drawFastHLine(cx - 4, cy + 4, 9, c);
}

static void draw_s_soil(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 5, cy + 4, 11, 2, 1, c);
    tft.fillRect(cx - 1, cy - 2, 2, 7, c);
    tft.fillTriangle(cx, cy - 1, cx - 5, cy + 1, cx - 2, cy - 5, c);
    tft.fillTriangle(cx, cy - 2, cx + 5, cy, cx + 2, cy - 6, c);
}

static void draw_s_probe(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 4, cy - 3, 10, 5, 2, c);
    tft.fillRoundRect(cx + 6, cy - 2, 4, 3, 1, c);
    tft.drawLine(cx - 4, cy, cx - 8, cy + 3, c);
    tft.drawLine(cx - 8, cy + 3, cx - 10, cy + 7, c);
}

// ── Medium icons (~18px tall, solid flat) ─────────────────────────────────────

static void draw_m_temp(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 5, cy - 11, 10, 16, 4, c);
    tft.fillCircle(cx, cy + 6, 7, c);
    tft.fillRect(cx - 1, cy - 8, 2, 12, kBg);
    tft.drawFastHLine(cx + 6, cy - 6, 2, c);
    tft.drawFastHLine(cx + 6, cy - 2, 2, c);
}

static void draw_m_hum(int cx, int cy, uint16_t c) {
    tft.fillTriangle(cx, cy - 12, cx - 8, cy - 1, cx + 8, cy - 1, c);
    tft.fillCircle(cx, cy + 3, 8, c);
    tft.fillCircle(cx, cy + 4, 3, kBg);
}

static void draw_m_light(int cx, int cy, uint16_t c) {
    tft.drawCircle(cx, cy, 8, c);
    tft.drawCircle(cx, cy, 7, c);
    tft.drawFastHLine(cx - 14, cy, 4, c);
    tft.drawFastHLine(cx + 11, cy, 4, c);
    tft.drawFastVLine(cx, cy - 14, 4, c);
    tft.drawFastVLine(cx, cy + 11, 4, c);
    tft.drawLine(cx - 10, cy - 10, cx - 13, cy - 13, c);
    tft.drawLine(cx + 10, cy - 10, cx + 13, cy - 13, c);
    tft.drawLine(cx - 10, cy + 10, cx - 13, cy + 13, c);
    tft.drawLine(cx + 10, cy + 10, cx + 13, cy + 13, c);
}

static void draw_m_sound(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 6, cy - 11, 12, 18, 6, c);
    tft.drawFastVLine(cx, cy + 7, 6, c);
    tft.drawFastHLine(cx - 8, cy + 12, 17, c);
}

static void draw_m_soil(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 10, cy + 10, 20, 3, 1, c);
    tft.fillRect(cx - 1, cy - 3, 2, 13, c);
    tft.fillTriangle(cx, cy + 1, cx - 8, cy + 2, cx - 3, cy - 7, c);
    tft.fillTriangle(cx, cy, cx + 9, cy + 1, cx + 4, cy - 8, c);
}

static void draw_m_probe(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 6, cy - 4, 14, 8, 3, c);
    tft.fillRoundRect(cx + 8, cy - 3, 5, 6, 2, c);
    tft.drawLine(cx - 6, cy, cx - 11, cy + 4, c);
    tft.drawLine(cx - 11, cy + 4, cx - 14, cy + 10, c);
    tft.drawLine(cx + 13, cy, cx + 18, cy - 4, c);
}

// ── Large icons (~28px tall, solid filled) ───────────────────────────────────

static void draw_l_temp(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 7, cy - 16, 14, 24, 6, c);
    tft.fillCircle(cx, cy + 9, 9, c);
    tft.fillRoundRect(cx - 2, cy - 13, 4, 18, 2, kBg);
    tft.fillRoundRect(cx + 7, cy - 12, 6, 3, 1, c);
    tft.fillRoundRect(cx + 7, cy - 6, 5, 3, 1, c);
}

static void draw_l_hum(int cx, int cy, uint16_t c) {
    tft.fillTriangle(cx, cy - 18, cx - 12, cy - 3, cx + 12, cy - 3, c);
    tft.fillCircle(cx, cy + 6, 12, c);
    tft.fillCircle(cx, cy + 8, 5, kBg);
}

static void draw_l_light(int cx, int cy, uint16_t c) {
    tft.fillCircle(cx, cy, 10, c);
    tft.fillRoundRect(cx - 2, cy - 20, 4, 7, 2, c);
    tft.fillRoundRect(cx - 2, cy + 14, 4, 7, 2, c);
    tft.fillRoundRect(cx - 20, cy - 2, 7, 4, 2, c);
    tft.fillRoundRect(cx + 14, cy - 2, 7, 4, 2, c);
    tft.fillRoundRect(cx - 15, cy - 15, 4, 7, 2, c);
    tft.fillRoundRect(cx + 11, cy - 15, 4, 7, 2, c);
    tft.fillRoundRect(cx - 15, cy + 8, 4, 7, 2, c);
    tft.fillRoundRect(cx + 11, cy + 8, 4, 7, 2, c);
}

static void draw_l_sound(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 8, cy - 16, 16, 22, 7, c);
    tft.drawFastVLine(cx, cy + 6, 8, c);
    tft.drawFastHLine(cx - 11, cy + 14, 23, c);
    tft.drawLine(cx - 13, cy, cx - 16, cy - 3, c);
    tft.drawLine(cx + 13, cy, cx + 16, cy - 3, c);
}

static void draw_l_soil(int cx, int cy, uint16_t c) {
    tft.fillRect(cx - 1, cy - 5, 3, 17, c);
    tft.fillTriangle(cx - 1, cy + 4, cx - 13, cy - 2, cx - 5, cy - 12, c);
    tft.fillTriangle(cx + 1, cy + 2, cx + 13, cy - 4, cx + 5, cy - 14, c);
    tft.fillRoundRect(cx - 14, cy + 13, 28, 5, 2, c);
}

static void draw_l_probe(int cx, int cy, uint16_t c) {
    tft.fillRoundRect(cx - 14, cy - 4, 20, 9, 3, c);
    tft.fillRoundRect(cx + 6, cy - 3, 7, 7, 2, c);
    tft.drawLine(cx + 13, cy, cx + 19, cy - 3, c);
    tft.drawLine(cx + 13, cy + 1, cx + 19, cy - 2, c);
    tft.drawLine(cx - 14, cy + 1, cx - 18, cy + 4, c);
    tft.drawLine(cx - 18, cy + 4, cx - 22, cy + 10, c);
}

// ── helpers ──────────────────────────────────────────────────────────────────

static void draw_size_labels() {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(tft.color565(100, 110, 130), kBg);
    tft.drawString("S", kColS, 30);
    tft.drawString("M", kColM, 30);
    tft.drawString("L", kColL, 30);
    tft.setTextFont(0);
}

static void draw_row_label(const char* text, int y, uint16_t c) {
    tft.setTextDatum(ML_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(c, kBg);
    tft.drawString(text, kLabelX, y);
    tft.setTextFont(0);
}

typedef void (*DrawFn)(int, int, uint16_t);

struct IconRow {
    const char* label;
    uint16_t    color;
    DrawFn      fn_s;
    DrawFn      fn_m;
    DrawFn      fn_l;
    int         row_y;
};

static void draw_size_page(LangKey title_key, const IconRow* rows, int n) {
    tft.fillScreen(kBg);
    drawHeader(L(title_key));
    draw_size_labels();

    for (int i = 0; i < n; ++i) {
        const auto& r = rows[i];
        draw_row_label(r.label, r.row_y, r.color);
        r.fn_s(kColS, r.row_y, r.color);
        r.fn_m(kColM, r.row_y, r.color);
        r.fn_l(kColL, r.row_y, r.color);
    }
}

} // namespace

void draw_lab_icon_sizes_env_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    if (!screen_changed) return;

    static const IconRow kRows[3] = {
        { "TEMP",  kOrange,  draw_s_temp,  draw_m_temp,  draw_l_temp,  kRow1 },
        { "HUM",   kCyan,    draw_s_hum,   draw_m_hum,   draw_l_hum,   kRow2 },
        { "LUZ",   kYellow,  draw_s_light, draw_m_light, draw_l_light, kRow3 },
    };
    draw_size_page(TIT_LAB_ICON_SZ_ENV, kRows, 3);
}

void draw_lab_icon_sizes_ext_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    if (!screen_changed) return;

    static const IconRow kRows[3] = {
        { "MIC",   kMagenta, draw_s_sound, draw_m_sound, draw_l_sound, kRow1 },
        { "SUELO", kGreen,   draw_s_soil,  draw_m_soil,  draw_l_soil,  kRow2 },
        { "SONDA", kPurple,  draw_s_probe, draw_m_probe, draw_l_probe, kRow3 },
    };
    draw_size_page(TIT_LAB_ICON_SZ_EXT, kRows, 3);
}
