// ui_test.cpp
#include "ui_test.h"
#include "tft_display.h"
#include "io.h"                 
#include "img_dashboard_fondo.h" // 🟢 RESTAURADO: Fondo de imagen
#include "img_termometer_34.h"   // Ícono de termómetro
#include "ui_widgets.h"          // Para tft, getTempColor
#include <TFT_eSPI.h>
#include <Arduino.h>     
#include <stdio.h>               // Para snprintf

extern TFT_eSPI tft;
extern uint16_t getTempColor(float temp); 
extern bool g_is_fahrenheit;
extern Reading global_readings; 

/**
 * @brief Dibuja la pantalla de Prueba (Dashboard con fondo de imagen).
 * Implementa Delta Redraw (Fase IVb) y snprintf (Fase IVa).
 */
void draw_test_screen(bool screen_changed, bool data_changed) { // 🟢 Implementación con argumentos
    
    // --- CONSTANTES DE CONFIGURACIÓN DE UI ---
    const int FONT_MAIN_TITLE = 4; // Para títulos
    const int FONT_VALUE = 4;      // Dato Principal
    const int FONT_UNIT_LABEL = 1; // Etiqueta Celcius/Fahrenheit
    const int FONT_UNIT = 4;       // Símbolo °C / °F
    const int FONT_INSTRUCTION = 2;
    const uint16_t NO_TRANSPARENCY_COLOR = 0x0000; // Sin transparencia

    // Coordenadas y variables
    float temp_c = global_readings.temperature; // Usamos la temp. principal (DHT11)
    float temp_display = temp_c; 
    
    // 🟢 FASE IVa: Buffers estáticos
    char unit_symbol[4];   
    char unit_label[10];   
    char instruction_text[12]; 
    char tempStr[8]; 
    
    // LÓGICA DE CONVERSIÓN
    if (g_is_fahrenheit) {
        temp_display = temp_c * 1.8 + 32;
        snprintf(unit_symbol, sizeof(unit_symbol), "°F");
        snprintf(unit_label, sizeof(unit_label), "Fahrenheit");
        snprintf(instruction_text, sizeof(instruction_text), "Push for C");
    } else {
        snprintf(unit_symbol, sizeof(unit_symbol), "°C");
        snprintf(unit_label, sizeof(unit_label), "Celcius");
        snprintf(instruction_text, sizeof(instruction_text), "Push for F");
    }

    uint16_t tempColor = getTempColor(temp_c); 

    // --- Definiciones de Widget ---
    const int FONT_MAIN_TITLE_Y = 25;
    const int VALUE_CX = tft.width() / 3 + 28; 
    const int VALUE_Y = 85; 
    const int SYMBOL_X = 115;
    const int SYMBOL_Y = 93; 
    const int LABEL_CX = 135; 
    const int LABEL_Y = 107;
    const int ICON_CX = 135; 
    const int ICON_CY = 72; 
    const int ICON_W = TEMP_M_34_WIDTH; 
    const int ICON_H = TEMP_M_34_HEIGHT;
    
    // -----------------------------------------------------------
    
    // 🟢 FASE IVb: DELTA REDRAW - Dibuja Estático solo si la pantalla cambió
    if (screen_changed) {
        // 1. DIBUJAR EL FONDO DEL DASHBOARD (solo una vez)
        tft.pushImage(0, 0, IMG_DASHBOARD_FONDO_WIDTH, IMG_DASHBOARD_FONDO_HEIGHT, img_dashboard_fondo);

        // 3. TÍTULO PRINCIPAL (Estático)
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_BLUE); 
        tft.drawString("Temperature", tft.width() / 2, FONT_MAIN_TITLE_Y, FONT_MAIN_TITLE); 
        
        // 7. 🌡️ ÍCONO DE TERMÓMETRO (Estático)
        int icon_start_x = ICON_CX - ICON_W / 2;
        int icon_start_y = ICON_CY - ICON_H / 2;
        tft.pushImage(icon_start_x, icon_start_y, ICON_W, ICON_H, temp_M_34_data, NO_TRANSPARENCY_COLOR);
    }

    // 🟢 FASE IVb: ACTUALIZACIÓN DINÁMICA - Si hay cambio de datos O cambio de pantalla
    if (data_changed || screen_changed) {
        
        // 2. INSTRUCCIÓN DINÁMICA (Puede cambiar al alternar C/F)
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_DARKGREY); 
        tft.drawString(instruction_text, tft.width() / 3 + 3, 45, FONT_INSTRUCTION); 
        
        // 4. Valor Principal (Dato GRANDE)
        tft.setTextDatum(MR_DATUM); 
        tft.setTextColor(TFT_WHITE); 
        
        // 🔴 Borra solo el área del dato sobre el recuadro AZUL
        tft.fillRect(20, 56, 90, 60, TFT_BLUE); 
        
        // 🟢 Formatear valor (Fase IVa)
        snprintf(tempStr, sizeof(tempStr), "%.1f", temp_display); 
        tft.drawString(tempStr, VALUE_CX, VALUE_Y, FONT_VALUE); 
        
        // 5. Símbolo de Unidad (°C / °F)
        tft.setTextDatum(TL_DATUM); 
        tft.setTextColor(TFT_WHITE); 
        tft.drawString(unit_symbol, SYMBOL_X, SYMBOL_Y, FONT_UNIT);

        // 6. Etiqueta de Unidad (Celcius/Fahrenheit)
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_DARKGREY); 
        tft.drawString(unit_label, LABEL_CX, LABEL_Y, FONT_UNIT_LABEL);
    }
}