// ui_light.cpp

#include "ui_light.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

/**
 * @brief Dibuja la pantalla de Luz (LIGHT_SCREEN).
 * Escala logarítmica (1-20000 lux) con categorías educativas.
 * Bar color changes dynamically to match the active light category.
 */
void draw_light_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float lux = g_ui_readings_snapshot.ldr;

    // --- Categorías educativas ---
    const char* categoryText;
    uint16_t categoryColor;

    if (lux < 10.0f) {
        categoryText  = L(ST_DARK);
        categoryColor = TFT_DARKGREY;
    } else if (lux < 100.0f) {
        categoryText  = L(ST_DIM);
        categoryColor = TFT_CYAN;
    } else if (lux < 500.0f) {
        categoryText  = L(ST_INDOOR);
        categoryColor = TFT_GREEN;
    } else if (lux < 2000.0f) {
        categoryText  = L(ST_BRIGHT);
        categoryColor = TFT_YELLOW;
    } else {
        categoryText  = L(ST_SUNLIGHT);
        categoryColor = TFT_ORANGE;
    }

    // --- Escala logarítmica: mapea 1-20000 lux → 0-100% ---
    float lux_log = (lux < 1.0f) ? 1.0f : lux;
    float log_pct = (log10f(lux_log) / log10f(20000.0f)) * 100.0f;
    if (log_pct < 0.0f)   log_pct = 0.0f;
    if (log_pct > 100.0f) log_pct = 100.0f;

    const int cx         = tft.width() / 2;

    char luxStr[10];
    snprintf(luxStr, sizeof(luxStr), "%.0f", lux);

    // -----------------------------------------------------------

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_LIGHT), TFT_YELLOW);
    }

    static int last_lux_drawn = -1;
    static int last_category_id = -1;
    int lux_rounded = (int)roundf(lux);
    int category_id =
        (lux < 10.0f)   ? 0 :
        (lux < 100.0f)  ? 1 :
        (lux < 500.0f)  ? 2 :
        (lux < 2000.0f) ? 3 : 4;
    bool value_changed = screen_changed || (lux_rounded != last_lux_drawn);
    bool category_changed = screen_changed || (category_id != last_category_id);
    if (!value_changed && !category_changed) return;
    last_lux_drawn = lux_rounded;
    last_category_id = category_id;

    if (data_changed || screen_changed) {

        if (value_changed) {
            tft.fillRect(0, LB_VALUE_TOP - 4, tft.width(), 52, BACKGROUND_COLOR);
            tft.setFreeFont(FONT_VALUE);
            int numW  = tft.textWidth(luxStr);
            tft.setFreeFont(FONT_BODY);
            int unitW = tft.textWidth("lux");
            int startX = cx - (numW + unitW) / 2;
            tft.setTextDatum(TL_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
            tft.drawString(luxStr, startX, LB_VALUE_TOP);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("lux", startX + numW, LB_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont
        }

        // Barra logarítmica (color = categoría activa)
        if (value_changed || category_changed) {
            drawBarGraph(LB_BAR_X, LB_BAR_Y, LB_BAR_W, LB_BAR_H, categoryColor, log_pct, 0.0f, 100.0f);
        }

        // Categoría educativa — limpiar zona antes de dibujar (evita ghosting)
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
