#pragma once
#include <TFT_eSPI.h>

// 🟢 FIX: Declaración externa del objeto TFT global
extern TFT_eSPI tft;

// --- Prototipos de Widgets ---
uint16_t getTempColor(float temp);
void drawCard(int x, int y, int w, int h, uint16_t color);
void drawHeader(const char* title, uint16_t color);
void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal);
void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time);