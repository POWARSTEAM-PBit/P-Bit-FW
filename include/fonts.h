#pragma once
#include <TFT_eSPI.h>

// Declaraciones externas — datos de bitmap definidos en src/fonts.cpp (compilados UNA sola vez).
// NO incluir los archivos Inter_*.h aquí: cada include duplicaría ~47KB de PROGMEM por TU.
extern const GFXfont Roboto_Regular9pt8bBitmaps;
extern const GFXfont Roboto_Regular12pt8bBitmaps;
extern const GFXfont Roboto_Bold18pt8bBitmaps;
extern const GFXfont Roboto_Regular24pt8bBitmaps;

// Aliases semánticos para roles de UI.
// Tipo: GFXfont* (no int) — distinto de los "const int FONT_VALUE=7" locales que quedan comentados.
static const GFXfont* const FONT_SMALL  = Roboto_Regular9pt8bBitmaps;   //  9pt, línea 21px — instrucciones, hints
static const GFXfont* const FONT_BODY   = Roboto_Regular12pt8bBitmaps;   //  9pt, línea 21px — instrucciones, hints
static const GFXfont* const FONT_HEADER = Roboto_Bold18pt8bBitmaps; // SemiBold 18pt, línea 43px — cabeceras
static const GFXfont* const FONT_VALUE  = Roboto_Regular24pt8bBitmaps;  // 24pt, línea 57px — valores numéricos grandes
