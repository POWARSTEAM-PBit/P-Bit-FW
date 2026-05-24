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
    const int tick_len = (s >= 3) ? s : 4*s;
    tft.drawFastHLine(cx + 3*s, cy - 5*s, tick_len, c);         // tick high
    tft.drawFastHLine(cx + 3*s, cy - 2*s, tick_len, c);         // tick mid
}

static void impl_probe(int cx, int cy, uint16_t c, int s) {
    tft.fillCircle(cx, cy - 5*s, 2*s, c);                      // sensor node
    tft.fillRect(cx, cy - 3*s, 1, 2*s, c);                     // neck
    tft.fillRoundRect(cx - 3*s, cy - s, 6*s, 7*s, s, c);       // compact body
    tft.drawFastHLine(cx - 2*s, cy + 2*s, 4*s, TFT_BLACK);     // connector stripe
    tft.fillRect(cx, cy + 6*s, 1, 2*s, c);                     // straight lead
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
    tft.fillRoundRect(cx - 4*s, cy + 4*s, 8*s, 3*s, s, c);    // pot / base
    tft.fillRect(cx - s, cy - 4*s, 2*s, 8*s, c);              // stem
    tft.fillEllipse(cx - 4*s, cy,     4*s, 2*s, c);           // left leaf
    tft.fillEllipse(cx + 4*s, cy - 2*s, 4*s, 2*s, c);         // right leaf
    tft.fillEllipse(cx,       cy - 5*s, 2*s, 3*s, c);         // top leaf
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
// Public XL API — s=3, ~42×42 px. Used for gauge screen center icon.
// ---------------------------------------------------------------------------

void pbit_draw_temp_icon_xl    (int cx, int cy, uint16_t c) { impl_temp    (cx, cy, c, 3); }
void pbit_draw_probe_icon_xl   (int cx, int cy, uint16_t c) { impl_probe   (cx, cy, c, 3); }
void pbit_draw_humidity_icon_xl(int cx, int cy, uint16_t c) { impl_humidity(cx, cy, c, 3); }
void pbit_draw_light_icon_xl   (int cx, int cy, uint16_t c) { impl_light   (cx, cy, c, 3); }
void pbit_draw_sound_icon_xl   (int cx, int cy, uint16_t c) { impl_sound   (cx, cy, c, 3); }
void pbit_draw_plant_icon_xl   (int cx, int cy, uint16_t c) { impl_plant   (cx, cy, c, 3); }

// ---------------------------------------------------------------------------
// Public XXL API — detailed dial icons, kept compact enough to stay clear of
// the upper-left unit label on the 160×128 gauge screen.
// The four-argument variants add a secondary detail color without breaking the
// existing three-argument API used by cards and older screens.
// ---------------------------------------------------------------------------

static void impl_temp_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    tft.fillRoundRect(cx - 2*s, cy - 7*s, 5*s, 10*s, 2*s, c);
    tft.fillCircle(cx, cy + 3*s, 4*s, c);
    tft.fillRoundRect(cx - s, cy - 5*s, 2*s, 7*s, 1, TFT_WHITE);
    tft.fillCircle(cx, cy + 3*s, 2*s, TFT_WHITE);
    tft.fillCircle(cx - s, cy + 2*s, s, a);
    const int tick_len = (s >= 3) ? s : 4*s;
    tft.drawFastHLine(cx + 3*s, cy - 5*s, tick_len, c);
    tft.drawFastHLine(cx + 3*s, cy - 2*s, tick_len, c);
}

static void impl_probe_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    const uint16_t wire = a;
    const uint16_t stripe = TFT_WHITE;
    tft.fillCircle(cx, cy - 7*s, 3*s, c);
    tft.fillCircle(cx, cy - 7*s, s, TFT_BLACK);
    tft.fillRect(cx - 1, cy - 4*s, 3, 3*s, wire);
    tft.fillRoundRect(cx - 4*s, cy - s, 8*s, 8*s, s, c);
    tft.drawFastHLine(cx - 3*s, cy + 2*s, 6*s, stripe);
    tft.drawFastHLine(cx - 3*s, cy + 3*s, 6*s, TFT_BLACK);
    tft.fillRect(cx - 1, cy + 7*s, 3, 4*s, wire);
}

