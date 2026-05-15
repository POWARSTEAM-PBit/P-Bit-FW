#include "ui_lab_sound_vu.h"

#include "alert_engine.h"
#include "fonts.h"
#include "hw.h"
#include "languages.h"
#include "layout.h"
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
constexpr const char* kSoundLabTitle = "SOUND LAB";

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
    for (int i = 0; i < count; ++i) {
        history[i] = value;
    }
}

static void push_history(uint8_t* history, int count, uint8_t value) {
    for (int i = 0; i < count - 1; ++i) {
        history[i] = history[i + 1];
    }
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
    if (!valid) {
        return {L(ST_WAITING), TFT_DARKGREY, 255};
    }

    const int normal_max = get_sound_threshold_normal();
    const int loud_max = get_sound_threshold_loud();

    if ((int)level < normal_max) {
        return {L(ST_NORMAL), kNeonGreen, 0};
    }
    if ((int)level < loud_max) {
        return {L(ST_LOUD), kWarmOrange, 2};
    }
    return {L(ST_VERY_LOUD), TFT_RED, 3};
}

static void draw_sound_alert_jewel(int cx, int cy, uint8_t alert_code, bool alerts_enabled) {
    AlertJewelState jewel_state = ALERT_JEWEL_OK;
    uint16_t jewel_color = TFT_GREEN;

    if (!alerts_enabled) {
        jewel_state = ALERT_JEWEL_OFF;
        jewel_color = TFT_DARKGREY;
    } else if (alert_code == ALERT_CODE_HIGH) {
        jewel_state = ALERT_JEWEL_WARN;
        jewel_color = TFT_ORANGE;
    } else if (alert_code == ALERT_CODE_CRITICAL) {
        jewel_state = ALERT_JEWEL_CRIT;
        jewel_color = TFT_RED;
    }

    drawAlertJewel(cx, cy, jewel_state, jewel_color);
}

static void draw_panel_title_chip(int x, int y, int w, uint16_t bg, uint16_t accent, const char* label) {
    (void)w;
    (void)bg;
    pbit_draw_sound_icon(x + 10, y + 12, accent);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kPanel);
    tft.drawString(label, x + 20, y + 5);
    tft.setTextFont(0);
}

static void draw_value_badge(int right_x, int y, bool valid, uint8_t level, uint16_t color) {
    tft.setTextDatum(TR_DATUM);
    tft.setFreeFont(FONT_TIMER);
    tft.setTextColor(valid ? color : TFT_DARKGREY, kBg);

    char value_buf[10];
    if (valid) {
        snprintf(value_buf, sizeof(value_buf), "%u%%", (unsigned)level);
    } else {
        snprintf(value_buf, sizeof(value_buf), "--");
    }
    tft.drawString(value_buf, right_x, y);
    tft.setTextFont(0);
}

static void draw_status_footer(const SoundVisual& visual) {
    tft.fillRect(4, 111, 152, 12, kPanel);
    drawFooterHint(visual.label, tft.width() / 2, 117, visual.color);
}

static uint16_t stack_segment_color(int segment_index) {
    if (segment_index >= kStackSegments - 2) return TFT_RED;
    if (segment_index >= kStackSegments - 4) return kWarmOrange;
    if (segment_index >= kStackSegments - 6) return kNeonYellow;
    return kNeonGreen;
}

static void draw_stack_meter(int x, int y, int w, int h) {
    const int col_gap = 3;
    const int seg_gap = 1;
    const int col_w = (w - ((kStackCols - 1) * col_gap)) / kStackCols;
    const int seg_h = (h - ((kStackSegments - 1) * seg_gap)) / kStackSegments;

    for (int i = 0; i < kStackCols; ++i) {
        const int lit = (int)roundf(((float)g_stack_history[i] / 100.0f) * (float)kStackSegments);
        const int sx = x + i * (col_w + col_gap);

        for (int seg = 0; seg < kStackSegments; ++seg) {
            const int sy = y + h - seg_h - (seg * (seg_h + seg_gap));
            uint16_t color = tft.color565(12, 20, 16);
            if (seg < lit) {
                color = stack_segment_color(seg);
                if (seg == lit - 1 && lit > 0) {
                    color = TFT_WHITE;
                }
            }
            tft.fillRoundRect(sx, sy, col_w, seg_h, 1, color);
        }
    }
}

static uint16_t wave_bar_color(uint8_t sample, int distance) {
    if (sample >= 85) return TFT_RED;
    if (sample >= 65) return kWarmOrange;
    if (sample >= 40) return kHotPink;
    return (distance <= 1) ? kWaveBlue : kElectricBlue;
}

