#include "ui_lab_sound_vu.h"

#include "alert_engine.h"
#include "fonts.h"
#include "hw.h"
#include "languages.h"
#include "layout.h"
#include "sensor_zone.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

namespace {

constexpr uint16_t kBg = TFT_BLACK;
constexpr uint16_t kPanel = 0x1082;
constexpr uint16_t kPanelAlt = 0x0861;
constexpr uint16_t kPanelBorder = 0x2965;
constexpr uint16_t kNeonGreen = 0x3FE8;
constexpr uint16_t kNeonYellow = 0xFFE0;
constexpr uint16_t kElectricBlue = 0x35FF;
constexpr uint16_t kWaveBlue = 0x2CFF;
constexpr uint16_t kHotPink = 0xF81F;
constexpr uint16_t kWarmOrange = TFT_ORANGE;
constexpr int kStackCols = 10;
constexpr int kStackSegments = 11;
constexpr int kWavePairs = 6;
constexpr int kWaveHistory = kWavePairs + 1;

// Compact top-right value field. It is intentionally text-only: no badge/card
// under the number, and the clear rect stays far away from MIC/title/VU chrome.
constexpr int kValueFieldX = 101;
constexpr int kValueFieldY = 31;
constexpr int kValueFieldW = 49;
constexpr int kValueFieldH = 22;
constexpr int kValueTextX = kValueFieldX + kValueFieldW - 1;
constexpr int kValueTextY = kValueFieldY;
constexpr int kSoundCardTop = LC_CARD_TOP + 3;
constexpr int kVuCardX = 12;
constexpr int kVuCardY = 55;
constexpr int kVuCardW = 136;
constexpr int kVuCardH = 54;

struct SoundVisual {
    const char* label;
    uint16_t color;
    uint8_t category_id;
};

struct VuCache {
    bool valid = false;
    bool sound_valid = false;
    int level = INT_MIN;
    uint8_t alert_code = ALERT_CODE_OFF;
    bool alerts_enabled = false;
    uint8_t category_id = 255;
};

static VuCache g_stack_cache;
static VuCache g_wave_cache;
static uint8_t g_stack_history[kStackCols] = {0};
static uint8_t g_wave_history[kWaveHistory] = {0};

static void fill_history(uint8_t* history, int count, uint8_t value) {
    for (int i = 0; i < count; ++i) history[i] = value;
}

static void push_history(uint8_t* history, int count, uint8_t value) {
    for (int i = 0; i < count - 1; ++i) history[i] = history[i + 1];
    history[count - 1] = value;
}

static uint8_t mic_level_key(bool* out_valid = nullptr) {
    const bool valid = !isnan(g_ui_readings_snapshot.mic);
    if (out_valid) *out_valid = valid;
    if (!valid) return 0;
    const int clamped = constrain((int)lroundf(g_ui_readings_snapshot.mic), 0, 100);
    return (uint8_t)clamped;
}

static SoundVisual describe_sound(bool valid, uint8_t level) {
    if (!valid) return {L(ST_WAITING), TFT_DARKGREY, 255};
    const int normal_max = get_sound_threshold_normal();
    const int loud_max   = get_sound_threshold_loud();
    if ((int)level < normal_max) return {L(ST_NORMAL),    kNeonGreen,  0};
    if ((int)level < loud_max)   return {L(ST_LOUD),      kWarmOrange, 2};
    return                              {L(ST_VERY_LOUD), TFT_RED,     3};
}

static void draw_sound_alert_jewel(int cx, int cy, uint8_t alert_code, bool alerts_enabled) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;
    if (!alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;  jewel_color = TFT_DARKGREY;
    } else if (alert_code == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_WARN; jewel_color = TFT_ORANGE;
    } else if (alert_code == ALERT_CODE_CRITICAL) {
        jewel_state = ALERT_JEWEL_CRIT; jewel_color = TFT_RED;
    }
    drawAlertJewel(cx, cy, jewel_state, jewel_color);
}

static void draw_panel_title_chip(int x, int y, int w, uint16_t bg, uint16_t accent, const char* label) {
    (void)w;
    (void)bg;
    pbit_draw_sound_icon(x + 10, y + 17, accent);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kPanel);
    tft.drawString(label, x + 20, y + 9);
    tft.setTextFont(0);
}