static void impl_humidity_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    impl_humidity(cx, cy, c, s);
    tft.fillCircle(cx - s, cy + s, 2*s, a);
    tft.fillCircle(cx - 2*s, cy - s, 1*s, TFT_WHITE);
}

static void impl_light_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    tft.fillCircle(cx, cy, 4*s, c);
    tft.fillCircle(cx, cy, 2*s, a);
    tft.drawFastHLine(cx - 8*s, cy, 3*s, a);
    tft.drawFastHLine(cx - 8*s, cy + 1, 3*s, a);
    tft.drawFastHLine(cx + 5*s, cy, 3*s, a);
    tft.drawFastHLine(cx + 5*s, cy + 1, 3*s, a);
    tft.drawFastVLine(cx, cy - 8*s, 3*s, a);
    tft.drawFastVLine(cx + 1, cy - 8*s, 3*s, a);
    tft.drawFastVLine(cx, cy + 5*s, 3*s, a);
    tft.drawFastVLine(cx + 1, cy + 5*s, 3*s, a);
    tft.drawLine(cx - 4*s, cy - 4*s, cx - 6*s, cy - 6*s, a);
    tft.drawLine(cx - 4*s + 1, cy - 4*s, cx - 6*s + 1, cy - 6*s, a);
    tft.drawLine(cx + 4*s, cy - 4*s, cx + 6*s, cy - 6*s, a);
    tft.drawLine(cx + 4*s + 1, cy - 4*s, cx + 6*s + 1, cy - 6*s, a);
    tft.drawLine(cx - 4*s, cy + 4*s, cx - 6*s, cy + 6*s, a);
    tft.drawLine(cx - 4*s + 1, cy + 4*s, cx - 6*s + 1, cy + 6*s, a);
    tft.drawLine(cx + 4*s, cy + 4*s, cx + 6*s, cy + 6*s, a);
    tft.drawLine(cx + 4*s + 1, cy + 4*s, cx + 6*s + 1, cy + 6*s, a);
}

static void impl_sound_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    tft.fillRoundRect(cx - 5*s, cy - 7*s, 10*s, 10*s, 3*s, c);
    tft.fillRoundRect(cx - 2*s - 1, cy - 4*s, 3, 4*s, 1, TFT_WHITE);
    tft.fillRoundRect(cx - 1,       cy - 5*s, 3, 5*s, 1, TFT_WHITE);
    tft.fillRoundRect(cx + 2*s - 1, cy - 4*s, 3, 4*s, 1, TFT_WHITE);
    tft.fillRect(cx - s, cy + 4*s, 2*s, 2*s, a);
    tft.fillRoundRect(cx - 6*s, cy + 6*s, 12*s, 2*s, s, a);
}

static void impl_plant_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    tft.fillRoundRect(cx - 4*s, cy + 3*s, 8*s, 4*s, s, a);
    tft.drawFastHLine(cx - 3*s, cy + 3*s, 6*s, c);
    tft.fillRect(cx - 1, cy - 4*s, 3, 7*s, c);
    tft.fillEllipse(cx - 4*s, cy - 1*s, 4*s, 2*s, c);
    tft.fillEllipse(cx + 4*s, cy - 3*s, 4*s, 2*s, c);
    tft.fillEllipse(cx,       cy - 5*s, 2*s, 4*s, c);
    tft.drawLine(cx - 1, cy - 3*s, cx - 6*s, cy - 1*s, a);
    tft.drawLine(cx + 1, cy - 4*s, cx + 6*s, cy - 3*s, a);
    tft.drawLine(cx,     cy - 4*s, cx,       cy - 7*s, a);
}

