// PLANTA LAB
// Multisensor plant-health screen: soil + air temperature + air humidity + light.

#include "ui_lab_plant.h"

#include "external_sensor_state.h"
#include "fonts.h"
#include "io.h"
#include "languages.h"
#include "light_display.h"
#include "palette.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

namespace {

enum PlantState : uint8_t {
    PLANT_STATE_OK = 0,
    PLANT_STATE_THIRSTY,
    PLANT_STATE_SOGGY,
    PLANT_STATE_STRESSED,
};

enum SoilBand : uint8_t {
    SOIL_BAND_DRY = 0,
    SOIL_BAND_OPTIMAL,
    SOIL_BAND_WET,
};

enum PlantCard : uint8_t {
    PLANT_CARD_SOIL = 0,
    PLANT_CARD_TEMP,
    PLANT_CARD_AIR,
    PLANT_CARD_LIGHT,
    PLANT_CARD_NONE = 0xFF,
};

struct PlantLabCache {
    uint8_t  state;
    uint8_t  soil_band;
    uint8_t  culprit;
    int16_t  soil_x10;
    int16_t  temp_x10;
    int16_t  hum_x10;
    uint32_t lux;
    uint8_t  anim_phase;
    bool     chrome_done;
};
static PlantLabCache g_plant_cache = { 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, false };

struct PlantLeaf {
    int8_t dx;
    int8_t cy;
    int8_t rx;
    int8_t ry;
};

struct RangeSpec {
    float min_value;
    float max_value;
    float opt_low;
    float opt_high;
    bool logarithmic;
};

constexpr uint16_t kCardBg = 0x0841;
constexpr uint16_t kIdleBorder = 0x2945;

constexpr int kTerrariumX = 2;
constexpr int kTerrariumY = 27;
constexpr int kTerrariumW = 56;
constexpr int kTerrariumH = 99;
constexpr int kDirtX = 4;
constexpr int kDirtY = 110;
constexpr int kDirtW = 52;
constexpr int kDirtH = 14;
constexpr int kPlantCx = 30;

constexpr int kRowX = 62;
constexpr int kRowW = 96;
constexpr int kRowH = 17;
constexpr int kRowRadius = 3;
constexpr int kIconX = 70;
constexpr int kBarX = 80;
constexpr int kBarW = 42;
constexpr int kBarH = 7;
constexpr int kValueX = 156;
constexpr int kStateX = 62;
constexpr int kStateY = 104;
constexpr int kStateW = 96;
constexpr int kStateH = 22;

constexpr int kPlantClearX = kTerrariumX + 3;
constexpr int kPlantClearY = kTerrariumY + 3;
constexpr int kPlantClearW = kTerrariumW - 6;
constexpr int kPlantClearH = kDirtY - kTerrariumY - 5;

constexpr uint32_t kInvalidLux = 0xFFFFFFFFUL;

static const int kRowY[4] = { 27, 46, 65, 84 };
static const PlantLeaf kBaseLeaves[5] = {
    { -12, 88, 8, 3 },
    {  11, 80, 7, 3 },
    {  -9, 72, 6, 2 },
    {  10, 64, 6, 2 },
    {  -6, 58, 5, 2 },
};

static bool valid_number(float value) {
    return !isnan(value) && isfinite(value);
}

static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
    return tft.color565(r, g, b);
}

static uint16_t screen_bg() { return rgb(8, 12, 18); }
static uint16_t bar_track_color() { return rgb(22, 36, 46); }
static uint16_t bar_low_color() { return rgb(122, 64, 48); }
static uint16_t bar_opt_color() { return rgb(46, 125, 74); }
static uint16_t bar_high_color() { return rgb(42, 90, 122); }

static uint16_t state_color(uint8_t state) {
    switch (state) {
        case PLANT_STATE_THIRSTY:  return rgb(232, 162, 42);
        case PLANT_STATE_SOGGY:    return rgb(63, 163, 224);
        case PLANT_STATE_STRESSED: return rgb(232, 84, 42);
        case PLANT_STATE_OK:
        default:                   return rgb(46, 204, 90);
    }
}

static uint16_t state_highlight(uint8_t state) {
    switch (state) {
        case PLANT_STATE_SOGGY:    return rgb(140, 218, 255);
        case PLANT_STATE_THIRSTY:  return rgb(255, 220, 90);
        case PLANT_STATE_STRESSED: return rgb(255, 128, 70);
        case PLANT_STATE_OK:
        default:                   return rgb(168, 255, 176);
    }
}

