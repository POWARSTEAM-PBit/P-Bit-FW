#include "ui_lab_sensor_cards.h"

#include "alert_engine.h"
#include "fonts.h"
#include "hw.h"
#include "languages.h"
#include "layout.h"
#include "runtime_events.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern bool g_is_fahrenheit;
extern uint16_t getTempColor(float temp);

namespace {

constexpr uint16_t kBg     = TFT_BLACK;
constexpr uint16_t kCardBg = 0x1082;

constexpr int kCardX = LC_SCREEN_X;
constexpr int kCardY = LC_CARD_TOP;
constexpr int kCardW = LC_SCREEN_W;
constexpr int kCardH = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;

// Left panel (value area): x=2..117
constexpr int kValueCx = 60;   // center of value text (left of rail)
constexpr int kValueY  = kCardY + 18;

// Unit / bottom text
constexpr int kUnitCx  = kValueCx;
constexpr int kUnitY   = kCardY + 74;

// Right rail: 34×82 px vertical indicator
constexpr int kRailX = 120;
constexpr int kRailY = 32;
constexpr int kRailW = 34;
constexpr int kRailH = 84;

// Alert jewel sits below the value area, left-aligned
constexpr int kAlertJewelX = kCardX + 10;
constexpr int kAlertJewelY = kCardY + 86;

// Small sensor icon, centered between value and unit text
constexpr int kIconCy = kCardY + 60;  // = 87

// ── Canonical secondary palette (matches GaugeSpec secondaries) ──────
constexpr uint16_t kTempSecondary  = 0xF81F;  // magenta/hot-pink
constexpr uint16_t kHumSecondary   = 0x881F;  // deep purple
constexpr uint16_t kLightSecondary = 0xFD20;  // warm orange (contrasts with yellow)
constexpr uint16_t kSoundSecondary = 0x3FE8;  // neon green
constexpr uint16_t kSoilSecondary  = 0x35FF;  // electric blue
// DS18 shares TEMP secondary

// ── Primary (normal-state) accent per sensor ─────────────────────────
constexpr uint16_t kHumPrimary   = 0x069F;  // cian eléctrico
constexpr uint16_t kLightPrimary = 0xFFE0;  // amarillo
constexpr uint16_t kSoundPrimary = 0xF81F;  // magenta punk
constexpr uint16_t kSoilPrimary  = 0x2F85;  // verde cálido

// ── Enums and structs ─────────────────────────────────────────────────

enum LabSensorCardId : uint8_t {
    CARD_TEMP = 0,
    CARD_DS18,
    CARD_HUM,
    CARD_LIGHT,
    CARD_SOUND,
    CARD_SOIL,
    CARD_COUNT
};

struct CardCache {
    bool valid       = false;
    bool sensor_valid = false;
    int  value_key   = INT_MIN;
    uint8_t alert_code = ALERT_CODE_OFF;
    bool unit_mode   = false;
    bool alerts_enabled = false;
};

struct CardRenderState {
    bool    sensor_valid  = false;
    float   temp_c        = 0.0f;   // raw sensor value (°C for temp, unit value for others)
    float   shown_value   = 0.0f;   // display value (after C/F conversion for temp)
    int     value_key     = INT_MIN;
    uint8_t alert_code    = ALERT_CODE_OFF;
    bool    alerts_enabled = false;
    uint16_t accent       = TFT_DARKGREY;  // primary accent (varies with alert state)
    AlertJewelState jewel = ALERT_JEWEL_OFF;
};

struct LabSensorCardSpec {
    LabSensorCardId id;
    LangKey         title_key;
    const char*     device_label;
    uint16_t        secondary;
    LangKey         invalid_bottom_key;
    uint16_t        invalid_bottom_color;
    AlertSensor     alert_sensor;
    bool            is_temperature;
    bool   (*is_valid)(const Reading&);
    float  (*value_c)(const Reading&);
    bool   (*alerts_enabled)();
    const char* (*unit_fn)();
    uint16_t (*accent)(bool, float, uint8_t);
    void (*draw_rail)(bool, float, uint16_t);
    void (*icon_fn)(int, int, uint16_t);
};

static CardCache       g_card_cache[CARD_COUNT];
static LabSensorCardId g_selected_card = CARD_TEMP;

// ── Unit and conversion helpers ───────────────────────────────────────

static float to_display(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static const char* unit_temp() {
    return g_is_fahrenheit ? L(MENU_UNIT_F) : L(MENU_UNIT_C);
}
static const char* unit_pct() { return "%"; }
static const char* unit_lux() { return L(ST_LUX_UNIT); }

// ── Validity and value helpers ────────────────────────────────────────

static bool  temp_is_valid(const Reading& r)  { return !isnan(r.temperature); }
static float temp_value_c(const Reading& r)   { return r.temperature; }

static bool  ds18_is_valid(const Reading& r)  { return r.temp_ds18b20 >= -100.0f; }
static float ds18_value_c(const Reading& r)   { return r.temp_ds18b20; }

static bool  hum_is_valid(const Reading& r)   { return !isnan(r.humidity); }
static float hum_value_c(const Reading& r)    { return r.humidity; }

static bool  light_is_valid(const Reading& r) { return !isnan(r.ldr); }
static float light_value_c(const Reading& r)  { return r.ldr; }

static bool  sound_is_valid(const Reading& r) { return !isnan(r.mic); }
static float sound_value_c(const Reading& r)  { return r.mic; }

static bool  soil_is_valid(const Reading& r)  { return !isnan(r.soil_humidity); }
static float soil_value_c(const Reading& r)   { return r.soil_humidity; }

// ── Accent (primary color) functions ─────────────────────────────────

static uint16_t temp_accent(bool valid, float temp_c, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW)  return TFT_BLUE;
    if (alert_code == ALERT_CODE_HIGH) return TFT_RED;
    return getTempColor(temp_c);
}

static uint16_t ds18_accent(bool valid, float temp_c, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW)  return TFT_BLUE;
    if (alert_code == ALERT_CODE_HIGH) return TFT_RED;
    return (temp_c < 0.0f) ? TFT_CYAN : getTempColor(temp_c);
}

