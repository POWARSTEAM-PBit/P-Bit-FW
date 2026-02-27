// ui_soil.cpp
// Pantalla de humedad del suelo (sensor capacitivo externo J6).

#include "ui_soil.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading global_readings;

// =============================================================
// SOIL_SCREEN — tanque vertical + valor
// =============================================================
void draw_soil_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_GREEN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    // Layout idéntico a Temp/DS18 — centro del panel izquierdo y tanque derecho
    const int FIRST_THIRD_CX = tft.width() / 3 + 10;  // 63
    const int VALUE_Y = tft.height() / 2 + 15;          // 79
    const int topY    = VALUE_Y - 24;                    // 55 — top de font 7
    const int BAR_X   = 120;
    const int BAR_Y   = 40;
    const int BAR_W   = 30;
    const int BAR_H   = 80;

    float soil = (float)global_readings.soil_humidity;

    const char*   categoryText;
    uint16_t      tankColor;
    uint16_t      categoryColor;
    if (soil < 20.0f) {
        categoryText = L(ST_DRY);       tankColor = TFT_ORANGE; categoryColor = TFT_ORANGE;
    } else if (soil < 55.0f) {
        categoryText = L(ST_OPTIMAL);   tankColor = TFT_GREEN;  categoryColor = TFT_GREEN;
    } else if (soil < 80.0f) {
        categoryText = L(ST_MOIST);     tankColor = TFT_CYAN;   categoryColor = TFT_CYAN;
    } else {
        categoryText = L(ST_SATURATED); tankColor = TFT_BLUE;   categoryColor = TFT_BLUE;
    }

    char soilStr[6];
    snprintf(soilStr, sizeof(soilStr), "%.0f", soil);

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_SOIL), TITLE_COLOR);
        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);
    }

    // Salida temprana si el valor no cambió
    static int last_soil_drawn = -1;
    if (!screen_changed && (int)roundf(soil) == last_soil_drawn) return;
    last_soil_drawn = (int)roundf(soil);

    // --- Dinámicos ---
    if (data_changed || screen_changed) {
        drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, tankColor, soil, 0.0f, 100.0f, 3);
        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);

        // Número + "%" — entero en FONT_VALUE, símbolo en FONT_BODY, mismo topY
        // OLD: int intW = tft.textWidth(soilStr, 7); int unitW = tft.textWidth("%", 4);
        const char* unitStr = "%";
        tft.setFreeFont(FONT_VALUE);
        int intW  = tft.textWidth(soilStr);
        tft.setFreeFont(FONT_BODY);
        int unitW = tft.textWidth(unitStr);
        int startX = FIRST_THIRD_CX - (intW + unitW) / 2;
        tft.fillRect(0, topY - 2, BAR_X - 1, 52, BACKGROUND_COLOR);
        tft.setTextDatum(TL_DATUM);
        // OLD: tft.drawString(soilStr, startX, topY, 7);
        tft.setFreeFont(FONT_VALUE);
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.drawString(soilStr, startX, topY);
        // OLD: tft.drawString(unitStr, startX + intW, topY, 4);
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(TITLE_COLOR, BACKGROUND_COLOR);
        tft.drawString(unitStr, startX + intW, topY);
        tft.setTextFont(0); // liberar GFXfont

        // Categoría — centrada en panel izquierdo, debajo del número
        // OLD (FreeSans9pt7b — sustituida por Inter Body, mismo soporte Latin-1):
        // tft.setFreeFont(&FreeSans9pt7b);  tft.setTextFont(2);
        tft.fillRect(0, topY + 50, BAR_X - 1, tft.height() - (topY + 50), BACKGROUND_COLOR);
        tft.setFreeFont(FONT_BODY);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(categoryText, FIRST_THIRD_CX, tft.height() - 14);
        tft.setTextFont(0); // liberar GFXfont
    }
}
