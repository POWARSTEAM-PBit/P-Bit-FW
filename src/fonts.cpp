// fonts.cpp
// Punto único de compilación de los datos de fuente Inter (PROGMEM).
// Incluir este archivo UNA sola vez evita duplicar ~47KB de datos en flash.
// Los demás archivos solo incluyen fonts.h (declaraciones extern).

#include <TFT_eSPI.h>
#include "Inter_Regular9pt8b.h"
#include "Inter_Regular12pt8b.h"
#include "Inter_Regular24pt8b.h"
#include "Inter_SemiBold18pt8b.h"
