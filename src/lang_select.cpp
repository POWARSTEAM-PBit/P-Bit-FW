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
/* ST_CLIMATE_FRESH */ { "Fresco",             "Fresc",                "Cool"               },
/* ST_CLIMATE_WARM */  { "Cálido",             "Càlid",                "Warm"               },

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

// Shared menu strings
/* MENU_SAVED        */  { "Guardado",              "Desat",                "Saved"              },
/* MENU_RESET        */  { "Reset",                 "Reset",                "Reset"              },
/* MENU_ALERTS       */  { "Alertas",               "Alertes",              "Alerts"             },
/* MENU_EXIT         */  { "Salir",                 "Sortir",               "Exit"               },
/* MENU_NO           */  { "NO",                    "NO",                   "NO"                 },
/* MENU_YES          */  { "SÍ",                    "SÍ",                   "YES"                },
/* MENU_DEFAULTS     */  { "Valores por defecto",   "Valors per defecte",   "Default values"     },
/* MENU_ERROR        */  { "Error",                 "Error",                "Error"              },
/* MENU_LIMITS       */  { "Límites",               "Límits",               "Limits"             },
/* MENU_CALIBRATION  */  { "Calibración",           "Calibració",           "Calibration"        },
/* MENU_UNIT         */  { "Unidad",                "Unitat",               "Unit"               },
/* MENU_RESET_DONE   */  { "Reset aplicado",        "Reset aplicat",        "Reset applied"      },
/* MENU_LOW          */  { "Bajo",                  "Baix",                 "Low"                },
/* MENU_HIGH         */  { "Alto",                  "Alt",                  "High"               },
/* MENU_LIMIT_LOW    */  { "Límite bajo",           "Límit baix",           "Low limit"          },
/* MENU_LIMIT_HIGH   */  { "Límite alto",           "Límit alt",            "High limit"         },
/* MENU_UNIT_F       */  { "Fahrenheit",            "Fahrenheit",           "Fahrenheit"         },
/* MENU_UNIT_C       */  { "Celsius",               "Celsius",              "Celsius"            },

// System menu
/* MENU_SETTINGS     */  { "AJUSTES",               "AJUSTOS",              "SETTINGS"           },
/* MENU_SOUND        */  { "Sonido",                "Soroll",               "Sound"              },
/* MENU_SLEEP        */  { "Reposo",                "Repos",                "Sleep"              },
/* MENU_NEVER        */  { "Nunca",                 "Mai",                  "Never"              },
/* MENU_SLEEP_30S    */  { "30 seg",                "30 s",                 "30 sec"             },
/* MENU_SLEEP_1M     */  { "1 min",                 "1 min",                "1 min"              },
/* MENU_SLEEP_2M     */  { "2 min",                 "2 min",                "2 min"              },
/* MENU_SLEEP_5M     */  { "5 min",                 "5 min",                "5 min"              },
/* MENU_SLEEP_10M    */  { "10 min",                "10 min",               "10 min"             },
/* MENU_FULL_RESET   */  { "Reset total",           "Reset total",          "Full reset"         },
/* MENU_RESTORE_ALL  */  { "Todo vuelve",           "Tot torna",            "Restore all"        },
/* MENU_TO_DEFAULTS  */  { "a fábrica",             "de fàbrica",           "to defaults"        },

// Light menu
/* MENU_DISPLAY_MODE     */  { "Modo display",      "Mode display",         "Display mode"       },
/* MENU_LIGHT_MAX_DIM    */  { "Max penumbra",      "Max penombra",         "Max dim"            },
/* MENU_LIGHT_MAX_INDOOR */  { "Max interior",      "Max interior",         "Max indoor"         },
/* MENU_LIGHT_MAX_BRIGHT */  { "Max brillante",     "Max brillant",         "Max bright"         },
/* MENU_RESET_SUB_LIGHT  */  { "de luz",            "de llum",              "for light"          },
/* MENU_LIGHT_ABR_DIM    */  { "Pen",               "Pen",                  "Dim"                },
/* MENU_LIGHT_ABR_IN     */  { "Int",               "Int",                  "In"                 },
/* MENU_LIGHT_ABR_BRIGHT */  { "Bri",               "Bri",                  "Bri"                },

