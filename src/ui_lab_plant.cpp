// PLANTA LAB — sensor-driven pixel pet; model and raster are independent of UI.
#include "ui_lab_plant.h"
#include "demo_mode.h"
#include "external_sensor_state.h"
#include "fonts.h"
#include "languages.h"
#include "light_display.h"
#include "palette.h"
#include "plant_pet.h"
#include "plant_pet_art.h"
#include "tft_display.h"
#include "ui_widgets.h"
#include "ui_plant_settings.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern bool g_is_fahrenheit;

namespace {
constexpr uint16_t kCardBg = 0x0841;
constexpr uint16_t kIdleBorder = 0x2945;
constexpr int kTerrariumX = 2, kTerrariumY = 27, kTerrariumW = 56, kTerrariumH = 99;
constexpr int kRowX = 62, kRowW = 96, kRowH = 17;
constexpr int kIconX = 68, kBarX = 77, kBarW = 48, kBarH = 7, kValueX = 156;
constexpr int kValueW = 30;
constexpr int kStateX = 62, kStateY = 104, kStateW = 96, kStateH = 22;
constexpr uint32_t kInvalidLux = 0xFFFFFFFFUL;
constexpr uint32_t kAnimationStepMs = 220;
const int kRowY[4] = {27, 46, 65, 84};
static_assert(PLANT_PET_LIGHT_MAX == LIGHT_LUX_MAX, "Plant profile must match the LDR range");
static_assert(PLANT_PET_ART_W <= kTerrariumW - 2 && PLANT_PET_ART_H <= kTerrariumH - 2,
              "Plant raster must stay inside the terrarium chrome");

PlantPetModel g_real_pet;
PlantPetModel g_demo_pet;
bool g_using_demo = false;
Reading g_real_plant_reading = {}, g_demo_plant_reading = {};
uint32_t g_applied_settings_revision = UINT32_MAX;
bool g_applied_soil_review = false;
// Fixed buffer: no frame-time allocation and one opaque burst per changed pose.
uint16_t g_plant_pixels[PLANT_PET_ART_W * PLANT_PET_ART_H];
struct PlantLabCache {
    PlantPetSnapshot pet;
    int16_t soil_x10 = INT16_MIN, temp_x10 = INT16_MIN, hum_x10 = INT16_MIN;
    uint32_t lux = kInvalidLux, animation_tick = 0;
    bool fahrenheit = false, chrome_done = false, soil_review = false;
};
PlantLabCache g_plant_cache;

bool valid_number(float v) { return isfinite(v); }
uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return tft.color565(r, g, b); }
uint16_t screen_bg() { return rgb(8, 12, 18); }
uint16_t unknown_color() { return rgb(118, 137, 139); }
uint16_t mood_color(PlantMood mood) {
    switch (mood) {
        case PlantMood::Thirsty: case PlantMood::AirDry: case PlantMood::Dim:
            return rgb(176, 120, 255);
        case PlantMood::Soggy: case PlantMood::AirHumid:
            return PB_HUM_P1;
        case PlantMood::Cold: return rgb(80, 160, 255);
        case PlantMood::Hot: return PB_TEMP_P1;
        case PlantMood::Unknown: return unknown_color();
        default: return PLANT_PET_GREEN;
    }
}
LangKey state_key(const PlantPetSnapshot& pet) {
    if (pet.recovering) return PLANT_STATE_RECOVERING_KEY;
    switch (pet.mood) {
        case PlantMood::Thirsty: return PLANT_STATE_THIRSTY_KEY;
        case PlantMood::Soggy: return PLANT_STATE_SOGGY_KEY;
        case PlantMood::Cold: return PLANT_STATE_COLD_KEY;
        case PlantMood::Hot: return PLANT_STATE_HOT_KEY;
        case PlantMood::AirDry: return PLANT_STATE_AIR_DRY_KEY;
        case PlantMood::AirHumid: return PLANT_STATE_AIR_HUMID_KEY;
        case PlantMood::Dim: return PLANT_STATE_DIM_KEY;
        case PlantMood::Unknown: return PLANT_STATE_UNKNOWN_KEY;
        default: return pet.unknown_mask ? PLANT_STATE_UNKNOWN_KEY : PLANT_STATE_OK_KEY;
    }
}
PlantPetRange range_spec(uint8_t card) {
    PlantPetRange range = (g_using_demo ? g_demo_pet : g_real_pet).range(static_cast<PlantChannel>(card));
    if (card == 1) {
        if (range.opt_low < range.min_value) range.min_value=range.opt_low;
        if (range.opt_high > range.max_value) range.max_value=range.opt_high;
    }
    return range;
}
float range_fraction(uint8_t card, float value) {
    const PlantPetRange& spec = range_spec(card);
    value = constrain(value, spec.min_value, spec.max_value);
    if (spec.logarithmic) return log1pf(value) / log1pf(spec.max_value);
    return (value - spec.min_value) / (spec.max_value - spec.min_value);
}
int row_y(uint8_t card) { return kRowY[card]; }
uint16_t row_accent(uint8_t card, PlantBand band) {
    if (band == PlantBand::Unknown) return unknown_color();
    if (band == PlantBand::Normal || card == 3) return kIdleBorder;
    if (card == 1) return mood_color(band == PlantBand::Low ? PlantMood::Cold : PlantMood::Hot);
    return mood_color(band == PlantBand::Low ? PlantMood::Thirsty : PlantMood::Soggy);
}
void draw_sensor_icon(uint8_t card, bool valid) {
    const int x = kIconX, y = row_y(card) + 8;
    // Compact variants belong to this 17px row; shared sensor icons stay intact.
    tft.fillRect(x - 4, row_y(card) + 2, 11, 13, kCardBg);
    const uint16_t color = valid ? pb_primary(card == 0 ? 4 : card == 1 ? 0 : card == 2 ? 1 : 2)
                                 : unknown_color();
    switch (card) {
        case 0:
            tft.drawFastVLine(x, y - 4, 7, color);
            tft.fillTriangle(x - 1, y, x - 4, y - 3, x - 1, y - 3, color);
            tft.fillTriangle(x + 1, y - 2, x + 4, y - 4, x + 4, y - 1, color);
            tft.drawFastHLine(x - 3, y + 3, 7, color);
            tft.drawFastHLine(x - 2, y + 4, 5, color);
            break;
        case 1:
            tft.fillRoundRect(x - 1, y - 5, 3, 8, 1, color);
            tft.drawFastVLine(x, y - 4, 3, kCardBg);
            tft.fillCircle(x, y + 3, 2, color);
            tft.drawPixel(x + 3, y - 3, color);
            tft.drawPixel(x + 3, y, color);
            break;
        case 2:
            tft.fillCircle(x, y + 1, 3, color);
            tft.fillTriangle(x, y - 5, x - 2, y - 1, x + 2, y - 1, color);
            tft.drawPixel(x - 1, y, kCardBg);
            break;
        case 3:
            tft.fillCircle(x, y, 2, color);
            tft.drawPixel(x - 4, y, color); tft.drawPixel(x + 4, y, color);
            tft.drawPixel(x, y - 4, color); tft.drawPixel(x, y + 4, color);
            tft.drawPixel(x - 3, y - 3, color); tft.drawPixel(x + 3, y - 3, color);
            tft.drawPixel(x - 3, y + 3, color); tft.drawPixel(x + 3, y + 3, color);
            break;
    }
}
void draw_row_border(uint8_t card, const PlantPetSnapshot& pet) {
    const int y = row_y(card);
    const bool dominant = static_cast<uint8_t>(pet.culprit) == card;
    const uint16_t color = row_accent(card, pet.bands[card]);
    tft.drawRoundRect(kRowX, y, kRowW, kRowH, 3, color);
    tft.drawRoundRect(kRowX + 1, y + 1, kRowW - 2, kRowH - 2, 2,
                      dominant ? color : kCardBg);
}
void draw_row_shell(uint8_t card) {
    tft.fillRoundRect(kRowX, row_y(card), kRowW, kRowH, 3, kCardBg);
    tft.drawRoundRect(kRowX, row_y(card), kRowW, kRowH, 3, kIdleBorder);
}
void draw_bar_zones(uint8_t card) {
    const PlantPetRange& spec = range_spec(card);
    const int y = row_y(card) + 5;
    const int opt_start = static_cast<int>(lroundf(range_fraction(card, spec.opt_low) * (kBarW - 1)));
    const int opt_end = static_cast<int>(lroundf(range_fraction(card, spec.opt_high) * (kBarW - 1))) + 1;
    // Sensor identity throughout: the profile interval is brighter, both outer
    // intervals keep a darker version of that hue. White marks the measurement.
    const uint16_t primary[4] = {PB_SOIL_P1, PB_TEMP_P1, PB_HUM_P1, rgb(176, 120, 255)};
    const uint16_t track[4] = {rgb(16, 96, 48), rgb(96, 16, 80), rgb(0, 80, 96), rgb(64, 32, 112)};
    tft.fillRect(kBarX, y, kBarW, kBarH, track[card]);
    tft.fillRect(kBarX + opt_start, y, opt_end - opt_start, kBarH, primary[card]);
}
bool row_value_valid(uint8_t card, const PlantLabCache& next) {
    switch (card) {
        case 0: return next.soil_x10 != INT16_MIN;
        case 1: return next.temp_x10 != INT16_MIN;
        case 2: return next.hum_x10 != INT16_MIN;
        default: return next.lux != kInvalidLux;
    }
}
float row_value_float(uint8_t card, const PlantLabCache& next) {
    switch (card) {
        case 0: return next.soil_x10 / 10.0f;
        case 1: return next.temp_x10 / 10.0f;
        case 2: return next.hum_x10 / 10.0f;
        default: return static_cast<float>(next.lux);
    }
}
void format_row_value(char* out, size_t size, uint8_t card, const PlantLabCache& next) {
    if (!row_value_valid(card, next)) { snprintf(out, size, "--"); return; }
    const float value = row_value_float(card, next);
    if (card == 1) {
        const float shown = next.fahrenheit ? value * 1.8f + 32.0f : value;
        // Font 1 is CP437, not Latin-1. Reserve one ASCII cell for our degree glyph.
        snprintf(out, size, "%.0f %c", shown, next.fahrenheit ? 'F' : 'C');
    } else if (card == 3) {
        if (next.lux < 1000) {
            snprintf(out, size, "%lulx", static_cast<unsigned long>(next.lux));
        } else {
            const unsigned long tenths_k = (next.lux + 50UL) / 100UL;
            if (tenths_k % 10UL == 0) snprintf(out, size, "%luK", tenths_k / 10UL);
            else snprintf(out, size, "%lu.%luK", tenths_k / 10UL, tenths_k % 10UL);
        }
    } else {
        snprintf(out, size, "%.0f%%", value);
    }
}
int marker_offset(uint8_t card, const PlantLabCache& next) {
    if (!row_value_valid(card, next)) return -1;
    return constrain(static_cast<int>(lroundf(range_fraction(card, row_value_float(card, next)) * (kBarW - 1))),
                     0, kBarW - 1);
}
bool row_data_dirty(uint8_t card, const PlantLabCache& next) {
    char old_text[16], new_text[16];
    format_row_value(old_text, sizeof(old_text), card, g_plant_cache);
    format_row_value(new_text, sizeof(new_text), card, next);
    return strcmp(old_text, new_text) != 0
        || marker_offset(card, next) != marker_offset(card, g_plant_cache)
        || ((next.pet.unknown_mask ^ g_plant_cache.pet.unknown_mask) & (1U << card));
}
void draw_row_data(uint8_t card, const PlantLabCache& next) {
    const int y = row_y(card);
    const bool known = row_value_valid(card, next);
    // Marker is taller than the track: clear its entire lane to avoid ghosting.
    tft.fillRect(kBarX, y + 4, kBarW, 9, kCardBg);
    if (known && next.pet.bands[card] != PlantBand::Unknown && !(card == 0 && next.soil_review)) draw_bar_zones(card);
    else tft.fillRect(kBarX, y + 5, kBarW, kBarH, rgb(22, 36, 46));
    const int marker = marker_offset(card, next);
    if (known && marker >= 0) tft.drawFastVLine(kBarX + marker, y + 4, 9, TFT_WHITE);

    char text[16];
    format_row_value(text, sizeof(text), card, next);
    // Font 1: 6px cells; 999lx and -40 F fit the 30px lane. K means 1000 lux.
    tft.fillRect(kValueX - kValueW, y + 3, kValueW, 12, kCardBg);
    tft.setTextFont(1);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(known ? TFT_WHITE : unknown_color(), kCardBg);
    tft.drawString(text, kValueX, y + 4);
    if (card == 1 && row_value_valid(card, next)) {
        const uint16_t color = known ? TFT_WHITE : unknown_color();
        const int dx = kValueX - 12 + 2, dy = y + 5;
        tft.drawPixel(dx, dy - 1, color); tft.drawPixel(dx - 1, dy, color);
        tft.drawPixel(dx + 1, dy, color); tft.drawPixel(dx, dy + 1, color);
    }
    tft.setTextFont(0);
    draw_sensor_icon(card, known);
}
void draw_state_card(const PlantLabCache& next) {
    const uint16_t color = mood_color(next.pet.mood);
    tft.fillRoundRect(kStateX + 2, kStateY + 2, kStateW - 4, kStateH - 4, 2, kCardBg);
    tft.drawRoundRect(kStateX, kStateY, kStateW, kStateH, 4, color);
    tft.drawRoundRect(kStateX + 1, kStateY + 1, kStateW - 2, kStateH - 2, 3, color);
    tft.setTextDatum(C_BASELINE);
    // The existing body font includes accented ES/CAT glyphs, unlike GLCD font 1.
    tft.setFreeFont(FONT_BODY);
    const char* label = L(next.soil_review && next.pet.attention_mask == 0
                          ? PLANT_SOIL_REVIEW_KEY : state_key(next.pet));
    if (tft.textWidth(label) > kStateW - 8) tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, kCardBg);
    tft.drawString(label, kStateX + kStateW / 2, kStateY + kStateH / 2 + 5);
    tft.setTextFont(0);
}
int16_t number_key(float value, bool valid) {
    return valid ? static_cast<int16_t>(lroundf(constrain(value, -3000.0f, 3000.0f) * 10.0f)) : INT16_MIN;
}
bool sample_usable(const ReadingSampleStamp& sample, uint32_t now, uint8_t card) {
    return sample.sequence != 0
        && (sample.quality == ReadingSampleQuality::Valid || sample.quality == ReadingSampleQuality::Retained)
        && static_cast<uint32_t>(now - sample.sampled_at_ms) <= plant_pet_sample_max_age_ms(static_cast<PlantChannel>(card));
}
PlantLabCache build_next_cache(uint32_t now) {
    g_using_demo = demo_mode_is_active();
    const Reading& r = g_using_demo ? g_demo_plant_reading : g_real_plant_reading;
    PlantLabCache next;
    next.pet = (g_using_demo ? g_demo_pet : g_real_pet).snapshot(now);
    next.soil_review = !g_using_demo && g_applied_soil_review;
    next.soil_x10 = number_key(r.soil_humidity, valid_number(r.soil_humidity) && sample_usable(r.soil_sample,now,0));
    next.temp_x10 = number_key(r.temperature, valid_number(r.temperature) && sample_usable(r.temperature_sample,now,1));
    next.hum_x10 = number_key(r.humidity, valid_number(r.humidity) && sample_usable(r.humidity_sample,now,2));
    next.lux = light_lux_valid(r.ldr) && sample_usable(r.light_sample,now,3)
        ? static_cast<uint32_t>(lroundf(r.ldr)) : kInvalidLux;
    next.animation_tick = now / kAnimationStepMs;
    next.fahrenheit = g_is_fahrenheit;
    next.chrome_done = true;
    return next;
}

