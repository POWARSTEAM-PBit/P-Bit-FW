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

extern const GFXfont Inter_18pt_Regular24pt8b;
extern const GFXfont Inter_18pt_Regular12pt8b;
extern const GFXfont Inter_18pt_Regular9pt8b;
extern const GFXfont Inter_18pt_SemiBold18pt8b;

#define Inter_Regular24pt8b  Inter_18pt_Regular24pt8b
#define Inter_Regular12pt8b  Inter_18pt_Regular12pt8b
#define Inter_Regular9pt8b   Inter_18pt_Regular9pt8b
#define Inter_SemiBold18pt8b Inter_18pt_SemiBold18pt8b

extern const GFXfont IBMPlexSans_Regular24pt8b;
extern const GFXfont IBMPlexSans_Regular12pt8b;
extern const GFXfont IBMPlexSans_Regular9pt8b;
extern const GFXfont IBMPlexSans_SemiBold18pt8b;

// Audiowide — candidatas para VALUE
extern const GFXfont Audiowide_Regular20pt8b;
extern const GFXfont Audiowide_Regular24pt8b;
extern const GFXfont Audiowide_Regular26pt8b;

// combinación de prueba actual
static const GFXfont* const FONT_VALUE  = &IBMPlexSans_Regular24pt8b; // VALUE  → probar: 20 / 24 / 26
static const GFXfont* const FONT_HEADER = &Roboto_Medium10pt8b;     // HEADER → Roboto Medium 10pt
static const GFXfont* const FONT_BODY   = &Roboto_Regular7pt8b;     // BODY   → Roboto Regular 7pt
static const GFXfont* const FONT_SMALL  = &Roboto_Light6pt8b;       // SMALL  → Roboto Light 6pt
static const GFXfont* const FONT_MENU   = &IBMPlexSans_Regular9pt8b;     // MENU   → opciones menú idioma
static const GFXfont* const FONT_INFO   = &Roboto_Light6pt8b;     // INFO   → etiquetas/valores System Info
static const GFXfont* const FONT_TIMER  = &Inter_Regular12pt8b;     // TIMER  → tiempo del cronómetro (= FONT_BODY por ahora)

// se conservan todas las demás declaraciones para futuras combinaciones
