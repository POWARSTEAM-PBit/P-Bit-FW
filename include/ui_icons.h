#pragma once

#include <stdint.h>

// Small reusable icon set for the P-Bit TFT UI.
// All functions draw around a visual center (cx, cy).

void pbit_draw_temp_icon(int cx, int cy, uint16_t color);
void pbit_draw_probe_icon(int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon(int cx, int cy, uint16_t color);
void pbit_draw_light_icon(int cx, int cy, uint16_t color);
void pbit_draw_sound_icon(int cx, int cy, uint16_t color);
void pbit_draw_plant_icon(int cx, int cy, uint16_t color);