void draw_plant_area(const PlantLabCache& next, uint32_t now) {
    PlantPetInputs visual = {};
    for (uint8_t card = 0; card < 4; ++card) {
        visual.values[card] = row_value_float(card, next);
        visual.valid[card] = row_value_valid(card, next);
    }
    plant_pet_render(g_plant_pixels, next.pet, now, &visual,
                     &(g_using_demo ? g_demo_pet : g_real_pet).profile());
    // Preserve the six top-corner pixels of the static radius-4 border.
    g_plant_pixels[0] = g_plant_pixels[1] = g_plant_pixels[PLANT_PET_ART_W] = kIdleBorder;
    g_plant_pixels[PLANT_PET_ART_W - 1] = g_plant_pixels[PLANT_PET_ART_W - 2]
        = g_plant_pixels[2 * PLANT_PET_ART_W - 1] = kIdleBorder;
    // Buffer holds native RGB565 words. Preserve swap state used by other screens.
    const bool previous_swap = tft.getSwapBytes();
    tft.setSwapBytes(true);
    tft.pushImage(kTerrariumX + 1, kTerrariumY + 1,
                  PLANT_PET_ART_W, PLANT_PET_ART_H, g_plant_pixels);
    tft.setSwapBytes(previous_swap);
}
void draw_chrome() {
    tft.fillScreen(screen_bg());
    drawHeader(L(TIT_LAB_PLANT));
    tft.fillRoundRect(kTerrariumX, kTerrariumY, kTerrariumW, kTerrariumH, 4, kCardBg);
    tft.drawRoundRect(kTerrariumX, kTerrariumY, kTerrariumW, kTerrariumH, 4, kIdleBorder);
    for (uint8_t card = 0; card < 4; ++card) draw_row_shell(card);
    tft.fillRoundRect(kStateX, kStateY, kStateW, kStateH, 4, kCardBg);
    tft.drawRoundRect(kStateX, kStateY, kStateW, kStateH, 4, kIdleBorder);
}
} // namespace

