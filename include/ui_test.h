// ui_test.h
#pragma once
#include "tft_display.h" 
#include "io.h" // Necesario para la estructura Reading

// 🟢 MODIFICACIÓN: La función ahora acepta banderas de Delta Redraw
void draw_test_screen(bool screen_changed, bool data_changed);