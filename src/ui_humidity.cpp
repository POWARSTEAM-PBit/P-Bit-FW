// ui_humidity.cpp
#include "ui_humidity.h"
#include "tft_display.h"
#include "io.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>
#include <math.h>

extern TFT_eSPI tft;
extern Reading global_readings;
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
extern void drawHeader(const char* title, uint16_t color);

void draw_humidity_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_STATUS = 2;
    const uint16_t HUMIDITY_COLOR   = TFT_CYAN;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    // Layout idéntico a Temp/DS18 — centro del panel izquierdo y tanque derecho
    const int FIRST_THIRD_CX = tft.width() / 3 + 10;  // 63
    const int VALUE_Y = tft.height() / 2 + 15;          // 79
    const int topY    = VALUE_Y - 24;                    // 55 — top de font 7
    const int BAR_X   = 120;
    const int BAR_Y   = 40;
    const int BAR_W   = 30;
    const int BAR_H   = 80;

    float hum      = global_readings.humidity;
    bool  no_dht_h = isnan(hum);

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_HUM), HUMIDITY_COLOR);
        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);
    }

    // Salida temprana si el valor no cambió
    static int last_hum_drawn = -1;
    int hum_cache = no_dht_h ? -9999 : (int)roundf(hum);
    if (!screen_changed && hum_cache == last_hum_drawn) return;
    last_hum_drawn = hum_cache;

    // --- Dinámicos ---
    if (data_changed || screen_changed) {

        if (no_dht_h) {
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, HUMIDITY_COLOR, 0.0f, 0.0f, 100.0f, 3);
            tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);
            tft.fillRect(0, topY - 2, BAR_X - 1, 52, BACKGROUND_COLOR);
            tft.setTextDatum(TC_DATUM);
            // OLD (sin Latin-1): tft.drawString("---", FIRST_THIRD_CX, topY, 7);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("---", FIRST_THIRD_CX, topY);
            tft.setTextFont(0); // liberar GFXfont
            tft.fillRect(0, topY + 50, BAR_X - 1, tft.height() - (topY + 50), BACKGROUND_COLOR);

        } else {
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, HUMIDITY_COLOR, hum, 0.0f, 100.0f, 3);
            tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);

            // Número + "%" — entero en FONT_VALUE, símbolo en FONT_BODY, mismo topY
            // OLD: int intW = tft.textWidth(humStr, 7); int unitW = tft.textWidth("%", 4);
            char humStr[6];
            snprintf(humStr, sizeof(humStr), "%.0f", hum);
            const char* unitStr = "%";
            tft.setFreeFont(FONT_VALUE);
            int intW  = tft.textWidth(humStr);
            tft.setFreeFont(FONT_BODY);
            int unitW = tft.textWidth(unitStr);
            int startX = FIRST_THIRD_CX - (intW + unitW) / 2;
            tft.fillRect(0, topY - 2, BAR_X - 1, 52, BACKGROUND_COLOR);
            tft.setTextDatum(TL_DATUM);
            // OLD: tft.drawString(humStr, startX, topY, 7);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
            tft.drawString(humStr, startX, topY);
            // OLD: tft.drawString(unitStr, startX + intW, topY, 4);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(HUMIDITY_COLOR, BACKGROUND_COLOR);
            tft.drawString(unitStr, startX + intW, topY);
            tft.setTextFont(0); // liberar GFXfont

            // Estado — centrado en panel izquierdo, debajo del número
            tft.fillRect(0, topY + 50, BAR_X - 1, tft.height() - (topY + 50), BACKGROUND_COLOR);
            const char* statusText;
            uint16_t statusColor;
            if      (hum > 70.0) { statusText = L(ST_MOLD_RISK); statusColor = TFT_RED; }
            else if (hum < 30.0) { statusText = L(ST_TOO_DRY);  statusColor = TFT_ORANGE; }
            else                  { statusText = L(ST_OPTIMAL);  statusColor = TFT_GREEN; }
            // OLD: tft.drawString(statusText, FIRST_THIRD_CX, tft.height() - 14, 2);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(statusColor, BACKGROUND_COLOR);
            tft.drawString(statusText, FIRST_THIRD_CX, tft.height() - 14);
            tft.setTextFont(0); // liberar GFXfont
        }
    }
}