void update_lab_plant_readings(const Reading& r, uint32_t now_ms, bool demo) {
    if (demo) g_demo_plant_reading = r;
    else {
        g_real_plant_reading = r;
        const PlantSettingsSnapshot settings = get_plant_settings_snapshot();
        if (settings.revision != g_applied_settings_revision || settings.soil_needs_review != g_applied_soil_review) {
            const PlantPetProfile& profile = settings.settings.mode == PlantProfileMode::Custom
                ? settings.settings.custom_profile : plant_pet_general_profile();
            g_real_pet.setProfile(profile, now_ms, true);
            g_applied_settings_revision = settings.revision;
            g_applied_soil_review = settings.soil_needs_review;
            g_plant_cache.chrome_done = false;
        }
    }
    PlantPetInputs input = {};
    input.values[0]=r.soil_humidity; input.values[1]=r.temperature;
    input.values[2]=r.humidity; input.values[3]=r.ldr;
    const ReadingSampleStamp stamps[] = {r.soil_sample,r.temperature_sample,r.humidity_sample,r.light_sample};
    for (uint8_t card=0;card<4;++card) {
        input.valid[card] = card == 3 ? light_lux_valid(input.values[card]) : valid_number(input.values[card]);
        input.samples[card] = {stamps[card].sequence,stamps[card].sampled_at_ms,
            stamps[card].quality == ReadingSampleQuality::Valid || stamps[card].quality == ReadingSampleQuality::Retained};
    }
    if (!demo && g_applied_soil_review) input.valid[0]=false;
    input.has_sample_metadata=true;
    (demo ? g_demo_pet : g_real_pet).update(input, now_ms);
}

