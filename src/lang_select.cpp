// lang_select.cpp
// Diccionario multilingüe y menú de selección de idioma.

#include "lang_select.h"
#include "languages.h"
#include "ui_widgets.h"  // Para tft
#include "rotary.h"      // Para DI_ENCODER_A/B/SW y rotaryEncoder
#include <ESP32RotaryEncoder.h>
#include <Preferences.h>
#include <Arduino.h>

// ---------------------------------------------------------------
// Variable global de idioma activo (extern en languages.h)
// ---------------------------------------------------------------
Language g_language = LANG_ES;

// ---------------------------------------------------------------
// Tabla de cadenas [clave][idioma]   ES=0  CAT=1  EN=2
// ---------------------------------------------------------------
static const char* const STRINGS[LANG_KEY_COUNT][3] = {
//                              ES (0)                    CAT (1)                  EN (2)
/* TIT_TEMP     */  { "Temperatura",           "Temperatura",          "Temperature"        },
/* TIT_HUM      */  { "Humedad",               "Humitat",              "Humidity"           },
/* TIT_LIGHT    */  { "Luz",                   "Llum",                 "Light"              },
/* TIT_SOUND    */  { "Sonido",                "So",                   "Sound"              },
/* TIT_SOIL     */  { "Suelo",                 "Sol",                  "Soil"               },
/* TIT_THERM    */  { "Termometro",            "Termometre",           "Thermometer"        },
/* TIT_SYS      */  { "Info Sistema",          "Info Sistema",         "System Info"        },
/* TIT_TIMER    */  { "Temporizador",          "Temporitzador",        "Timer"              },

/* ST_SILENT    */  { "SILENCIO",              "SILENCI",              "SILENT"             },
/* ST_QUIET     */  { "TRANQUILO",             "TRANQUIL",             "QUIET"              },
/* ST_NORMAL    */  { "NORMAL",                "NORMAL",               "NORMAL"             },
/* ST_LOUD      */  { "RUIDOSO",               "FORT",                 "LOUD"               },
/* ST_VERY_LOUD */  { "MUY RUIDOSO",           "MOLT FORT",            "VERY LOUD"          },

/* ST_DARK      */  { "OSCURO",                "FOSC",                 "DARK"               },
/* ST_DIM       */  { "TENUE",                 "PENOMBRA",             "DIM"                },
/* ST_INDOOR    */  { "INTERIOR",              "INTERIOR",             "INDOOR"             },
/* ST_BRIGHT    */  { "BRILLANTE",             "BRILLANT",             "BRIGHT"             },
/* ST_SUNLIGHT  */  { "LUZ SOLAR",             "LLUM SOLAR",           "SUNLIGHT"           },

/* ST_DRY       */  { "SECO",                  "SEC",                  "DRY"                },
/* ST_OPTIMAL   */  { "OPTIMO",               "OPTIM",                "OPTIMAL"            },
/* ST_MOIST     */  { "HUMEDO",               "HUMIT",                "MOIST"              },
/* ST_SATURATED */  { "SATURADO",              "SATURAT",              "SATURATED"          },

/* ST_MOLD_RISK */  { "Riesgo Moho",           "Risc Floridura",       "Mold Risk"          },
/* ST_TOO_DRY   */  { "Muy Seco",              "Massa Sec",            "Too Dry"            },

/* ST_DISCONN   */  { "DESCONECTADO",          "DESCONNECTAT",         "DISCONNECTED"       },
/* ST_CONNECTED */  { "CONECTADO",             "CONNECTAT",            "CONNECTED"          },

/* ST_SND_ON    */  { "Sonido: ON (Pulsa)",    "So: ON (Prem)",        "Sound: ON (Push)"   },
/* ST_SND_OFF   */  { "Sonido: OFF (Pulsa)",   "So: OFF (Prem)",       "Sound: OFF (Push)"  },

/* INSTR_F      */  { "Pulsa > F",             "Prem > F",             "Push > F"           },
/* INSTR_C      */  { "Pulsa > C",             "Prem > C",             "Push > C"           },
/* INSTR_SEL    */  { "Pulsa p. elegir",       "Prem per triar",       "Push to Select"     },

/* ST_NO_SENSOR */  { "Sin sensor",            "Sense sensor",         "No sensor"          },

/* ST_TIMER_RDY */  { "LISTO",                 "LLEST",                "READY"              },
/* ST_TIMER_RUN */  { "CORRIENDO",             "EN CURS",              "RUNNING"            },
/* ST_TIMER_PAU */  { "PAUSADO",               "PAUSAT",               "PAUSED"             },

/* ST_PUSH_START*/  { "Pulsa-Iniciar",         "Prem-Iniciar",         "Push-Start"         },
/* ST_PUSH_PAUSE*/  { "Pulsa-Pausar",          "Prem-Pausar",          "Push-Pause"         },
/* ST_PUSH_RESET*/  { "Mant>Rst|Pulsa>Ini",   "Mant>Rst|Prem>Ini",    "Hold-Rst|Push-Run"  },
};