static uint16_t hum_accent(bool valid, float, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW)  return TFT_BLUE;
    if (alert_code == ALERT_CODE_HIGH || alert_code == ALERT_CODE_CRITICAL) return TFT_RED;
    return kHumPrimary;
}

static uint16_t light_accent(bool valid, float, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_HIGH || alert_code == ALERT_CODE_CRITICAL) return TFT_RED;
    return kLightPrimary;
}

static uint16_t sound_accent(bool valid, float, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_HIGH || alert_code == ALERT_CODE_CRITICAL) return TFT_RED;
    return kSoundPrimary;
}

static uint16_t soil_accent(bool valid, float, uint8_t alert_code) {
    if (!valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW)  return TFT_RED;
    if (alert_code == ALERT_CODE_HIGH || alert_code == ALERT_CODE_MOIST) return TFT_BLUE;
    return kSoilPrimary;
}

// ── Alert jewel helpers ───────────────────────────────────────────────

static AlertJewelState jewel_state(bool sv, bool alerts_en, uint8_t code) {
    if (!sv || !alerts_en) return ALERT_JEWEL_OFF;
    if (code == ALERT_CODE_LOW) return ALERT_JEWEL_WARN;
    if (code == ALERT_CODE_HIGH || code == ALERT_CODE_CRITICAL) return ALERT_JEWEL_CRIT;
    return ALERT_JEWEL_OK;
}

static uint16_t jewel_color(AlertJewelState state, uint16_t accent) {
    switch (state) {
        case ALERT_JEWEL_OFF:  return TFT_DARKGREY;
        case ALERT_JEWEL_WARN: return (accent == TFT_BLUE) ? TFT_BLUE : TFT_ORANGE;
        case ALERT_JEWEL_CRIT: return TFT_RED;
        case ALERT_JEWEL_OK:
        default:               return TFT_GREEN;
    }
}

// ── Shell helpers ─────────────────────────────────────────────────────

static void draw_lab_card_shell(const char* title) {
    tft.fillScreen(kBg);
    drawHeader(title);
}

static void draw_card_frame(uint16_t accent) {
    tft.fillRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, kCardBg);
    tft.drawRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, accent);
}

// Device label uses secondary color for visual identity
static void draw_device_label(const char* label, uint16_t color) {
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, kCardBg);
    tft.drawString(label, kCardX + 8, kCardY + 4);
    tft.setTextFont(0);
}

static void draw_bottom_line(const char* text, uint16_t color) {
    tft.fillRect(kCardX + 4, kCardY + 64, 110, 22, kCardBg);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(color, kCardBg);
    if (tft.textWidth(text) > 100) {
        tft.setFreeFont(FONT_SMALL);
    }
    tft.drawString(text, kUnitCx, kUnitY);
    tft.setTextFont(0);
}

