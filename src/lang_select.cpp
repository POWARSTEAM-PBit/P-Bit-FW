// lang_select.cpp
// Multilingual dictionary and boot-time language selector.

#include "lang_select.h"
#include "languages.h"
#include "ui_widgets.h"  // Para tft
#include "fonts.h"       // para FONT_MENU, FONT_HEADER
#include "rotary.h"      // Para DI_ENCODER_A/B/SW y rotaryEncoder
#include <ESP32RotaryEncoder.h>
#include <Preferences.h>
#include <Arduino.h>

// ---------------------------------------------------------------
// Active language state shared across the whole firmware.
// ---------------------------------------------------------------
Language g_language = LANG_ES;

// ---------------------------------------------------------------
// Translation table [key][language]  ES=0  CAT=1  EN=2
// ---------------------------------------------------------------
static const char* const STRINGS[LANG_KEY_COUNT][3] = {
//                              ES (0)                    CAT (1)                  EN (2)
/* TIT_TEMP     */  { "TEMPERATURA",           "TEMPERATURA",          "TEMPERATURE"        },
/* TIT_HUM      */  { "HUMEDAD",               "HUMITAT",              "HUMIDITY"           },
/* TIT_LIGHT    */  { "LUZ",                   "LLUM",                 "LIGHT"              },
/* TIT_SOUND    */  { "SONIDO",                "SOROLL",               "SOUND"              },
/* TIT_SOIL     */  { "SUELO",                 "SÒL",                  "SOIL"               },
/* TIT_THERM    */  { "TERMÓMETRO",            "TERMÒMETRE",           "THERMOMETER"        },
/* TIT_SYS      */  { "INFO SISTEMA",          "INFO SISTEMA",         "SYSTEM INFO"        },
/* TIT_TIMER    */  { "TEMPORIZADOR",          "TEMPORITZADOR",        "TIMER"              },

/* ST_SILENT    */  { "Silencio",              "Silenci",              "Silent"             },
/* ST_QUIET     */  { "Tranquilo",             "Tranquil",             "Quiet"              },
/* ST_NORMAL    */  { "Normal",                "Normal",               "Normal"             },
/* ST_LOUD      */  { "Ruidoso",               "Fort",                 "Loud"               },
/* ST_VERY_LOUD */  { "Muy ruidoso",           "Molt fort",            "Very loud"          },

/* ST_DARK      */  { "Oscuro",                "Fosc",                 "Dark"               },
/* ST_DIM       */  { "Tenue",                 "Penombra",             "Dim"                },
/* ST_INDOOR    */  { "Interior",              "Interior",             "Indoor"             },
/* ST_BRIGHT    */  { "Brillante",             "Brillant",             "Bright"             },
/* ST_SUNLIGHT  */  { "Luz solar",             "Llum solar",           "Sunlight"           },

/* ST_DRY       */  { "Seco",                  "Sec",                  "Dry"                },
/* ST_OPTIMAL   */  { "Óptimo",                "Òptim",                "Optimal"            },
/* ST_MOIST     */  { "Húmedo",                "Humit",                "Moist"              },
/* ST_SATURATED */  { "Muy húmedo",            "Molt humit",           "Very moist"         },

/* ST_MOLD_RISK */  { "Riesgo de moho",        "Risc de floridura",    "Mold risk"          },
/* ST_TOO_DRY   */  { "Muy seco",              "Massa sec",            "Too dry"            },

/* ST_DISCONN   */  { "DESCONECTADO",          "DESCONNECTAT",         "DISCONNECTED"       },
/* ST_CONNECTED */  { "CONECTADO",             "CONNECTAT",            "CONNECTED"          },

/* ST_SND_ON    */  { "Sonido: ON (Pulsa)",    "Soroll: ON (Prem)",    "Sound: ON (Push)"   },
/* ST_SND_OFF   */  { "Sonido: OFF (Pulsa)",   "Soroll: OFF (Prem)",   "Sound: OFF (Push)"  },

/* INSTR_F      */  { "Pulsa > F",             "Prem > F",             "Push > F"           },
/* INSTR_C      */  { "Pulsa > C",             "Prem > C",             "Push > C"           },
/* INSTR_SEL    */  { "Pulsa para elegir",    "Prem per triar",       "Push to Select"     },
/* ST_SLEEPING  */  { "Reposo",                "Repòs",                "Sleeping"           },
/* ST_PUSH_TO_WAKE */ { "Pulsa para volver",   "Prem per tornar",      "Push to wake"       },
/* ST_RESTARTING */  { "Reiniciando...",        "Reiniciant...",        "Restarting..."      },
/* ST_ON        */  { "ON",                    "ON",                   "ON"                 },
/* ST_OFF       */  { "OFF",                   "OFF",                  "OFF"                },
/* ST_TURN_PUSH */  { "Gira y pulsa",          "Gira i prem",          "Turn and push"      },
/* ST_PUSH_MENU */  { "Pulsa para menú",       "Prem per menú",        "Push for menu"      },
/* ST_CHECK_DS18 */ { "Check J4",              "Check J4",             "Check J4"           },
/* ST_CHECK_SOIL */ { "Check J6 (GPIO35)",     "Check J6 (GPIO35)",    "Check J6 (GPIO35)"  },
/* SYS_DEV_LABEL */ { "DEV",                   "DEV",                  "DEV"                },
/* SYS_UP_LABEL */  { "UP",                    "UP",                   "UP"                 },
/* SYS_BLE_LABEL */ { "BLE",                   "BLE",                  "BLE"                },
/* SYS_LANG_LABEL*/ { "IDI",                   "IDI",                  "LAN"                },

/* SUB_AIR_REL  */  { "Relativa del aire",     "Relativa de l'aire",   "Relative humidity"  },
/* SUB_SOIL_MOIST*/ { "Humedad",               "Humitat",              "Moisture"           },

  /* ST_NO_SENSOR */  { "Sin sensor",            "Sense sensor",         "No sensor"          },
  /* ST_UNIT_C_SHORT */ { "C",                   "C",                    "C"                  },
  /* ST_UNIT_F_SHORT */ { "F",                   "F",                    "F"                  },
  /* ST_LUX_UNIT */   { "lux",                   "lux",                  "lux"                },
  /* ST_RAW_ADC */    { "Raw ADC",               "Raw ADC",              "Raw ADC"            },
  /* ST_LOG_PCT */    { "% log",                 "% log",                "% log"              },

  /* ST_SOIL_CAL_DRY */ { "Seco al aire",        "Sec a l'aire",         "Dry in air"         },
  /* ST_SOIL_CAL_WET */ { "En agua",             "En aigua",             "In water"           },
  /* ST_SOIL_PUSH_CAPTURE */ { "Pulsa para guardar", "Prem per desar",   "Push to save"       },
  /* ST_SOIL_CAL_SAVED */ { "Calibrado",         "Calibrat",             "Calibrated"         },
  /* ST_SOIL_CAL_ERROR */ { "Valores inválidos", "Valors invàlids",      "Invalid values"     },
  /* ST_PUSH_EXIT */ { "Pulsa para salir",       "Prem per sortir",      "Push to exit"       },
  /* ST_SOIL_DRY_REF */ { "SECO",                "SEC",                  "DRY"                },
  /* ST_SOIL_WET_REF */ { "MOJADO",              "MULLAT",               "WET"                },
  /* ST_SOIL_MENU_CAL */ { "Calibrar sensor",    "Calibrar sensor",      "Calibrate sensor"   },
  /* ST_SOIL_MENU_THRESH */ { "Editar umbrales", "Editar llindars",      "Edit thresholds"    },
  /* ST_SOIL_MENU_BACK */ { "Salir",             "Sortir",               "Exit"               },
  /* ST_SOIL_THRESH_SAVED */ { "Umbrales OK",        "Llindars OK",       "Limits OK"          },
  /* ST_SOIL_TURN_ADJUST */ { "Gira y pulsa",    "Gira i prem",          "Turn and push"      },

  /* ST_TIMER_RDY */  { "LISTO",                 "LLEST",                "READY"              },
/* ST_TIMER_RUN */  { "CORRIENDO",             "EN CURS",              "RUNNING"            },
/* ST_TIMER_PAU */  { "PAUSADO",               "PAUSAT",               "PAUSED"             },

/* ST_PUSH_START*/  { "Pulsa iniciar",         "Prem iniciar",         "Push start"         },
/* ST_PUSH_PAUSE*/  { "Pulsa pausar",          "Prem pausar",          "Push pause"         },
/* ST_PUSH_RESET*/  { "Pulsa seguir · Mant reset", "Prem segueix · Mant reset", "Push resume · Hold reset" },
/* ST_TIMER_MINUTES */ { "MINUTOS",            "MINUTS",               "MINUTES"            },
/* ST_TIMER_DURATION */ { "DURACIÓN",          "DURADA",               "DURATION"           },
/* ST_TIMER_STOPWATCH */ { "Cronómetro",       "Cronòmetre",           "Stopwatch"          },
/* ST_TIMER_CFG_SELECT */ { "Pulsa editar · Mant guardar", "Prem editar · Mant desar", "Push edit · Hold save" },
/* ST_TIMER_CFG_EDIT */ { "Gira ajusta",      "Gira ajusta",          "Turn adjust"        },

/* MENU_TITLE   */  { "Idioma",                "Idioma",               "Language"           },
/* LANG_ES_NAME */  { "Español",               "Castellà",             "Spanish"            },
/* LANG_CAT_NAME*/  { "Catalán",               "Català",               "Catalan"            },
/* LANG_EN_NAME */  { "Inglés",                "Anglès",               "English"            },
};

