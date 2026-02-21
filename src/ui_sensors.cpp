// ui_sensors.cpp
// Pantallas de sensores adicionales: Sonido, Humedad del Suelo, DS18B20.

#include "ui_sensors.h"
#include "tft_display.h"
#include "io.h"
#include "ui_widgets.h"
#include <TFT_eSPI.h>
#include <Arduino.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading global_readings;
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

// =============================================================
// SOUND_SCREEN — barra horizontal + categoría (igual que LIGHT_SCREEN)
// =============================================================
void draw_sound_screen(bool screen_changed, bool data_changed) {

    const int FONT_VALUE    = 7;
    const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_MAGENTA;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float level = (float)global_readings.mic;

    const char*   categoryText;
    uint16_t      categoryColor;
    if (level < 15.0f) {
        categoryText = "SILENT";    categoryColor = TFT_DARKGREY;
    } else if (level < 40.0f) {
        categoryText = "QUIET";     categoryColor = TFT_GREEN;
    } else if (level < 70.0f) {
        categoryText = "NORMAL";    categoryColor = TFT_YELLOW;
    } else if (level < 88.0f) {
        categoryText = "LOUD";      categoryColor = TFT_ORANGE;
    } else {
        categoryText = "VERY LOUD"; categoryColor = TFT_RED;
    }

    const int cx         = tft.width() / 2;
    const int VALUE_Y    = 57;
    const int BAR_X      = 20;
    const int BAR_Y      = 85;
    const int BAR_W      = tft.width() - 40;
    const int BAR_H      = 18;
    const int CATEGORY_Y = 109;

    char levelStr[5];
    snprintf(levelStr, sizeof(levelStr), "%.0f", level);

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader("Sound Level", TITLE_COLOR);
    }

    if (data_changed || screen_changed) {
        tft.fillRect(0, VALUE_Y - 26, tft.width(), 52, BACKGROUND_COLOR);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.drawString(levelStr, cx, VALUE_Y, FONT_VALUE);

        drawBarGraph(BAR_X, BAR_Y, BAR_W, BAR_H, categoryColor, level, 0.0f, 100.0f);

        tft.fillRect(0, CATEGORY_Y - 9, tft.width(), 18, BACKGROUND_COLOR);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(categoryText, cx, CATEGORY_Y, FONT_CATEGORY);
    }
}

// =============================================================
// SOIL_SCREEN — tanque vertical + valor (igual que HUMIDITY_SCREEN)
// =============================================================
void draw_soil_screen(bool screen_changed, bool data_changed) {

    const int FONT_VALUE    = 6;
    const int FONT_UNIT     = 4;
    const int FONT_CATEGORY = 2;
    const uint16_t TITLE_COLOR      = TFT_ORANGE;
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    float soil = (float)global_readings.soil_humidity;

    const char*   categoryText;
    uint16_t      tankColor;
    uint16_t      categoryColor;
    if (soil < 20.0f) {
        categoryText = "DRY";       tankColor = TFT_ORANGE; categoryColor = TFT_ORANGE;
    } else if (soil < 55.0f) {
        categoryText = "OPTIMAL";   tankColor = TFT_GREEN;  categoryColor = TFT_GREEN;
    } else if (soil < 80.0f) {
        categoryText = "MOIST";     tankColor = TFT_CYAN;   categoryColor = TFT_CYAN;
    } else {
        categoryText = "SATURATED"; tankColor = TFT_BLUE;   categoryColor = TFT_BLUE;
    }

    const int cx     = tft.width() / 2;
    const int tank_w = 50;
    const int tank_h = 70;
    const int tank_x = 105;
    const int tank_y = 40;

    char soilStr[6];
    snprintf(soilStr, sizeof(soilStr), "%.0f", soil);

    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR);
        drawHeader("Soil", TITLE_COLOR);
        tft.drawRoundRect(tank_x, tank_y, tank_w, tank_h, 3, TFT_DARKGREY);
    }

    if (data_changed || screen_changed) {
        drawFillTank(tank_x, tank_y, tank_w, tank_h, tankColor, soil, 0.0f, 100.0f);

        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.drawString(soilStr, tft.width() / 3 - 5, 55, FONT_VALUE);

        tft.setTextColor(TITLE_COLOR, BACKGROUND_COLOR);
        tft.drawString("%", tft.width() / 3 + 35, 85, FONT_UNIT);

        tft.fillRect(0, tft.height() - 16, tft.width(), 13, BACKGROUND_COLOR);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(categoryColor, BACKGROUND_COLOR);
        tft.drawString(categoryText, cx, tft.height() - 15, FONT_CATEGORY);
    }
}

// =============================================================
// DS18B20_SCREEN — Sonda externa 1-Wire, diseño espejo de TEMP_SCREEN
// =============================================================
void draw_ds18_screen(bool screen_changed, bool data_changed) {

    const int   FONT_VALUE  = 7;
    const int   FONT_UNIT   = 2;
    const uint16_t TITLE_COLOR      = 0x07FF; // Cyan-teal para distinguirla de TEMP_SCREEN (naranja)
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;

    extern bool g_is_fahrenheit;
    extern uint16_t getTempColor(float temp);

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

        int cx = tft.width() / 2;
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TITLE_COLOR, BACKGROUND_COLOR);
        tft.drawString("Temp Probe", cx, 8, 4);

        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY);
    }

    // --- Dinámicos ---
    if (data_changed || screen_changed) {

        if (no_sensor) {
            // Sensor desconectado — aviso centrado
            tft.fillRect(0, VALUE_Y - 30, BAR_X - 2, 70, BACKGROUND_COLOR);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_RED, BACKGROUND_COLOR);
            tft.drawString("No sensor", FIRST_THIRD_CX, VALUE_Y - 5, 2);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString("Check J4 (GPIO33)", FIRST_THIRD_CX, VALUE_Y + 12, 1);
            // Tanque vacío
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, TFT_DARKGREY, 0.0f, 0.0f, 50.0f);
        } else {
            float temp_display = g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
            uint16_t tempColor = getTempColor(temp_c);

            // Instrucción C/F
            const char* instruction = g_is_fahrenheit ? "Push > C" : "Push > F";
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_DARKGREY, BACKGROUND_COLOR);
            tft.drawString(instruction, FIRST_THIRD_CX, 40, 1);

            // Valor numérico
            char tempStr[8];
            snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display);
            tft.fillRect(0, VALUE_Y - 26, BAR_X - 2, 52, BACKGROUND_COLOR);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(tempColor, BACKGROUND_COLOR);
            tft.drawString(tempStr, FIRST_THIRD_CX, VALUE_Y, FONT_VALUE);

            // Unidad
            const char* unit = g_is_fahrenheit ? "Fahrenheit" : "Celsius";
            tft.fillRect(0, UNIT_Y + 13, BAR_X - 2, 18, BACKGROUND_COLOR);
            tft.setTextDatum(TC_DATUM);
            tft.setTextColor(TFT_LIGHTGREY, BACKGROUND_COLOR);
            tft.drawString(unit, FIRST_THIRD_CX, UNIT_Y + 15, FONT_UNIT);

            // Tanque termómetro (rango -10 a 60°C para sonda exterior)
            drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, tempColor, temp_c, -10.0f, 60.0f);
        }
    }
}