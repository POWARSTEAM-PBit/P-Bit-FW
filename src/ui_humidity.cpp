// ui_humidity.cpp
#include "ui_humidity.h"
#include "tft_display.h" 
#include "io.h"         
#include <TFT_eSPI.h>
#include <Arduino.h>     
#include <stdio.h>      // Necesario para snprintf()

extern TFT_eSPI tft; 
extern Reading global_readings;
extern void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal); 

void draw_humidity_screen(bool screen_changed, bool data_changed) { 
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    const int FONT_TITLE = 4;   
    const int FONT_STATUS = 2; 
    const int FONT_VALUE = 6;   
    const uint16_t HUMIDITY_COLOR = TFT_CYAN; 
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    
    // Coordenadas y variables
    int cx = tft.width() / 2;
    float hum = global_readings.humidity;
    
    // Buffers
    char humStr[6]; 
    snprintf(humStr, sizeof(humStr), "%.0f", hum); 
    
    // --- Posicionamiento (Tu Diseño) ---
    const int tank_w = 50;
    const int tank_h = 70;
    const int tank_x = 105;
    const int tank_y = 40;
    
    // -----------------------------------------------------------
    
    // 🟢 DELTA REDRAW: Dibuja Estáticos solo si la pantalla cambió
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR); 
        
        // 1. Título: "Humidity" (Estático)
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(HUMIDITY_COLOR, BACKGROUND_COLOR); 
        tft.drawString("Humidity", cx, 8, FONT_TITLE); 
        
        // 2. Contorno del Tanque (Estático)
        tft.drawRoundRect(tank_x, tank_y, tank_w, tank_h, 3, TFT_DARKGREY); 
    }
    
    // 🟢 DELTA REDRAW: ACTUALIZACIÓN DINÁMICA
    if (data_changed || screen_changed) {
        
        // 3. Dibuja el interior del Tanque
        drawFillTank(tank_x, tank_y, tank_w, tank_h, HUMIDITY_COLOR, hum, 0.0, 100.0);

        // --- Dato Principal ---
        const int VALUE_Y = 55; 
        
        // 🔴 FIX FLICKEO: Eliminamos el fillRect que causaba parpadeo.
        // tft.fillRect(tank_x, VALUE_Y, tank_w, 25, BACKGROUND_COLOR); // ELIMINADO
        
        // 4. Valor Numérico GRANDE
        tft.setTextDatum(TC_DATUM); 
        // Usamos fondo negro en setTextColor para sobrescribir limpiamente
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        
        // Tu coordenada original: (tft.width() / 3-5)
        tft.drawString(humStr, (tft.width() / 3 - 5), VALUE_Y, FONT_VALUE); 
        
        // 5. Unidad %
        const int UNIT_Y = VALUE_Y + 30; 
        
        tft.setTextDatum(TC_DATUM); 
        tft.setTextColor(HUMIDITY_COLOR, BACKGROUND_COLOR); 
        // Tu coordenada original: tft.width() / 3+35
        tft.drawString("%", tft.width() / 3 + 35, UNIT_Y, FONT_TITLE);
        
        // 6. Mensaje de Estado (Pie de página - Dinámico)
        // Aquí mantenemos el fillRect porque el largo del texto cambia mucho y puede dejar basura
        tft.fillRect(0, tft.height() - 15, tft.width(), 13, BACKGROUND_COLOR); 
        
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_LIGHTGREY, BACKGROUND_COLOR);

        const char* statusText;
        if(hum > 70.0) statusText = "Wet (Risk of Mold)";
        else if (hum < 30.0) statusText = "Dry (Low)";
        else statusText = "Optimal";
        
        tft.drawString(statusText, cx, tft.height() - 15, FONT_STATUS);
    }
}