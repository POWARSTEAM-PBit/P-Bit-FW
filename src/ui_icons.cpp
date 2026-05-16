#include "ui_icons.h"

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// ---------------------------------------------------------------------------
// Internal helpers — s=1 → small (~14×14 px), s=2 → large (~28×28 px).
// Every icon fits within ±7·s px from (cx, cy).
// ---------------------------------------------------------------------------

static void impl_temp(int cx, int cy, uint16_t c, int s) {
    // Bounds: left=cx-4s, right=cx+7s (ticks), top=cy-7s, bottom=cy+7s → 11s×14s
    tft.fillRoundRect(cx - 2*s, cy - 7*s, 5*s, 10*s, 2*s, c);  // tube
    tft.fillCircle(cx, cy + 3*s, 4*s, c);                        // bulb
    tft.fillRect(cx - s, cy - 5*s, 2*s, 7*s, TFT_BLACK);        // inner channel
    tft.drawFastHLine(cx + 3*s, cy - 5*s, 4*s, c);              // tick high
    tft.drawFastHLine(cx + 3*s, cy - 2*s, 4*s, c);              // tick mid
}

static void impl_probe(int cx, int cy, uint16_t c, int s) {
    tft.fillRect(cx - s, cy - 8*s, 2*s, 5*s, c);                // cable
    tft.fillRoundRect(cx - 3*s, cy - 3*s, 6*s, 11*s, 3*s, c);  // DS18B20 capsule
}

static void impl_humidity(int cx, int cy, uint16_t c, int s) {
    tft.fillCircle(cx, cy + 2*s, 5*s, c);
    // Base width ±3s is tangent to the circle at cy-2s (sqrt(5²-4²)=3), giving a smooth teardrop.
    tft.fillTriangle(cx, cy - 7*s, cx - 3*s, cy - 2*s, cx + 3*s, cy - 2*s, c);
}

static void impl_light(int cx, int cy, uint16_t c, int s) {
    tft.fillCircle(cx, cy, 3*s, c);                                            // core
    tft.drawFastHLine(cx - 7*s, cy, 2*s, c);                                  // W ray
    tft.drawFastHLine(cx + 5*s, cy, 2*s, c);                                  // E ray
    tft.drawFastVLine(cx, cy - 7*s, 2*s, c);                                  // N ray
    tft.drawFastVLine(cx, cy + 5*s, 2*s, c);                                  // S ray
    tft.drawLine(cx - 4*s, cy - 4*s, cx - 6*s, cy - 6*s, c);                // NW ray
    tft.drawLine(cx + 4*s, cy - 4*s, cx + 6*s, cy - 6*s, c);                // NE ray
    tft.drawLine(cx - 4*s, cy + 4*s, cx - 6*s, cy + 6*s, c);                // SW ray
    tft.drawLine(cx + 4*s, cy + 4*s, cx + 6*s, cy + 6*s, c);                // SE ray
}

static void impl_sound(int cx, int cy, uint16_t c, int s) {
    // Bounds: ±6s wide (base), ±7s tall → 12s×14s
    tft.fillRoundRect(cx - 5*s, cy - 7*s, 10*s, 10*s, 4*s, c);  // mic capsule
    tft.fillRect(cx - s, cy + 3*s, 2*s, 2*s, c);                  // neck
    tft.fillRoundRect(cx - 6*s, cy + 5*s, 12*s, 2*s, s, c);      // base
}

static void impl_plant(int cx, int cy, uint16_t c, int s) {
    tft.fillRect(cx - s, cy - 3*s, 2*s, 8*s, c);               // stem
    tft.fillRoundRect(cx - 7*s, cy - 3*s, 8*s, 4*s, 2*s, c);  // left leaf
    tft.fillRoundRect(cx - s,   cy - 7*s, 8*s, 4*s, 2*s, c);  // right leaf (upper)
    tft.fillRoundRect(cx - 4*s, cy + 4*s, 8*s, 3*s, s, c);    // pot / base
}

// ---------------------------------------------------------------------------
// Public small API — s=1, ~14×14 px, compatible with SensorIconDrawFn.
// ---------------------------------------------------------------------------

void pbit_draw_temp_icon    (int cx, int cy, uint16_t c) { impl_temp    (cx, cy, c, 1); }
void pbit_draw_probe_icon   (int cx, int cy, uint16_t c) { impl_probe   (cx, cy, c, 1); }
void pbit_draw_humidity_icon(int cx, int cy, uint16_t c) { impl_humidity(cx, cy, c, 1); }
void pbit_draw_light_icon   (int cx, int cy, uint16_t c) { impl_light   (cx, cy, c, 1); }
void pbit_draw_sound_icon   (int cx, int cy, uint16_t c) { impl_sound   (cx, cy, c, 1); }
void pbit_draw_plant_icon   (int cx, int cy, uint16_t c) { impl_plant   (cx, cy, c, 1); }

// ---------------------------------------------------------------------------
// Public large API — s=2, ~28×28 px.
// ---------------------------------------------------------------------------

void pbit_draw_temp_icon_large    (int cx, int cy, uint16_t c) { impl_temp    (cx, cy, c, 2); }
void pbit_draw_probe_icon_large   (int cx, int cy, uint16_t c) { impl_probe   (cx, cy, c, 2); }
void pbit_draw_humidity_icon_large(int cx, int cy, uint16_t c) { impl_humidity(cx, cy, c, 2); }
void pbit_draw_light_icon_large   (int cx, int cy, uint16_t c) { impl_light   (cx, cy, c, 2); }
void pbit_draw_sound_icon_large   (int cx, int cy, uint16_t c) { impl_sound   (cx, cy, c, 2); }
void pbit_draw_plant_icon_large   (int cx, int cy, uint16_t c) { impl_plant   (cx, cy, c, 2); }

// ---------------------------------------------------------------------------
// Public XL API — s=3, ~42×42 px. Used for gauge screen center icon.
// ---------------------------------------------------------------------------

void pbit_draw_temp_icon_xl    (int cx, int cy, uint16_t c) { impl_temp    (cx, cy, c, 3); }
void pbit_draw_probe_icon_xl   (int cx, int cy, uint16_t c) { impl_probe   (cx, cy, c, 3); }
void pbit_draw_humidity_icon_xl(int cx, int cy, uint16_t c) { impl_humidity(cx, cy, c, 3); }
void pbit_draw_light_icon_xl   (int cx, int cy, uint16_t c) { impl_light   (cx, cy, c, 3); }
void pbit_draw_sound_icon_xl   (int cx, int cy, uint16_t c) { impl_sound   (cx, cy, c, 3); }
void pbit_draw_plant_icon_xl   (int cx, int cy, uint16_t c) { impl_plant   (cx, cy, c, 3); }
