#include "ui_lab_sensor_cards.h"

#include "alert_engine.h"
#include "fonts.h"
#include "hw.h"
#include "languages.h"
#include "layout.h"
#include "runtime_events.h"
#include "tft_display.h"
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

constexpr uint16_t kBg = TFT_BLACK;
constexpr uint16_t kCardBg = 0x1082;
constexpr int kCardX = LC_SCREEN_X;
constexpr int kCardY = LC_CARD_TOP;
constexpr int kCardW = LC_SCREEN_W;
constexpr int kCardH = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;
constexpr int kValueCx = 66;
constexpr int kValueY = kCardY + 18;
constexpr int kUnitCx = kValueCx - 4;
constexpr int kUnitY = kCardY + 78;
constexpr int kRailX = 118;
constexpr int kRailY = 34;
constexpr int kRailW = 34;
constexpr int kRailH = 82;
constexpr int kAlertJewelX = kCardX + 15;
constexpr int kAlertJewelY = kCardY + 81;

enum LabSensorCardId : uint8_t {
    CARD_TEMP = 0,
    CARD_DS18,
    CARD_COUNT
};

struct CardCache {
    bool valid = false;
    bool sensor_valid = false;
    int value_key = INT_MIN;
    uint8_t alert_code = ALERT_CODE_OFF;
    bool unit_mode = false;
    bool alerts_enabled = false;
};

struct CardRenderState {
    bool sensor_valid = false;
    float temp_c = 0.0f;
    float shown_temp = 0.0f;
    int value_key = INT_MIN;
    uint8_t alert_code = ALERT_CODE_OFF;
    bool alerts_enabled = false;
    uint16_t accent = TFT_DARKGREY;
    AlertJewelState jewel = ALERT_JEWEL_OFF;
};

struct LabSensorCardSpec {
    LabSensorCardId id;
    LangKey title_key;
    const char* device_label;
    LangKey invalid_bottom_key;
    uint16_t invalid_bottom_color;
    AlertSensor alert_sensor;
    bool (*is_valid)(const Reading& reading);
    float (*value_c)(const Reading& reading);
    bool (*alerts_enabled)();
    uint16_t (*accent)(bool sensor_valid, float temp_c, uint8_t alert_code);
    void (*draw_rail)(bool sensor_valid, float temp_c, uint16_t accent);
};

static CardCache g_card_cache[CARD_COUNT];
static LabSensorCardId g_selected_card = CARD_TEMP;

static float to_display(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static const char* unit_name() {
    return g_is_fahrenheit ? L(MENU_UNIT_F) : L(MENU_UNIT_C);
}

static bool temp_is_valid(const Reading& reading) {
    return !isnan(reading.temperature);
}

static float temp_value_c(const Reading& reading) {
    return reading.temperature;
}

static bool ds18_is_valid(const Reading& reading) {
    return reading.temp_ds18b20 >= -100.0f;
}

static float ds18_value_c(const Reading& reading) {
    return reading.temp_ds18b20;
}

static uint16_t temp_accent(bool sensor_valid, float temp_c, uint8_t alert_code) {
    if (!sensor_valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW) return TFT_BLUE;
    if (alert_code == ALERT_CODE_HIGH) return TFT_RED;
    return getTempColor(temp_c);
}

static uint16_t ds18_accent(bool sensor_valid, float temp_c, uint8_t alert_code) {
    if (!sensor_valid) return TFT_DARKGREY;
    if (alert_code == ALERT_CODE_LOW) return TFT_BLUE;
    if (alert_code == ALERT_CODE_HIGH) return TFT_RED;
    return (temp_c < 0.0f) ? TFT_CYAN : getTempColor(temp_c);
}

static AlertJewelState jewel_state(bool sensor_valid, bool alerts_enabled, uint8_t alert_code) {
    if (!sensor_valid || !alerts_enabled) return ALERT_JEWEL_OFF;
    if (alert_code == ALERT_CODE_LOW) return ALERT_JEWEL_WARN;
    if (alert_code == ALERT_CODE_HIGH || alert_code == ALERT_CODE_CRITICAL) return ALERT_JEWEL_CRIT;
    return ALERT_JEWEL_OK;
}

static uint16_t jewel_color(AlertJewelState state, uint16_t accent) {
    switch (state) {
        case ALERT_JEWEL_OFF: return TFT_DARKGREY;
        case ALERT_JEWEL_WARN: return (accent == TFT_BLUE) ? TFT_BLUE : TFT_ORANGE;
        case ALERT_JEWEL_CRIT: return TFT_RED;
        case ALERT_JEWEL_OK:
        default:
            return TFT_GREEN;
    }
}

static void draw_lab_card_shell(const char* title) {
    tft.fillScreen(kBg);
    drawHeader(title);
}

static void draw_card_frame(uint16_t accent) {
    tft.fillRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, kCardBg);
    tft.drawRoundRect(kCardX, kCardY, kCardW, kCardH, LC_CARD_RADIUS, accent);
}

