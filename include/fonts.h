#pragma once
#include <TFT_eSPI.h>

// ── Fuentes compiladas (definidas en src/fonts.cpp) ──────────────────────────
// Solo estas 6 entran al Flash. Usar siempre a través de los alias FONT_*
// de abajo — no referenciar el nombre de fuente directamente en pantallas.

// Roboto
extern const GFXfont Roboto_Regular7pt8b;
extern const GFXfont Roboto_Light6pt8b;
extern const GFXfont Roboto_Medium10pt8b;

// IBM Plex Sans
extern const GFXfont IBMPlexSans_Regular9pt8b;

// IBM Plex Mono
extern const GFXfont IBMPlexMono_Regular12pt8b;
extern const GFXfont IBMPlexMono_Regular24pt8b;

// ── Alias semánticos (usar estos en el código de pantallas) ───────────────────
static const GFXfont* const FONT_VALUE  = &IBMPlexMono_Regular24pt8b; // Valor principal grande
static const GFXfont* const FONT_HEADER = &Roboto_Medium10pt8b;       // Titulo de pantalla
static const GFXfont* const FONT_BODY   = &Roboto_Regular7pt8b;       // Texto secundario y categorias
static const GFXfont* const FONT_SMALL  = &Roboto_Light6pt8b;         // Hints y subtitulos pequenos
static const GFXfont* const FONT_MENU   = &IBMPlexSans_Regular9pt8b;  // Opciones del menu de idioma
static const GFXfont* const FONT_INFO   = &Roboto_Light6pt8b;         // Etiquetas y valores cortos de System Info
static const GFXfont* const FONT_TIMER  = &IBMPlexMono_Regular12pt8b; // Tiempo del cronometro

// ── Fuentes disponibles pero no activas ──────────────────────────────────────
// Los archivos .h están en include/ y listos para activar.
// Para usar una: 1) añadir su #include en src/fonts.cpp,
//                2) agregar su extern aquí,
//                3) actualizar el alias FONT_* correspondiente.
//
// Disponibles: Roboto (Regular 6–24pt, Light 7–9pt, Bold 10–18pt),
//              Inter (Regular 9/12/24pt, SemiBold 18pt),
//              IBMPlexSans (Regular 12/24pt, SemiBold 18pt),
//              IBMPlexMono (Regular 20pt, SemiBold 12/20/24pt),
//              RobotoMono (Regular/Medium 12/20/24pt),
//              Audiowide (Regular 20/24/26pt)
