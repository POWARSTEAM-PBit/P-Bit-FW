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

constexpr int kCardX = LC_SCREEN_X;   // = 2
constexpr int kCardY = LC_CARD_TOP;   // = 27
constexpr int kCardW = LC_SCREEN_W;   // = 156
constexpr int kCardH = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;  // = 100

// ── Layout zones ──────────────────────────────────────────────────────
// Header strip  y = 27..43  (17 px): icon + device label + jewel
// Separator     y = 44      (1 px)
// Value zone    y = 46..86  (41 px): large value + compact unit
// Viz labels    y = 98..104
// Viz bars      y = 109..124 (bottom = card bottom - 2)

constexpr int kIconCx    = kCardX + 11;          // = 13
constexpr int kIconCy    = kCardY + 12;          // = 39

constexpr int kDevLabelX = kCardX + 23;           // = 25 (TL datum)
constexpr int kDevLabelY = kCardY + 4;            // = 31

constexpr int kJewelPadRight = 10;
constexpr int kAlertJewelX = kCardX + kCardW - 8; // = 150
constexpr int kAlertJewelY = kCardY + 8;          // = 35
constexpr int kHeaderRightTextX = kAlertJewelX - 8;

constexpr int kSepY1     = kCardY + 19;           // = 46
constexpr int kValueTopY = kCardY + 25;           // = 52
constexpr int kValueRightX = kCardX + 110;        // = 112
constexpr int kUnitValueX = kCardX + 114;         // = 116
constexpr int kUnitTopY  = kCardY + 37;           // = 64
constexpr int kInvalidValueY = kCardY + 21;       // = 48
constexpr int kSepY2     = kCardY + 57;           // = 84

constexpr int kVizLabelY = kCardY + 60;           // = 87
constexpr int kVizX      = kCardX + 6;            // = 8
constexpr int kVizW      = kCardW - 12;           // = 144
constexpr int kVizBottomY = kCardY + kCardH - 3;  // = 124
constexpr int kVizTrackTopY = kCardY + 82;        // = 109
constexpr int kVizTrackH = kVizBottomY - kVizTrackTopY + 1; // = 16

// ── Canonical secondary palette ───────────────────────────────────────
constexpr uint16_t kTempSecondary  = 0xF81F;
constexpr uint16_t kHumSecondary   = 0x881F;
constexpr uint16_t kLightSecondary = 0xFD20;
constexpr uint16_t kSoundSecondary = 0x3FE8;
constexpr uint16_t kSoilSecondary  = 0x35FF;

constexpr uint16_t kHumPrimary   = 0x069F;
constexpr uint16_t kLightPrimary = 0xFFE0;
constexpr uint16_t kSoundPrimary = 0xF81F;
constexpr uint16_t kSoilPrimary  = 0x2F85;

constexpr uint16_t kVizTrack = 0x1084;
constexpr uint16_t kSepColor = 0x2104;

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
    bool valid        = false;
    bool sensor_valid = false;
    int  value_key    = INT_MIN;
    uint8_t alert_code  = ALERT_CODE_OFF;
    bool unit_mode    = false;
    bool alerts_enabled = false;
};

