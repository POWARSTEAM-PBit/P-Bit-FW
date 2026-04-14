#include "ui_lab_icon_gallery.h"

#include "languages.h"
#include "fonts.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

namespace {

struct GalleryItem {
    LangKey label;
};

constexpr GalleryItem kItems[6] = {
    { LAB_TEMP_SHORT },
    { LAB_HUM_SHORT },
    { LAB_LIGHT_SHORT },
    { LAB_SOUND_SHORT },
    { LAB_SOIL_SHORT },
    { LAB_PROBE_SHORT },
};

constexpr uint8_t kGalleryOrder[4] = { 0, 1, 4, 5 };
constexpr int kCellX[2] = { 6, 82 };
constexpr int kCellY[2] = { 34, 74 };
constexpr int kCellW = 72;
constexpr int kCellH = 40;
constexpr int kIconOffsetY = 13;
constexpr int kLabelY = 28;

// Outline neon ---------------------------------------------------------------

void draw_outline_temp(int cx, int cy, uint16_t color) {
    tft.drawRoundRect(cx - 5, cy - 11, 10, 18, 4, color);
    tft.drawRoundRect(cx - 2, cy - 8, 4, 11, 2, color);
    tft.drawCircle(cx, cy + 7, 6, color);
    tft.drawCircle(cx, cy + 7, 5, color);
    tft.drawFastVLine(cx, cy - 7, 9, color);
    tft.drawFastVLine(cx + 8, cy - 6, 3, color);
    tft.drawFastVLine(cx + 8, cy - 1, 2, color);
}

void draw_outline_drop(int cx, int cy, uint16_t color) {
    tft.drawLine(cx, cy - 12, cx - 8, cy - 1, color);
    tft.drawLine(cx, cy - 12, cx + 8, cy - 1, color);
    tft.drawCircle(cx, cy + 3, 8, color);
    tft.drawCircle(cx, cy + 3, 7, color);
    tft.drawCircle(cx, cy + 3, 3, color);
}

void draw_outline_plant(int cx, int cy, uint16_t color) {
    tft.drawFastVLine(cx, cy - 5, 15, color);
    tft.drawLine(cx, cy + 2, cx - 8, cy - 3, color);
    tft.drawLine(cx - 8, cy - 3, cx - 4, cy - 9, color);
    tft.drawLine(cx, cy, cx + 9, cy - 5, color);
    tft.drawLine(cx + 9, cy - 5, cx + 5, cy - 11, color);
    tft.drawFastHLine(cx - 10, cy + 11, 21, color);
}

void draw_outline_mic(int cx, int cy, uint16_t color) {
    tft.drawRoundRect(cx - 6, cy - 11, 12, 18, 6, color);
    tft.drawFastVLine(cx, cy + 7, 6, color);
    tft.drawFastHLine(cx - 8, cy + 12, 17, color);
    tft.drawFastVLine(cx - 9, cy - 1, 6, color);
    tft.drawFastVLine(cx + 9, cy - 1, 6, color);
}

void draw_outline_sun(int cx, int cy, uint16_t color) {
    tft.drawCircle(cx, cy, 8, color);
    tft.drawCircle(cx, cy, 7, color);
    tft.drawFastHLine(cx - 14, cy, 4, color);
    tft.drawFastHLine(cx + 11, cy, 4, color);
    tft.drawFastVLine(cx, cy - 14, 4, color);
    tft.drawFastVLine(cx, cy + 11, 4, color);
    tft.drawLine(cx - 10, cy - 10, cx - 13, cy - 13, color);
    tft.drawLine(cx + 10, cy - 10, cx + 13, cy - 13, color);
    tft.drawLine(cx - 10, cy + 10, cx - 13, cy + 13, color);
    tft.drawLine(cx + 10, cy + 10, cx + 13, cy + 13, color);
}

void draw_outline_probe(int cx, int cy, uint16_t color) {
    tft.drawRoundRect(cx - 4, cy - 4, 16, 8, 2, color);
    tft.drawLine(cx + 12, cy, cx + 18, cy - 6, color);
    tft.drawLine(cx + 12, cy + 1, cx + 18, cy - 5, color);
    tft.drawLine(cx - 4, cy, cx - 10, cy + 6, color);
    tft.drawLine(cx - 10, cy + 6, cx - 12, cy + 12, color);
}

// Solid bold -----------------------------------------------------------------

void draw_solid_temp(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 4, cy - 11, 8, 17, 4, color);
    tft.fillCircle(cx, cy + 7, 7, color);
    tft.fillRect(cx - 1, cy - 7, 2, 10, TFT_BLACK);
}

void draw_solid_drop(int cx, int cy, uint16_t color) {
    tft.fillTriangle(cx, cy - 13, cx - 8, cy - 2, cx + 8, cy - 2, color);
    tft.fillCircle(cx, cy + 3, 8, color);
    tft.fillCircle(cx, cy + 4, 4, TFT_BLACK);
}

