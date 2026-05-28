#include "ui_icons.h"

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

// ---------------------------------------------------------------------------
// Internal helpers — s=1 → small (~14×14 px), s=2 → large (~28×28 px).
// Every icon fits within ±7·s px from (cx, cy).
// ---------------------------------------------------------------------------

static void impl_temp(int cx, int cy, uint16_t c, int s) {
    // Canvas ±7s. Tubo 4s ancho (paredes 1s, canal 2s), bulbo r=3s en cy+4s.
    // Total: 6s ancho × 14s alto (cy-7s → cy+7s). Llena el canvas como los demás íconos.
    // NOTA: canal usa 0x1082 (bg card). Pendiente fix con parametro bg.
    tft.fillRoundRect(cx - 2*s, cy - 7*s, 4*s, 11*s, s, c);   // tubo: cy-7s → cy+4s
    tft.fillRect     (cx - s,   cy - 6*s, 2*s, 10*s, 0x1082); // canal interior completo
    tft.fillCircle   (cx,       cy + 4*s, 3*s,         c);     // bulbo: r=3s, bottom=cy+7s
    tft.drawFastHLine(cx + 2*s, cy - 5*s, 2*s, c);             // tick alto
    tft.drawFastHLine(cx + 2*s, cy - 2*s, 2*s, c);             // tick medio
}

static void impl_probe(int cx, int cy, uint16_t c, int s) {
    // DS18B20 waterproof probe: cable vertical centrado + collar + cuerpo cilíndrico + punta.
    // Canvas: ±7s. Rango usado: cy-7s..cy+6s.
    //
    // Mapa de píxeles a s=1 (14×14, cx=7, cy=7):
    //   y=0: ·······■·····  cable (top, centrado en cx)
    //   y=1: ·······■·····  cable
    //   y=2: ·······■·····  cable
    //   y=3: ·······■·····  cable llega al collar
    //   y=4: ·····■■■■■··  collar ring (5px ancho)
    //   y=5: ·····■■■■■··  collar (2ª fila)
    //   y=6: ·····■■■■■··  housing tube (cuerpo recto)
    //   y=7: ·····■■■■■··  housing
    //   y=8: ·····■■■■■··  housing
    //   y=9: ·····■■■■■··  housing
    //   y=10:·····■■■■■··  housing / transición
    //   y=11:·····■■■■■··  punta redondeada (centro del círculo)
    //   y=12:······■■■····  punta (narrowing)
    //   y=13:·······■·····  punta tip (extremo redondeado)

    // Cable v6: 2px ancho (cx, cx+1), 3s alto
    tft.drawFastVLine(cx,     cy - 7*s, 3*s, c);
    tft.drawFastVLine(cx + 1, cy - 7*s, 3*s, c);

    // Collar 5s ancho, 2s alto — 1s gap hasta el body (3s)
    tft.fillRect(cx - (5*s)/2, cy - 4*s, 5*s, 2*s, c);

    // Body 3s ancho, 6s alto — mas estrecho y largo que v5; gap 1s bajo collar
    tft.fillRect(cx - (3*s)/2, cy - s, 3*s, 6*s, c);

    // Punta: r=s, centro en cy+5s (borde inferior del body)
    tft.fillCircle(cx, cy + 5*s, s, c);
}

