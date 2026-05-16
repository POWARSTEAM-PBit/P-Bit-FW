#pragma once

#include <stdint.h>

// P-Bit sensor icon library — two size tiers, both centered at (cx, cy).
//
//   Small  pbit_draw_*_icon        ≈ 16×16 px  (s=1)
//   Large  pbit_draw_*_icon_large  ≈ 32×32 px  (s=2)
//
// Geometry scales exactly ×2 between tiers.
// The 3-argument small variants are compatible with SensorIconDrawFn pointers.

void pbit_draw_temp_icon        (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon       (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon    (int cx, int cy, uint16_t color);
void pbit_draw_light_icon       (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon       (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon       (int cx, int cy, uint16_t color);

void pbit_draw_temp_icon_large      (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon_large     (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon_large  (int cx, int cy, uint16_t color);
void pbit_draw_light_icon_large     (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon_large     (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon_large     (int cx, int cy, uint16_t color);

void pbit_draw_temp_icon_xl         (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon_xl     (int cx, int cy, uint16_t color);
void pbit_draw_light_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon_xl        (int cx, int cy, uint16_t color);
