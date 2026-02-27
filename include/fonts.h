#pragma once
#include <TFT_eSPI.h>

// todas las variantes Roboto disponibles en include/
extern const GFXfont Roboto_Regular20pt8b;
extern const GFXfont Roboto_Regular18pt8b;
extern const GFXfont Roboto_Regular24pt8b;
extern const GFXfont Roboto_Regular12pt8b;
extern const GFXfont Roboto_Regular10pt8b;
extern const GFXfont Roboto_Regular9pt8b;
extern const GFXfont Roboto_Regular8pt8b;
extern const GFXfont Roboto_Regular7pt8b;
extern const GFXfont Roboto_Regular6pt8b;

extern const GFXfont Roboto_Medium10pt8b;   // usado como header 9pt

extern const GFXfont Roboto_Light9pt8b;
extern const GFXfont Roboto_Light8pt8b;
extern const GFXfont Roboto_Light7pt8b;
extern const GFXfont Roboto_Light6pt8b;

extern const GFXfont Roboto_Bold18pt8b;
extern const GFXfont Roboto_Bold12pt8b;
extern const GFXfont Roboto_Bold10pt8b;

// combinación de prueba actual
static const GFXfont* const FONT_VALUE  = &Roboto_Regular18pt8b; // VALUE → Roboto Regular 18
static const GFXfont* const FONT_HEADER = &Roboto_Medium10pt8b;  // HEADER → Roboto Medium ≈9
static const GFXfont* const FONT_BODY   = &Roboto_Regular7pt8b;  // BODY → Roboto Regular 7
static const GFXfont* const FONT_SMALL  = &Roboto_Light6pt8b;    // SMALL → Roboto Light 6

// se conservan todas las demás declaraciones para futuras combinaciones
