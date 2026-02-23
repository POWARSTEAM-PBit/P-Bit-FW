// ui_sound.cpp
// Pantalla de nivel de sonido (micrófono integrado).

#include "ui_sound.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading global_readings;
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

// =============================================================
// SOUND_SCREEN — barra horizontal + categoría (igual que LIGHT_SCREEN)
// =============================================================
void draw_sound_screen(bool screen_changed, bool data_changed) {

    const int FONT_VALUE    = 7;
    const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_MAGENTA;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float level = (float)global_readings.mic;

    const char*   categoryText;
    uint16_t      categoryColor;
    if (level < 15.0f) {
        categoryText = L(ST_SILENT);    categoryColor = TFT_DARKGREY;
    } else if (level < 40.0f) {
        categoryText = L(ST_QUIET);     categoryColor = TFT_GREEN;
    } else if (level < 70.0f) {
        categoryText = L(ST_NORMAL);    categoryColor = TFT_YELLOW;
    } else if (level < 88.0f) {
        categoryText = L(ST_LOUD);      categoryColor = TFT_ORANGE;
    } else {
        categoryText = L(ST_VERY_LOUD); categoryColor = TFT_RED;
    }

    const int cx         = tft.width() / 2;
    const int VALUE_Y    = 65;
    const int BAR_X      = 20;
    const int BAR_Y      = 93;
    const int BAR_W      = tft.width() - 40;
    const int BAR_H      = 14;
    const int CATEGORY_Y = 118;

    char levelStr[5];
    snprintf(levelStr, sizeof(levelStr), "%.0f", level);

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_SOUND), TITLE_COLOR);
    }

    // Salida temprana si el valor no cambió — evita fillRects innecesarios que causan flickering.
    static int last_sound_drawn = -1;
    if (!screen_changed && (int)roundf(level) == last_sound_drawn) return;
    last_sound_drawn = (int)roundf(level);

    if (data_changed || screen_changed) {
        tft.fillRect(0, VALUE_Y - 26, tft.width(), 52, BACKGROUND_COLOR);
        int numW  = tft.textWidth(levelStr, FONT_VALUE);
        int unitW = tft.textWidth("%", 2);
        int startX = cx - (numW + unitW) / 2;
        int topY   = VALUE_Y - 24;
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(levelStr, startX, topY, FONT_VALUE);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString("%", startX + numW, topY, 2);

        drawBarGraph(BAR_X, BAR_Y, BAR_W, BAR_H, categoryColor, level, 0.0f, 100.0f);

        tft.fillRect(0, CATEGORY_Y - 9, tft.width(), 18, BACKGROUND_COLOR);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(categoryText, cx, CATEGORY_Y, FONT_CATEGORY);
    }
}