static void draw_big_value(bool sensor_valid, float shown_value, uint16_t accent,
                           bool is_temperature) {
    tft.fillRect(kCardX + 4, kCardY + 14, 110, 52, kCardBg);
    if (sensor_valid) {
        if (is_temperature) {
            drawSplitDecimalValue(shown_value, kValueCx, kValueY, accent, kCardBg);
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%.0f", shown_value);
            tft.setTextDatum(TC_DATUM);
            tft.setFreeFont(FONT_VALUE);
            tft.setTextColor(accent, kCardBg);
            tft.drawString(buf, kValueCx, kValueY);
            tft.setTextFont(0);
        }
        return;
    }
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_VALUE);
    tft.setTextColor(TFT_DARKGREY, kCardBg);
    tft.drawString("---", kValueCx, kValueY);
    tft.setTextFont(0);
}

// ── Rail draw functions ───────────────────────────────────────────────

// TEMP: gradient thermometer (cold=blue → hot=warm)
static uint16_t temp_gradient_color(float ratio) {
    ratio = constrain(ratio, 0.0f, 1.0f);
    const int r = (int)roundf(64.0f  + ratio * (255.0f - 64.0f));
    const int g = (int)roundf(120.0f + ratio * (32.0f - 120.0f));
    const int b = (int)roundf(255.0f + ratio * (40.0f - 255.0f));
    return tft.color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

static void draw_temp_rail(bool sv, float temp_c, uint16_t accent) {
    const int inner_h = kRailH - 2;
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sv) {
        const float clamped = constrain(temp_c, 0.0f, 50.0f);
        const int fill_px   = (int)roundf((clamped / 50.0f) * inner_h);
        for (int row = 0; row < fill_px; ++row) {
            const float ratio = (float)row / (float)max(1, inner_h - 1);
            tft.drawFastHLine(kRailX + 1, kRailY + 1 + (inner_h - 1 - row),
                              kRailW - 2, temp_gradient_color(ratio));
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
}

// DS18B20: fill above/below zero, with zero reference line
static void draw_probe_rail(bool sv, float temp_c, uint16_t accent) {
    const int inner_h  = kRailH - 2;
    const int zero_fill = (int)(55.0f / 180.0f * inner_h);
    const int zero_y   = kRailY + 1 + (inner_h - zero_fill);
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, kRailH - 2, kCardBg);
    if (sv) {
        const float norm    = constrain((temp_c + 55.0f) / 180.0f, 0.0f, 1.0f);
        const int fill_px   = (int)(norm * inner_h);
        const int empty_px  = inner_h - fill_px;
        if (temp_c >= 0.0f) {
            const int pos_fill = fill_px - zero_fill;
            if (pos_fill > 0)
                tft.fillRect(kRailX+1, kRailY+1+empty_px, kRailW-2, pos_fill, accent);
            tft.fillRect(kRailX+1, zero_y, kRailW-2, zero_fill, TFT_BLUE);
        } else if (fill_px > 0) {
            tft.fillRect(kRailX+1, kRailY+1+empty_px, kRailW-2, fill_px, TFT_BLUE);
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
    tft.drawFastHLine(kRailX - 2, zero_y, kRailW + 4, TFT_WHITE);
}

// HUM: segmented blocks (10 blocks, fills from bottom)
static void draw_hum_rail(bool sv, float value, uint16_t accent) {
    const int inner_h = kRailH - 2;
    constexpr int kBlocks = 10;
    const int block_h = (inner_h - (kBlocks - 1)) / kBlocks;
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sv) {
        const int filled = (int)roundf(constrain(value, 0.0f, 100.0f) / 100.0f * kBlocks);
        for (int i = 0; i < filled; i++) {
            const int by = kRailY + 1 + inner_h - (i + 1) * (block_h + 1);
            tft.fillRect(kRailX + 1, by, kRailW - 2, block_h, accent);
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
}

// LIGHT: gradient dark-grey (low lux) → bright yellow (high lux)
static void draw_light_rail(bool sv, float value, uint16_t accent) {
    const int inner_h = kRailH - 2;
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sv) {
        const int fill_px = (int)roundf(constrain(value / 1023.0f, 0.0f, 1.0f) * inner_h);
        for (int row = 0; row < fill_px; ++row) {
            const float ratio = (float)row / (float)max(1, inner_h - 1);
            const uint8_t r = (uint8_t)roundf(ratio * 255.0f);
            const uint8_t g = (uint8_t)roundf(ratio * 255.0f);
            const uint8_t b = (uint8_t)roundf(ratio * 20.0f);
            tft.drawFastHLine(kRailX + 1, kRailY + 1 + (inner_h - 1 - row),
                              kRailW - 2, tft.color565(r, g, b));
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
}

// SOUND: 3-zone VU (verde→naranja→rojo desde abajo)
static void draw_sound_rail(bool sv, float value, uint16_t accent) {
    const int inner_h = kRailH - 2;
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sv) {
        const int fill_px = (int)roundf(constrain(value / 100.0f, 0.0f, 1.0f) * inner_h);
        for (int row = 0; row < fill_px; ++row) {
            const float pos = (float)row / (float)inner_h;
            uint16_t seg_color;
            if      (pos < 0.45f) seg_color = 0x27E0;   // verde ácido
            else if (pos < 0.75f) seg_color = 0xFD20;   // naranja
            else                  seg_color = 0xF800;   // rojo alerta
            tft.drawFastHLine(kRailX + 1, kRailY + 1 + (inner_h - 1 - row),
                              kRailW - 2, seg_color);
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
}

// SOIL: zona de color según humedad + líneas de umbral (seco/saturado)
static void draw_soil_rail(bool sv, float value, uint16_t accent) {
    const int inner_h = kRailH - 2;
    // Threshold reference lines at 20% and 80%
    const int dry_y = kRailY + 1 + (int)roundf((1.0f - 0.20f) * inner_h);
    const int wet_y = kRailY + 1 + (int)roundf((1.0f - 0.80f) * inner_h);
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sv) {
        const int fill_px = (int)roundf(constrain(value / 100.0f, 0.0f, 1.0f) * inner_h);
        uint16_t fill_color;
        if      (value < 20.0f)  fill_color = TFT_RED;
        else if (value > 80.0f)  fill_color = TFT_BLUE;
        else                     fill_color = accent;
        tft.fillRect(kRailX + 1, kRailY + 1 + (inner_h - fill_px), kRailW - 2, fill_px, fill_color);
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sv ? accent : TFT_DARKGREY);
    // Threshold markers (drawn on top of fill)
    tft.drawFastHLine(kRailX - 2, dry_y, kRailW + 4, TFT_RED);
    tft.drawFastHLine(kRailX - 2, wet_y, kRailW + 4, TFT_BLUE);
}

// ── Spec array ────────────────────────────────────────────────────────

static const LabSensorCardSpec kCardSpecs[CARD_COUNT] = {
    {
        CARD_TEMP, TIT_LAB_TEMP_CARD, "DHT11", kTempSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Temp, true,
        temp_is_valid, temp_value_c, get_temp_alerts_enabled, unit_temp,
        temp_accent, draw_temp_rail, pbit_draw_temp_icon
    },
    {
        CARD_DS18, TIT_LAB_PROBE_CARD, "DS18B20", kTempSecondary,
        ST_CHECK_DS18, TFT_CYAN, AlertSensor::Ds18, true,
        ds18_is_valid, ds18_value_c, get_ds18_alerts_enabled, unit_temp,
        ds18_accent, draw_probe_rail, pbit_draw_probe_icon
    },
    {
        CARD_HUM, TIT_LAB_HUM_CARD, "DHT11", kHumSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Humidity, false,
        hum_is_valid, hum_value_c, get_humidity_alerts_enabled, unit_pct,
        hum_accent, draw_hum_rail, pbit_draw_humidity_icon
    },
    {
        CARD_LIGHT, TIT_LAB_LIGHT_CARD, "LDR", kLightSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Light, false,
        light_is_valid, light_value_c, get_light_alerts_enabled, unit_lux,
        light_accent, draw_light_rail, pbit_draw_light_icon
    },
    {
        CARD_SOUND, TIT_LAB_SOUND_CARD, "MIC", kSoundSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Sound, false,
        sound_is_valid, sound_value_c, get_sound_alerts_enabled, unit_pct,
        sound_accent, draw_sound_rail, pbit_draw_sound_icon
    },
    {
        CARD_SOIL, TIT_LAB_SOIL_CARD, "SOIL", kSoilSecondary,
        ST_CHECK_SOIL, TFT_DARKGREY, AlertSensor::Soil, false,
        soil_is_valid, soil_value_c, get_soil_alerts_enabled, unit_pct,
        soil_accent, draw_soil_rail, pbit_draw_plant_icon
    }
};

static const LabSensorCardSpec& spec_for(LabSensorCardId id) {
    return kCardSpecs[(id < CARD_COUNT) ? id : CARD_TEMP];
}

// ── State / cache ─────────────────────────────────────────────────────

static CardRenderState read_state(const LabSensorCardSpec& spec) {
    CardRenderState state;
    state.sensor_valid = spec.is_valid(g_ui_readings_snapshot);
    state.temp_c       = state.sensor_valid ? spec.value_c(g_ui_readings_snapshot) : 0.0f;
    if (spec.is_temperature) {
        state.shown_value = state.sensor_valid ? to_display(state.temp_c) : 0.0f;
        state.value_key   = state.sensor_valid ? (int)lroundf(state.shown_value * 10.0f) : INT_MIN;
    } else {
        state.shown_value = state.temp_c;
        state.value_key   = state.sensor_valid ? (int)lroundf(state.temp_c) : INT_MIN;
    }
    state.alert_code    = alert_engine_get_code(spec.alert_sensor);
    state.alerts_enabled = spec.alerts_enabled();
    state.accent = spec.accent(state.sensor_valid, state.temp_c, state.alert_code);
    state.jewel  = jewel_state(state.sensor_valid, state.alerts_enabled, state.alert_code);
    return state;
}

static bool cache_dirty(const CardCache& cache, const CardRenderState& state) {
    return !cache.valid
        || (cache.sensor_valid  != state.sensor_valid)
        || (cache.value_key     != state.value_key)
        || (cache.alert_code    != state.alert_code)
        || (cache.unit_mode     != g_is_fahrenheit)
        || (cache.alerts_enabled != state.alerts_enabled);
}

static void commit_cache(CardCache& cache, const CardRenderState& state) {
    cache.valid          = true;
    cache.sensor_valid   = state.sensor_valid;
    cache.value_key      = state.value_key;
    cache.alert_code     = state.alert_code;
    cache.unit_mode      = g_is_fahrenheit;
    cache.alerts_enabled = state.alerts_enabled;
}

// ── Dynamic draw ──────────────────────────────────────────────────────

static void draw_card_dynamic(const LabSensorCardSpec& spec, const CardRenderState& state) {
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);
    draw_card_frame(state.accent);

    // Device label in secondary color for visual identity
    draw_device_label(spec.device_label, spec.secondary);

    // Big value (handles "---" when invalid; no separate hint_line to avoid overlaps)
    draw_big_value(state.sensor_valid, state.shown_value, state.accent, spec.is_temperature);

    // Unit / error message below value — secondary color when valid
    draw_bottom_line(
        state.sensor_valid ? spec.unit_fn() : L(spec.invalid_bottom_key),
        state.sensor_valid ? spec.secondary : spec.invalid_bottom_color
    );

    drawAlertJewel(kAlertJewelX, kAlertJewelY,
                   state.jewel, jewel_color(state.jewel, state.accent));

    spec.draw_rail(state.sensor_valid, state.temp_c, state.accent);

    // Icon drawn last — after all fillRect clears — so nothing erases it
    spec.icon_fn(kValueCx, kIconCy, state.accent);
}

static void draw_card_screen_for(LabSensorCardId id,
                                 bool screen_changed,
                                 bool sensor_data_changed,
                                 bool redraw_shell_when_cache_invalid) {
    const LabSensorCardSpec& spec = spec_for(id);
    CardCache& cache = g_card_cache[id];
    const CardRenderState state = read_state(spec);
    const bool dirty = cache_dirty(cache, state);
    const bool needs_shell = screen_changed || (redraw_shell_when_cache_invalid && !cache.valid);

    if (needs_shell) {
        draw_lab_card_shell(L(spec.title_key));
        draw_card_dynamic(spec, state);
        commit_cache(cache, state);
        return;
    }

    if (dirty && (sensor_data_changed || !cache.valid)) {
        draw_card_dynamic(spec, state);
        commit_cache(cache, state);
    }
}

} // namespace

// ── Public API ────────────────────────────────────────────────────────

void lab_sensor_card_cycle() {
    g_selected_card = (LabSensorCardId)(((uint8_t)g_selected_card + 1) % (uint8_t)CARD_COUNT);
    g_card_cache[g_selected_card].valid = false;
    runtime_request_ui_full_redraw();
}

void draw_lab_sensor_card_screen(bool screen_changed, bool sensor_data_changed) {
    draw_card_screen_for(g_selected_card, screen_changed, sensor_data_changed, true);
}

void draw_lab_temp_card_screen(bool screen_changed, bool sensor_data_changed) {
    draw_card_screen_for(CARD_TEMP, screen_changed, sensor_data_changed, false);
}

void draw_lab_ds18_card_screen(bool screen_changed, bool sensor_data_changed) {
    draw_card_screen_for(CARD_DS18, screen_changed, sensor_data_changed, false);
}
