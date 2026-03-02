// ui_widgets.cpp
// Implementación de todos los widgets gráficos reutilizables.

#include "ui_widgets.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"

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
    // OLD (FreeSans9pt7b — sustituida por Inter SemiBold con mejor diseño):
    // tft.setFreeFont(&FreeSans9pt7b);
    // tft.setTextFont(2);  // restauración antigua
    tft.setFreeFont(FONT_HEADER); // Inter SemiBold 18pt — Latin-1 completo
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(title, cx, L_HEADER_Y);
    tft.setTextFont(0);  // liberar GFXfont
    tft.drawFastHLine(L_MARGIN_SIDE, L_HEADER_LINE, tft.width() - (L_MARGIN_SIDE * 2), color);
}

// Dibuja str usando bigFont si cabe en maxW px; si no, usa smallFont.
// Respeta el setTextDatum() previo del llamador.
void drawStringFit(const char* str, int x, int y, uint8_t bigFont, uint8_t smallFont, int maxW) {
    uint8_t font = (tft.textWidth(str, bigFont) <= maxW) ? bigFont : smallFont;
    tft.drawString(str, x, y, font);
}

void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal) {
    tft.drawRoundRect(x, y, w, h, 3, TFT_DARKGREY);
    value = constrain(value, minVal, maxVal); 
    int fill_w = map(value, minVal, maxVal, 0, w - 4);
    tft.fillRoundRect(x + 2, y + 2, fill_w, h - 4, 2, color);
    tft.fillRect(x + 2 + fill_w, y + 2, (w - 4) - fill_w, h - 4, TFT_BLACK);
}

void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius) {
    float normalized = constrain((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    int fill_height  = (int)(normalized * (h - 2));
    int empty_height = (h - 2) - fill_height;

    // 1. Zona vacía (arriba) — negro
    if (empty_height > 0)
        tft.fillRect(x + 1, y + 1, w - 2, empty_height, TFT_BLACK);
    // 2. Zona llena (abajo) — color, con esquinas redondeadas si radius > 0
    if (fill_height > 0) {
        int r = radius;
        if (r > fill_height / 2) r = fill_height / 2;
        if (r > (w - 2) / 2)    r = (w - 2) / 2;
        if (r > 0)
            tft.fillRoundRect(x + 1, y + 1 + empty_height, w - 2, fill_height, r, fixedColor);
        else
            tft.fillRect(x + 1, y + 1 + empty_height, w - 2, fill_height, fixedColor);
    }
}

/**
 * @brief Función auxiliar para dibujar y limpiar el contenido dinámico de la tarjeta del Temporizador.
 */
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time) {
    
    // 1. Limpieza Localizada para evitar el corte
    tft.fillRect(10, LT_CARD_Y - 2, tft.width() - 20, LT_CARD_H + 4, TFT_BLACK);

    // 2. Dibujar el Marco y el fondo interior
    tft.fillRect(LT_CARD_X + 3, LT_CARD_Y + 3, LT_CARD_W - 6, LT_CARD_H - 6, TFT_BLACK);
    drawCard(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, borderColor);
    
    // 3. Dibujar Estado
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(stateText, cx, LT_STATE_Y);

    // 4. Dibujar Tiempo
    tft.setFreeFont(FONT_TIMER);
    tft.setTextColor(newColor, TFT_BLACK);
    tft.drawString(time, cx, LT_TIME_Y);
    tft.setTextFont(0); // liberar GFXfont
}