void draw_solid_plant(int cx, int cy, uint16_t color) {
    tft.fillRect(cx - 1, cy - 2, 3, 12, color);
    tft.fillTriangle(cx, cy + 1, cx - 10, cy + 2, cx - 4, cy - 8, color);
    tft.fillTriangle(cx, cy, cx + 10, cy + 1, cx + 4, cy - 9, color);
    tft.fillRoundRect(cx - 11, cy + 10, 22, 4, 2, color);
}

void draw_solid_mic(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 7, cy - 12, 14, 18, 6, color);
    tft.drawFastVLine(cx, cy + 6, 6, color);
    tft.drawFastHLine(cx - 8, cy + 12, 17, color);
}

void draw_solid_sun(int cx, int cy, uint16_t color) {
    tft.fillCircle(cx, cy, 8, color);
    tft.fillRoundRect(cx - 2, cy - 16, 4, 6, 2, color);
    tft.fillRoundRect(cx - 2, cy + 11, 4, 6, 2, color);
    tft.fillRoundRect(cx - 16, cy - 2, 6, 4, 2, color);
    tft.fillRoundRect(cx + 11, cy - 2, 6, 4, 2, color);
    tft.fillRoundRect(cx - 12, cy - 12, 4, 6, 2, color);
    tft.fillRoundRect(cx + 8, cy - 12, 4, 6, 2, color);
    tft.fillRoundRect(cx - 12, cy + 8, 4, 6, 2, color);
    tft.fillRoundRect(cx + 8, cy + 8, 4, 6, 2, color);
}

void draw_solid_probe(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 10, cy - 3, 16, 7, 3, color);
    tft.fillRoundRect(cx + 6, cy - 2, 5, 5, 2, color);
    tft.drawLine(cx + 11, cy, cx + 17, cy - 3, color);
    tft.drawLine(cx - 10, cy, cx - 15, cy + 4, color);
    tft.drawLine(cx - 15, cy + 4, cx - 18, cy + 9, color);
}

// Pixel premium --------------------------------------------------------------

void px(int x, int y, int s, uint16_t color) {
    tft.fillRect(x, y, s, s, color);
}

void draw_pixel_temp(int cx, int cy, uint16_t color) {
    const int s = 2;
    px(cx - 2, cy - 12, s, color); px(cx, cy - 12, s, color);
    px(cx - 4, cy - 10, s, color); px(cx + 2, cy - 10, s, color);
    for (int y = cy - 8; y <= cy; y += 2) {
        px(cx - 4, y, s, color); px(cx + 2, y, s, color);
    }
    px(cx - 2, cy + 2, s, color); px(cx, cy + 2, s, color);
    px(cx - 6, cy + 4, s, color); px(cx + 4, cy + 4, s, color);
    px(cx - 6, cy + 6, s, color); px(cx + 4, cy + 6, s, color);
    px(cx - 4, cy + 8, s, color); px(cx + 2, cy + 8, s, color);
    px(cx - 2, cy + 10, s, color); px(cx, cy + 10, s, color);
}

void draw_pixel_drop(int cx, int cy, uint16_t color) {
    const int s = 2;
    px(cx, cy - 12, s, color);
    px(cx - 2, cy - 10, s, color); px(cx + 2, cy - 10, s, color);
    px(cx - 4, cy - 8, s, color); px(cx + 4, cy - 8, s, color);
    px(cx - 6, cy - 6, s, color); px(cx + 6, cy - 6, s, color);
    px(cx - 6, cy - 4, s, color); px(cx + 6, cy - 4, s, color);
    px(cx - 4, cy - 2, s, color); px(cx + 4, cy - 2, s, color);
    px(cx - 4, cy, s, color); px(cx + 4, cy, s, color);
    px(cx - 2, cy + 2, s, color); px(cx + 2, cy + 2, s, color);
    px(cx, cy + 4, s, color);
}

void draw_pixel_plant(int cx, int cy, uint16_t color) {
    const int s = 2;
    for (int x = cx - 8; x <= cx + 8; x += 2) px(x, cy + 10, s, color);
    px(cx, cy - 2, s, color); px(cx, cy, s, color); px(cx, cy + 2, s, color); px(cx, cy + 4, s, color); px(cx, cy + 6, s, color);
    px(cx - 4, cy, s, color); px(cx - 6, cy - 2, s, color); px(cx - 8, cy - 2, s, color); px(cx - 6, cy - 4, s, color);
    px(cx + 4, cy - 1, s, color); px(cx + 6, cy - 3, s, color); px(cx + 8, cy - 3, s, color); px(cx + 6, cy - 5, s, color);
}

void draw_pixel_mic(int cx, int cy, uint16_t color) {
    const int s = 2;
    for (int y = cy - 10; y <= cy; y += 2) {
        px(cx - 4, y, s, color); px(cx + 2, y, s, color);
    }
    px(cx - 2, cy - 12, s, color); px(cx, cy - 12, s, color);
    px(cx - 2, cy + 2, s, color); px(cx, cy + 2, s, color);
    px(cx - 2, cy + 4, s, color); px(cx, cy + 4, s, color);
    for (int x = cx - 6; x <= cx + 4; x += 2) px(x, cy + 8, s, color);
}