static void draw_device_label(const char* label) {
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kCardBg);
    tft.drawString(label, kCardX + 8, kCardY + 4);
    tft.setTextFont(0);
}

static void draw_hint_line(const char* text, uint16_t color) {
    tft.fillRect(kCardX + 12, kCardY + 8, 86, 14, kCardBg);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, kCardBg);
    tft.drawString(text, kCardX + 14, kCardY + 13);
    tft.setTextFont(0);
}

static void draw_bottom_line(const char* text, uint16_t color) {
    tft.fillRect(kCardX + 24, kCardY + 64, 94, 22, kCardBg);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(color, kCardBg);
    if (tft.textWidth(text) > 84) {
        tft.setFreeFont(FONT_SMALL);
    }
    tft.drawString(text, kUnitCx, kUnitY);
    tft.setTextFont(0);
}

static void draw_big_value(bool sensor_valid, float shown_temp, uint16_t accent) {
    tft.fillRect(kCardX + 10, kCardY + 16, 104, 48, kCardBg);
    if (sensor_valid) {
        drawSplitDecimalValue(shown_temp, kValueCx, kValueY, accent, kCardBg);
        return;
    }

    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_VALUE);
    tft.setTextColor(TFT_DARKGREY, kCardBg);
    tft.drawString("---", kValueCx, kValueY);
    tft.setTextFont(0);
}

static uint16_t temp_gradient_color(float ratio) {
    ratio = constrain(ratio, 0.0f, 1.0f);
    const int r = (int)roundf(64.0f + ratio * (255.0f - 64.0f));
    const int g = (int)roundf(120.0f + ratio * (32.0f - 120.0f));
    const int b = (int)roundf(255.0f + ratio * (40.0f - 255.0f));
    return tft.color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

static void draw_temp_rail(bool sensor_valid, float temp_c, uint16_t accent) {
    const int inner_h = kRailH - 2;
    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, inner_h, kCardBg);
    if (sensor_valid) {
        const float clamped = constrain(temp_c, 0.0f, 50.0f);
        const int fill_px = (int)roundf((clamped / 50.0f) * inner_h);
        for (int row = 0; row < fill_px; ++row) {
            const float ratio = (float)row / (float)max(1, inner_h - 1);
            const uint16_t color = temp_gradient_color(ratio);
            tft.drawFastHLine(kRailX + 1,
                              kRailY + 1 + (inner_h - 1 - row),
                              kRailW - 2,
                              color);
        }
    }
    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sensor_valid ? accent : TFT_DARKGREY);
}

static void draw_probe_rail(bool sensor_valid, float temp_c, uint16_t accent) {
    const int inner_h = kRailH - 2;
    const int zero_fill = (int)(55.0f / 180.0f * inner_h);
    const int zero_y = kRailY + 1 + (inner_h - zero_fill);

    tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, kRailH - 2, kCardBg);

    if (sensor_valid) {
        const float norm = constrain((temp_c + 55.0f) / 180.0f, 0.0f, 1.0f);
        const int fill_px = (int)(norm * inner_h);
        const int empty_px = inner_h - fill_px;

        if (empty_px > 0) {
            tft.fillRect(kRailX + 1, kRailY + 1, kRailW - 2, empty_px, kCardBg);
        }

        if (temp_c >= 0.0f) {
            const int pos_fill = fill_px - zero_fill;
            if (pos_fill > 0) {
                tft.fillRect(kRailX + 1, kRailY + 1 + empty_px, kRailW - 2, pos_fill, accent);
            }
            tft.fillRect(kRailX + 1, zero_y, kRailW - 2, zero_fill, TFT_BLUE);
        } else if (fill_px > 0) {
            tft.fillRect(kRailX + 1, kRailY + 1 + empty_px, kRailW - 2, fill_px, TFT_BLUE);
        }
    }

    tft.drawRoundRect(kRailX, kRailY, kRailW, kRailH, 3, sensor_valid ? accent : TFT_DARKGREY);
    tft.drawFastHLine(kRailX - 2, zero_y, kRailW + 4, TFT_WHITE);
}