static void impl_humidity(int cx, int cy, uint16_t c, int s) {
    tft.fillCircle(cx, cy + 2*s, 5*s, c);
    // Base width ±3s is tangent to the circle at cy-2s (sqrt(5²-4²)=3), giving a smooth teardrop.
    tft.fillTriangle(cx, cy - 7*s, cx - 3*s, cy - 2*s, cx + 3*s, cy - 2*s, c);
    // Highlight reflejo: punto de brillo en cuadrante superior-izquierdo de la gota.
    // Usa 0x1082 (bg card) — mismo paradigma que el canal de aire del termometro.
    // Unifica el set: todos los iconos tienen UN elemento de estructura interna.
    tft.fillCircle(cx - 2*s, cy - s, s, 0x1082);
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
    // Cápsula más estrecha que alta (8s×10s, r=3s) para diferenciarse
    // de un círculo; a s=1 da 8×10px — claramente cápsula de micrófono.
    tft.fillRoundRect(cx - 4*s, cy - 7*s, 8*s, 10*s, 3*s, c);   // cápsula mic
    // Linea de membrana: franja horizontal en 0x1082 — separa visualmente
    // la parte superior de la capsula del diafragma. Mismo paradigma que
    // el canal de aire del termometro: estructura interior en color bg.
    tft.drawFastHLine(cx - 3*s, cy - 3*s, 6*s, 0x1082);           // membrana mic
    tft.fillRect     (cx - s,   cy + 3*s, 2*s, 2*s,  c);         // cuello
    tft.fillRoundRect(cx - 5*s, cy + 5*s, 10*s, 2*s, s, c);      // base
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
    // Misma geometria que impl_temp + canal vacío arriba y mercurio (acento) abajo.
    // Canvas lleno: cy-7s → cy+7s. A s=3 (Dial): tubo 12px, bulbo 18px Ø.
    tft.fillRoundRect(cx - 2*s, cy - 7*s, 4*s, 11*s, s, c);   // tubo: cy-7s → cy+4s
    tft.fillRect     (cx - s,   cy - 6*s, 2*s,  4*s, 0x1082); // canal vacío: cy-6s → cy-2s
    tft.fillRect     (cx - s,   cy - 2*s, 2*s,  6*s, a);       // mercurio: cy-2s → cy+4s
    tft.fillCircle   (cx,       cy + 4*s, 3*s,         c);     // bulbo último — cubre tip del mercurio
    tft.drawFastHLine(cx + 2*s, cy - 5*s, 2*s, c);             // tick alto
    tft.drawFastHLine(cx + 2*s, cy - 2*s, 2*s, c);             // tick medio
}

static void impl_probe_detail(int cx, int cy, uint16_t c, uint16_t a, int s) {
    // DS18B20 waterproof probe — versión detallada (XXL, s=3).
    // Misma estructura que impl_probe + detalles de material/construcción:
    //   - Cable centrado vertical (2 líneas adyacentes) en color acento
    //   - Collar más ancho que el housing (reborde visible)
    //   - Línea groove en el collar (ranura de separación)
    //   - V-highlight blanco en borde IZQUIERDO del housing (brillo lateral)
    //   - Punta redondeada en color principal
    //
    // Centrado: collar y housing usan cx - W/2 (integer) para quedar
    // simétricos respecto al cable. A s=1 queda igual que antes; a s=3
    // elimina el desplazamiento de 2px a la derecha que tenía fillRect(cx-2s).

    // Cable 2px (cx, cx+1), 3s alto, color acento
    tft.drawFastVLine(cx,     cy - 7*s, 3*s, a);
    tft.drawFastVLine(cx + 1, cy - 7*s, 3*s, a);

    // Collar 5s ancho, 2s alto — 1s gap hasta el body
    tft.fillRect(cx - (5*s)/2, cy - 4*s, 5*s, 2*s, c);

    // Body 3s ancho, 6s alto
    tft.fillRect(cx - (3*s)/2, cy - s, 3*s, 6*s, c);

    // V-highlight izquierdo (brillo lateral metalico), 4s alto
    tft.drawFastVLine(cx - (3*s)/2, cy - s + 1, 4*s, TFT_WHITE);

    // Punta: r=s, centro en cy+5s (borde inferior del body)
    tft.fillCircle(cx, cy + 5*s, s, c);
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
    // Cápsula alineada con impl_sound (8s×10s, r=3s) + grilla de ranuras blancas
    tft.fillRoundRect(cx - 4*s, cy - 7*s, 8*s, 10*s, 3*s, c);
    // Tres ranuras verticales (grilla de cápsula) centradas en el cuerpo
    tft.fillRoundRect(cx - 2*s - 1, cy - 4*s, 3, 4*s, 1, TFT_WHITE);   // ranura L
    tft.fillRoundRect(cx - 1,       cy - 5*s, 3, 5*s, 1, TFT_WHITE);   // ranura C (más alta)
    tft.fillRoundRect(cx + 2*s - 1, cy - 4*s, 3, 4*s, 1, TFT_WHITE);   // ranura R
    // Cuello y base (acento)
    tft.fillRect     (cx - s,   cy + 3*s, 2*s, 2*s,  a);
    tft.fillRoundRect(cx - 5*s, cy + 5*s, 10*s, 2*s, s, a);
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
void pbit_draw_probe_icon_xxl   (int cx, int cy, uint16_t c, uint16_t a) { impl_probe_detail   (cx, cy, c, a, 4); }  // s=4 (no s=3): icono mas visible en el Dial
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