struct CardRenderState {
    bool    sensor_valid  = false;
    float   temp_c        = 0.0f;
    float   shown_value   = 0.0f;
    int     value_key     = INT_MIN;
    uint8_t alert_code    = ALERT_CODE_OFF;
    bool    alerts_enabled = false;
    uint16_t accent       = TFT_DARKGREY;
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
    const char* (*compact_unit_fn)();
    uint16_t (*accent)(bool, float, uint8_t);
    void (*draw_viz)(bool, float, uint16_t);
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

static const char* unit_temp_compact() {
    return g_is_fahrenheit ? "F" : "C";
}

static const char* unit_lux_compact() { return "lux"; }
static const char* unit_pct_compact() { return "%"; }

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

// ── Shell helper ──────────────────────────────────────────────────────

static void draw_lab_card_shell(const char* title) {
    tft.fillScreen(kBg);
    drawHeader(title);
}

// ── Header strip (icon + device label + unit) ─────────────────────────

static void draw_header_strip(const char* device_label, uint16_t secondary,
                              const char* status_str, bool sensor_valid) {
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(secondary, kCardBg);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(device_label, kDevLabelX, kDevLabelY);

    if (status_str && status_str[0] != '\0') {
        tft.setTextColor(sensor_valid ? 0x8C71 : TFT_DARKGREY, kCardBg);
        tft.setTextDatum(TR_DATUM);
        tft.drawString(status_str, kHeaderRightTextX, kDevLabelY);
    }
    tft.setTextFont(0);

    tft.drawFastHLine(kCardX + 4, kSepY1, kCardW - 8, kSepColor);
}

// ── Value zone (large value + compact unit) ──────────────────────────

static void draw_value_compact(bool sensor_valid,
                               float shown_value,
                               uint16_t accent,
                               bool is_temperature,
                               const char* compact_unit,
                               const char* invalid_str) {
    if (!sensor_valid) {
        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_VALUE);
        tft.setTextColor(TFT_DARKGREY, kCardBg);
        tft.drawString("---", kCardX + kCardW / 2, kInvalidValueY);

        tft.setTextDatum(TC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, kCardBg);
        tft.drawString(invalid_str, kCardX + kCardW / 2, kInvalidValueY + 30);
        tft.setTextFont(0);
        return;
    }

    char value_str[12];
    if (is_temperature) {
        snprintf(value_str, sizeof(value_str), "%.1f", shown_value);
    } else {
        snprintf(value_str, sizeof(value_str), "%.0f", shown_value);
    }

    tft.setFreeFont(FONT_VALUE);
    const int value_w = tft.textWidth(value_str);

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(accent, kCardBg);
    tft.drawString(value_str, kValueRightX - value_w, kValueTopY);

    tft.setTextColor(0x8C71, kCardBg);
    tft.setFreeFont(FONT_BODY);
    tft.drawString(compact_unit, kUnitValueX, kUnitTopY);
    tft.setTextFont(0);

    tft.drawFastHLine(kCardX + 4, kSepY2, kCardW - 8, kSepColor);
}

// ── Visualization functions — horizontal, per-sensor identity ─────────