static LangKey state_key(uint8_t state) {
    switch (state) {
        case PLANT_STATE_THIRSTY:  return PLANT_STATE_THIRSTY_KEY;
        case PLANT_STATE_SOGGY:    return PLANT_STATE_SOGGY_KEY;
        case PLANT_STATE_STRESSED: return PLANT_STATE_STRESSED_KEY;
        case PLANT_STATE_OK:
        default:                   return PLANT_STATE_OK_KEY;
    }
}

static const RangeSpec& range_spec(uint8_t card) {
    static const RangeSpec specs[4] = {
        { 0.0f, 100.0f, 30.0f, 70.0f, false },
        { 0.0f,  40.0f, 18.0f, 26.0f, false },
        { 0.0f, 100.0f, 40.0f, 70.0f, false },
        { 0.0f, 30000.0f, 200.0f, 10000.0f, true },
    };
    return specs[(card < 4) ? card : 0];
}

static float range_fraction(uint8_t card, float value) {
    const RangeSpec& spec = range_spec(card);
    value = constrain(value, spec.min_value, spec.max_value);
    if (spec.logarithmic) {
        return log1pf(value) / log1pf(spec.max_value);
    }
    return (value - spec.min_value) / (spec.max_value - spec.min_value);
}

static int row_y(uint8_t card) {
    return kRowY[(card < 4) ? card : 0];
}

static void draw_sensor_icon(uint8_t card, int cx, int cy) {
    switch (card) {
        case PLANT_CARD_SOIL:
            pbit_draw_plant_icon(cx, cy, PB_SOIL_P1);
            break;
        case PLANT_CARD_TEMP:
            pbit_draw_temp_icon(cx, cy - 1, PB_TEMP_P1);
            break;
        case PLANT_CARD_AIR:
            pbit_draw_humidity_icon(cx, cy - 1, PB_HUM_P1);
            break;
        case PLANT_CARD_LIGHT:
            pbit_draw_light_icon(cx, cy, PB_LUZ_P1);
            break;
        default:
            break;
    }
}

static void draw_row_border(uint8_t card, uint8_t culprit, uint8_t state) {
    const int y = row_y(card);
    const uint16_t color = (card == culprit) ? state_color(state) : kIdleBorder;
    tft.drawRoundRect(kRowX, y, kRowW, kRowH, kRowRadius, color);
    if (card == culprit) {
        tft.drawRoundRect(kRowX + 1, y + 1, kRowW - 2, kRowH - 2, 2, color);
    } else {
        tft.drawRoundRect(kRowX + 1, y + 1, kRowW - 2, kRowH - 2, 2, kCardBg);
    }
}

static void draw_row_shell(uint8_t card) {
    const int y = row_y(card);
    tft.fillRoundRect(kRowX, y, kRowW, kRowH, kRowRadius, kCardBg);
    tft.drawRoundRect(kRowX, y, kRowW, kRowH, kRowRadius, kIdleBorder);
    draw_sensor_icon(card, kIconX, y + 8);
}

static void draw_bar_zones(uint8_t card, int y, int bar_w) {
    const RangeSpec& spec = range_spec(card);
    const int bar_y = y + 5;
    const int opt_start = kBarX + (int)lroundf(range_fraction(card, spec.opt_low) * (float)bar_w);
    const int opt_end = kBarX + (int)lroundf(range_fraction(card, spec.opt_high) * (float)bar_w);
    const int x_end = kBarX + bar_w;

    tft.fillRect(kBarX, bar_y, bar_w, kBarH, bar_track_color());
    tft.fillRect(kBarX, bar_y, max(0, opt_start - kBarX), kBarH, bar_low_color());
    tft.fillRect(opt_start, bar_y, max(0, opt_end - opt_start), kBarH, bar_opt_color());
    tft.fillRect(opt_end, bar_y, max(0, x_end - opt_end), kBarH, bar_high_color());
}

static void format_row_value(char* out, size_t out_size, uint8_t card,
                             int16_t soil_x10, int16_t temp_x10,
                             int16_t hum_x10, uint32_t lux) {
    if (!out || out_size == 0) return;
    switch (card) {
        case PLANT_CARD_SOIL:
            if (soil_x10 == INT16_MIN) snprintf(out, out_size, "--");
            else snprintf(out, out_size, "%.0f%%", (float)soil_x10 / 10.0f);
            break;
        case PLANT_CARD_TEMP:
            if (temp_x10 == INT16_MIN) snprintf(out, out_size, "--");
            else snprintf(out, out_size, "%.0f°", (float)temp_x10 / 10.0f);
            break;
        case PLANT_CARD_AIR:
            if (hum_x10 == INT16_MIN) snprintf(out, out_size, "--");
            else snprintf(out, out_size, "%.0f%%", (float)hum_x10 / 10.0f);
            break;
        case PLANT_CARD_LIGHT:
            if (lux == kInvalidLux) {
                snprintf(out, out_size, "--");
            } else if (lux >= 10000UL) {
                snprintf(out, out_size, "%luk", (unsigned long)lroundf((float)lux / 1000.0f));
            } else {
                snprintf(out, out_size, "%lulx", (unsigned long)lux);
            }
            break;
        default:
            snprintf(out, out_size, "--");
            break;
    }
}

