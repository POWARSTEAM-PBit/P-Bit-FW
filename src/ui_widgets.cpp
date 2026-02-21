// ui_widgets.cpp
// Implementación de todos los widgets gráficos reutilizables.

#include "ui_widgets.h"

// 🟢 FIX CRÍTICO: Definición global del objeto TFT
// (Esto resuelve el error 'undefined reference to tft')
TFT_eSPI tft = TFT_eSPI();

// --- DEFINICIÓN DE WIDGETS GLOBALES (IMPLEMENTACIÓN) ---

uint16_t getTempColor(float temp) {
    if (temp <= 15.0f) return TFT_BLUE;
    if (temp > 15.0f && temp <= 22.0f) return TFT_CYAN;
    if (temp > 22.0f && temp <= 27.0f) return TFT_GREEN;
    if (temp > 27.0f && temp <= 32.0f) return TFT_ORANGE;
    return TFT_RED;
}

void drawCard(int x, int y, int w, int h, uint16_t color) {
    tft.drawRoundRect(x, y, w, h, 4, color);
}

void drawHeader(const char* title, uint16_t color) {
    int cx = tft.width() / 2;
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(title, cx, 8, 4);
    tft.drawFastHLine(20, 32, tft.width() - 40, color);
}

void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal) {
    tft.drawRoundRect(x, y, w, h, 3, TFT_DARKGREY);
    value = constrain(value, minVal, maxVal); 
    int fill_w = map(value, minVal, maxVal, 0, w - 4);
    tft.fillRoundRect(x + 2, y + 2, fill_w, h - 4, 2, color);
    tft.fillRect(x + 2 + fill_w, y + 2, (w - 4) - fill_w, h - 4, TFT_BLACK);
}

void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal) {
    float normalized = constrain((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    int fill_height  = (int)(normalized * (h - 2));
    int empty_height = (h - 2) - fill_height;

    // Dibuja directamente los dos segmentos sin pre-limpiar todo a negro:
    // 1. Zona vacía (arriba) — negro
    if (empty_height > 0)
        tft.fillRect(x + 1, y + 1, w - 2, empty_height, TFT_BLACK);
    // 2. Zona llena (abajo) — color
    if (fill_height > 0)
        tft.fillRect(x + 1, y + 1 + empty_height, w - 2, fill_height, fixedColor);
}

/**
 * @brief Función auxiliar para dibujar y limpiar el contenido dinámico de la tarjeta del Temporizador.
 */
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time) {
    
    // Dimensiones de la Tarjeta
    const int CARD_X = 15;
    const int CARD_Y = 55;
    const int CARD_W = tft.width() - 30;
    const int CARD_H = 65;
    
    // 1. Limpieza Localizada para evitar el corte
    tft.fillRect(10, 50, tft.width() - 20, 75, TFT_BLACK); 
    
    // 2. Dibujar el Marco y el fondo interior
    tft.fillRect(CARD_X + 3, CARD_Y + 3, CARD_W - 6, CARD_H - 6, TFT_BLACK); 
    drawCard(CARD_X, CARD_Y, CARD_W, CARD_H, borderColor);
    
    // 3. Dibujar Estado
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(stateText, cx, 65, 2);

    // 4. Dibujar Tiempo
    tft.setTextColor(newColor, TFT_BLACK);
    tft.drawString(time, cx, 95, 4); 
}