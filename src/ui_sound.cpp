// ui_sound.cpp
// Pantalla de nivel de sonido (micrófono integrado).

#include "ui_sound.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

// =============================================================
// SOUND_SCREEN — barra horizontal + categoría (igual que LIGHT_SCREEN)
// =============================================================
void draw_sound_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_MAGENTA;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float level = (float)g_ui_readings_snapshot.mic;

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

    char levelStr[5];
    snprintf(levelStr, sizeof(levelStr), "%.0f", level);

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_SOUND), TITLE_COLOR);
    }

    static int last_sound_drawn = -1;
    static int last_category_id = -1;
    int level_rounded = (int)roundf(level);
    int category_id =
        (level < 15.0f) ? 0 :
        (level < 40.0f) ? 1 :
        (level < 70.0f) ? 2 :
        (level < 88.0f) ? 3 : 4;
    bool value_changed = screen_changed || (level_rounded != last_sound_drawn);
    bool category_changed = screen_changed || (category_id != last_category_id);
    if (!value_changed && !category_changed) return;
    last_sound_drawn = level_rounded;
    last_category_id = category_id;

    if (data_changed || screen_changed) {
        if (value_changed) {
            tft.fillRect(0, LB_VALUE_TOP - 4, tft.width(), 52, BACKGROUND_COLOR);
            tft.setFreeFont(FONT_VALUE);
            int numW  = tft.textWidth(levelStr);
            tft.setFreeFont(FONT_BODY);
            int unitW = tft.textWidth("%");
            int startX = cx - (numW + unitW) / 2;
            tft.setTextDatum(TL_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(categoryColor, BACKGROUND_COLOR);
            tft.drawString(levelStr, startX, LB_VALUE_TOP);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("%", startX + numW, LB_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont
        }

        if (value_changed || category_changed) {
            drawBarGraph(LB_BAR_X, LB_BAR_Y, LB_BAR_W, LB_BAR_H, categoryColor, level, 0.0f, 100.0f);
        }

        if (category_changed) {
            tft.fillRect(0, LB_CATEGORY_Y - 8, tft.width(), 16, BACKGROUND_COLOR);
            tft.setTextDatum(MC_DATUM);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(categoryColor, BACKGROUND_COLOR);
            tft.drawString(categoryText, cx, LB_CATEGORY_Y);
            tft.setTextFont(0); // liberar GFXfont
        }
    }
}
