// lang_select.cpp
// Multilingual dictionary and boot-time language selector.

#include "lang_select.h"
#include "languages.h"
#include "ui_widgets.h"  // Para tft
#include "fonts.h"       // para FONT_MENU, FONT_HEADER
#include "rotary.h"      // Para DI_ENCODER_A/B/SW y rotaryEncoder
#include "runtime_events.h"
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
static const char* const STRINGS[LANG_KEY_COUNT][LANG_COUNT] = {
//                              ES (0)                    CAT (1)                  EN (2)
/* TIT_TEMP     */  { "TEMPERATURA",           "TEMPERATURA",          "TEMPERATURE"        },
/* TIT_HUM      */  { "HUMEDAD",               "HUMITAT",              "HUMIDITY"           },
/* TIT_LIGHT    */  { "LUZ",                   "LLUM",                 "LIGHT"              },
/* TIT_SOUND    */  { "RUIDO",                 "SOROLL",               "NOISE"              },
/* TIT_SOIL     */  { "SUELO",                 "SÒL",                  "SOIL"               },
/* TIT_THERM    */  { "TERMÓMETRO",            "TERMÒMETRE",           "THERMOMETER"        },
/* TIT_SYS      */  { "SISTEMA",               "SISTEMA",              "SYSTEM"             },
/* TIT_TIMER    */  { "TEMPORIZADOR",          "TEMPORITZADOR",        "TIMER"              },

/* ST_SILENT    */  { "Silencio",              "Silenci",              "Silent"             },
/* ST_QUIET     */  { "Suave",                 "Suau",                 "Quiet"              },
/* ST_NORMAL    */  { "Normal",                "Normal",               "Normal"             },
/* ST_LOUD      */  { "Ruidoso",               "Sorollós",             "Loud"               },
/* ST_VERY_LOUD */  { "Muy fuerte",            "Molt fort",            "Very loud"          },

/* ST_DARK      */  { "Oscuro",                "Fosc",                 "Dark"               },
/* ST_DIM       */  { "Poca luz",              "Poca llum",            "Dim"                },
/* ST_INDOOR    */  { "Interior",              "Interior",             "Indoor"             },
/* ST_BRIGHT    */  { "Brillante",             "Brillant",             "Bright"             },
/* ST_SUNLIGHT  */  { "Sol",                   "Sol",                  "Sun"                },

/* ST_DRY       */  { "Seco",                  "Sec",                  "Dry"                },
/* ST_OPTIMAL   */  { "Óptimo",                "Òptim",                "Optimal"            },
/* ST_MOIST     */  { "Húmedo",                "Humit",                "Moist"              },
/* ST_SATURATED */  { "Muy húmedo",            "Molt humit",           "Very moist"         },

/* ST_MOLD_RISK */  { "Riesgo de moho",        "Risc de floridura",    "Mold risk"          },
/* ST_TOO_DRY   */  { "Muy seco",              "Massa sec",            "Too dry"            },
/* ST_CLIMATE_FRESH */ { "Fresco",             "Fresc",                "Cool"               },
/* ST_CLIMATE_WARM */  { "Cálido",             "Càlid",                "Warm"               },

/* ST_DISCONN   */  { "SIN BLE",               "SENSE BLE",            "NO BLE"             },
/* ST_CONNECTED */  { "BLE OK",                "BLE OK",               "BLE OK"             },

/* ST_SND_ON    */  { "Bip ON: pulsa",         "Bip ON: prem",         "Beep ON: press"     },
/* ST_SND_OFF   */  { "Bip OFF: pulsa",        "Bip OFF: prem",        "Beep OFF: press"    },

/* INSTR_F      */  { "Pulsa: F",              "Prem: F",              "Press: F"           },
/* INSTR_C      */  { "Pulsa: C",              "Prem: C",              "Press: C"           },
/* INSTR_SEL    */  { "Pulsa: elegir",         "Prem: triar",          "Press: select"      },
/* ST_SLEEPING  */  { "Reposo",                "Repòs",                "Sleeping"           },
/* ST_PUSH_TO_WAKE */ { "Pulsa: volver",       "Prem: torna",          "Press: wake"        },
/* ST_RESTARTING */  { "Reiniciando...",        "Reiniciant...",        "Restarting..."      },
/* ST_ON        */  { "ON",                    "ON",                   "ON"                 },
/* ST_OFF       */  { "OFF",                   "OFF",                  "OFF"                },
/* ST_TURN_PUSH */  { "Gira+pulsa",            "Gira+prem",            "Turn+press"         },
/* ST_PUSH_MENU */  { "Pulsa: menú",           "Prem: menú",           "Press: menu"        },
/* ST_CHECK_DS18 */ { "Revisa J4",             "Comprova J4",          "Check J4"           },
/* ST_CHECK_SOIL */ { "Revisa J6 (GPIO35)",    "Comprova J6 (GPIO35)", "Check J6 (GPIO35)"  },
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
  /* ST_RAW_ADC */    { "ADC bruto",             "ADC cru",              "Raw ADC"            },
  /* ST_ADC_UNIT */   { "ADC",                   "ADC",                  "ADC"                },
  /* ST_LOG_PCT */    { "% log",                 "% log",                "% log"              },

  /* ST_SOIL_CAL_DRY */ { "En aire",             "A l'aire",             "In air"             },
  /* ST_SOIL_CAL_WET */ { "En agua",             "En aigua",             "In water"           },
  /* ST_SOIL_PUSH_CAPTURE */ { "Pulsa: guardar",     "Prem: desa",       "Press: save"        },
  /* ST_SOIL_CAL_SAVED */ { "Calibrado",         "Calibrat",             "Calibrated"         },
  /* ST_SOIL_CAL_ERROR */ { "Valores inválidos", "Valors invàlids",      "Invalid values"     },
  /* ST_PUSH_EXIT */ { "Pulsa: salir",           "Prem: sortir",         "Press: exit"        },
  /* ST_SOIL_DRY_REF */ { "SECO",                "SEC",                  "DRY"                },
  /* ST_SOIL_WET_REF */ { "MOJADO",              "MULLAT",               "WET"                },
  /* ST_SOIL_MENU_CAL */ { "Calibrar",           "Calibrar",             "Calibrate"          },
  /* ST_SOIL_MENU_THRESH */ { "Umbrales",        "Llindars",             "Limits"             },
  /* ST_SOIL_MENU_BACK */ { "Salir",             "Sortir",               "Exit"               },
  /* ST_SOIL_THRESH_SAVED */ { "Umbrales OK",        "Llindars OK",       "Limits OK"          },
  /* ST_SOIL_TURN_ADJUST */ { "Gira+pulsa",      "Gira+prem",            "Turn+press"         },

  /* ST_TIMER_RDY */  { "LISTO",                 "LLEST",                "READY"              },
/* ST_TIMER_RUN */  { "EN CURSO",              "EN CURS",              "RUNNING"            },
/* ST_TIMER_PAU */  { "PAUSADO",               "PAUSAT",               "PAUSED"             },

/* ST_PUSH_START*/  { "Pulsa iniciar",         "Prem iniciar",         "Press start"        },
/* ST_PUSH_PAUSE*/  { "Pulsa pausar",          "Prem pausar",          "Press pause"        },
/* ST_PUSH_RESET*/  { "Pulsa seguir · Mant. reset", "Prem seguir · Mant. reset", "Press resume · Hold reset" },
/* ST_TIMER_MINUTES */ { "MINUTOS",            "MINUTS",               "MINUTES"            },
/* ST_TIMER_DURATION */ { "DURACIÓN",          "DURADA",               "DURATION"           },
/* ST_TIMER_STOPWATCH */ { "Cronómetro",       "Cronòmetre",           "Stopwatch"          },
/* ST_TIMER_CFG_SELECT */ { "Pulsa editar · Mant. guardar", "Prem editar · Mant. desar", "Press edit · Hold save" },
/* ST_TIMER_CFG_EDIT */ { "Gira para ajustar", "Gira per ajustar",     "Turn to adjust"     },

// Shared menu strings
/* MENU_SAVED        */  { "Guardado",              "Desat",                "Saved"              },
/* MENU_RESET        */  { "Reset",                 "Reset",                "Reset"              },
/* MENU_ALERTS       */  { "Alertas",               "Alertes",              "Alerts"             },
/* MENU_EXIT         */  { "Salir",                 "Sortir",               "Exit"               },
/* MENU_NO           */  { "NO",                    "NO",                   "NO"                 },
/* MENU_YES          */  { "SÍ",                    "SÍ",                   "YES"                },
/* MENU_DEFAULTS     */  { "Por defecto",           "Per defecte",          "Defaults"           },
/* MENU_ERROR        */  { "Error",                 "Error",                "Error"              },
/* MENU_LIMITS       */  { "Límites",               "Límits",               "Limits"             },
/* MENU_CALIBRATION  */  { "Calibración",           "Calibratge",           "Calibration"        },
/* MENU_UNIT         */  { "Unidad",                "Unitat",               "Unit"               },
/* MENU_RESET_DONE   */  { "Reset aplicado",        "Reset aplicat",        "Reset applied"      },
/* MENU_LOW          */  { "Bajo",                  "Baix",                 "Low"                },
/* MENU_HIGH         */  { "Alto",                  "Alt",                  "High"               },
/* MENU_LIMIT_LOW    */  { "Límite bajo",           "Límit baix",           "Low limit"          },
/* MENU_LIMIT_HIGH   */  { "Límite alto",           "Límit alt",            "High limit"         },
/* MENU_UNIT_F       */  { "Fahrenheit",            "Fahrenheit",           "Fahrenheit"         },
/* MENU_UNIT_C       */  { "Celsius",               "Celsius",              "Celsius"            },

// System menu
/* MENU_SETTINGS     */  { "AJUSTES",               "AJUSTS",               "SETTINGS"           },
/* MENU_SOUND        */  { "Bip",                   "Bip",                  "Beep"               },
/* MENU_SLEEP        */  { "Reposo",                "Repòs",                "Sleep"              },
/* MENU_NEVER        */  { "Nunca",                 "Mai",                  "Never"              },
/* MENU_SLEEP_30S    */  { "30 seg",                "30 s",                 "30 sec"             },
/* MENU_SLEEP_1M     */  { "1 min",                 "1 min",                "1 min"              },
/* MENU_SLEEP_2M     */  { "2 min",                 "2 min",                "2 min"              },
/* MENU_SLEEP_5M     */  { "5 min",                 "5 min",                "5 min"              },
/* MENU_SLEEP_10M    */  { "10 min",                "10 min",               "10 min"             },
/* MENU_FULL_RESET   */  { "Reset total",           "Reset total",          "Full reset"         },
/* MENU_RESTORE_ALL  */  { "Restaurar",             "Restaura",             "Restore"            },
/* MENU_TO_DEFAULTS  */  { "fábrica",               "fàbrica",              "defaults"           },

// Light menu
/* MENU_DISPLAY_MODE     */  { "Modo lectura",      "Mode lectura",         "Reading mode"       },
/* MENU_LIGHT_MAX_DIM    */  { "Máx. poca luz",     "Màx. poca llum",       "Max dim"            },
/* MENU_LIGHT_MAX_INDOOR */  { "Máx. interior",     "Màx. interior",        "Max indoor"         },
/* MENU_LIGHT_MAX_BRIGHT */  { "Máx. brillante",    "Màx. brillant",        "Max bright"         },
/* MENU_RESET_SUB_LIGHT  */  { "de luz",            "de llum",              "for light"          },
/* MENU_LIGHT_ABR_DIM    */  { "Pen",               "Pen",                  "Dim"                },
/* MENU_LIGHT_ABR_IN     */  { "Int",               "Int",                  "In"                 },
/* MENU_LIGHT_ABR_BRIGHT */  { "Bri",               "Bri",                  "Bri"                },

// Sound menu
/* MENU_SND_MAX_QUIET    */  { "Máx. suave",        "Màx. suau",            "Max quiet"          },
/* MENU_SND_MAX_NORMAL   */  { "Máx. normal",       "Màx. normal",          "Max normal"         },
/* MENU_SND_MAX_LOUD     */  { "Máx. fuerte",       "Màx. fort",            "Max loud"           },
/* MENU_RESET_SUB_SOUND  */  { "de ruido",          "de soroll",            "for noise"          },
/* MENU_SND_ABR_QUIET    */  { "Sil",               "Sil",                  "Qui"                },
/* MENU_SND_ABR_NORMAL   */  { "Nor",               "Nor",                  "Nor"                },
/* MENU_SND_ABR_LOUD     */  { "Rui",               "Sor",                  "Loud"               },

// Soil menu
/* MENU_SOIL_SENSOR_LIMITS */ { "Sensor+límites",   "Sensor+llindars",     "Sensor+limits"      },
/* MENU_RESET_SUB_SOIL   */  { "del suelo",         "del sòl",              "for soil"           },
/* MENU_RESTORED         */  { "restaurados",       "restaurats",           "restored"           },
/* MENU_PUSH_CAPTURE     */  { "Pulsa: captura",      "Prem: captura",      "Press: capture"     },

// DS18 / temperature menu
/* MENU_OFFSET           */  { "Corrección",        "Correcció",            "Offset"             },
/* MENU_RESET_SUB_TEMP   */  { "de temperatura",    "de temperatura",       "for temperature"    },
/* MENU_RESET_SUB_PROBE  */  { "del termómetro",    "del termòmetre",       "for probe menu"     },
/* MENU_RESET_SUB_HUM    */  { "de humedad",        "d'humitat",            "for humidity"       },

/* MENU_TITLE   */  { "Idioma",                "Idioma",               "Language"           },
/* LANG_ES_NAME */  { "Español",               "Castellà",             "Spanish"            },
/* LANG_CAT_NAME*/  { "Catalán",               "Català",               "Catalan"            },
/* LANG_EN_NAME */  { "Inglés",                "Anglès",               "English"            },

// Graph screen
/* TIT_GRAPH         */ { "GRÁFICA",              "GRÀFICA",              "GRAPH"              },
/* GRAPH_PUSH_SENSOR */ { "Pulsa: sensor",       "Prem: sensor",          "Press: sensor"      },
/* ST_WAITING        */ { "Esperando...",         "Esperant...",          "Waiting..."         },
/* GRAPH_LABEL_TEMP_AIR */ { "Temp. del aire",  "Temp. de l'aire",       "Air temp"           },
/* GRAPH_LABEL_HUM_AIR  */ { "Humedad",          "Humitat",               "Humidity"           },
/* GRAPH_LABEL_LIGHT    */ { "Luz",              "Llum",                  "Light"              },
/* GRAPH_LABEL_SOUND    */ { "Ruido",            "Soroll",                "Noise"              },
/* GRAPH_LABEL_SOIL_HUM */ { "Hum. suelo",       "Hum. sòl",              "Soil moisture"      },
/* GRAPH_LABEL_DS18     */ { "Temp. sonda",      "Temp. sonda",           "Probe temp"         },

// Temporary lab screens
/* TIT_LAB_DASH      */ { "ESTADO LAB",           "ESTAT LAB",            "LAB OVERVIEW"       },
/* TIT_LAB_FOCUS     */ { "SENSOR LAB",           "SENSOR LAB",           "SENSOR LAB"         },
/* TIT_LAB_DUAL_TH   */ { "CLIMA LAB",            "CLIMA LAB",            "CLIMATE LAB"        },
/* TIT_LAB_ICON_A    */ { "CONTORNO",             "CONTORN",              "OUTLINE"            },
/* TIT_LAB_ICON_B    */ { "SÓLIDO",               "SÒLID",                "SOLID"              },
/* TIT_LAB_ICON_C    */ { "PÍXEL",                "PÍXEL",                "PIXEL"              },
/* TIT_LAB_GAUGE     */ { "DIAL LAB",             "DIAL LAB",             "GAUGE LAB"          },
/* TIT_LAB_VALUE     */ { "VALOR LAB",            "VALOR LAB",            "VALUE LAB"          },
/* TIT_LAB_TEMP_CARD */ { "TEMP TARJETA",         "TEMP TARGETA",         "TEMP CARD"          },
/* TIT_LAB_PROBE_CARD */ { "SONDA TARJETA",       "SONDA TARGETA",        "PROBE CARD"         },
/* TIT_LAB_HUM_CARD  */ { "HUM TARJETA",          "HUM TARGETA",          "HUM CARD"           },
/* TIT_LAB_LIGHT_CARD*/ { "LUZ TARJETA",          "LLUM TARGETA",         "LIGHT CARD"         },
/* TIT_LAB_SOUND_CARD*/ { "RUIDO TARJETA",        "SOROLL TARGETA",       "NOISE CARD"         },
/* TIT_LAB_SOIL_CARD */ { "SUELO TARJETA",        "SÒL TARGETA",          "SOIL CARD"          },
/* TIT_LAB_WIDGETS   */ { "TEMP LAB",             "TEMP LAB",             "TEMP LAB"           },
/* TIT_LAB_VU_STACK  */ { "RUIDO LAB",            "SOROLL LAB",           "NOISE LAB"          },
/* TIT_LAB_VU_WAVE   */ { "RUIDO LAB",            "SOROLL LAB",           "NOISE LAB"          },
/* LAB_PUSH_VIEW     */ { "Pulsa: vista",         "Prem: vista",          "Press: view"        },
/* LAB_VIEW_STACK    */ { "BARRAS",               "BARRES",               "STACK"              },
/* LAB_VIEW_WAVE     */ { "ONDA",                 "ONA",                  "WAVE"               },
/* LAB_COMPARE_HINT  */ { "4 iconos",             "4 icones",             "4 icons"            },
/* LAB_EXPERIMENT_HINT */ { "Experimental",       "Experimental",         "Experimental"       },
/* LAB_TEMP_SHORT    */ { "TEMP",                 "TEMP",                 "TEMP"               },
/* LAB_HUM_SHORT     */ { "HUM",                  "HUM",                  "HUM"                },
/* LAB_LIGHT_SHORT   */ { "LUZ",                  "LLUM",                 "LIGHT"              },
/* LAB_SOUND_SHORT   */ { "MIC",                  "MIC",                  "MIC"                },
/* LAB_SOIL_SHORT    */ { "SUELO",                "SÒL",                  "SOIL"               },
/* LAB_PROBE_SHORT   */ { "SONDA",                "SONDA",                "PROBE"              },
/* LAB_TEMP_DIFF     */ { "DIF TEMP",             "DIF TEMP",             "TEMP DIFF"          },
/* TIT_LAB_ICON_SZ_ENV */ { "TAM ICONO AMB",     "MIDA ICONA AMB",       "ICON SIZE ENV"      },
/* TIT_LAB_ICON_SZ_EXT */ { "TAM ICONO EXT",     "MIDA ICONA EXT",       "ICON SIZE EXT"      },
/* TIT_LAB_ICON_TEST */ { "TEST ICONOS",          "TEST ICONES",          "ICON TEST"          },
/* LAB_ICON_SIZE_S   */ { "S",                    "S",                    "S"                  },
/* LAB_ICON_SIZE_M   */ { "M",                    "M",                    "M"                  },
/* LAB_ICON_SIZE_L   */ { "L",                    "L",                    "L"                  },
/* LAB_ICON_TEST_PROC_SHORT */ { "PROC",          "PROC",                 "PROC"               },
/* LAB_ICON_TEST_BITMAP */ { "bitmap",            "mapa bits",            "bitmap"             },
/* LAB_ICON_TEST_PRIMITIVES */ { "primitivas",    "primitives",           "primitives"         },
/* LAB_ICON_TEST_FOOTER */ { "32x32 / colores fijos", "32x32 / colors fixos", "32x32 / baked colors" },
/* TIT_LAB_HOME_CARDS  */ { "INICIO",             "INICI",                "HOME"               },
/* TIT_LAB_LINEAR_DASH */ { "PLANTAS LAB",        "PLANTES LAB",          "PLANT LAB"          },
/* SOIL_ZONE_DRY       */ { "Sec",                "Sec",                  "Dry"                },
/* SOIL_ZONE_OK        */ { "Ok",                 "Bé",                   "Ok"                 },
/* SOIL_ZONE_WET       */ { "Hum",                "Hum",                  "Wet"                },
/* SZ_SUFFIX_CARD      */ { "TARJETA",            "TARGETA",              "CARD"               },
/* SZ_SUFFIX_VALUE     */ { "LAB",                "LAB",                  "LAB"                },
/* SZ_SUFFIX_GRAPH     */ { "GRAF",               "GRÀF",                 "GRAPH"              },
/* SZ_SUFFIX_DIAL      */ { "DIAL",               "DIAL",                 "DIAL"               },
};

