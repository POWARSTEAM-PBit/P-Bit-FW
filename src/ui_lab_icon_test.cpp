// ui_lab_icon_test.cpp
// Lab screen: visual comparison between the procedural icon (existing)
// and the new RGB565 pushImage icon (icon_temp_32.h).
//
// Layout (160x128):
//   Header  Y=0..28
//   Left zone  X=0..82:  RGB565 icon via pushImage
//   Divider    X=83
//   Right zone X=84..159: procedural icon via pbit_draw_temp_icon
//   Footer  Y=118..127
//
// This is a fully static screen — only redraws on screen_changed.

#include "ui_lab_icon_test.h"
#include "tft_display.h"
#include "layout.h"
#include "ui_icons.h"
#include "icon_temp_32.h"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// ── Color palette ──────────────────────────────────────────
static constexpr uint16_t C_HEADER    = 0xFD00; // orange
static constexpr uint16_t C_DIVIDER   = 0x39CC; // blue-grey
static constexpr uint16_t C_LABEL     = 0x8410; // mid-grey
static constexpr uint16_t C_SUBLABEL  = 0x4208; // dim grey
// Left icon uses baked RGB565 colors — no tint constant needed
static constexpr uint16_t C_PROC_ICON = 0xFD00; // orange

// ── Zone geometry ──────────────────────────────────────────
// Left zone: RGB565 icon centered in X=0..82
static constexpr int ZONE_L_CENTER_X = 41;
static constexpr int ICON_RGB_X      = ZONE_L_CENTER_X - 16; // 25
static constexpr int ICON_RGB_Y      = 40;

// Right zone: procedural icon centered in X=84..159
static constexpr int ZONE_R_CENTER_X = 121;
static constexpr int ICON_PROC_CX    = ZONE_R_CENTER_X;
static constexpr int ICON_PROC_CY    = 57;

// Shared vertical positions
static constexpr int LABEL_Y         = 80;
static constexpr int SUBLABEL_Y      = 90;
static constexpr int FOOTER_Y        = 121;

void draw_lab_icon_test_screen(bool screen_changed, bool /*sensor_data_changed*/) {
    // Static screen — nothing to update unless the screen just switched.
    if (!screen_changed) return;

    tft.fillScreen(TFT_BLACK);

    // ── Header ────────────────────────────────────────────
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(C_HEADER, TFT_BLACK);
    tft.drawString("ICON TEST", 80, L_HEADER_Y, 2);
    tft.drawFastHLine(0, L_HEADER_LINE, 160, C_DIVIDER);

    // ── Divider vertical ──────────────────────────────────
    tft.drawFastVLine(83, L_CONTENT_TOP, 84, C_DIVIDER);

    // ── Left: RGB565 icon ─────────────────────────────────
    // pushImage(x, y, w, h, data, transparent_color)
    // Pixels equal to 0x0000 are skipped — background shows through.
    tft.pushImage(ICON_RGB_X, ICON_RGB_Y, 32, 32, (uint16_t*)ICON_TEMP_32, (uint16_t)0x0000);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(C_LABEL, TFT_BLACK);
    tft.drawString("RGB565", ZONE_L_CENTER_X, LABEL_Y, 1);
    tft.setTextColor(C_SUBLABEL, TFT_BLACK);
    tft.drawString("pushImage", ZONE_L_CENTER_X, SUBLABEL_Y, 1);

    // ── Right: procedural icon ────────────────────────────
    pbit_draw_temp_icon(ICON_PROC_CX, ICON_PROC_CY, C_PROC_ICON);

    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(C_LABEL, TFT_BLACK);
    tft.drawString("PROC", ZONE_R_CENTER_X, LABEL_Y, 1);
    tft.setTextColor(C_SUBLABEL, TFT_BLACK);
    tft.drawString("primitives", ZONE_R_CENTER_X, SUBLABEL_Y, 1);

    // ── Footer ────────────────────────────────────────────
    tft.drawFastHLine(0, 118, 160, C_DIVIDER);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(C_SUBLABEL, TFT_BLACK);
    tft.drawString("32x32 / baked colors", 80, FOOTER_Y, 1);
}