static bool row_value_valid(uint8_t card, const PlantLabCache& next) {
    switch (card) {
        case PLANT_CARD_SOIL:  return next.soil_x10 != INT16_MIN;
        case PLANT_CARD_TEMP:  return next.temp_x10 != INT16_MIN;
        case PLANT_CARD_AIR:   return next.hum_x10 != INT16_MIN;
        case PLANT_CARD_LIGHT: return next.lux != kInvalidLux;
        default:               return false;
    }
}

static float row_value_float(uint8_t card, const PlantLabCache& next) {
    switch (card) {
        case PLANT_CARD_SOIL:  return (float)next.soil_x10 / 10.0f;
        case PLANT_CARD_TEMP:  return (float)next.temp_x10 / 10.0f;
        case PLANT_CARD_AIR:   return (float)next.hum_x10 / 10.0f;
        case PLANT_CARD_LIGHT: return (float)next.lux;
        default:               return 0.0f;
    }
}

static void draw_row_data(uint8_t card, const PlantLabCache& next) {
    const int y = row_y(card);
    char buf[16];
    format_row_value(buf, sizeof(buf), card, next.soil_x10, next.temp_x10, next.hum_x10, next.lux);
    const int bar_w = (card == PLANT_CARD_LIGHT && next.lux != kInvalidLux && next.lux >= 10000UL) ? 38 : kBarW;
    draw_bar_zones(card, y, bar_w);

    if (row_value_valid(card, next)) {
        const int marker_x = kBarX + (int)lroundf(range_fraction(card, row_value_float(card, next)) * (float)bar_w);
        tft.drawFastVLine(constrain(marker_x, kBarX, kBarX + bar_w), y + 3, 11, TFT_WHITE);
    }

    tft.fillRect(kBarX + bar_w + 2, y + 3, kValueX - (kBarX + bar_w + 2), 12, kCardBg);
    tft.setTextDatum(TR_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(TFT_WHITE, kCardBg);
    if (tft.textWidth(buf) > (kValueX - (kBarX + bar_w + 3))) {
        tft.setTextFont(0);
    }
    tft.drawString(buf, kValueX, y + 4);
    tft.setTextFont(0);
}

static uint8_t soil_band_from_value(float soil) {
    if (!valid_number(soil)) return SOIL_BAND_DRY;
    if (soil < 25.0f) return SOIL_BAND_DRY;
    if (soil > 80.0f) return SOIL_BAND_WET;
    return SOIL_BAND_OPTIMAL;
}

static void draw_dirt_band(uint8_t band) {
    uint16_t fill = rgb(74, 48, 22);
    uint16_t top = rgb(110, 74, 36);

    if (band == SOIL_BAND_DRY) {
        fill = rgb(110, 74, 36);
        top = rgb(134, 96, 46);
    } else if (band == SOIL_BAND_WET) {
        fill = rgb(46, 36, 24);
        top = state_color(PLANT_STATE_SOGGY);
    }

    tft.fillRect(kDirtX, kDirtY, kDirtW, kDirtH, fill);
    tft.fillRect(kDirtX, kDirtY, kDirtW, 2, top);

    if (band == SOIL_BAND_DRY) {
        const uint16_t crack = rgb(74, 48, 22);
        tft.drawLine(kDirtX + 11, kDirtY + 4, kDirtX + 16, kDirtY + 10, crack);
        tft.drawLine(kDirtX + 37, kDirtY + 3, kDirtX + 32, kDirtY + 11, crack);
    }
}

static int state_tallo_top(uint8_t state) {
    return (state == PLANT_STATE_OK) ? 44 : 46;
}

static int state_leaf_dy(uint8_t state) {
    switch (state) {
        case PLANT_STATE_OK:       return -4;
        case PLANT_STATE_THIRSTY:  return 4;
        case PLANT_STATE_SOGGY:    return 6;
        case PLANT_STATE_STRESSED: return 7;
        default:                   return 0;
    }
}

static float state_leaf_scale(uint8_t state) {
    switch (state) {
        case PLANT_STATE_OK:       return 1.0f;
        case PLANT_STATE_THIRSTY:  return 0.85f;
        case PLANT_STATE_SOGGY:    return 0.9f;
        case PLANT_STATE_STRESSED: return 0.7f;
        default:                   return 1.0f;
    }
}

static uint8_t state_leaf_count(uint8_t state) {
    return (state == PLANT_STATE_STRESSED) ? 3 : 5;
}

static void draw_hero_plant(uint8_t state, uint8_t anim_phase) {
    static const int8_t kSwayX[4][5] = {
        {  0,  1, -1,  0,  1 },
        {  1,  0,  1, -1,  0 },
        {  0, -1,  0,  1, -1 },
        { -1,  0, -1,  0,  1 },
    };
    static const int8_t kRxPulse[4][5] = {
        { 0, 1, 0, 0, 1 },
        { 1, 0, 0, 1, 0 },
        { 0, 0, 1, 0, 0 },
        { 0, 1, 0, 1, 0 },
    };

    const uint16_t color = state_color(state);
    const int top = state_tallo_top(state);
    const int group_x = (state == PLANT_STATE_STRESSED && (anim_phase & 0x01)) ? 1 : 0;
    const int phase = anim_phase & 0x03;
    const int leaf_dy = state_leaf_dy(state);
    const float scale = state_leaf_scale(state);
    const uint8_t leaf_count = state_leaf_count(state);

    tft.fillRect(kPlantCx + group_x - 1, top, 3, kDirtY - top, color);
    tft.fillEllipse(kPlantCx + group_x, top - 1, 4, 6, color);
    if (state == PLANT_STATE_OK) {
        const int pulse = (anim_phase & 0x02) ? 2 : 1;
        tft.fillCircle(kPlantCx + group_x, top - 4, pulse, state_highlight(state));
    }

    for (uint8_t i = 0; i < leaf_count; ++i) {
        const PlantLeaf& leaf = kBaseLeaves[i];
        const int rx = max(2, (int)lroundf((float)leaf.rx * scale) + kRxPulse[phase][i]);
        const int ry = max(1, (int)lroundf((float)leaf.ry * scale));
        tft.fillEllipse(kPlantCx + group_x + leaf.dx + kSwayX[(phase + i) & 0x03][i],
                        leaf.cy + leaf_dy,
                        rx,
                        ry,
                        color);
    }

    if (state == PLANT_STATE_SOGGY) {
        const uint16_t drop = state_highlight(state);
        const int dy1 = (int)((anim_phase % 6) * 5);
        const int dy2 = (int)(((anim_phase + 3) % 6) * 4);
        tft.fillEllipse(18, 66 + dy1, 2, 3, drop);
        tft.fillEllipse(43, 78 + dy2, 2, 3, drop);
    }
}

static void clear_plant_icon_area() {
    tft.fillRect(kPlantClearX, kPlantClearY, kPlantClearW, kPlantClearH, kCardBg);
}

static void clear_full_terrarium() {
    tft.fillRect(kTerrariumX + 1, kTerrariumY + 1, kTerrariumW - 2, kTerrariumH - 2, kCardBg);
}

static void draw_plant_area(uint8_t state, uint8_t band, uint8_t anim_phase, bool full_terrarium) {
    if (full_terrarium) {
        clear_full_terrarium();
        draw_dirt_band(band);
    } else {
        clear_plant_icon_area();
    }
    draw_hero_plant(state, anim_phase);
}

static void draw_state_card(const PlantLabCache& next) {
    const uint16_t color = state_color(next.state);
    tft.fillRect(kStateX + 3, kStateY + 5, kStateW - 6, kStateH - 10, kCardBg);
    tft.drawRoundRect(kStateX, kStateY, kStateW, kStateH, 4, color);
    tft.drawRoundRect(kStateX + 1, kStateY + 1, kStateW - 2, kStateH - 2, 3, color);

    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(color, kCardBg);
    tft.drawString(L(state_key(next.state)), kStateX + (kStateW / 2), kStateY + (kStateH / 2) + 1);
    tft.setTextFont(0);
}

static uint8_t animation_phase_for_state(uint8_t state) {
    uint16_t divisor_ms = 320;
    if (state == PLANT_STATE_OK) {
        divisor_ms = 180;
    } else if (state == PLANT_STATE_STRESSED) {
        divisor_ms = 120;
    } else if (state == PLANT_STATE_THIRSTY) {
        divisor_ms = 480;
    } else if (state == PLANT_STATE_SOGGY) {
        divisor_ms = 420;
    }
    return (uint8_t)((millis() / divisor_ms) & 0x0F);
}

static PlantLabCache build_next_cache() {
    const Reading& r = g_ui_readings_snapshot;
    PlantLabCache next = g_plant_cache;

    const bool soil_valid = !pbit_external_sensor_missing(SZ_SOIL, r);
    const bool temp_valid = valid_number(r.temperature);
    const bool hum_valid = valid_number(r.humidity);
    const bool lux_valid = light_lux_valid(r.ldr);

    next.soil_x10 = soil_valid ? (int16_t)lroundf(r.soil_humidity * 10.0f) : INT16_MIN;
    next.temp_x10 = temp_valid ? (int16_t)lroundf(r.temperature * 10.0f) : INT16_MIN;
    next.hum_x10 = hum_valid ? (int16_t)lroundf(r.humidity * 10.0f) : INT16_MIN;
    next.lux = lux_valid ? (uint32_t)lroundf(r.ldr) : kInvalidLux;
    next.soil_band = soil_band_from_value(r.soil_humidity);

    const bool soil_ok = soil_valid && r.soil_humidity >= 30.0f && r.soil_humidity <= 70.0f;
    const bool temp_ok = temp_valid && r.temperature >= 18.0f && r.temperature <= 26.0f;
    const bool hum_ok = hum_valid && r.humidity >= 40.0f && r.humidity <= 70.0f;
    const bool light_ok = lux_valid && r.ldr >= 200.0f && r.ldr <= 10000.0f;

    if (!soil_ok) {
        next.state = (soil_valid && r.soil_humidity > 70.0f) ? PLANT_STATE_SOGGY : PLANT_STATE_THIRSTY;
        next.culprit = PLANT_CARD_SOIL;
    } else if (!temp_ok) {
        next.state = PLANT_STATE_STRESSED;
        next.culprit = PLANT_CARD_TEMP;
    } else if (!hum_ok) {
        next.state = PLANT_STATE_STRESSED;
        next.culprit = PLANT_CARD_AIR;
    } else if (!light_ok) {
        next.state = PLANT_STATE_STRESSED;
        next.culprit = PLANT_CARD_LIGHT;
    } else {
        next.state = PLANT_STATE_OK;
        next.culprit = PLANT_CARD_NONE;
    }

    next.anim_phase = animation_phase_for_state(next.state);
    next.chrome_done = true;
    return next;
}

static void draw_chrome() {
    tft.fillScreen(screen_bg());
    drawHeader(L(TIT_LAB_PLANT));

    tft.fillRoundRect(kTerrariumX, kTerrariumY, kTerrariumW, kTerrariumH, 4, kCardBg);
    tft.drawRoundRect(kTerrariumX, kTerrariumY, kTerrariumW, kTerrariumH, 4, kIdleBorder);

    for (uint8_t card = 0; card < 4; ++card) {
        draw_row_shell(card);
    }

    tft.fillRoundRect(kStateX, kStateY, kStateW, kStateH, 4, kCardBg);
    tft.drawRoundRect(kStateX, kStateY, kStateW, kStateH, 4, kIdleBorder);
}

} // namespace

