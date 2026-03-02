// ui_soil.cpp
// Pantalla de humedad del suelo (sensor capacitivo externo J6).

#include "ui_soil.h"
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

// =============================================================
// SOIL_SCREEN — tanque vertical + valor
// =============================================================
void draw_soil_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_GREEN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    // Coordenadas definidas en layout.h (Familia A)

    float soil = (float)g_ui_readings_snapshot.soil_humidity;

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
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
    }

    static int last_soil_drawn = -1;
    static int last_category_id = -1;
    int soil_rounded = (int)roundf(soil);
    int category_id =
        (soil < 20.0f) ? 0 :
        (soil < 55.0f) ? 1 :
        (soil < 80.0f) ? 2 : 3;
    bool value_changed = screen_changed || (soil_rounded != last_soil_drawn);
    bool category_changed = screen_changed || (category_id != last_category_id);
    if (!value_changed && !category_changed) return;
    last_soil_drawn = soil_rounded;
    last_category_id = category_id;

    // --- Dinámicos ---
    if (data_changed || screen_changed) {
        if (value_changed || category_changed) {
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, tankColor, soil, 0.0f, 100.0f, 3);
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
        }

        const char* unitStr = "%";
        if (value_changed) {
            tft.setFreeFont(FONT_VALUE);
            int intW  = tft.textWidth(soilStr);
            tft.setFreeFont(FONT_BODY);
            int unitW = tft.textWidth(unitStr);
            int startX = LA_LEFT_CX - (intW + unitW) / 2;
            tft.fillRect(0, LA_VALUE_TOP - 4, LA_TANK_X - 1, 52, BACKGROUND_COLOR);
            tft.setTextDatum(TL_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
            tft.drawString(soilStr, startX, LA_VALUE_TOP);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(TITLE_COLOR, BACKGROUND_COLOR);
            tft.drawString(unitStr, startX + intW, LA_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont
        }

        // Categoría — centrada en panel izquierdo, debajo del número
        if (category_changed) {
            tft.fillRect(0, LA_CATEGORY_Y - 8, LA_TANK_X - 1, tft.height() - (LA_CATEGORY_Y - 8), BACKGROUND_COLOR);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(categoryColor, BACKGROUND_COLOR);
            tft.drawString(categoryText, LA_LEFT_CX, LA_CATEGORY_Y);
            tft.setTextFont(0); // liberar GFXfont
        }
    }
}