// ---------------------------------------------------------------
// L() translates the requested key using the active language.
// ---------------------------------------------------------------
const char* L(LangKey key) {
    if (key >= LANG_KEY_COUNT) return "?";
    return STRINGS[key][(uint8_t)g_language];
}

// ---------------------------------------------------------------
// loadLanguage() restores the last saved language from NVS.
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

void saveLanguage(Language language) {
    g_language = language;

    Preferences prefs;
    prefs.begin("pbit", false);
    prefs.putUChar("lang", (uint8_t)g_language);
    prefs.end();
}

// ---------------------------------------------------------------
// Internal boot menu helpers.
// ---------------------------------------------------------------
static const Language MENU_LANGS[]  = { LANG_ES, LANG_CAT, LANG_EN };
static const LangKey MENU_LANG_NAMES[] = { LANG_ES_NAME, LANG_CAT_NAME, LANG_EN_NAME };
constexpr int MENU_CURSOR_FONT = 2;
constexpr int MENU_CURSOR_Y_OFFSET = 2;

// Draw the options and footer hint for the boot language picker.
// The menu is rendered in the currently selected language so every label matches.
static void drawMenuOptions(int sel, Language current_menu_lang) {
    const int cx         = tft.width() / 2;
    const int y_opts[]   = { 40, 64, 88 };
    const int x_cursor   = 12;

    for (int i = 0; i < 3; i++) {
        bool active = (i == sel);
        tft.fillRect(0, y_opts[i] - 2, tft.width(), 20, TFT_BLACK);

        // Selection cursor.
        tft.setTextFont(MENU_CURSOR_FONT);
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(active ? TFT_YELLOW : TFT_BLACK, TFT_BLACK);
        tft.drawString(">", x_cursor, y_opts[i] + MENU_CURSOR_Y_OFFSET, MENU_CURSOR_FONT);

        // Render the language name in the menu language, not in English.
        tft.setFreeFont(FONT_MENU);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(active ? TFT_WHITE : TFT_DARKGREY, TFT_BLACK);
        
        // Temporarily switch the global language to fetch the localized label.
        Language saved_lang = g_language;
        g_language = current_menu_lang;
        
        // Fetch the name using the current menu language.
        const char* lang_name = L(MENU_LANG_NAMES[i]);
        tft.drawString(lang_name, cx, y_opts[i]);
        
        // Restore the previous global language.
        g_language = saved_lang;
        
        tft.setTextFont(0);  // liberar fuente tras usar GFXfont
    }

    // Footer instruction in the highlighted language.
    const int y_instr = 116;
    tft.fillRect(0, y_instr - 2, tft.width(), 16, TFT_BLACK);
    // Temporarily switch the language to fetch the selected-language hint.
    Language saved_lang = g_language;
    g_language = current_menu_lang;
    const char* instr_text = L(INSTR_SEL);
    g_language = saved_lang;

    drawFooterHint(instr_text, cx, y_instr);
}