void pbit_draw_temp_icon_xxl    (int cx, int cy, uint16_t c, uint16_t a) { impl_temp_detail    (cx, cy, c, a, 3); }
void pbit_draw_probe_icon_xxl   (int cx, int cy, uint16_t c, uint16_t a) { impl_probe_detail   (cx, cy, c, a, 3); }
void pbit_draw_humidity_icon_xxl(int cx, int cy, uint16_t c, uint16_t a) { impl_humidity_detail(cx, cy, c, a, 3); }
void pbit_draw_light_icon_xxl   (int cx, int cy, uint16_t c, uint16_t a) { impl_light_detail   (cx, cy, c, a, 3); }
void pbit_draw_sound_icon_xxl   (int cx, int cy, uint16_t c, uint16_t a) { impl_sound_detail   (cx, cy, c, a, 3); }
void pbit_draw_plant_icon_xxl   (int cx, int cy, uint16_t c, uint16_t a) { impl_plant_detail   (cx, cy, c, a, 3); }

void pbit_draw_temp_icon_xxl    (int cx, int cy, uint16_t c) { pbit_draw_temp_icon_xxl    (cx, cy, c, c); }
void pbit_draw_probe_icon_xxl   (int cx, int cy, uint16_t c) { pbit_draw_probe_icon_xxl   (cx, cy, c, c); }
void pbit_draw_humidity_icon_xxl(int cx, int cy, uint16_t c) { pbit_draw_humidity_icon_xxl(cx, cy, c, c); }
void pbit_draw_light_icon_xxl   (int cx, int cy, uint16_t c) { pbit_draw_light_icon_xxl   (cx, cy, c, c); }
void pbit_draw_sound_icon_xxl   (int cx, int cy, uint16_t c) { pbit_draw_sound_icon_xxl   (cx, cy, c, c); }
void pbit_draw_plant_icon_xxl   (int cx, int cy, uint16_t c) { pbit_draw_plant_icon_xxl   (cx, cy, c, c); }

// ---------------------------------------------------------------------------
// Bluetooth icon — classic ᛒ rune shape: spine + two right-pointing ">" wings
// + short left tips from top/bottom endpoints.
// ---------------------------------------------------------------------------

static void impl_bluetooth(int cx, int cy, uint16_t c, int s) {
    // Spine: 3px wide for s>=2, 1px for s=1.
    tft.drawFastVLine(cx,     cy - 7*s, 14*s + 1, c);
    if (s >= 2) tft.drawFastVLine(cx - 1, cy - 7*s, 14*s + 1, c);
    if (s >= 2) tft.drawFastVLine(cx + 1, cy - 7*s, 14*s + 1, c);

    // Wings: elbows at (cx+4s, cy∓3s).
    //   Outer segments T→E1 and E2→B: 45°  (Δx=4s, Δy=4s).
    //   Inner segments E1→M and M→E2: ~37°  (Δx=4s, Δy=3s — gives proper B arc).
    // For s>=2: 4 parallel lines per segment (d = −1, 0, +1, +2) to match spine weight.
    const int d0 = (s >= 2) ? -1 : 0;
    const int d1 = (s >= 2) ?  2 : 0;
    for (int d = d0; d <= d1; ++d) {
        // Upper wing: T → E1 → M
        tft.drawLine(cx + d,       cy - 7*s, cx + 4*s + d, cy - 3*s, c);
        tft.drawLine(cx + 4*s + d, cy - 3*s, cx + d,       cy,       c);
        // Lower wing: M → E2 → B
        tft.drawLine(cx + d,       cy,       cx + 4*s + d, cy + 3*s, c);
        tft.drawLine(cx + 4*s + d, cy + 3*s, cx + d,       cy + 7*s, c);
        // Tips: 45° from spine endpoints toward center-left
        tft.drawLine(cx + d,       cy - 7*s, cx - 3*s + d, cy - 4*s, c);
        tft.drawLine(cx + d,       cy + 7*s, cx - 3*s + d, cy + 4*s, c);
    }
}

void pbit_draw_bluetooth_icon   (int cx, int cy, uint16_t c) { impl_bluetooth(cx, cy, c, 1); }
void pbit_draw_bluetooth_icon_xl(int cx, int cy, uint16_t c) { impl_bluetooth(cx, cy, c, 3); }