// TEMP: horizontal gradient thermometer, 0°..50°C
static void draw_temp_viz(bool sv, float temp_c, uint16_t accent) {
    constexpr int segs = 12;
    const int gap  = 1;
    const int sw   = (kVizW - (segs - 1) * gap) / segs;
    const int bh   = 14;
    const int by   = kVizBottomY - bh + 1;

    const float norm = sv ? constrain(temp_c, 0.0f, 50.0f) / 50.0f : 0.0f;
    const int filled = (int)roundf(norm * segs);

    for (int i = 0; i < segs; i++) {
        const int sx = kVizX + i * (sw + gap);
        uint16_t c;
        if (!sv || i >= filled) {
            c = kVizTrack;
        } else {
            const float t = (float)i / (float)(segs - 1);
            c = tft.color565(
                (uint8_t)roundf(40 + t * 215),
                (uint8_t)roundf(190 - t * 150),
                (uint8_t)roundf(240 - t * 220));
        }
        tft.fillRoundRect(sx, by, sw, bh, 2, c);
        // Top pixel brighter for filled segments (highlight)
        if (sv && i < filled) {
            tft.drawFastHLine(sx + 1, by + 1, sw - 2, tft.color565(
                (uint8_t)min(255, (int)roundf(80 + (float)i / segs * 215)),
                (uint8_t)max(0, (int)roundf(220 - (float)i / segs * 180)),
                255 - i * 18));
        }
    }

    // End labels
    tft.setFreeFont(FONT_SMALL);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(0x6B6D, kCardBg);
    tft.drawString("0\xb0", kVizX, kVizLabelY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("50\xb0", kVizX + kVizW, kVizLabelY);
    tft.setTextFont(0);
}

// DS18B20: horizontal bar -55..+125°C, zero reference line, icy/warm split
static void draw_probe_viz(bool sv, float temp_c, uint16_t accent) {
    constexpr int segs = 14;
    constexpr float kMin = -55.0f;
    constexpr float kMax = 125.0f;
    constexpr float kRange = kMax - kMin;  // 180
    const int gap = 1;
    const int sw  = (kVizW - (segs - 1) * gap) / segs;
    const int bh  = 14;
    const int by  = kVizBottomY - bh + 1;

    // 0°C lands at seg index = (55/180)*14 = 4.28 → seg 4
    const int zero_seg = (int)roundf((55.0f / kRange) * segs);

    const float norm   = sv ? constrain((temp_c - kMin) / kRange, 0.0f, 1.0f) : 0.0f;
    const int   filled = (int)roundf(norm * segs);

    for (int i = 0; i < segs; i++) {
        const int sx = kVizX + i * (sw + gap);
        uint16_t c;
        if (!sv || i >= filled) {
            c = kVizTrack;
        } else if (i < zero_seg) {
            const float t = (float)i / (float)max(1, zero_seg - 1);
            c = tft.color565((uint8_t)roundf(20 + t * 30),
                             (uint8_t)roundf(120 + t * 80),
                             (uint8_t)roundf(200 + t * 55));
        } else {
            const float t = (float)(i - zero_seg) / (float)max(1, segs - zero_seg - 1);
            c = tft.color565((uint8_t)roundf(40 + t * 215),
                             (uint8_t)roundf(190 - t * 150),
                             (uint8_t)roundf(240 - t * 220));
        }
        tft.fillRoundRect(sx, by, sw, bh, 2, c);
    }

    // Zero reference tick (white vertical line between segs)
    const int zero_x = kVizX + zero_seg * (sw + gap) - 1;
    tft.drawFastVLine(zero_x, by - 2, bh + 4, TFT_WHITE);

    // End labels
    tft.setFreeFont(FONT_SMALL);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(0x6B6D, kCardBg);
    tft.drawString("-55\xb0", kVizX, kVizLabelY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("+125\xb0", kVizX + kVizW, kVizLabelY);
    tft.setTextFont(0);
}

// HUM: bubble drops — N rounded squares in a row, cyan gradient when filled
static void draw_hum_viz(bool sv, float value, uint16_t accent) {
    constexpr int segs = 10;
    const int gap = 3;
    const int sw  = (kVizW - (segs - 1) * gap) / segs;
    const int bh  = 12;
    const int by  = kVizBottomY - bh + 1;

    const int filled = sv ? (int)roundf(constrain(value, 0.0f, 100.0f) / 100.0f * segs) : 0;

    for (int i = 0; i < segs; i++) {
        const int sx = kVizX + i * (sw + gap);
        uint16_t c;
        if (i < filled) {
            const float t = (float)i / (float)(segs - 1);
            c = tft.color565(
                (uint8_t)roundf(10 + t * 30),
                (uint8_t)roundf(160 + t * 80),
                (uint8_t)roundf(200 + t * 55));
        } else {
            c = kVizTrack;
        }
        // Rounder top pill to suggest droplets
        tft.fillRoundRect(sx, by, sw, bh, sw / 2, c);
        // Highlight dot inside filled segments
        if (i < filled && sw > 6) {
            tft.fillCircle(sx + sw / 2 - 1, by + 4, 1,
                tft.color565(100, 220, 255));
        }
    }

    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(0x6B6D, kCardBg);
    tft.setTextDatum(TL_DATUM);
    tft.drawString("0%", kVizX, kVizLabelY);
    tft.setTextDatum(TR_DATUM);
    tft.drawString("100%", kVizX + kVizW, kVizLabelY);
    tft.setTextFont(0);
}

// LIGHT: 8 radiance bars of increasing height, dark → bright yellow
static void draw_light_viz(bool sv, float value, uint16_t accent) {
    constexpr int bars = 8;
    const int gap = 4;
    const int bw  = (kVizW - (bars - 1) * gap) / bars;
    const float norm = sv ? constrain(value, 0.0f, 1023.0f) / 1023.0f : 0.0f;
    const int max_h  = kVizTrackH;
    const int base_y = kVizBottomY;

    for (int i = 0; i < bars; i++) {
        const float bar_t  = (float)(i + 1) / (float)bars;
        // Bar only appears if level >= bar threshold
        const bool lit     = sv && norm >= (bar_t - 1.0f / bars);
        const int  bar_h   = (int)roundf(bar_t * max_h);
        const int  sx      = kVizX + i * (bw + gap);
        const int  sy      = base_y - bar_h;
        uint16_t c;
        if (lit) {
            c = tft.color565(
                (uint8_t)roundf(50  + bar_t * 205),
                (uint8_t)roundf(50  + bar_t * 205),
                (uint8_t)roundf(bar_t * 20));
        } else {
            c = kVizTrack;
        }
        tft.fillRoundRect(sx, sy, bw, bar_h, 2, c);
    }
}

// SOUND: 7 VU columns, all same height = level, colored green/orange/red
static void draw_sound_viz(bool sv, float value, uint16_t accent) {
    constexpr int cols = 7;
    const int gap  = 3;
    const int cw   = (kVizW - (cols - 1) * gap) / cols;
    const float norm = sv ? constrain(value, 0.0f, 100.0f) / 100.0f : 0.0f;
    const int max_h  = kVizTrackH;
    const int base_y = kVizBottomY;
    const int bar_h  = (int)roundf(norm * max_h);

    for (int i = 0; i < cols; i++) {
        const int sx = kVizX + i * (cw + gap);
        // Track
        tft.fillRoundRect(sx, base_y - max_h, cw, max_h, 2, kVizTrack);
        if (!sv || bar_h == 0) continue;
        // Filled: color depends on column position (simulate VU zones)
        const float zone = (float)(i + 1) / (float)cols;
        uint16_t c;
        if (zone > 0.72f)      c = TFT_RED;
        else if (zone > 0.43f) c = 0xFD20;  // orange
        else                   c = 0x27E0;  // acid green
        tft.fillRoundRect(sx, base_y - bar_h, cw, bar_h, 2, c);
        // Peak pixel
        tft.drawFastHLine(sx, base_y - bar_h - 1, cw, TFT_WHITE);
    }
}

// SOIL: 3-zone labeled bar (DRY | GOOD | WET) + position marker diamond
static void draw_soil_viz(bool sv, float value, uint16_t accent) {
    const int bh = 12;
    const int by = kVizBottomY - bh + 1;

    // Three zones: 0-30 red, 30-70 green, 70-100 blue
    const int dry_w  = (int)roundf(0.30f * kVizW);
    const int good_w = (int)roundf(0.40f * kVizW);
    const int wet_w  = kVizW - dry_w - good_w;

    tft.fillRoundRect(kVizX,           by, dry_w,  bh, 3, 0x2800);  // dark red
    tft.fillRoundRect(kVizX + dry_w,   by, good_w, bh, 0, 0x0640);  // dark green
    tft.fillRoundRect(kVizX + dry_w + good_w, by, wet_w, bh, 3, 0x000A);  // dark blue

    // Zone labels
    tft.setFreeFont(FONT_SMALL);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_RED,   kCardBg);
    tft.drawString("DRY",  kVizX + dry_w / 2, kVizLabelY);
    tft.setTextColor(0x2F85, kCardBg);
    tft.drawString("OK",   kVizX + dry_w + good_w / 2, kVizLabelY);
    tft.setTextColor(TFT_CYAN, kCardBg);
    tft.drawString("WET",  kVizX + dry_w + good_w + wet_w / 2, kVizLabelY);
    tft.setTextFont(0);

    // Position marker (filled diamond)
    if (!sv) return;
    const float norm = constrain(value, 0.0f, 100.0f) / 100.0f;
    const int mx = kVizX + (int)roundf(norm * (kVizW - 1));

    // Draw filled diamond
    tft.drawFastVLine(mx, by + bh + 1, 5, TFT_WHITE);
    tft.fillTriangle(mx - 4, by + bh + 6, mx + 4, by + bh + 6, mx, by + bh + 2, TFT_WHITE);
}

// ── Spec array ────────────────────────────────────────────────────────

static const LabSensorCardSpec kCardSpecs[CARD_COUNT] = {
    {
        CARD_TEMP, TIT_LAB_TEMP_CARD, "DHT11", kTempSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Temp, true,
        temp_is_valid, temp_value_c, get_temp_alerts_enabled, unit_temp, unit_temp_compact,
        temp_accent, draw_temp_viz, pbit_draw_temp_icon
    },
    {
        CARD_DS18, TIT_LAB_PROBE_CARD, "DS18B20", kTempSecondary,
        ST_CHECK_DS18, TFT_CYAN, AlertSensor::Ds18, true,
        ds18_is_valid, ds18_value_c, get_ds18_alerts_enabled, unit_temp, unit_temp_compact,
        ds18_accent, draw_probe_viz, pbit_draw_probe_icon
    },
    {
        CARD_HUM, TIT_LAB_HUM_CARD, "DHT11", kHumSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Humidity, false,
        hum_is_valid, hum_value_c, get_humidity_alerts_enabled, unit_pct, unit_pct_compact,
        hum_accent, draw_hum_viz, pbit_draw_humidity_icon
    },
    {
        CARD_LIGHT, TIT_LAB_LIGHT_CARD, "LDR", kLightSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Light, false,
        light_is_valid, light_value_c, get_light_alerts_enabled, unit_lux, unit_lux_compact,
        light_accent, draw_light_viz, pbit_draw_light_icon
    },
    {
        CARD_SOUND, TIT_LAB_SOUND_CARD, "MIC", kSoundSecondary,
        ST_NO_SENSOR, TFT_DARKGREY, AlertSensor::Sound, false,
        sound_is_valid, sound_value_c, get_sound_alerts_enabled, unit_pct, unit_pct_compact,
        sound_accent, draw_sound_viz, pbit_draw_sound_icon
    },
    {
        CARD_SOIL, TIT_LAB_SOIL_CARD, "SOIL", kSoilSecondary,
        ST_CHECK_SOIL, TFT_DARKGREY, AlertSensor::Soil, false,
        soil_is_valid, soil_value_c, get_soil_alerts_enabled, unit_pct, unit_pct_compact,
        soil_accent, draw_soil_viz, pbit_draw_plant_icon
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
        || (cache.sensor_valid   != state.sensor_valid)
        || (cache.value_key      != state.value_key)
        || (cache.alert_code     != state.alert_code)
        || (cache.unit_mode      != g_is_fahrenheit)
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
    // Full clear first — all sub-draws rely on this, no partial clears needed
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);
    tft.fillRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, kCardBg);
    tft.drawRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, state.accent);

    // Header: device label + short status only when invalid
    const char* header_status = state.sensor_valid ? "" : nullptr;
    draw_header_strip(spec.device_label, spec.secondary, header_status, state.sensor_valid);

    // Large value + compact unit in one visual group
    draw_value_compact(state.sensor_valid,
                       state.shown_value,
                       state.accent,
                       spec.is_temperature,
                       spec.compact_unit_fn(),
                       L(spec.invalid_bottom_key));

    // Sensor-specific horizontal visualization
    spec.draw_viz(state.sensor_valid, state.temp_c, state.accent);

    // Alert jewel
    drawAlertJewel(kAlertJewelX, kAlertJewelY,
                   state.jewel, jewel_color(state.jewel, state.accent));

    // Icon drawn last so nothing erases it
    spec.icon_fn(kIconCx, kIconCy, state.accent);
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
