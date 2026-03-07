#pragma once
#include <TFT_eSPI.h>

// Roboto
extern const GFXfont Roboto_Regular20pt8b;
extern const GFXfont Roboto_Regular18pt8b;
extern const GFXfont Roboto_Regular24pt8b;
extern const GFXfont Roboto_Regular12pt8b;
extern const GFXfont Roboto_Regular10pt8b;
extern const GFXfont Roboto_Regular9pt8b;
extern const GFXfont Roboto_Regular8pt8b;
extern const GFXfont Roboto_Regular7pt8b;
extern const GFXfont Roboto_Regular6pt8b;

extern const GFXfont Roboto_Light9pt8b;
extern const GFXfont Roboto_Light8pt8b;
extern const GFXfont Roboto_Light7pt8b;
extern const GFXfont Roboto_Light6pt8b;

extern const GFXfont Roboto_Medium10pt8b;

extern const GFXfont Roboto_Bold18pt8b;
extern const GFXfont Roboto_Bold12pt8b;
extern const GFXfont Roboto_Bold10pt8b;

// Inter
extern const GFXfont Inter_18pt_Regular24pt8b;
extern const GFXfont Inter_18pt_Regular12pt8b;
extern const GFXfont Inter_18pt_Regular9pt8b;
extern const GFXfont Inter_18pt_SemiBold18pt8b;

#define Inter_Regular24pt8b  Inter_18pt_Regular24pt8b
#define Inter_Regular12pt8b  Inter_18pt_Regular12pt8b
#define Inter_Regular9pt8b   Inter_18pt_Regular9pt8b
#define Inter_SemiBold18pt8b Inter_18pt_SemiBold18pt8b

// IBM Plex Sans
extern const GFXfont IBMPlexSans_Regular24pt8b;
extern const GFXfont IBMPlexSans_Regular12pt8b;
extern const GFXfont IBMPlexSans_Regular9pt8b;
extern const GFXfont IBMPlexSans_SemiBold18pt8b;

// IBM Plex Mono
extern const GFXfont IBMPlexMono_Regular12pt8b;
extern const GFXfont IBMPlexMono_Regular20pt8b;
extern const GFXfont IBMPlexMono_Regular24pt8b;
extern const GFXfont IBMPlexMono_SemiBold12pt8b;
extern const GFXfont IBMPlexMono_SemiBold20pt8b;
extern const GFXfont IBMPlexMono_SemiBold24pt8b;

// Roboto Mono
extern const GFXfont RobotoMono_Regular12pt8b;
extern const GFXfont RobotoMono_Regular20pt8b;
extern const GFXfont RobotoMono_Regular24pt8b;
extern const GFXfont RobotoMono_Medium12pt8b;
extern const GFXfont RobotoMono_Medium20pt8b;
extern const GFXfont RobotoMono_Medium24pt8b;

// Audiowide
extern const GFXfont Audiowide_Regular20pt8b;
extern const GFXfont Audiowide_Regular24pt8b;
extern const GFXfont Audiowide_Regular26pt8b;

// Combinacion activa
static const GFXfont* const FONT_VALUE  = &IBMPlexMono_Regular24pt8b; // Valor principal grande
static const GFXfont* const FONT_HEADER = &Roboto_Medium10pt8b;       // Titulo de pantalla
static const GFXfont* const FONT_BODY   = &Roboto_Regular7pt8b;       // Texto secundario y categorias
static const GFXfont* const FONT_SMALL  = &Roboto_Light6pt8b;         // Hints y subtitulos pequenos
static const GFXfont* const FONT_MENU   = &IBMPlexSans_Regular9pt8b;  // Opciones del menu de idioma
static const GFXfont* const FONT_INFO   = &Roboto_Light6pt8b;         // Etiquetas y valores cortos de System Info
static const GFXfont* const FONT_TIMER  = &IBMPlexMono_Regular12pt8b;  // Tiempo del cronometro