void draw_lab_plant_screen(bool screen_changed, bool sensor_data_changed) {
    const uint32_t now = millis();
    const PlantLabCache next = build_next_cache(now);
    const bool first = screen_changed || !g_plant_cache.chrome_done;
    const bool state_dirty = first || next.pet.mood != g_plant_cache.pet.mood
        || next.pet.culprit != g_plant_cache.pet.culprit
        || next.pet.affected_mask != g_plant_cache.pet.affected_mask
        || next.pet.unknown_mask != g_plant_cache.pet.unknown_mask
        || next.pet.recovering != g_plant_cache.pet.recovering
        || next.soil_review != g_plant_cache.soil_review
        || next.pet.profile_revision != g_plant_cache.pet.profile_revision;
    bool bands_dirty = first;
    bool rows_dirty[4];
    bool any_row_dirty = false;
    for (uint8_t card = 0; card < 4; ++card) {
        bands_dirty |= next.pet.bands[card] != g_plant_cache.pet.bands[card];
        rows_dirty[card] = first || row_data_dirty(card, next);
        any_row_dirty |= rows_dirty[card];
    }
    const bool anim_dirty = first || next.animation_tick != g_plant_cache.animation_tick;
    const bool environment_dirty = first || next.soil_x10 != g_plant_cache.soil_x10
        || next.temp_x10 != g_plant_cache.temp_x10 || next.hum_x10 != g_plant_cache.hum_x10
        || next.lux != g_plant_cache.lux;
    (void)sensor_data_changed; // Quantized fields and animation own their dirty keys.
    if (!first && !state_dirty && !bands_dirty && !anim_dirty && !environment_dirty && !any_row_dirty) return;
    if (first) draw_chrome();
    if (anim_dirty || state_dirty || bands_dirty || environment_dirty) draw_plant_area(next, now);
    for (uint8_t card = 0; card < 4; ++card) {
        if (rows_dirty[card]) draw_row_data(card, next);
        if (state_dirty || bands_dirty) draw_row_border(card, next.pet);
    }
    if (state_dirty) draw_state_card(next);
    g_plant_cache = next;
}
