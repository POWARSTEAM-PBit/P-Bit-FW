// ui_temp.cpp
#include "ui_temp.h"
#include "tft_display.h" 
#include "io.h"          
#include <TFT_eSPI.h>
#include <Arduino.h>     
#include <stdio.h>       // Necesario para snprintf()

extern TFT_eSPI tft; 
extern uint16_t getTempColor(float temp); 
extern bool g_is_fahrenheit;
extern Reading global_readings; 
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal); 

// --- FUNCIÓN DE DIBUJO DE PANTALLA PRINCIPAL (UI_TEMP) ---
/**
 * @brief Dibuja la pantalla de Temperatura, implementando Delta Redraw y snprintf.
 */
void draw_temp_screen(bool screen_changed, bool data_changed) { // 🟢 IMPLEMENTACIÓN CON ARGUMENTOS
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    const int FONT_TITLE = 4;   
    const int FONT_UNIT = 2;    
    const int FONT_VALUE = 7;   
    const uint16_t COLOR_TITLE = TFT_ORANGE; 
    
    // Coordenadas y variables
    float temp_c = global_readings.temperature; 
    float temp_display = temp_c; 
    
    // 🟢 FASE IVa: Buffers estáticos (Reemplaza String)
    char unit_text[10];
    char instruction_text[12];
    char tempStr[8]; 
    
    // LÓGICA DE CONVERSIÓN
    if (g_is_fahrenheit) {
        temp_display = temp_c * 1.8 + 32;
        snprintf(unit_text, sizeof(unit_text), "Farenheit");
        snprintf(instruction_text, sizeof(instruction_text), "Push for Celcius");
    } else {
        snprintf(unit_text, sizeof(unit_text), "Celcius");
        snprintf(instruction_text, sizeof(instruction_text), "Push for Farenheit");
    }

    uint16_t tempColor = getTempColor(temp_c); 

    // --- Definiciones de Widget ---
    const int FIRST_THIRD_CX = tft.width() / 3 + 10;
    const int VALUE_Y = tft.height() / 2 + 15; 
    const int UNIT_Y = VALUE_Y + 20; 
    const int BAR_X = tft.width() - 40; 
    const int BAR_Y = 40;
    const int BAR_W = 30;
    const int BAR_H = 80;
    
    // -----------------------------------------------------------
    
    // 🟢 FASE IVb: DELTA REDRAW - Dibuja Estáticos solo si la pantalla cambió
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK); 
        
        // 2. Título: "Temperature" (Estático)
        int cx = tft.width() / 2;
        tft.setTextDatum(TC_DATUM); 
        tft.setTextColor(COLOR_TITLE, TFT_BLACK); 
        tft.drawString("Temperature", cx, 8, FONT_TITLE); 
        
        // Dibuja el contorno del Tanque (Estático)
        tft.drawRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, 3, TFT_DARKGREY); 
    }

    // 🟢 FASE IVb: ACTUALIZACIÓN DINÁMICA - Si hay cambio de datos O cambio de pantalla
    if (data_changed || screen_changed) {
        
        // 1. Área del Dato Principal - Limpieza localizada antes de escribir
        tft.fillRect(FIRST_THIRD_CX - 50, 30, 100, 100, TFT_BLACK); 
        
        // 3. Instrucción Dinámica
        tft.setTextDatum(TC_DATUM); 
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
        tft.drawString(instruction_text, FIRST_THIRD_CX, 40, 1); 


        // 4. Valor Numérico GRANDE
        tft.setTextDatum(MC_DATUM); 
        tft.setTextColor(tempColor, TFT_BLACK);

        // 🟢 FASE IVa: Formatear valor (snprintf)
        snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display); 
        tft.drawString(tempStr, FIRST_THIRD_CX, VALUE_Y, FONT_VALUE); 
        
        // 5. Unidad (Celcius / Farenheit)
        tft.setTextDatum(TC_DATUM); 
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK); 
        tft.drawString(unit_text, FIRST_THIRD_CX, UNIT_Y+15, FONT_UNIT); 

        // 6. Dibuja el interior del Bar Graph Vertical (Elemento costoso pero necesario)
        drawFillTank(BAR_X, BAR_Y, BAR_W, BAR_H, tempColor, temp_c, 0, 50); 
    }
}