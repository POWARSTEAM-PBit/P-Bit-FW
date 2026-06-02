#pragma once

#include <stdint.h>

// P-Bit sensor icon library — centered at (cx, cy).
//
//   Small   pbit_draw_*_icon        ≈ 14×14 px  (s=1)
//   XL      pbit_draw_*_icon_xl     ≈ 42×42 px  (s=3)
//   XXL     pbit_draw_*_icon_xxl    ≈ 42×42 px  (s=3, optional accent)
//
// The 3-argument small variants are compatible with SensorIconDrawFn pointers.

void pbit_draw_temp_icon        (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon       (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon    (int cx, int cy, uint16_t color);
void pbit_draw_light_icon       (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon       (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon       (int cx, int cy, uint16_t color);

void pbit_draw_temp_icon_xl         (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon_xl     (int cx, int cy, uint16_t color);
void pbit_draw_light_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon_xl        (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon_xl        (int cx, int cy, uint16_t color);

void pbit_draw_temp_icon_xxl        (int cx, int cy, uint16_t color);
void pbit_draw_probe_icon_xxl       (int cx, int cy, uint16_t color);
void pbit_draw_humidity_icon_xxl    (int cx, int cy, uint16_t color);
void pbit_draw_light_icon_xxl       (int cx, int cy, uint16_t color);
void pbit_draw_sound_icon_xxl       (int cx, int cy, uint16_t color);
void pbit_draw_plant_icon_xxl       (int cx, int cy, uint16_t color);
void pbit_draw_temp_icon_xxl        (int cx, int cy, uint16_t color, uint16_t accent);
void pbit_draw_probe_icon_xxl       (int cx, int cy, uint16_t color, uint16_t accent);
void pbit_draw_humidity_icon_xxl    (int cx, int cy, uint16_t color, uint16_t accent);
void pbit_draw_light_icon_xxl       (int cx, int cy, uint16_t color, uint16_t accent);
void pbit_draw_sound_icon_xxl       (int cx, int cy, uint16_t color, uint16_t accent);
void pbit_draw_plant_icon_xxl       (int cx, int cy, uint16_t color, uint16_t accent);

void pbit_draw_bluetooth_icon   (int cx, int cy, uint16_t color);
void pbit_draw_bluetooth_icon_xl(int cx, int cy, uint16_t color);