// ---------------------------------------------------------------
// L() translates the requested key using the active language.
// ---------------------------------------------------------------
Language normalizeLanguage(Language language) {
    return ((uint8_t)language < (uint8_t)LANG_COUNT) ? language : LANG_ES;
}

const char* LIn(Language language, LangKey key) {
    if (key >= LANG_KEY_COUNT) return "?";
    return STRINGS[key][(uint8_t)normalizeLanguage(language)];
}

const char* L(LangKey key) {
    return LIn(normalizeLanguage(g_language), key);
}

// ---------------------------------------------------------------
// loadLanguage() restores the last saved language from NVS.
// ---------------------------------------------------------------
void loadLanguage() {
    Preferences prefs;
    prefs.begin("pbit", true);
    if (prefs.isKey("lang")) {
        g_language = normalizeLanguage((Language)prefs.getUChar("lang", LANG_ES));
    } else {
        g_language = LANG_ES;
    }
    prefs.end();
}

void saveLanguage(Language language) {
    g_language = normalizeLanguage(language);

    Preferences prefs;
    prefs.begin("pbit", false);
    prefs.putUChar("lang", (uint8_t)g_language);
    prefs.end();

    runtime_request_ui_full_redraw();
}

// ---------------------------------------------------------------
// Internal boot menu helpers.
// ---------------------------------------------------------------
static const Language MENU_LANGS[]  = { LANG_ES, LANG_CAT, LANG_EN };
static const LangKey MENU_LANG_NAMES[] = { LANG_ES_NAME, LANG_CAT_NAME, LANG_EN_NAME };
constexpr int MENU_LANG_COUNT = sizeof(MENU_LANGS) / sizeof(MENU_LANGS[0]);
constexpr int MENU_CURSOR_FONT = 2;
constexpr int MENU_CURSOR_Y_OFFSET = 2;

