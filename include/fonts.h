#pragma once
#include <TFT_eSPI.h>

extern const GFXfont Roboto_Regular9pt8b;
extern const GFXfont Roboto_Regular12pt8b;
extern const GFXfont Roboto_Regular24pt8b;
extern const GFXfont Roboto_Bold18pt8b;

static const GFXfont* const FONT_SMALL  = &Roboto_Regular9pt8b;
static const GFXfont* const FONT_BODY   = &Roboto_Regular12pt8b;
static const GFXfont* const FONT_HEADER = &Roboto_Bold18pt8b;
static const GFXfont* const FONT_VALUE  = &Roboto_Regular24pt8b;