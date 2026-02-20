// ui_light.cpp

#include "ui_light.h"
#include "tft_display.h" 
#include "io.h"         
#include "ui_widgets.h" // Necesario para drawHeader y drawBarGraph
#include <TFT_eSPI.h>
#include <Arduino.h>     
#include <stdio.h>       // Necesario para snprintf()

extern TFT_eSPI tft; 
extern Reading global_readings;
// Prototipos de los widgets que usaremos
extern void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
extern void drawHeader(const char* title, uint16_t color);

/**
 * @brief Dibuja la pantalla de Luz (LIGHT_SCREEN).
 * Implementa Delta Redraw (Fase IVb) y snprintf (Fase IVa).
 * 🟢 NUEVO DISEÑO: Usa un gráfico de barras horizontal.
 */
void draw_light_screen(bool screen_changed, bool data_changed) { 
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    const int FONT_VALUE = 7; // Fuente GRANDE para el valor
    const int FONT_STATUS = 2; // Fuente para el pie de página
    const uint16_t LIGHT_COLOR = TFT_YELLOW; 
    const uint16_t BACKGROUND_COLOR = TFT_BLACK;
    
    // Coordenadas y variables
    int cx = tft.width() / 2;
    // (Coordenadas Y ajustadas para 160x128)
    int cy = 78; 
    float lux = global_readings.ldr;
    
    // Buffers estáticos
    char luxStr[8]; 
    char statusText[30]; 

    // --- Posicionamiento del Widget de Barra ---
    const int BAR_W = tft.width() - 40; 
    const int BAR_H = 20;               
    const int BAR_X = 20;               
    const int BAR_Y = cy + 14; // Y=87

    // --- Posicionamiento del Valor Numérico ---
    const int VALUE_Y = cy - 15; // Y=60
    
    // -----------------------------------------------------------
    
    // 🟢 DELTA REDRAW: Dibuja Estáticos solo si la pantalla cambió
    if (screen_changed) {
        tft.fillScreen(BACKGROUND_COLOR); 
        
        // 1. Título (Estático) - Usamos el widget de cabecera
        drawHeader("Light (Lux)", LIGHT_COLOR);
    }
    
    // 🟢 ACTUALIZACIÓN DINÁMICA
    if (data_changed || screen_changed) {
        
        // 1. Valor Numérico (Dinámico)
        snprintf(luxStr, sizeof(luxStr), "%.0f", lux); 
        
        // 🔴 FIX (Problema 3: Ghosting):
        // ELIMINADO: tft.fillRect(20, VALUE_Y - 35, tft.width() - 40, 45, BACKGROUND_COLOR); 
        // La limpieza ahora la hace drawString con el color de fondo.
        
        tft.setTextDatum(MC_DATUM); 
        // 🟢 FIX: Se añade BACKGROUND_COLOR para limpiar el "fantasma"
        tft.setTextColor(TFT_WHITE, BACKGROUND_COLOR);
        tft.drawString(luxStr, cx, VALUE_Y, FONT_VALUE); 
        
        // 2. Relleno del Widget (Dinámico)
        drawBarGraph(BAR_X, BAR_Y, BAR_W, BAR_H, LIGHT_COLOR, lux, 0, 1000);

        // 3. Mensaje de Estado (Pie de página - Dinámico)
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(LIGHT_COLOR, BACKGROUND_COLOR);
        tft.fillRect(0, 115, tft.width(), 13, BACKGROUND_COLOR); // Limpiar barra inferior

        const char* statusPtr;
        if(lux > 500) statusPtr = "BRIGHT";
        else if (lux < 50) statusPtr = "DARK";
        else statusPtr = "AMBIENT";
        
        tft.drawString(statusPtr, cx, 115, FONT_STATUS);
    }
}