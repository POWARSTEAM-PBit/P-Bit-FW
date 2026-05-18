// layout_validation_snippet.cpp
// Snippet de calibración de geometría master para la pantalla ST7735 160x128.
// NO pertenece al build de producción. Para usarlo, copiar el bloque if(...){...}
// dentro de setup() en main.cpp justo después de init_tft_display(), cambiar
// kShowStartupLayoutValidation a true, y reflashear. Quitar después.
//
// Dependencias necesarias en main.cpp:
//   #include "fonts.h"  (o declarar los GFXfont extern directamente)
//   #include "layout.h"
//
// El bloque usa LC_MASTER_* de layout.h y fonts Roboto de fonts.cpp.

// ---------------------------------------------------------------------------
// Pegar dentro de setup(), después de init_tft_display():
// ---------------------------------------------------------------------------
//
//    // ── LAYOUT VALIDATION TEST ── enable only when recalibrating master geometry ──
//    constexpr bool kShowStartupLayoutValidation = true;  // <-- cambiar a true para activar
//    if (kShowStartupLayoutValidation) {
//        extern const GFXfont Roboto_Medium10pt8b;
//        extern const GFXfont Roboto_Regular7pt8b;
//        extern const GFXfont Roboto_Light6pt8b;
//
//        tft.fillScreen(TFT_BLACK);
//
//        // Pixel boundary reference: red dot at each corner
//        tft.fillRect(0,   0,   1, 1, TFT_RED);
//        tft.fillRect(159, 0,   1, 1, TFT_RED);
//        tft.fillRect(0,   127, 1, 1, TFT_RED);
//        tft.fillRect(159, 127, 1, 1, TFT_RED);
//
//        // ── Header: C_BASELINE y=16 → text top=3, bottom=17 ──────────
//        // TC/MC datums use glyph_ab=18 (global font max, char Å), not string ascent=13.
//        // C_BASELINE bypasses that: y IS the baseline. top = y - 13 = 3.
//        tft.setTextDatum(C_BASELINE);
//        tft.setTextColor(TFT_WHITE, TFT_BLACK);
//        tft.setFreeFont(&Roboto_Medium10pt8b);
//        tft.drawString("SENSOR LAB", 80, LC_MASTER_HEADER_BASELINE);
//        tft.setTextFont(0);
//        tft.drawFastHLine(LC_MASTER_HEADER_LINE_X,
//                          LC_MASTER_HEADER_LINE_Y,
//                          LC_MASTER_HEADER_LINE_W,
//                          TFT_WHITE);
//
//        // ── Mock 2x2 card grid ─────────────────────────────────────────
//        // margin=2px, gap=4px, card=76x48, row0 y=27, row1 y=79
//        const uint16_t kFrame = tft.color565(50, 70, 110);
//        tft.drawRoundRect(LC_MASTER_CARD_X0, LC_MASTER_CARD_Y0,
//                          LC_MASTER_CARD_W, LC_MASTER_CARD_H,
//                          LC_MASTER_CARD_RADIUS, kFrame);
//        tft.drawRoundRect(LC_MASTER_CARD_X1, LC_MASTER_CARD_Y0,
//                          LC_MASTER_CARD_W, LC_MASTER_CARD_H,
//                          LC_MASTER_CARD_RADIUS, kFrame);
//        tft.drawRoundRect(LC_MASTER_CARD_X0, LC_MASTER_CARD_Y1,
//                          LC_MASTER_CARD_W, LC_MASTER_CARD_H,
//                          LC_MASTER_CARD_RADIUS, kFrame);
//        tft.drawRoundRect(LC_MASTER_CARD_X1, LC_MASTER_CARD_Y1,
//                          LC_MASTER_CARD_W, LC_MASTER_CARD_H,
//                          LC_MASTER_CARD_RADIUS, kFrame);
//
//        // Card size labels
//        char card_label[16];
//        snprintf(card_label, sizeof(card_label), "%dx%d", LC_MASTER_CARD_W, LC_MASTER_CARD_H);
//        tft.setTextDatum(MC_DATUM);
//        tft.setFreeFont(&Roboto_Regular7pt8b);
//        tft.setTextColor(tft.color565(90, 90, 90), TFT_BLACK);
//        tft.drawString(card_label, LC_MASTER_CARD_CX0, LC_MASTER_CARD_CY0);
//        tft.drawString(card_label, LC_MASTER_CARD_CX1, LC_MASTER_CARD_CY0);
//        tft.drawString(card_label, LC_MASTER_CARD_CX0, LC_MASTER_CARD_CY1);
//        tft.drawString(card_label, LC_MASTER_CARD_CX1, LC_MASTER_CARD_CY1);
//        tft.setTextFont(0);
//
//        // ── Y-axis markers on right edge (1px wide) ───────────────────
//        // Each dot shows a key layout boundary
//        tft.fillRect(159, LC_MASTER_HEADER_TEXT_TOP,  1, 1, TFT_YELLOW);
//        tft.fillRect(159, LC_MASTER_HEADER_LINE_Y,    1, 1, TFT_CYAN);
//        tft.fillRect(159, LC_MASTER_CARD_Y0,          1, 1, TFT_WHITE);
//        tft.fillRect(159, LC_MASTER_CARD_Y1,          1, 1, TFT_WHITE);
//        tft.fillRect(159, LC_MASTER_CARD_BOTTOM,      1, 1, TFT_WHITE);
//        tft.fillRect(159, LC_MASTER_FOOTER_TEXT_TOP,  1, 1, TFT_MAGENTA);
//
//        delay(15000);
//    }
//    // ── END LAYOUT VALIDATION TEST ──────────────────────────────────────
