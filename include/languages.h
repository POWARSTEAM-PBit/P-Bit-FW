#pragma once
#include <stdint.h>

// Idiomas soportados — el valor es el índice de columna en la tabla STRINGS
enum Language : uint8_t {
    LANG_ES  = 0,
    LANG_CAT = 1,
    LANG_EN  = 2
};

// Claves del diccionario de cadenas
enum LangKey : uint8_t {
    // Títulos de pantalla
    TIT_TEMP = 0,
    TIT_HUM,
    TIT_LIGHT,
    TIT_SOUND,
    TIT_SOIL,
    TIT_THERM,
    TIT_SYS,
    TIT_TIMER,

    // Categorías sonido
    ST_SILENT,
    ST_QUIET,
    ST_NORMAL,
    ST_LOUD,
    ST_VERY_LOUD,

    // Categorías luz
    ST_DARK,
    ST_DIM,
    ST_INDOOR,
    ST_BRIGHT,
    ST_SUNLIGHT,

    // Categorías suelo
    ST_DRY,
    ST_OPTIMAL,
    ST_MOIST,
    ST_SATURATED,

    // Estado humedad relativa
    ST_MOLD_RISK,
    ST_TOO_DRY,

    // Estado BLE
    ST_DISCONN,
    ST_CONNECTED,

    // Pie de página sonido
    ST_SND_ON,
    ST_SND_OFF,

    // Instrucciones C/F
    INSTR_F,
    INSTR_C,
    INSTR_SEL,      // Menú de idioma

    // Error sensor
    ST_NO_SENSOR,

    // Estado temporizador
    ST_TIMER_RDY,
    ST_TIMER_RUN,
    ST_TIMER_PAU,

    // Instrucciones temporizador
    ST_PUSH_START,
    ST_PUSH_PAUSE,
    ST_PUSH_RESET,

    LANG_KEY_COUNT  // Centinela — debe ser el último
};

// Idioma activo (definido en lang_select.cpp)
extern Language g_language;

// Devuelve la cadena traducida para la clave dada
const char* L(LangKey key);
