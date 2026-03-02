// ui_temp.cpp
#include "ui_temp.h"
#include "tft_display.h"
#include "io.h"
#include "languages.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>       // Necesario para snprintf()

extern TFT_eSPI tft;
extern uint16_t getTempColor(float temp);
extern bool g_is_fahrenheit;
extern Reading g_ui_readings_snapshot;
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
extern void drawHeader(const char* title, uint16_t color);

// --- FUNCIÓN DE DIBUJO DE PANTALLA PRINCIPAL (UI_TEMP) ---
/**
 * @brief Dibuja la pantalla de Temperatura, implementando Delta Redraw y snprintf.
 */
void draw_temp_screen(bool screen_changed, bool data_changed) { // 🟢 IMPLEMENTACIÓN CON ARGUMENTOS
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    // OLD (sin Latin-1): const int FONT_UNIT = 2; const int FONT_VALUE = 7;
    const uint16_t COLOR_TITLE = TFT_RED;
    
    // Coordenadas y variables
    float temp_c  = g_ui_readings_snapshot.temperature;
    bool  no_dht  = isnan(temp_c);
    float temp_display = temp_c;
    
    char tempStr[8];

    // LÓGICA DE CONVERSIÓN
    const char* unit_text;
    const char* instruction_text;
    if (g_is_fahrenheit) {
        temp_display = temp_c * 1.8 + 32;
        unit_text        = "Fahrenheit";
        instruction_text = L(INSTR_C);
    } else {
        unit_text        = "Celsius";
        instruction_text = L(INSTR_F);
    }

    uint16_t tempColor = no_dht ? TFT_DARKGREY : getTempColor(temp_c);

    // Coordenadas definidas en layout.h (Familia A)
    
    // -----------------------------------------------------------
    
    // Salida temprana si el valor no cambió — evita fillRects innecesarios que causan flickering.
    static int last_temp_drawn = -1000;
    int temp_cache = no_dht ? -9999 : (int)roundf(temp_display * 10.0f);
    if (!screen_changed && temp_cache == last_temp_drawn) return;
    last_temp_drawn = temp_cache;

    // 🟢 FASE IVb: DELTA REDRAW - Dibuja Estáticos solo si la pantalla cambió
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_TEMP), COLOR_TITLE);

        // Dibuja el contorno del Tanque (Estático)
        tft.drawRoundRect(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, 3, TFT_DARKGREY);
    }

    // 🟢 FASE IVb: ACTUALIZACIÓN DINÁMICA - Si hay cambio de datos O cambio de pantalla
    if (data_changed || screen_changed) {

        // Zona del número — limpiar siempre antes de dibujar
        tft.fillRect(0, LA_VALUE_TOP - 4, LA_TANK_X - 2, 52, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);

        if (no_dht) {
            // Sin sensor: "---" en gris + tanque vacío
            // OLD (sin Latin-1): tft.drawString("---", FIRST_THIRD_CX, VALUE_Y, 7);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("---", LA_LEFT_CX, LA_VALUE_TOP);
            tft.setTextFont(0); // liberar GFXfont
            tft.fillRect(LA_TANK_X + 1, LA_TANK_Y + 1, LA_TANK_W - 2, LA_TANK_H - 2, TFT_BLACK);
        } else {
            // 3. Instrucción C/F — clearRect antes para evitar ghost al cambiar C↔F
            // h=14: cubre y=34..47 y empalma con el fillRect del valor que empieza en y=46
            tft.fillRect(0, LA_HINT_Y, LA_TANK_X - 2, 14, TFT_BLACK);
            tft.setFreeFont(FONT_SMALL);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString(instruction_text, LA_LEFT_CX, LA_HINT_Y);
            tft.setTextFont(0); // liberar GFXfont

            // 4. Valor numérico — entero con FONT_VALUE, decimal con FONT_BODY
            // OLD: int intW = tft.textWidth(intStr, 7); int decW = tft.textWidth(decStr, 4);
            // OLD: tft.drawString(intStr, startX, topY, 7); tft.drawString(decStr, ..., 4);
            snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display);
            tft.setTextColor(tempColor, TFT_BLACK);
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
                int startX = LA_LEFT_CX - (intW + decW) / 2;
                int topY   = LA_VALUE_TOP;
                tft.setTextDatum(TL_DATUM);
                tft.setFreeFont(FONT_VALUE);
                tft.drawString(intStr, startX, topY);
                tft.setFreeFont(FONT_BODY);
                tft.drawString(decStr, startX + intW, topY);
                tft.setTextFont(0); // liberar GFXfont
            }

            // 5. Unidad (Celsius / Fahrenheit)
            // OLD (sin Latin-1): tft.drawString(unit_text, FIRST_THIRD_CX, UNIT_Y+15, 2);
            tft.fillRect(0, LA_CATEGORY_Y - 2, LA_TANK_X - 2, 14, TFT_BLACK);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            tft.drawString(unit_text, LA_LEFT_CX, LA_CATEGORY_Y);
            tft.setTextFont(0); // liberar GFXfont

            // 6. Tanque de temperatura
            drawFillTank(LA_TANK_X, LA_TANK_Y, LA_TANK_W, LA_TANK_H, tempColor, temp_c, 0, 50);
        }
    }
}
