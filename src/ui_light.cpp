// ui_light.cpp

#include "ui_light.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading global_readings;
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

/**
 * @brief Dibuja la pantalla de Luz (LIGHT_SCREEN).
 * Escala logarítmica (1-20000 lux) con categorías educativas.
 * Bar color changes dynamically to match the active light category.
 */
void draw_light_screen(bool screen_changed, bool data_changed) {

    const int FONT_VALUE    = 7;
    const int FONT_CATEGORY = 2;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float lux = global_readings.ldr;

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

    // --- Layout (pantalla 128x160) ---
    const int cx         = tft.width() / 2;
    const int VALUE_Y    = 65;
    const int BAR_X      = 20;
    const int BAR_Y      = 93;
    const int BAR_W      = tft.width() - 40;
    const int BAR_H      = 14;
    const int CATEGORY_Y = 118;

    char luxStr[10];
    snprintf(luxStr, sizeof(luxStr), "%.0f", lux);

    // -----------------------------------------------------------

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_LIGHT), TFT_YELLOW);
    }

    // Salida temprana si el valor no cambió — evita fillRects innecesarios que causan flickering.
    static int last_lux_drawn = -1;
    if (!screen_changed && (int)roundf(lux) == last_lux_drawn) return;
    last_lux_drawn = (int)roundf(lux);

    if (data_changed || screen_changed) {

        // Valor numérico grande — limpiar zona antes de dibujar (evita ghosting)
        tft.fillRect(0, VALUE_Y - 26, tft.width(), 52, BACKGROUND_COLOR);
        int numW  = tft.textWidth(luxStr, FONT_VALUE);
        int unitW = tft.textWidth("lux", 2);
        int startX = cx - (numW + unitW) / 2;
        int topY   = VALUE_Y - 24;
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.drawString(luxStr, startX, topY, FONT_VALUE);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString("lux", startX + numW, topY, 2);

        // Barra logarítmica (color = categoría activa)
        drawBarGraph(BAR_X, BAR_Y, BAR_W, BAR_H, categoryColor, log_pct, 0.0f, 100.0f);

        // Categoría educativa — limpiar zona antes de dibujar (evita ghosting)
        tft.fillRect(0, CATEGORY_Y - 9, tft.width(), 18, BACKGROUND_COLOR);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(categoryText, cx, CATEGORY_Y, FONT_CATEGORY);
    }
}