void draw_pixel_sun(int cx, int cy, uint16_t color) {
    const int s = 2;
    px(cx - 2, cy - 2, s, color); px(cx, cy - 2, s, color); px(cx + 2, cy - 2, s, color);
    px(cx - 2, cy, s, color); px(cx, cy, s, color); px(cx + 2, cy, s, color);
    px(cx - 2, cy + 2, s, color); px(cx, cy + 2, s, color); px(cx + 2, cy + 2, s, color);
    px(cx, cy - 10, s, color); px(cx, cy - 8, s, color);
    px(cx, cy + 8, s, color); px(cx, cy + 10, s, color);
    px(cx - 10, cy, s, color); px(cx - 8, cy, s, color);
    px(cx + 8, cy, s, color); px(cx + 10, cy, s, color);
    px(cx - 8, cy - 8, s, color); px(cx + 8, cy - 8, s, color);
    px(cx - 8, cy + 8, s, color); px(cx + 8, cy + 8, s, color);
}

void draw_pixel_probe(int cx, int cy, uint16_t color) {
    const int s = 2;
    for (int x = cx - 10; x <= cx + 4; x += 2) px(x, cy - 2, s, color);
    for (int x = cx - 10; x <= cx + 4; x += 2) px(x, cy, s, color);
    px(cx + 6, cy - 2, s, color); px(cx + 8, cy - 2, s, color);
    px(cx + 6, cy, s, color); px(cx + 8, cy, s, color);
    px(cx + 10, cy - 2, s, color); px(cx + 12, cy - 4, s, color);
    px(cx - 12, cy, s, color); px(cx - 14, cy + 2, s, color); px(cx - 16, cy + 4, s, color);
}

void draw_compact_footer(const char* text) {
    tft.setTextDatum(TC_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(tft.color565(90, 100, 122), TFT_BLACK);
    tft.drawString(text, 80, 119);
    tft.setTextFont(0);
}

uint16_t item_color(int index) {
    switch (index) {
        case 0: return TFT_ORANGE;
        case 1: return TFT_CYAN;
        case 2: return tft.color565(255, 255, 72);
        case 3: return TFT_MAGENTA;
        case 4: return TFT_GREEN;
        case 5: return tft.color565(180, 100, 255);
        default: return TFT_WHITE;
    }
}

void draw_icon_variant(int variant, int index, int cx, int cy, uint16_t color) {
    if (variant == 0) {
        switch (index) {
            case 0: draw_outline_temp(cx, cy, color); break;
            case 1: draw_outline_drop(cx, cy, color); break;
            case 2: draw_outline_sun(cx, cy, color); break;
            case 3: draw_outline_mic(cx, cy, color); break;
            case 4: draw_outline_plant(cx, cy, color); break;
            case 5: draw_outline_probe(cx, cy, color); break;
        }
        return;
    }
    if (variant == 1) {
        switch (index) {
            case 0: draw_solid_temp(cx, cy, color); break;
            case 1: draw_solid_drop(cx, cy, color); break;
            case 2: draw_solid_sun(cx, cy, color); break;
            case 3: draw_solid_mic(cx, cy, color); break;
            case 4: draw_solid_plant(cx, cy, color); break;
            case 5: draw_solid_probe(cx, cy, color); break;
        }
    } else {
        switch (index) {
            case 0: draw_pixel_temp(cx, cy, color); break;
            case 1: draw_pixel_drop(cx, cy, color); break;
            case 2: draw_pixel_sun(cx, cy, color); break;
            case 3: draw_pixel_mic(cx, cy, color); break;
            case 4: draw_pixel_plant(cx, cy, color); break;
            case 5: draw_pixel_probe(cx, cy, color); break;
        }
    }
}

void draw_gallery(LangKey title_key, int variant) {
    tft.fillScreen(TFT_BLACK);
    drawHeader(L(title_key), TFT_WHITE);

    for (int i = 0; i < 4; ++i) {
        const int item_index = kGalleryOrder[i];
        const int col = i % 2;
        const int row = i / 2;
        const int x = kCellX[col];
        const int y = kCellY[row];
        const int cx = x + (kCellW / 2);
        const int cy = y + kIconOffsetY;
        const uint16_t color = item_color(item_index);
        tft.drawRoundRect(x, y, kCellW, kCellH, 4, tft.color565(32, 40, 64));
        draw_icon_variant(variant, item_index, cx, cy, color);

        tft.setTextDatum(TC_DATUM);
        tft.setTextFont(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(L(kItems[item_index].label), cx, y + kLabelY);
        tft.setTextFont(0);
    }

    draw_compact_footer(L(LAB_COMPARE_HINT));
}

} // namespace

void draw_lab_icon_set_a_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    if (screen_changed) draw_gallery(TIT_LAB_ICON_A, 0);
}

void draw_lab_icon_set_b_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    if (screen_changed) draw_gallery(TIT_LAB_ICON_B, 1);
}

void draw_lab_icon_set_c_screen(bool screen_changed, bool sensor_data_changed) {
    (void)sensor_data_changed;
    if (screen_changed) draw_gallery(TIT_LAB_ICON_C, 2);
}