// Draw the full boot menu, including the title and all options.
static void drawMenuFull(int sel, Language current_menu_lang) {
    tft.fillScreen(TFT_BLACK);
    const int cx = tft.width() / 2;
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    
    // Temporarily switch the language so the title is localized correctly.
    Language saved_lang = g_language;
    g_language = current_menu_lang;
    
    // Fetch the localized menu title.
    const char* menu_title = L(MENU_TITLE);
    tft.setFreeFont(FONT_HEADER);
    tft.drawString(menu_title, cx, 8);
    tft.setTextFont(0); // liberar
    
    // Restore the previous global language.
    g_language = saved_lang;
    
    tft.drawFastHLine(20, 32, tft.width() - 40, TFT_GREEN);
    drawMenuOptions(sel, current_menu_lang);
}

// ---------------------------------------------------------------
// showLanguageMenu() is the public entry point.
// It always appears on cold boot and preselects the last saved language.
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
    rotaryEncoder.begin(false);
    rotaryEncoder.setEncoderValue(initial_sel);
    pinMode((uint8_t)DI_ENCODER_SW, INPUT_PULLUP);

    int sel      = initial_sel;
    int last_val = initial_sel;
    drawMenuFull(sel, MENU_LANGS[sel]);  // Pasar el idioma actual del menú

    bool lastSW = (bool)digitalRead((uint8_t)DI_ENCODER_SW);

    while (true) {
        rotaryEncoder.loop();

        int val = (int)rotaryEncoder.getEncoderValue();
        if (val != last_val) {
            last_val = val;
            sel = val;
            // Redraw the full menu so the title and language names update together.
            drawMenuFull(sel, MENU_LANGS[sel]);
        }

        // Confirmar con botón (flanco de bajada)
        bool sw = (bool)digitalRead((uint8_t)DI_ENCODER_SW);
        if (lastSW == true && sw == false) {
            delay(50); // anti-rebote
            while ((bool)digitalRead((uint8_t)DI_ENCODER_SW) == false) {
                delay(5);
            }
            break;
        }
        lastSW = sw;
        delay(5);
    }

    // Guardar idioma seleccionado
    saveLanguage(MENU_LANGS[sel]);

    tft.fillScreen(TFT_BLACK);
}