// Sound menu
/* MENU_SND_MAX_QUIET    */  { "Max silencio",      "Max silenci",          "Max quiet"          },
/* MENU_SND_MAX_NORMAL   */  { "Max normal",        "Max normal",           "Max normal"         },
/* MENU_SND_MAX_LOUD     */  { "Max alto",          "Max alt",              "Max loud"           },
/* MENU_RESET_SUB_SOUND  */  { "de sonido",         "de so",                "for sound"          },
/* MENU_SND_ABR_QUIET    */  { "Sil",               "Sil",                  "Qui"                },
/* MENU_SND_ABR_NORMAL   */  { "Nor",               "Nor",                  "Nor"                },
/* MENU_SND_ABR_LOUD     */  { "Alt",               "Alt",                  "Loud"               },

// Soil menu
/* MENU_SOIL_SENSOR_LIMITS */ { "Sensor y umbrales", "Sensor i llindars",   "Sensor and limits"  },
/* MENU_RESET_SUB_SOIL   */  { "por defecto",       "per defecte",          "to defaults"        },
/* MENU_RESTORED         */  { "restaurados",       "restaurats",           "restored"           },
/* MENU_PUSH_CAPTURE     */  { "Pulsa para capturar", "Prem per capturar",  "Push to capture"    },

// DS18 / temperature menu
/* MENU_OFFSET           */  { "Offset",            "Offset",               "Offset"             },
/* MENU_RESET_SUB_TEMP   */  { "de temperatura",    "de temperatura",       "for temperature"    },
/* MENU_RESET_SUB_PROBE  */  { "del termómetro",    "del termòmetre",       "for probe menu"     },
/* MENU_RESET_SUB_HUM    */  { "de humedad",        "d'humitat",            "for humidity"       },

/* MENU_TITLE   */  { "Idioma",                "Idioma",               "Language"           },
/* LANG_ES_NAME */  { "Español",               "Castellà",             "Spanish"            },
/* LANG_CAT_NAME*/  { "Catalán",               "Català",               "Catalan"            },
/* LANG_EN_NAME */  { "Inglés",                "Anglès",               "English"            },

// Graph screen
/* TIT_GRAPH         */ { "GRÁFICA",              "GRÀFICA",              "GRAPH LAB"          },
/* GRAPH_PUSH_SENSOR */ { "Pulsa: cambiar sensor", "Prem: canviar sensor", "Push: change sensor" },
/* ST_WAITING        */ { "Esperando...",         "Esperant...",          "Waiting..."         },
/* GRAPH_LABEL_TEMP_AIR */ { "Temperatura aire", "Temperatura aire",      "Air temperature"    },
/* GRAPH_LABEL_HUM_AIR  */ { "Humedad",          "Humitat",               "Humidity"           },
/* GRAPH_LABEL_LIGHT    */ { "Luz",              "Llum",                  "Light"              },
/* GRAPH_LABEL_SOUND    */ { "Sonido",           "Soroll",                "Sound"              },
/* GRAPH_LABEL_SOIL_HUM */ { "Humedad suelo",    "Humitat sòl",           "Soil moisture"      },
/* GRAPH_LABEL_DS18     */ { "Temperatura sonda","Temperatura sonda",     "Probe temperature"  },

