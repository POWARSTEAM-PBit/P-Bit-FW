#pragma once
#include <stdint.h>

// Supported languages. The enum value matches the STRINGS column index.
enum Language : uint8_t {
    LANG_ES  = 0,
    LANG_CAT = 1,
    LANG_EN  = 2
};

// String keys grouped by screen, shared states, and helper labels.
enum LangKey : uint8_t {
    // Screen titles
    TIT_TEMP = 0,
    TIT_HUM,
    TIT_LIGHT,
    TIT_SOUND,
    TIT_SOIL,
    TIT_THERM,
    TIT_SYS,
    TIT_TIMER,

    // Sound categories
    ST_SILENT,
    ST_QUIET,
    ST_NORMAL,
    ST_LOUD,
    ST_VERY_LOUD,

    // Light categories
    ST_DARK,
    ST_DIM,
    ST_INDOOR,
    ST_BRIGHT,
    ST_SUNLIGHT,

    // Soil categories
    ST_DRY,
    ST_OPTIMAL,
    ST_MOIST,
    ST_SATURATED,

    // Relative humidity status
    ST_MOLD_RISK,
    ST_TOO_DRY,

    // BLE status
    ST_DISCONN,
    ST_CONNECTED,

    // Sound footer labels
    ST_SND_ON,
    ST_SND_OFF,

    // C/F instructions and shared prompts
    INSTR_F,
    INSTR_C,
    INSTR_SEL,      // Menú de idioma
    ST_SLEEPING,
    ST_PUSH_TO_WAKE,
    ST_RESTARTING,
    ST_ON,
    ST_OFF,
    ST_TURN_PUSH,
    ST_PUSH_MENU,
    ST_CHECK_DS18,
    ST_CHECK_SOIL,
    SYS_DEV_LABEL,
    SYS_UP_LABEL,
    SYS_BLE_LABEL,
    SYS_LANG_LABEL,

    // Auxiliary subtitles for tank-style screens
    SUB_AIR_REL,
    SUB_SOIL_MOIST,

    // Sensor error
    ST_NO_SENSOR,
    ST_UNIT_C_SHORT,
    ST_UNIT_F_SHORT,
    ST_LUX_UNIT,
    ST_RAW_ADC,
    ST_LOG_PCT,

    // Soil calibration
    ST_SOIL_CAL_DRY,
    ST_SOIL_CAL_WET,
    ST_SOIL_PUSH_CAPTURE,
    ST_SOIL_CAL_SAVED,
    ST_SOIL_CAL_ERROR,
    ST_PUSH_EXIT,
    ST_SOIL_DRY_REF,
    ST_SOIL_WET_REF,
    ST_SOIL_MENU_CAL,
    ST_SOIL_MENU_THRESH,
    ST_SOIL_MENU_BACK,
    ST_SOIL_THRESH_SAVED,
    ST_SOIL_TURN_ADJUST,

    // Timer status
    ST_TIMER_RDY,
    ST_TIMER_RUN,
    ST_TIMER_PAU,

    // Timer instructions
    ST_PUSH_START,
    ST_PUSH_PAUSE,
    ST_PUSH_RESET,
    ST_TIMER_MINUTES,
    ST_TIMER_DURATION,
    ST_TIMER_STOPWATCH,
    ST_TIMER_CFG_SELECT,
    ST_TIMER_CFG_EDIT,

    // Shared menu strings
    MENU_SAVED,
    MENU_RESET,
    MENU_ALERTS,
    MENU_EXIT,
    MENU_NO,
    MENU_YES,
    MENU_DEFAULTS,
    MENU_ERROR,
    MENU_LIMITS,
    MENU_CALIBRATION,
    MENU_UNIT,
    MENU_RESET_DONE,
    MENU_LOW,
    MENU_HIGH,
    MENU_LIMIT_LOW,
    MENU_LIMIT_HIGH,
    MENU_UNIT_F,
    MENU_UNIT_C,

    // System menu
    MENU_SETTINGS,
    MENU_SOUND,
    MENU_SLEEP,
    MENU_NEVER,
    MENU_SLEEP_30S,
    MENU_SLEEP_1M,
    MENU_SLEEP_2M,
    MENU_SLEEP_5M,
    MENU_SLEEP_10M,
    MENU_FULL_RESET,
    MENU_RESTORE_ALL,
    MENU_TO_DEFAULTS,

    // Light menu
    MENU_DISPLAY_MODE,
    MENU_LIGHT_MAX_DIM,
    MENU_LIGHT_MAX_INDOOR,
    MENU_LIGHT_MAX_BRIGHT,
    MENU_RESET_SUB_LIGHT,
    MENU_LIGHT_ABR_DIM,
    MENU_LIGHT_ABR_IN,
    MENU_LIGHT_ABR_BRIGHT,

    // Sound menu
    MENU_SND_MAX_QUIET,
    MENU_SND_MAX_NORMAL,
    MENU_SND_MAX_LOUD,
    MENU_RESET_SUB_SOUND,
    MENU_SND_ABR_QUIET,
    MENU_SND_ABR_NORMAL,
    MENU_SND_ABR_LOUD,

    // Soil menu
    MENU_SOIL_SENSOR_LIMITS,
    MENU_RESET_SUB_SOIL,
    MENU_RESTORED,
    MENU_PUSH_CAPTURE,

    // DS18 / temperature menu
    MENU_OFFSET,
    MENU_RESET_SUB_TEMP,
    MENU_RESET_SUB_PROBE,
    MENU_RESET_SUB_HUM,

    // Language menu: dynamic title and names
    MENU_TITLE,
    LANG_ES_NAME,
    LANG_CAT_NAME,
    LANG_EN_NAME,

    // Graph screen
    TIT_GRAPH,
    GRAPH_PUSH_SENSOR,
    ST_WAITING,

    // Temporary lab screens
    TIT_LAB_DASH,
    TIT_LAB_FOCUS,
    TIT_LAB_DUAL_TH,
    TIT_LAB_ICON_A,
    TIT_LAB_ICON_B,
    TIT_LAB_ICON_C,
    TIT_LAB_GAUGE,
    TIT_LAB_VALUE,
    TIT_LAB_WIDGETS,
    LAB_COMPARE_HINT,
    LAB_EXPERIMENT_HINT,
    LAB_TEMP_SHORT,
    LAB_HUM_SHORT,
    LAB_LIGHT_SHORT,
    LAB_SOUND_SHORT,
    LAB_SOIL_SHORT,
    LAB_PROBE_SHORT,
    TIT_LAB_ICON_SZ_ENV,
    TIT_LAB_ICON_SZ_EXT,
    TIT_LAB_HOME_CARDS,
    TIT_LAB_LINEAR_DASH,

    LANG_KEY_COUNT  // Centinela — debe ser el último
};

// Active language (defined in lang_select.cpp).
extern Language g_language;

// Returns the translated string for the requested key.
const char* L(LangKey key);