static const LabSensorCardSpec kCardSpecs[CARD_COUNT] = {
    {
        CARD_TEMP,
        TIT_LAB_TEMP_CARD,
        "DHT11",
        ST_NO_SENSOR,
        TFT_DARKGREY,
        AlertSensor::Temp,
        temp_is_valid,
        temp_value_c,
        get_temp_alerts_enabled,
        temp_accent,
        draw_temp_rail
    },
    {
        CARD_DS18,
        TIT_LAB_PROBE_CARD,
        "DS18B20",
        ST_CHECK_DS18,
        TFT_CYAN,
        AlertSensor::Ds18,
        ds18_is_valid,
        ds18_value_c,
        get_ds18_alerts_enabled,
        ds18_accent,
        draw_probe_rail
    }
};

static const LabSensorCardSpec& spec_for(LabSensorCardId id) {
    return kCardSpecs[(id < CARD_COUNT) ? id : CARD_TEMP];
}

static CardRenderState read_state(const LabSensorCardSpec& spec) {
    CardRenderState state;
    state.sensor_valid = spec.is_valid(g_ui_readings_snapshot);
    state.temp_c = state.sensor_valid ? spec.value_c(g_ui_readings_snapshot) : 0.0f;
    state.shown_temp = state.sensor_valid ? to_display(state.temp_c) : 0.0f;
    state.value_key = state.sensor_valid ? (int)lroundf(state.shown_temp * 10.0f) : INT_MIN;
    state.alert_code = alert_engine_get_code(spec.alert_sensor);
    state.alerts_enabled = spec.alerts_enabled();
    state.accent = spec.accent(state.sensor_valid, state.temp_c, state.alert_code);
    state.jewel = jewel_state(state.sensor_valid, state.alerts_enabled, state.alert_code);
    return state;
}

static bool cache_dirty(const CardCache& cache, const CardRenderState& state) {
    return !cache.valid
        || (cache.sensor_valid != state.sensor_valid)
        || (cache.value_key != state.value_key)
        || (cache.alert_code != state.alert_code)
        || (cache.unit_mode != g_is_fahrenheit)
        || (cache.alerts_enabled != state.alerts_enabled);
}

static void commit_cache(CardCache& cache, const CardRenderState& state) {
    cache.valid = true;
    cache.sensor_valid = state.sensor_valid;
    cache.value_key = state.value_key;
    cache.alert_code = state.alert_code;
    cache.unit_mode = g_is_fahrenheit;
    cache.alerts_enabled = state.alerts_enabled;
}

static void draw_card_dynamic(const LabSensorCardSpec& spec, const CardRenderState& state) {
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);
    draw_card_frame(state.accent);
    draw_device_label(spec.device_label);
    if (!state.sensor_valid) {
        draw_hint_line(L(ST_NO_SENSOR), TFT_RED);
    }
    draw_big_value(state.sensor_valid, state.shown_temp, state.accent);
    draw_bottom_line(state.sensor_valid ? unit_name() : L(spec.invalid_bottom_key),
                     state.sensor_valid ? TFT_WHITE : spec.invalid_bottom_color);
    drawAlertJewel(kAlertJewelX,
                   kAlertJewelY,
                   state.jewel,
                   jewel_color(state.jewel, state.accent));
    spec.draw_rail(state.sensor_valid, state.temp_c, state.accent);
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

void lab_sensor_card_cycle() {
    g_selected_card = (g_selected_card == CARD_TEMP) ? CARD_DS18 : CARD_TEMP;
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