// Temporary lab screens
/* TIT_LAB_DASH      */ { "ESTADO LAB",           "ESTAT LAB",            "LAB OVERVIEW"       },
/* TIT_LAB_FOCUS     */ { "SENSOR LAB",           "SENSOR LAB",           "LAB SENSOR"         },
/* TIT_LAB_DUAL_TH   */ { "CLIMA LAB",            "CLIMA LAB",            "LAB CLIMATE"        },
/* TIT_LAB_ICON_A    */ { "OUTLINE",              "OUTLINE",              "OUTLINE"            },
/* TIT_LAB_ICON_B    */ { "SOLID",                "SOLID",                "SOLID"              },
/* TIT_LAB_ICON_C    */ { "PIXEL",                "PIXEL",                "PIXEL"              },
/* TIT_LAB_GAUGE     */ { "GAUGE LAB",            "GAUGE LAB",            "GAUGE LAB"          },
/* TIT_LAB_VALUE     */ { "VALOR LAB",            "VALOR LAB",            "VALUE LAB"          },
/* TIT_LAB_TEMP_CARD */ { "TEMP CARD",            "TEMP CARD",            "TEMP CARD"          },
/* TIT_LAB_PROBE_CARD */ { "PROBE CARD",          "PROBE CARD",           "PROBE CARD"         },
/* TIT_LAB_HUM_CARD  */ { "HUM CARD",            "HUM CARD",             "HUM CARD"           },
/* TIT_LAB_LIGHT_CARD*/ { "LUZ CARD",            "LLUM CARD",            "LIGHT CARD"         },
/* TIT_LAB_SOUND_CARD*/ { "SONIDO CARD",         "SO CARD",              "SOUND CARD"         },
/* TIT_LAB_SOIL_CARD */ { "SUELO CARD",          "SÒL CARD",             "SOIL CARD"          },
/* TIT_LAB_WIDGETS   */ { "TEMP LAB",             "TEMP LAB",             "TEMP LAB"           },
/* TIT_LAB_VU_STACK  */ { "SOUND LAB",            "SOUND LAB",            "SOUND LAB"          },
/* TIT_LAB_VU_WAVE   */ { "SOUND LAB",            "SOUND LAB",            "SOUND LAB"          },
/* LAB_PUSH_VIEW     */ { "Pulsa: cambiar vista", "Prem: canviar vista",  "Push: change view"  },
/* LAB_VIEW_STACK    */ { "STACK",                "STACK",                "STACK"              },
/* LAB_VIEW_WAVE     */ { "WAVE",                 "WAVE",                 "WAVE"               },
/* LAB_COMPARE_HINT  */ { "4 iconos grandes",     "4 icones grans",       "4 large icons"      },
/* LAB_EXPERIMENT_HINT */ { "Vista experimental", "Vista experimental",   "Experimental view"  },
/* LAB_TEMP_SHORT    */ { "TEMP",                 "TEMP",                 "TEMP"               },
/* LAB_HUM_SHORT     */ { "HUM",                  "HUM",                  "HUM"                },
/* LAB_LIGHT_SHORT   */ { "LUZ",                  "LLUM",                 "LIGHT"              },
/* LAB_SOUND_SHORT   */ { "MIC",                  "MIC",                  "MIC"                },
/* LAB_SOIL_SHORT    */ { "SUELO",                "SÒL",                  "SOIL"               },
/* LAB_PROBE_SHORT   */ { "SONDA",                "SONDA",                "PROBE"              },
/* LAB_TEMP_DIFF     */ { "DIF TEMP",             "DIF TEMP",             "TEMP DIFF"          },
/* TIT_LAB_ICON_SZ_ENV */ { "TAM ICONO ENV",     "MIDA ICONA ENV",       "ICON SIZE ENV"      },
/* TIT_LAB_ICON_SZ_EXT */ { "TAM ICONO EXT",     "MIDA ICONA EXT",       "ICON SIZE EXT"      },
/* TIT_LAB_HOME_CARDS  */ { "HOME",               "HOME",                 "HOME"               },
/* TIT_LAB_LINEAR_DASH */ { "PLANT LAB",          "PLANT LAB",            "PLANT LAB"          },
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
    const int y_opts[]   = { 38, 62, 86 };
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
