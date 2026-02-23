// ui_temp.cpp
#include "ui_temp.h"
#include "tft_display.h"
#include "io.h"
#include "languages.h"
#include <TFT_eSPI.h>
#include <Arduino.h>     
#include <stdio.h>       // Necesario para snprintf()

extern TFT_eSPI tft;
extern uint16_t getTempColor(float temp);
extern bool g_is_fahrenheit;
extern Reading global_readings;
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
extern void drawHeader(const char* title, uint16_t color);

// --- FUNCIÓN DE DIBUJO DE PANTALLA PRINCIPAL (UI_TEMP) ---
/**
 * @brief Dibuja la pantalla de Temperatura, implementando Delta Redraw y snprintf.
 */
void draw_temp_screen(bool screen_changed, bool data_changed) { // 🟢 IMPLEMENTACIÓN CON ARGUMENTOS
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    const int FONT_UNIT = 2;
    const int FONT_VALUE = 7;
    const uint16_t COLOR_TITLE = TFT_RED;
    
    // Coordenadas y variables
    float temp_c  = global_readings.temperature;
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

    // --- Definiciones de Widget ---
    const int FIRST_THIRD_CX = tft.width() / 3 + 10;
    const int VALUE_Y = tft.height() / 2 + 15; 
    const int UNIT_Y = VALUE_Y + 20; 
    const int BAR_X = tft.width() - 40; 
    const int BAR_Y = 40;
    const int BAR_W = 30;
    const int BAR_H = 80;
    
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
        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);
    }

    // 🟢 FASE IVb: ACTUALIZACIÓN DINÁMICA - Si hay cambio de datos O cambio de pantalla
    if (data_changed || screen_changed) {

        // Zona del número — limpiar siempre antes de dibujar
        tft.fillRect(0, VALUE_Y - 26, BAR_X - 2, 52, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);

        if (no_dht) {
            // Sin sensor: "---" en gris + tanque vacío
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("---", FIRST_THIRD_CX, VALUE_Y, FONT_VALUE);
            tft.fillRect(0, UNIT_Y + 13, BAR_X - 2, 18, TFT_BLACK);
            tft.fillRect(BAR_X + 1, BAR_Y + 1, BAR_W - 2, BAR_H - 2, TFT_BLACK);
        } else {
            // 3. Instrucción C/F
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString(instruction_text, FIRST_THIRD_CX, 40, 1);

            // 4. Valor numérico — entero con font 7, decimal con font 4
            snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display);
            tft.setTextColor(tempColor, TFT_BLACK);
            {
                int dot = 0;
                while (tempStr[dot] && tempStr[dot] != '.') dot++;
                char intStr[6];
                for (int i = 0; i < dot; i++) intStr[i] = tempStr[i];
                intStr[dot] = '\0';
                const char* decStr = tempStr + dot;
                int intW   = tft.textWidth(intStr, FONT_VALUE);
                int decW   = tft.textWidth(decStr, 4);
                int startX = FIRST_THIRD_CX - (intW + decW) / 2;
                int topY   = VALUE_Y - 24;
                tft.setTextDatum(TL_DATUM);
                tft.drawString(intStr, startX, topY, FONT_VALUE);
                tft.drawString(decStr, startX + intW, topY, 4);
            }

            // 5. Unidad (Celsius / Fahrenheit)
            tft.fillRect(0, UNIT_Y + 13, BAR_X - 2, 18, TFT_BLACK);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
            tft.drawString(unit_text, FIRST_THIRD_CX, UNIT_Y+15, FONT_UNIT);

            // 6. Tanque de temperatura
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, tempColor, temp_c, 0, 50);
        }
    }
}