// Draw the options and footer hint for the boot language picker.
// The menu is rendered in the currently selected language so every label matches.
static void drawMenuOptions(int sel, Language current_menu_lang) {
    const int cx         = tft.width() / 2;
    const int y_opts[]   = { 38, 62, 86 };
    const int x_cursor   = 12;

    for (int i = 0; i < MENU_LANG_COUNT; i++) {
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
        
        const char* lang_name = LIn(current_menu_lang, MENU_LANG_NAMES[i]);
        tft.drawString(lang_name, cx, y_opts[i]);

        tft.setTextFont(0);  // liberar fuente tras usar GFXfont
    }

    // Footer instruction in the highlighted language.
    const int y_instr = 116;
    tft.fillRect(0, y_instr - 2, tft.width(), 16, TFT_BLACK);
    const char* instr_text = LIn(current_menu_lang, INSTR_SEL);

    drawFooterHint(instr_text, cx, y_instr);
}

// Draw the full boot menu, including the title and all options.
static void drawMenuFull(int sel, Language current_menu_lang) {
    tft.fillScreen(TFT_BLACK);
    const int cx = tft.width() / 2;
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    
    const char* menu_title = LIn(current_menu_lang, MENU_TITLE);
    tft.setFreeFont(FONT_HEADER);
    tft.drawString(menu_title, cx, 8);
    tft.setTextFont(0); // liberar

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
        if (saved < MENU_LANG_COUNT) initial_sel = (int)saved;
    }
    prefs.end();

    // Configurar encoder para el menú (límites 0-2, circular, sin callbacks)
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    rotaryEncoder.setBoundaries(0, MENU_LANG_COUNT - 1, true);
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