// ---------------------------------------------------------------
// L() — función de traducción
// ---------------------------------------------------------------
const char* L(LangKey key) {
    if (key >= LANG_KEY_COUNT) return "?";
    return STRINGS[key][(uint8_t)g_language];
}

// ---------------------------------------------------------------
// loadLanguage() — carga idioma desde NVS sin mostrar menú
// ---------------------------------------------------------------
void loadLanguage() {
    Preferences prefs;
    prefs.begin("pbit", true);
    if (prefs.isKey("lang")) {
        g_language = (Language)prefs.getUChar("lang", LANG_ES);
        if ((uint8_t)g_language >= 3) g_language = LANG_ES;
    } else {
        g_language = LANG_ES;
    }
    prefs.end();
}

// ---------------------------------------------------------------
// Menú interno — helpers privados
// ---------------------------------------------------------------
static const Language MENU_LANGS[]  = { LANG_ES, LANG_CAT, LANG_EN };
static const char* const MENU_NAMES[] = { "Espanol", "Catala", "English" };

// Dibuja solo las opciones y el texto de instrucción (se llama en cada cambio de selección)
static void drawMenuOptions(int sel) {
    const int cx         = tft.width() / 2;
    const int y_opts[]   = { 40, 64, 88 };
    const int x_cursor   = 12;

    for (int i = 0; i < 3; i++) {
        bool active = (i == sel);
        tft.fillRect(0, y_opts[i] - 2, tft.width(), 20, TFT_BLACK);

        // Cursor ">"
        tft.setTextFont(2);
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(active ? TFT_YELLOW : TFT_BLACK, TFT_BLACK);
        tft.drawString(">", x_cursor, y_opts[i], 2);

        // Nombre del idioma — FreeSans9pt7b para soportar Español, Català
        tft.setFreeFont(&FreeSans9pt7b);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(active ? TFT_WHITE : TFT_DARKGREY, TFT_BLACK);
        tft.drawString(MENU_NAMES[i], cx, y_opts[i]);
        tft.setTextFont(2);  // Restaurar font 2
    }

    // Texto de instrucción en el idioma resaltado
    const int y_instr = 116;
    tft.fillRect(0, y_instr - 2, tft.width(), 16, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(STRINGS[INSTR_SEL][(uint8_t)MENU_LANGS[sel]], cx, y_instr, 1);
}

// Dibuja el menú completo (header + opciones)
static void drawMenuFull(int sel) {
    tft.fillScreen(TFT_BLACK);
    const int cx = tft.width() / 2;
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Language", cx, 8, 4);
    tft.drawFastHLine(20, 32, tft.width() - 40, TFT_GREEN);
    drawMenuOptions(sel);
}

// ---------------------------------------------------------------
// showLanguageMenu() — punto de entrada público
// Siempre muestra el menú en cold boot. Carga el idioma previo
// como preselección. init_rotary() en main.cpp reconfigura el
// encoder con límites y callbacks correctos para la app principal.
// ---------------------------------------------------------------
void showLanguageMenu() {
    // Cargar idioma guardado como preselección inicial (si existe)
    Preferences prefs;
    prefs.begin("pbit", true);
    int initial_sel = 0;
    if (prefs.isKey("lang")) {
        uint8_t saved = prefs.getUChar("lang", 0);
        if (saved < 3) initial_sel = (int)saved;
    }
    prefs.end();

    // Configurar encoder para el menú (límites 0-2, circular, sin callbacks)
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    rotaryEncoder.setBoundaries(0, 2, true);
    rotaryEncoder.begin();
    rotaryEncoder.setEncoderValue(initial_sel);

    int sel      = initial_sel;
    int last_val = initial_sel;
    drawMenuFull(sel);

    bool lastSW = (bool)digitalRead((uint8_t)DI_ENCODER_SW);

    while (true) {
        rotaryEncoder.loop();

        int val = (int)rotaryEncoder.getEncoderValue();
        if (val != last_val) {
            last_val = val;
            sel = val;
            drawMenuOptions(sel);
        }

        // Confirmar con botón (flanco de bajada)
        bool sw = (bool)digitalRead((uint8_t)DI_ENCODER_SW);
        if (lastSW == true && sw == false) {
            delay(50); // anti-rebote
            break;
        }
        lastSW = sw;
        delay(5);
    }

    // Guardar idioma seleccionado
    g_language = MENU_LANGS[sel];
    prefs.begin("pbit", false);
    prefs.putUChar("lang", (uint8_t)g_language);
    prefs.end();

    tft.fillScreen(TFT_BLACK);
}
