// ui_humidity.cpp
#include "ui_humidity.h"
#include "tft_display.h"
#include "io.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
extern void drawHeader(const char* title, uint16_t color);

void draw_humidity_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_STATUS = 2;
    const uint16_t HUMIDITY_COLOR   = TFT_CYAN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    const int LEFT_PANEL_W = LA_TANK_X - 1;

    // Coordenadas definidas en layout.h (Familia A)

    float hum      = g_ui_readings_snapshot.humidity;
    bool  no_dht_h = isnan(hum);

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_HUM), HUMIDITY_COLOR);
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);

        tft.fillRect(0, LA_HINT_Y - 4, LEFT_PANEL_W, 18, BACKGROUND_COLOR);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
        tft.drawString(L(SUB_AIR_REL), LA_LEFT_CX, LA_HINT_Y);
        tft.setTextFont(0);
    }

    // Salida temprana si el valor no cambió
    static int last_hum_drawn = -1;
    int hum_cache = no_dht_h ? -9999 : (int)roundf(hum);
    if (!screen_changed && hum_cache == last_hum_drawn) return;
    last_hum_drawn = hum_cache;

    // --- Dinámicos ---
    if (data_changed || screen_changed) {

        if (no_dht_h) {
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, HUMIDITY_COLOR, 0.0f, 0.0f, 100.0f, 3);
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
            tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 46, BACKGROUND_COLOR);
            tft.setTextDatum(TC_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("---", LA_LEFT_CX, LA_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont
            tft.fillRect(0, LA_CATEGORY_Y - 10, LEFT_PANEL_W, 28, BACKGROUND_COLOR);

        } else {
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, HUMIDITY_COLOR, hum, 0.0f, 100.0f, 3);
            tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);

            // Número + "%" — entero en FONT_VALUE, símbolo en FONT_BODY, mismo topY
            // OLD: int intW = tft.textWidth(humStr, 7); int unitW = tft.textWidth("%", 4);
            char humStr[6];
            snprintf(humStr, sizeof(humStr), "%.0f", hum);
            const char* unitStr = "%";
            tft.setFreeFont(FONT_VALUE);
            int intW  = tft.textWidth(humStr);
            tft.setFreeFont(FONT_BODY);
            int unitW = tft.textWidth(unitStr);
            int startX = LA_LEFT_CX - (intW + unitW) / 2;
            tft.fillRect(0, LA_VALUE_TOP - 1, LEFT_PANEL_W, 46, BACKGROUND_COLOR);
            tft.setTextDatum(TL_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
            tft.drawString(humStr, startX, LA_VALUE_TOP);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(HUMIDITY_COLOR, BACKGROUND_COLOR);
            tft.drawString(unitStr, startX + intW, LA_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont

            // Estado — centrado en panel izquierdo, debajo del número
            tft.fillRect(0, LA_CATEGORY_Y - 10, LEFT_PANEL_W, 28, BACKGROUND_COLOR);
            const char* statusText;
            uint16_t statusColor;
            if      (hum > 70.0) { statusText = L(ST_MOLD_RISK); statusColor = TFT_RED; }
            else if (hum < 30.0) { statusText = L(ST_TOO_DRY);  statusColor = TFT_ORANGE; }
            else                  { statusText = L(ST_OPTIMAL);  statusColor = TFT_GREEN; }
            // OLD: tft.drawString(statusText, FIRST_THIRD_CX, tft.height() - 14, 2);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(statusColor, BACKGROUND_COLOR);
            tft.drawString(statusText, LA_LEFT_CX, LA_CATEGORY_Y);
            tft.setTextFont(0); // liberar GFXfont
        }
    }
}