void draw_lab_plant_screen(bool screen_changed, bool sensor_data_changed) {
    const PlantLabCache next = build_next_cache();
    const bool first = screen_changed || !g_plant_cache.chrome_done;
    const bool values_dirty = first
        || next.soil_x10 != g_plant_cache.soil_x10
        || next.temp_x10 != g_plant_cache.temp_x10
        || next.hum_x10 != g_plant_cache.hum_x10
        || next.lux != g_plant_cache.lux;
    const bool state_dirty = first
        || next.state != g_plant_cache.state
        || next.culprit != g_plant_cache.culprit;
    const bool band_dirty = first || next.soil_band != g_plant_cache.soil_band;
    const bool anim_dirty = first || next.anim_phase != g_plant_cache.anim_phase;

    if (first) {
        draw_chrome();
    }

    if (!first && !sensor_data_changed && !anim_dirty) {
        return;
    }

    if (state_dirty || anim_dirty || band_dirty) {
        draw_plant_area(next.state, next.soil_band, next.anim_phase, state_dirty || first || band_dirty);
    }

    if (values_dirty) {
        for (uint8_t card = 0; card < 4; ++card) {
            draw_row_data(card, next);
        }
    }

    if (state_dirty) {
        for (uint8_t card = 0; card < 4; ++card) {
            draw_row_border(card, next.culprit, next.state);
        }
        draw_state_card(next);
    }

    g_plant_cache = next;
}