static void draw_value_number(bool valid, uint8_t level, uint16_t color) {
    char value_buf[10];
    if (valid) {
        snprintf(value_buf, sizeof(value_buf), "%u%%", (unsigned)level);
    } else {
        snprintf(value_buf, sizeof(value_buf), "--");
    }

    tft.fillRect(kValueFieldX, kValueFieldY, kValueFieldW, kValueFieldH, kPanel);
    tft.setTextDatum(TR_DATUM);
    tft.setFreeFont(FONT_HEADER);
    if (tft.textWidth(value_buf) > kValueFieldW) {
        tft.setFreeFont(FONT_BODY);
    }
    tft.setTextColor(valid ? color : TFT_DARKGREY, kPanel);
    tft.drawString(value_buf, kValueTextX, kValueTextY);
    tft.setTextFont(0);
}

static void draw_status_footer(const SoundVisual& visual) {
    tft.fillRect(4, 110, 152, 12, kPanel);
    drawFooterHint(visual.label, tft.width() / 2, 116, visual.color);
}

static uint16_t stack_segment_color(int segment_index) {
    if (segment_index >= kStackSegments - 2) return TFT_RED;
    if (segment_index >= kStackSegments - 4) return kWarmOrange;
    if (segment_index >= kStackSegments - 6) return kNeonYellow;
    return kNeonGreen;
}

// Sprite for the stack meter — allocated once, reused every frame.
// Converts 110 individual SPI calls into a single DMA pushSprite burst.
static TFT_eSprite g_stack_spr(&tft);
static bool        g_stack_spr_ready = false;

static void draw_stack_meter(int x, int y, int w, int h) {
    if (!g_stack_spr_ready) {
        g_stack_spr.createSprite(w, h);
        g_stack_spr_ready = true;
    }
    const int col_gap = 3;
    const int seg_gap = 1;
    const int col_w = (w - ((kStackCols - 1) * col_gap)) / kStackCols;
    const int seg_h = (h - ((kStackSegments - 1) * seg_gap)) / kStackSegments;
    const uint16_t kDark = g_stack_spr.color565(12, 20, 16);
    g_stack_spr.fillSprite(kDark);

    // Idle pulse: triangular wave ~1 Hz (32 steps × ~30 ms ≈ 960 ms period).
    // Shown as one dim teal-green segment at the base of every silent column.
    static uint8_t s_pulse = 0;
    s_pulse = (s_pulse + 1) % 32;
    const uint8_t t = (s_pulse < 16) ? s_pulse : (32 - s_pulse);  // 0→15→0
    const uint16_t idle_color = g_stack_spr.color565(0, (uint8_t)(8 + t * 4), 4);

    for (int i = 0; i < kStackCols; ++i) {
        const int lit = (int)roundf(((float)g_stack_history[i] / 100.0f) * (float)kStackSegments);
        const int sx = i * (col_w + col_gap);
        if (lit == 0) {
            // Idle pulse: one breathing segment at the bottom — signals "listening".
            g_stack_spr.fillRect(sx, h - seg_h, col_w, seg_h, idle_color);
            continue;
        }
        for (int seg = 0; seg < lit; ++seg) {
            const int sy = h - seg_h - (seg * (seg_h + seg_gap));
            uint16_t color = stack_segment_color(seg);
            if (seg == lit - 1) color = TFT_WHITE;
            g_stack_spr.fillRect(sx, sy, col_w, seg_h, color);
        }
    }
    g_stack_spr.pushSprite(x, y);
}

static uint16_t wave_bar_color(uint8_t sample, int distance) {
    if (sample >= 85) return TFT_RED;
    if (sample >= 65) return kWarmOrange;
    if (sample >= 40) return kHotPink;
    return (distance <= 1) ? kWaveBlue : kElectricBlue;
}

// Sprite for the wave meter.
// Sprite height covers the max possible bar extent (±(3+max_half_h+2) from center).
// Push origin: (x, center_y - kWaveScy) on screen.
static TFT_eSprite g_wave_spr(&tft);
static bool        g_wave_spr_ready = false;
static constexpr int kWaveScy = 25;
static constexpr int kWaveSph = 52;

static void draw_wave_meter(int x, int center_y, int w, int max_half_h) {
    if (!g_wave_spr_ready) {
        g_wave_spr.createSprite(w, kWaveSph);
        g_wave_spr_ready = true;
    }
    g_wave_spr.fillSprite(TFT_BLACK);
    g_wave_spr.drawFastHLine(0, kWaveScy, w, g_wave_spr.color565(18, 42, 58));
    const int bars = (kWavePairs * 2) + 1;
    const int gap = 3;
    const int bar_w = (w - ((bars - 1) * gap)) / bars;
    for (int i = 0; i < bars; ++i) {
        const int distance = abs(i - kWavePairs);
        const int history_index = (kWaveHistory - 1) - distance;
        const uint8_t sample = g_wave_history[history_index];
        const int half_h = 3 + (int)roundf(((float)sample / 100.0f) * (float)max_half_h);
        const int sx = i * (bar_w + gap);
        const uint16_t color = wave_bar_color(sample, distance);
        g_wave_spr.fillRoundRect(sx, kWaveScy - half_h - 2, bar_w, half_h, 2, color);
        g_wave_spr.fillRoundRect(sx, kWaveScy + 3,          bar_w, half_h, 2, color);
    }
    g_wave_spr.pushSprite(x, center_y - kWaveScy);
}