static void draw_wave_meter(int x, int center_y, int w, int max_half_h) {
    const int bars = (kWavePairs * 2) + 1;
    const int gap = 3;
    const int bar_w = (w - ((bars - 1) * gap)) / bars;

    tft.drawFastHLine(x, center_y, w, tft.color565(18, 42, 58));

    for (int i = 0; i < bars; ++i) {
        const int distance = abs(i - kWavePairs);
        const int history_index = (kWaveHistory - 1) - distance;
        const uint8_t sample = g_wave_history[history_index];
        const int half_h = 3 + (int)roundf(((float)sample / 100.0f) * (float)max_half_h);
        const int sx = x + i * (bar_w + gap);
        const uint16_t color = wave_bar_color(sample, distance);

        tft.fillRoundRect(sx, center_y - half_h - 2, bar_w, half_h, 2, color);
        tft.fillRoundRect(sx, center_y + 3, bar_w, half_h, 2, color);
    }
}

static void draw_stack_shell() {
    tft.fillScreen(kBg);
    drawHeader(kSoundLabTitle);
}

static void draw_stack_dynamic(bool valid, uint8_t level, const SoundVisual& visual, uint8_t alert_code, bool alerts_enabled) {
    const int card_h = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);
    tft.fillRoundRect(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanel);
    tft.drawRoundRect(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanelBorder);

    draw_panel_title_chip(12, 26, 52, kPanelAlt, kNeonGreen, L(LAB_SOUND_SHORT));
    draw_value_badge(150, 25, valid, level, visual.color);

    tft.fillRoundRect(12, 54, 136, 56, 4, TFT_BLACK);
    tft.drawRoundRect(12, 54, 136, 56, 4, tft.color565(16, 70, 40));
    draw_stack_meter(16, 58, 128, 48);
    draw_status_footer(visual);
    draw_sound_alert_jewel(14, 119, alert_code, alerts_enabled);
}

static void draw_wave_shell() {
    tft.fillScreen(kBg);
    drawHeader(kSoundLabTitle);
}

static void draw_wave_dynamic(bool valid, uint8_t level, const SoundVisual& visual, uint8_t alert_code, bool alerts_enabled) {
    const int card_h = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;
    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);
    tft.fillRoundRect(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, card_h, LC_CARD_RADIUS, kPanel);
    tft.drawRoundRect(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, card_h, LC_CARD_RADIUS, tft.color565(36, 80, 110));

    draw_panel_title_chip(12, 26, 52, kPanelAlt, kWaveBlue, L(LAB_SOUND_SHORT));
    draw_value_badge(150, 25, valid, level, visual.color);

    tft.fillRoundRect(12, 54, 136, 56, 4, TFT_BLACK);
    tft.drawRoundRect(12, 54, 136, 56, 4, tft.color565(22, 54, 76));
    draw_wave_meter(16, 82, 128, 20);
    draw_status_footer(visual);
    draw_sound_alert_jewel(14, 119, alert_code, alerts_enabled);
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
    const bool dynamic_dirty = !g_stack_cache.valid
        || (g_stack_cache.sound_valid != valid)
        || (g_stack_cache.level != (int)level)
        || (g_stack_cache.alert_code != alert_code)
        || (g_stack_cache.alerts_enabled != alerts_enabled)
        || (g_stack_cache.category_id != visual.category_id);

    if (screen_changed) {
        fill_history(g_stack_history, kStackCols, level);
        draw_stack_shell();
        draw_stack_dynamic(valid, level, visual, alert_code, alerts_enabled);
        commit_stack_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
        return;
    }

    if (sensor_data_changed && dynamic_dirty) {
        push_history(g_stack_history, kStackCols, level);
        draw_stack_dynamic(valid, level, visual, alert_code, alerts_enabled);
        commit_stack_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
    }
}

void draw_lab_sound_vu_wave_screen(bool screen_changed, bool sensor_data_changed) {
    bool valid = false;
    const uint8_t level = mic_level_key(&valid);
    const uint8_t alert_code = alert_engine_get_code(AlertSensor::Sound);
    const bool alerts_enabled = get_sound_alerts_enabled();
    const SoundVisual visual = describe_sound(valid, level);
    const bool dynamic_dirty = !g_wave_cache.valid
        || (g_wave_cache.sound_valid != valid)
        || (g_wave_cache.level != (int)level)
        || (g_wave_cache.alert_code != alert_code)
        || (g_wave_cache.alerts_enabled != alerts_enabled)
        || (g_wave_cache.category_id != visual.category_id);

    if (screen_changed) {
        fill_history(g_wave_history, kWaveHistory, level);
        draw_wave_shell();
        draw_wave_dynamic(valid, level, visual, alert_code, alerts_enabled);
        commit_wave_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
        return;
    }

    if (sensor_data_changed && dynamic_dirty) {
        push_history(g_wave_history, kWaveHistory, level);
        draw_wave_dynamic(valid, level, visual, alert_code, alerts_enabled);
        commit_wave_cache(valid, level, alert_code, alerts_enabled, visual.category_id);
    }
}
