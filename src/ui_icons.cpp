#include "ui_icons.h"

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void pbit_draw_temp_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 3, cy - 8, 6, 10, 3, color);
    tft.fillCircle(cx, cy + 4, 4, color);
    tft.fillRect(cx - 1, cy - 5, 2, 8, TFT_BLACK);
    tft.drawFastHLine(cx + 3, cy - 4, 2, color);
    tft.drawFastHLine(cx + 3, cy - 1, 2, color);
}

void pbit_draw_probe_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 2, cy - 3, 9, 5, 2, color);
    tft.fillRoundRect(cx + 7, cy - 2, 4, 3, 1, color);
    tft.drawLine(cx - 2, cy, cx - 7, cy + 4, color);
    tft.drawLine(cx - 7, cy + 4, cx - 10, cy + 8, color);
}

void pbit_draw_humidity_icon(int cx, int cy, uint16_t color) {
    tft.fillTriangle(cx, cy - 9, cx - 5, cy - 1, cx + 5, cy - 1, color);
    tft.fillCircle(cx, cy + 2, 5, color);
    tft.fillCircle(cx, cy + 3, 2, TFT_BLACK);
}

void pbit_draw_light_icon(int cx, int cy, uint16_t color) {
    tft.fillCircle(cx, cy - 3, 3, color);
    tft.drawCircle(cx, cy - 3, 3, color);
    tft.drawFastHLine(cx - 6, cy - 3, 3, color);
    tft.drawFastHLine(cx + 4, cy - 3, 3, color);
    tft.drawFastVLine(cx, cy - 9, 3, color);
    tft.drawFastVLine(cx, cy + 2, 3, color);
    tft.drawLine(cx - 4, cy - 6, cx - 6, cy - 8, color);
    tft.drawLine(cx + 4, cy - 6, cx + 6, cy - 8, color);
    tft.drawLine(cx - 4, cy, cx - 6, cy + 2, color);
    tft.drawLine(cx + 4, cy, cx + 6, cy + 2, color);
}

void pbit_draw_sound_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 3, cy - 8, 6, 9, 3, color);
    tft.drawFastVLine(cx, cy + 1, 3, color);
    tft.drawFastHLine(cx - 4, cy + 3, 9, color);
}

void pbit_draw_plant_icon(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 5, cy + 5, 11, 2, 1, color);
    tft.fillRect(cx - 1, cy - 1, 2, 7, color);
    tft.fillTriangle(cx, cy - 1, cx - 6, cy + 1, cx - 2, cy - 6, color);
    tft.fillTriangle(cx, cy - 2, cx + 6, cy, cx + 2, cy - 7, color);
}