// Card chrome for the stack view: card bg, border, title chip, meter frame, footer, jewel.
// Does NOT draw the meter sprite or value number — those update on every sensor sample.
// Called only on screen_changed or meta_dirty (category / alert / valid state changed).
static void draw_stack_chrome(const SoundVisual& visual, uint8_t alert_code, bool alerts_enabled) {
    const int card_h = LC_SCREEN_BOTTOM - kSoundCardTop + 1;
    // Clear only the narrow gap between header and card (7 px) — no full-screen black flash.
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), LC_CARD_TOP - L_CONTENT_TOP, kBg);
    tft.fillRoundRect(LC_SCREEN_X, kSoundCardTop, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanel);
    tft.drawRoundRect(LC_SCREEN_X, kSoundCardTop, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanelBorder);
    draw_panel_title_chip(12, 25, 52, kPanelAlt, kNeonGreen, L(LAB_SOUND_SHORT));
    tft.fillRoundRect(kVuCardX, kVuCardY, kVuCardW, kVuCardH, 4, TFT_BLACK);
    tft.drawRoundRect(kVuCardX, kVuCardY, kVuCardW, kVuCardH, 4, tft.color565(16, 70, 40));
    draw_status_footer(visual);
    draw_sound_alert_jewel(14, 119, alert_code, alerts_enabled);
}

// Card chrome for the wave view.
static void draw_wave_chrome(const SoundVisual& visual, uint8_t alert_code, bool alerts_enabled) {
    const int card_h = LC_SCREEN_BOTTOM - kSoundCardTop + 1;
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), LC_CARD_TOP - L_CONTENT_TOP, kBg);
    tft.fillRoundRect(LC_SCREEN_X, kSoundCardTop, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanel);
    tft.drawRoundRect(LC_SCREEN_X, kSoundCardTop, LC_SCREEN_W, card_h, LC_CARD_RADIUS, tft.color565(36, 80, 110));
    draw_panel_title_chip(12, 25, 52, kPanelAlt, kWaveBlue, L(LAB_SOUND_SHORT));
    tft.fillRoundRect(kVuCardX, kVuCardY, kVuCardW, kVuCardH, 4, TFT_BLACK);
    tft.drawRoundRect(kVuCardX, kVuCardY, kVuCardW, kVuCardH, 4, tft.color565(22, 54, 76));
    draw_status_footer(visual);
    draw_sound_alert_jewel(14, 119, alert_code, alerts_enabled);
}

static void draw_stack_shell() {
    tft.fillScreen(kBg);
    if (!sz_is_active()) drawHeader(L(TIT_LAB_VU_STACK));
}

static void draw_wave_shell() {
    tft.fillScreen(kBg);
    if (!sz_is_active()) drawHeader(L(TIT_LAB_VU_WAVE));
}

static void commit_stack_cache(bool valid, uint8_t level, uint8_t alert_code, bool alerts_enabled, uint8_t category_id) {
    g_stack_cache.valid = true;
    g_stack_cache.sound_valid = valid;
    g_stack_cache.level = (int)level;
    g_stack_cache.alert_code = alert_code;
    g_stack_cache.alerts_enabled = alerts_enabled;
    g_stack_cache.category_id = category_id;
}

static void commit_wave_cache(bool valid, uint8_t level, uint8_t alert_code, bool alerts_enabled, uint8_t category_id) {
    g_wave_cache.valid = true;
    g_wave_cache.sound_valid = valid;
    g_wave_cache.level = (int)level;
    g_wave_cache.alert_code = alert_code;
    g_wave_cache.alerts_enabled = alerts_enabled;
    g_wave_cache.category_id = category_id;
}

} // namespace

