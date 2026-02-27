// ui_ds18.cpp
// Pantalla de sonda de temperatura externa DS18B20 (1-Wire, puerto J4/GPIO33).

#include "ui_ds18.h"
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
extern bool g_is_fahrenheit;
extern uint16_t getTempColor(float temp);

// =============================================================
// DS18B20_SCREEN — Sonda externa 1-Wire, diseño espejo de TEMP_SCREEN
// =============================================================
void draw_ds18_screen(bool screen_changed, bool data_changed) {

    // OLD (sin Latin-1): const int FONT_VALUE = 7; const int FONT_UNIT = 2;
    const uint16_t TITLE_COLOR      = TFT_ORANGE;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float temp_c = global_readings.temp_ds18b20;
    bool  no_sensor = (temp_c < -100.0f);

    // Layout — idéntico a TEMP_SCREEN
    const int FIRST_THIRD_CX = tft.width() / 3 + 10;
    const int VALUE_Y         = tft.height() / 2 + 15;
    const int UNIT_Y          = VALUE_Y + 20;
    const int BAR_X           = tft.width() - 40;
    const int BAR_Y           = 40;
    const int BAR_W           = 30;
    const int BAR_H           = 80;

    // --- Estáticos ---
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader(L(TIT_THERM), TITLE_COLOR);

        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);

        // Marca de 0°C — tick exterior + "0" a la derecha del tanque (estático)
        // Mismo mapeado que drawFillTank: fill = (0-(-55))/(125-(-55)) * (BAR_H-2) = 23px
        {
            const int inner_h   = BAR_H - 2;                          // 78px
            const int zero_fill = (int)(55.0f / 180.0f * inner_h);   // 23px desde abajo
            const int zero_y    = BAR_Y + 1 + (inner_h - zero_fill); // y=96
            tft.drawFastHLine(BAR_X + BAR_W + 1, zero_y, 2, TFT_WHITE);
            tft.setTextDatum(TL_DATUM);
            tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
            // OLD (sin Latin-1): tft.drawString("0", BAR_X + BAR_W + 3, zero_y - 4, 1);
            tft.setFreeFont(FONT_SMALL);
            tft.drawString("0", BAR_X + BAR_W + 3, zero_y - 4);
            tft.setTextFont(0); // liberar GFXfont
        }
    }

    // Salida temprana si el valor no cambió — evita fillRects innecesarios que causan flickering.
    // Cache basado en el valor mostrado × 10 (maneja toggle C/F y estado sin sensor).
    static int last_ds18_drawn = -9999;
    {
        int cache = no_sensor ? -9999 : (int)roundf((g_is_fahrenheit ? temp_c * 1.8f + 32.0f : temp_c) * 10.0f);
        if (!screen_changed && cache == last_ds18_drawn) return;
        last_ds18_drawn = cache;
    }

    // --- Dinámicos ---
    if (data_changed || screen_changed) {

        if (no_sensor) {
            // Sensor desconectado — aviso centrado.
            // El fillRect arranca en y=33 (justo bajo la línea del header) para limpiar también
            // la instrucción "Push > C/F" (y=40) que pudo quedar de cuando el sensor estaba conectado.
            tft.fillRect(0, 33, BAR_X - 2, 102, BACKGROUND_COLOR);
            tft.setTextDatum(MC_DATUM);
            // OLD (sin Latin-1): tft.drawString(L(ST_NO_SENSOR), ..., 2);
            tft.setFreeFont(FONT_BODY);
            tft.setTextColor(TFT_RED, BACKGROUND_COLOR);
            tft.drawString(L(ST_NO_SENSOR), FIRST_THIRD_CX, VALUE_Y - 5);
            // OLD (sin Latin-1): tft.drawString("Check J4 (GPIO33)", ..., 1);
            tft.setFreeFont(FONT_SMALL);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("Check J4 (GPIO33)", FIRST_THIRD_CX, VALUE_Y + 12);
            tft.setTextFont(0); // liberar GFXfont
            // Tanque vacío
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, TFT_DARKGREY, 0.0f, 0.0f, 50.0f);
        } else {
            float temp_display = g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
            // Número en cian cuando es negativo (distinto del azul de 0–15°C)
            uint16_t tempColor = (temp_c < 0.0f) ? TFT_CYAN : getTempColor(temp_c);

            // Instrucción C/F
            // OLD (sin Latin-1): tft.drawString(instruction, FIRST_THIRD_CX, 40, 1);
            const char* instruction = g_is_fahrenheit ? L(INSTR_C) : L(INSTR_F);
            tft.setFreeFont(FONT_SMALL);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString(instruction, FIRST_THIRD_CX, 40);
            tft.setTextFont(0); // liberar GFXfont

            // Valor numérico — entero con FONT_VALUE, decimal con FONT_BODY
            // OLD: int intW = tft.textWidth(intStr, 7); int decW = tft.textWidth(decStr, 4);
            // OLD: tft.drawString(intStr, startX, topY, 7); tft.drawString(decStr, ..., 4);
            char tempStr[8];
            snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display);
            tft.fillRect(0, VALUE_Y - 26, BAR_X - 2, 52, BACKGROUND_COLOR);
            tft.setTextColor(tempColor, BACKGROUND_COLOR);
            {
                int dot = 0;
                while (tempStr[dot] && tempStr[dot] != '.') dot++;
                char intStr[6];
                for (int i = 0; i < dot; i++) intStr[i] = tempStr[i];
                intStr[dot] = '\0';
                const char* decStr = tempStr + dot;
                tft.setFreeFont(FONT_VALUE);
                int intW   = tft.textWidth(intStr);
                tft.setFreeFont(FONT_BODY);
                int decW   = tft.textWidth(decStr);
                int startX = FIRST_THIRD_CX - (intW + decW) / 2;
                int topY   = VALUE_Y - 24;
                tft.setTextDatum(TL_DATUM);
                tft.setFreeFont(FONT_VALUE);
                tft.drawString(intStr, startX, topY);
                tft.setFreeFont(FONT_BODY);
                tft.drawString(decStr, startX + intW, topY);
                tft.setTextFont(0); // liberar GFXfont
            }

            // Unidad (no traducida — constante universal)
            // OLD (sin Latin-1): tft.drawString(unit, FIRST_THIRD_CX, UNIT_Y + 15, 2);
            const char* unit = g_is_fahrenheit ? "Fahrenheit" : "Celsius";
            tft.fillRect(0, UNIT_Y + 13, BAR_X - 2, 18, BACKGROUND_COLOR);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_LIGHTGREY, BACKGROUND_COLOR);
            tft.drawString(unit, FIRST_THIRD_CX, UNIT_Y + 15);
            tft.setTextFont(0); // liberar GFXfont

            // Tanque bicolor + línea interna en 0°C
            // Zona positiva en tempColor, zona fría (-55 a 0°C) siempre en azul.
            // La línea blanca en 0°C se dibuja al final para quedar encima del fill.
            {
                const int inner_h   = BAR_H - 2;                        // 78px
                const int zero_fill = (int)(55.0f / 180.0f * inner_h);  // 23px — zona -55 a 0°C
                const int zero_y    = BAR_Y + 1 + (inner_h - zero_fill);// y=96 en pantalla

                float norm   = constrain((temp_c + 55.0f) / 180.0f, 0.0f, 1.0f);
                int fill_px  = (int)(norm * inner_h);
                int empty_px = inner_h - fill_px;

                // Zona vacía (arriba, negro)
                if (empty_px > 0)
                    tft.fillRect(BAR_X + 1, BAR_Y + 1, BAR_W - 2, empty_px, TFT_BLACK);

                if (temp_c >= 0.0f) {
                    // Zona positiva encima (tempColor), zona fría abajo (siempre azul)
                    int pos_fill = fill_px - zero_fill;
                    if (pos_fill > 0)
                        tft.fillRect(BAR_X + 1, BAR_Y + 1 + empty_px, BAR_W - 2, pos_fill, tempColor);
                    tft.fillRect(BAR_X + 1, zero_y, BAR_W - 2, zero_fill, TFT_BLUE);
                } else {
                    // Solo zona negativa (azul hasta el nivel actual)
                    if (fill_px > 0)
                        tft.fillRect(BAR_X + 1, BAR_Y + 1 + empty_px, BAR_W - 2, fill_px, TFT_BLUE);
                }

                // Línea blanca en 0°C — separador de zonas (siempre visible encima del fill)
                tft.drawFastHLine(BAR_X + 1, zero_y, BAR_W - 2, TFT_WHITE);
            }
        }
    }
}