void draw_lab_sound_vu_stack_screen(bool screen_changed, bool sensor_data_changed) {
    bool valid = false;
    const uint8_t level = mic_level_key(&valid);
    const uint8_t alert_code = alert_engine_get_code(AlertSensor::Sound);
    const bool alerts_enabled = get_sound_alerts_enabled();
    const SoundVisual visual = describe_sound(valid, level);

    // Asymmetric EWMA for the scrolling history: instant rise, smooth ~250 ms decay.
    // Gives the classic VU "ballistic" feel — meter rises fast, falls gracefully.
    // The value numeral always shows the real level, not the smoothed one.
    static float s_stack_smooth = 0.0f;

    // meta_dirty: card chrome must refresh (category, alert state, or valid state changed).
    const bool meta_dirty = !g_stack_cache.valid
        || (g_stack_cache.sound_valid != valid)
        || (g_stack_cache.alert_code != alert_code)
        || (g_stack_cache.alerts_enabled != alerts_enabled)
        || (g_stack_cache.category_id != visual.category_id);
    // badge_dirty: numeral or value color changed.
    const bool badge_dirty = meta_dirty || (g_stack_cache.level != (int)level);

    if (screen_changed) {
        s_stack_smooth = (float)level;  // reset EWMA on screen enter — no stale hold
        fill_history(g_stack_history, kStackCols, level);
        draw_stack_shell();
        draw_stack_chrome(visual, alert_code, alerts_enabled);
        draw_value_number(valid, level, visual.color);
        draw_stack_meter(16, 57, 128, 48);
        commit_stack_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
        return;
    }

    if (!sensor_data_changed) return;

    // Apply asymmetric EWMA then push the smoothed value into the scrolling history.
    if (!valid) {
        s_stack_smooth = 0.0f;                                          // instant zero on no-sensor
    } else if ((float)level > s_stack_smooth) {
        s_stack_smooth = (float)level;                                  // instant rise
    } else {
        s_stack_smooth = 0.78f * s_stack_smooth + 0.22f * (float)level; // smooth decay (~250 ms)
    }
    const uint8_t smooth_level = (uint8_t)lroundf(s_stack_smooth);

    // Always push history and redraw the sprite — scrolling columns animate every sample
    // regardless of whether the rounded integer changed (fixes the intermittent freeze).
    push_history(g_stack_history, kStackCols, smooth_level);
    if (meta_dirty)  draw_stack_chrome(visual, alert_code, alerts_enabled);
    if (badge_dirty) draw_value_number(valid, level, visual.color);
    draw_stack_meter(16, 57, 128, 48);  // single DMA pushSprite — no chrome flicker
    commit_stack_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
}

void draw_lab_sound_vu_wave_screen(bool screen_changed, bool sensor_data_changed) {
    bool valid = false;
    const uint8_t level = mic_level_key(&valid);
    const uint8_t alert_code = alert_engine_get_code(AlertSensor::Sound);
    const bool alerts_enabled = get_sound_alerts_enabled();
    const SoundVisual visual = describe_sound(valid, level);

    // Asymmetric EWMA: instant rise, smooth ~200 ms decay (faster than stack — wave feels "live").
    static float s_wave_smooth = 0.0f;

    const bool meta_dirty = !g_wave_cache.valid
        || (g_wave_cache.sound_valid != valid)
        || (g_wave_cache.alert_code != alert_code)
        || (g_wave_cache.alerts_enabled != alerts_enabled)
        || (g_wave_cache.category_id != visual.category_id);
    const bool badge_dirty = meta_dirty || (g_wave_cache.level != (int)level);

    if (screen_changed) {
        s_wave_smooth = (float)level;  // reset EWMA on screen enter
        fill_history(g_wave_history, kWaveHistory, level);
        draw_wave_shell();
        draw_wave_chrome(visual, alert_code, alerts_enabled);
        draw_value_number(valid, level, visual.color);
        draw_wave_meter(16, 81, 128, 20);
        commit_wave_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
        return;
    }

    if (!sensor_data_changed) return;

    // Apply asymmetric EWMA then push smoothed value into the wave history.
    if (!valid) {
        s_wave_smooth = 0.0f;
    } else if ((float)level > s_wave_smooth) {
        s_wave_smooth = (float)level;                                    // instant rise
    } else {
        s_wave_smooth = 0.75f * s_wave_smooth + 0.25f * (float)level;   // smooth decay (~200 ms)
    }
    const uint8_t smooth_wave_level = (uint8_t)lroundf(s_wave_smooth);

    push_history(g_wave_history, kWaveHistory, smooth_wave_level);
    if (meta_dirty)  draw_wave_chrome(visual, alert_code, alerts_enabled);
    if (badge_dirty) draw_value_number(valid, level, visual.color);
    draw_wave_meter(16, 81, 128, 20);
    commit_wave_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
}
