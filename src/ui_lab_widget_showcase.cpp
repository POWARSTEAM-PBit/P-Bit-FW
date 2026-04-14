#include "ui_lab_widget_showcase.h"

#include "fonts.h"
#include "graph_buffer.h"
#include "io.h"
#include "languages.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <climits>
#include <math.h>
#include <stdio.h>

extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;
extern bool g_is_fahrenheit;
extern portMUX_TYPE g_graph_mux;

namespace {

constexpr int kFooterY = 121;
constexpr uint16_t kBg = TFT_BLACK;
constexpr uint16_t kDarkFrame = 0x2104;
constexpr uint16_t kCoolBlue = TFT_CYAN;
constexpr uint16_t kWarmOrange = TFT_ORANGE;
constexpr uint16_t kHotPink = TFT_MAGENTA;
constexpr uint16_t kNeonGreen = 0x3FE8;
constexpr uint16_t kElectricBlue = 0x35FF;
constexpr uint16_t kRoyalBlue = 0x21D9;
constexpr uint16_t kNeonYellow = 0xFFE0;
constexpr uint16_t kDeepPurple = 0x881F;

struct TempRenderCache {
    bool valid = false;
    bool temp_valid = false;
    int temp10 = INT_MIN;
    bool fahrenheit = false;
};

struct WidgetMixCache {
    bool valid = false;
    bool hum_valid = false;
    int hum = INT_MIN;
    bool light_valid = false;
    int light = INT_MIN;
    bool sound_valid = false;
    int sound = INT_MIN;
};

static TempRenderCache g_gauge_cache;
static TempRenderCache g_value_cache;
static WidgetMixCache g_mix_cache;

static float display_temp(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static int display_temp_key(bool* out_valid = nullptr) {
    const bool valid = !isnan(g_ui_readings_snapshot.temperature);
    if (out_valid) *out_valid = valid;
    if (!valid) return INT_MIN;
    return (int)lroundf(display_temp(g_ui_readings_snapshot.temperature) * 10.0f);
}

static const char* temp_unit() {
    return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
}

static void draw_solid_temp_large(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 7, cy - 18, 14, 24, 6, color);
    tft.fillCircle(cx, cy + 8, 10, color);
    tft.fillRoundRect(cx - 2, cy - 13, 4, 16, 2, TFT_BLACK);
}

static void draw_solid_drop_large(int cx, int cy, uint16_t color) {
    tft.fillTriangle(cx, cy - 18, cx - 12, cy - 3, cx + 12, cy - 3, color);
    tft.fillCircle(cx, cy + 6, 12, color);
    tft.fillCircle(cx, cy + 8, 5, TFT_BLACK);
}

static void draw_solid_light_large(int cx, int cy, uint16_t color) {
    tft.fillCircle(cx, cy, 10, color);
    tft.fillRoundRect(cx - 2, cy - 20, 4, 7, 2, color);
    tft.fillRoundRect(cx - 2, cy + 14, 4, 7, 2, color);
    tft.fillRoundRect(cx - 20, cy - 2, 7, 4, 2, color);
    tft.fillRoundRect(cx + 14, cy - 2, 7, 4, 2, color);
    tft.fillRoundRect(cx - 15, cy - 15, 4, 7, 2, color);
    tft.fillRoundRect(cx + 11, cy - 15, 4, 7, 2, color);
    tft.fillRoundRect(cx - 15, cy + 8, 4, 7, 2, color);
    tft.fillRoundRect(cx + 11, cy + 8, 4, 7, 2, color);
}

static void draw_solid_mic_large(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 8, cy - 16, 16, 22, 7, color);
    tft.drawFastVLine(cx, cy + 6, 8, color);
    tft.drawFastHLine(cx - 11, cy + 14, 23, color);
}

static void draw_section_label(const char* text, int x, int y, uint16_t color) {
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, kBg);
    tft.drawString(text, x, y);
    tft.setTextFont(0);
}

static void draw_compact_footer() {
    tft.setTextDatum(TC_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(TFT_DARKGREY, kBg);
    tft.drawString(L(LAB_EXPERIMENT_HINT), 80, 119);
    tft.setTextFont(0);
}

static void draw_temp_ring(int cx, int cy, int radius, int thickness, float ratio) {
    static const uint16_t gradient[] = {
        kElectricBlue, kCoolBlue, kNeonGreen, kNeonYellow, kWarmOrange, kHotPink
    };
    const int segs = 54;
    const float start_deg = 135.0f;
    const float sweep_deg = 270.0f;
    const int active_until = (int)roundf(ratio * (float)segs);

    for (int i = 0; i < segs; ++i) {
        const float a0 = (start_deg + ((float)i * sweep_deg / (float)segs)) * DEG_TO_RAD;
        const float a1 = (start_deg + ((float)(i + 1) * sweep_deg / (float)segs)) * DEG_TO_RAD;
        const uint16_t color = (i < active_until)
            ? gradient[(i * (int)(sizeof(gradient) / sizeof(gradient[0]))) / segs]
            : tft.color565(26, 30, 42);

        for (int t = 0; t < thickness; ++t) {
            const int r = radius - t;
            const int x0 = cx + (int)roundf(cosf(a0) * (float)r);
            const int y0 = cy + (int)roundf(sinf(a0) * (float)r);
            const int x1 = cx + (int)roundf(cosf(a1) * (float)r);
            const int y1 = cy + (int)roundf(sinf(a1) * (float)r);
            tft.drawLine(x0, y0, x1, y1, color);
        }
    }
}

static void draw_sparkline(int x, int y, int w, int h, const GraphBuffer& buf, uint16_t line_color, uint16_t grid_color) {
    float data[GRAPH_BUFFER_SIZE];
    size_t count = 0;
    portENTER_CRITICAL(&g_graph_mux);
    count = graph_buffer_get(buf, data, GRAPH_BUFFER_SIZE);
    portEXIT_CRITICAL(&g_graph_mux);

    tft.fillRect(x, y, w, h, kBg);
    for (int gy = 0; gy < h; gy += 6) {
        tft.drawFastHLine(x, y + gy, w, grid_color);
    }
    for (int gx = 0; gx < w; gx += 8) {
        tft.drawFastVLine(x + gx, y, h, grid_color);
    }

    if (count < 2) return;

    const size_t start = (count > (size_t)w) ? (count - (size_t)w) : 0;
    float vmin = data[start];
    float vmax = data[start];
    for (size_t i = start + 1; i < count; ++i) {
        if (data[i] < vmin) vmin = data[i];
        if (data[i] > vmax) vmax = data[i];
    }
    if (fabsf(vmax - vmin) < 0.01f) {
        vmax += 1.0f;
        vmin -= 1.0f;
    }

    auto py = [&](float v) -> int {
        float ratio = (v - vmin) / (vmax - vmin);
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        return y + h - 1 - (int)roundf(ratio * (float)(h - 1));
    };

    const size_t visible = count - start;
    const int xoff = (visible < (size_t)w) ? (int)(w - visible) : 0;
    for (size_t i = start + 1; i < count; ++i) {
        const int x0 = x + xoff + (int)(i - 1 - start);
        const int y0 = py(data[i - 1]);
        const int x1 = x + xoff + (int)(i - start);
        const int y1 = py(data[i]);
        tft.drawLine(x0, y0, x1, y1, line_color);
    }
}

static void draw_segment_bar(int x, int y, int w, int h, int segments, float ratio, uint16_t on_color, uint16_t off_color) {
    const int gap = 2;
    const int seg_w = (w - ((segments - 1) * gap)) / segments;
    const int on_count = (int)roundf(ratio * (float)segments);
    for (int i = 0; i < segments; ++i) {
        const int sx = x + i * (seg_w + gap);
        tft.fillRoundRect(sx, y, seg_w, h, 2, (i < on_count) ? on_color : off_color);
    }
}

static void draw_vu_bars(int x, int y, int w, int h, int bars, float ratio, uint16_t on_color, uint16_t accent_color) {
    const int gap = 2;
    const int bar_w = (w - ((bars - 1) * gap)) / bars;
    const int peak = (int)roundf(ratio * (float)bars);
    for (int i = 0; i < bars; ++i) {
        const int level = ((i % 4) + 2) * 3;
        const int bh = min(h, level + (i * 2));
        const int by = y + h - bh;
        uint16_t active_color = on_color;
        if (i >= (bars * 2) / 3) {
            active_color = accent_color;
        } else if (i >= bars / 3) {
            active_color = kNeonYellow;
        }
        const uint16_t color = (i < peak) ? active_color : tft.color565(26, 24, 34);
        tft.fillRoundRect(x + i * (bar_w + gap), by, bar_w, bh, 1, color);
    }
}

static void draw_lab_gauge_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_GAUGE), TFT_WHITE);
    draw_compact_footer();
}

static void draw_lab_gauge_dynamic() {
    const float raw_temp = g_ui_readings_snapshot.temperature;
    const bool valid = !isnan(raw_temp);
    const float shown_temp = valid ? display_temp(raw_temp) : 0.0f;
    const float min_temp = g_is_fahrenheit ? 32.0f : 0.0f;
    const float max_temp = g_is_fahrenheit ? 122.0f : 50.0f;
    const float ratio = valid ? constrain((shown_temp - min_temp) / (max_temp - min_temp), 0.0f, 1.0f) : 0.0f;

    tft.fillRect(8, 34, 144, 80, kBg);
    draw_section_label(L(LAB_TEMP_SHORT), 12, 36, kWarmOrange);
    draw_solid_temp_large(26, 72, kWarmOrange);
    draw_temp_ring(98, 72, 28, 5, ratio);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    tft.setFreeFont(FONT_BODY);
    if (valid) {
        char temp_buf[16];
        snprintf(temp_buf, sizeof(temp_buf), "%.1f", shown_temp);
        tft.drawString(temp_buf, 98, 73);
    } else {
        tft.drawString("--", 98, 73);
    }
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(valid ? kHotPink : TFT_DARKGREY, kBg);
    tft.drawString(temp_unit(), 130, 73);
    tft.setTextFont(0);

    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(kElectricBlue, kBg);
    tft.drawString(g_is_fahrenheit ? "32F" : "0C", 18, 101);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(kHotPink, kBg);
    tft.drawString(g_is_fahrenheit ? "122F" : "50C", 148, 101);
    tft.setTextFont(0);
}

static void draw_lab_value_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_VALUE), TFT_WHITE);
    drawCard(12, 38, 136, 72, kHotPink);
    draw_compact_footer();

    tft.fillRoundRect(18, 44, 60, 18, 4, tft.color565(24, 14, 24));
    pbit_draw_temp_icon(28, 54, kWarmOrange);
    draw_section_label(L(LAB_TEMP_SHORT), 40, 46, TFT_WHITE);

    tft.fillRoundRect(88, 44, 48, 18, 4, tft.color565(8, 18, 28));
    draw_section_label("DHT11", 98, 46, kElectricBlue);
}

static void draw_lab_value_dynamic() {
    const float raw_temp = g_ui_readings_snapshot.temperature;
    const bool valid = !isnan(raw_temp);
    const float shown_temp = valid ? display_temp(raw_temp) : 0.0f;
    const float min_temp = g_is_fahrenheit ? 32.0f : 0.0f;
    const float max_temp = g_is_fahrenheit ? 122.0f : 50.0f;

    tft.fillRect(18, 62, 122, 42, kBg);
    tft.fillRect(22, 92, 116, 10, kBg);

    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    if (valid) {
        char temp_buf[16];
        snprintf(temp_buf, sizeof(temp_buf), "%.1f", shown_temp);
        tft.drawString(temp_buf, 24, 70);
    } else {
        tft.drawString("--", 24, 70);
    }

    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(valid ? kHotPink : TFT_DARKGREY, kBg);
    tft.drawString(temp_unit(), 108, 72);
    tft.setTextFont(0);

    draw_segment_bar(22, 94, 116, 6, 9,
                     valid ? constrain((shown_temp - min_temp) / (max_temp - min_temp), 0.0f, 1.0f) : 0.0f,
                     kWarmOrange,
                     tft.color565(30, 24, 32));

    draw_sparkline(88, 45, 44, 14, g_graph_temp, kElectricBlue, tft.color565(18, 20, 30));
}

static void draw_mix_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_WIDGETS), TFT_WHITE);
    drawCard(6, 36, 70, 42, kElectricBlue);
    drawCard(84, 36, 70, 42, kNeonYellow);
    drawCard(6, 86, 148, 28, kHotPink);
    draw_compact_footer();
}

static void draw_mix_hum_dynamic() {
    const bool hum_valid = !isnan(g_ui_readings_snapshot.humidity);
    const float hum = hum_valid ? g_ui_readings_snapshot.humidity : 0.0f;
    tft.fillRect(8, 38, 66, 38, kBg);
    pbit_draw_humidity_icon(18, 57, kElectricBlue);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, kBg);
    tft.drawString(L(LAB_HUM_SHORT), 31, 40);
    tft.setTextFont(0);
    drawFillTank(68, 42, 3, 28, kElectricBlue, hum_valid ? hum : 0.0f, 0.0f, 100.0f, 1);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(hum_valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    if (hum_valid) {
        char hum_buf[12];
        snprintf(hum_buf, sizeof(hum_buf), "%d %%", (int)roundf(hum));
        tft.drawString(hum_buf, 31, 59);
    } else {
        tft.drawString("--", 31, 59);
    }
    tft.setTextFont(0);
}

static void draw_mix_light_dynamic() {
    const bool light_valid = !isnan(g_ui_readings_snapshot.ldr);
    const float ldr = light_valid ? g_ui_readings_snapshot.ldr : 0.0f;

    tft.fillRect(86, 38, 66, 38, kBg);
    pbit_draw_light_icon(97, 57, kNeonYellow);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, kBg);
    tft.drawString(L(LAB_LIGHT_SHORT), 109, 40);
    tft.setTextFont(0);
    drawFillTank(146, 42, 3, 28, kNeonYellow, light_valid ? ldr : 0.0f, 0.0f, 1023.0f, 1);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(light_valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    if (light_valid) {
        char lux_buf[12];
        snprintf(lux_buf, sizeof(lux_buf), "%.0f lx", ldr);
        tft.drawString(lux_buf, 109, 59);
    } else {
        tft.drawString("--", 109, 59);
    }
    tft.setTextFont(0);
}

static void draw_mix_sound_dynamic() {
    const bool sound_valid = !isnan(g_ui_readings_snapshot.mic);
    const float mic = sound_valid ? g_ui_readings_snapshot.mic : 0.0f;

    tft.fillRect(8, 88, 144, 24, kBg);
    pbit_draw_sound_icon(19, 102, kHotPink);
    draw_section_label(L(LAB_SOUND_SHORT), 31, 91, TFT_WHITE);
    draw_vu_bars(59, 94, 53, 14, 10,
                 sound_valid ? constrain(mic / 100.0f, 0.0f, 1.0f) : 0.0f,
                 kNeonGreen,
                 TFT_RED);
    tft.setTextDatum(TR_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(sound_valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    if (sound_valid) {
        char mic_buf[12];
        snprintf(mic_buf, sizeof(mic_buf), "%.0f%%", mic);
        tft.drawString(mic_buf, 148, 91);
    } else {
        tft.drawString("--", 148, 91);
    }
    tft.setTextFont(0);
}

} // namespace

void draw_lab_gauge_temp_screen(bool screen_changed, bool sensor_data_changed) {
    const bool temp_valid = !isnan(g_ui_readings_snapshot.temperature);
    const int temp_key = display_temp_key();
    const bool dynamic_dirty = !g_gauge_cache.valid
        || (g_gauge_cache.temp_valid != temp_valid)
        || (g_gauge_cache.temp10 != temp_key)
        || (g_gauge_cache.fahrenheit != g_is_fahrenheit);

    if (screen_changed) {
        draw_lab_gauge_shell();
        draw_lab_gauge_dynamic();
        g_gauge_cache.valid = true;
        g_gauge_cache.temp_valid = temp_valid;
        g_gauge_cache.temp10 = temp_key;
        g_gauge_cache.fahrenheit = g_is_fahrenheit;
        return;
    }
    if (sensor_data_changed && dynamic_dirty) {
        draw_lab_gauge_dynamic();
        g_gauge_cache.valid = true;
        g_gauge_cache.temp_valid = temp_valid;
        g_gauge_cache.temp10 = temp_key;
        g_gauge_cache.fahrenheit = g_is_fahrenheit;
    }
}

void draw_lab_value_modern_screen(bool screen_changed, bool sensor_data_changed) {
    const bool temp_valid = !isnan(g_ui_readings_snapshot.temperature);
    const int temp_key = display_temp_key();
    const bool dynamic_dirty = !g_value_cache.valid
        || (g_value_cache.temp_valid != temp_valid)
        || (g_value_cache.temp10 != temp_key)
        || (g_value_cache.fahrenheit != g_is_fahrenheit);

    if (screen_changed) {
        draw_lab_value_shell();
        draw_lab_value_dynamic();
        g_value_cache.valid = true;
        g_value_cache.temp_valid = temp_valid;
        g_value_cache.temp10 = temp_key;
        g_value_cache.fahrenheit = g_is_fahrenheit;
        return;
    }
    if (sensor_data_changed && dynamic_dirty) {
        draw_lab_value_dynamic();
        g_value_cache.valid = true;
        g_value_cache.temp_valid = temp_valid;
        g_value_cache.temp10 = temp_key;
        g_value_cache.fahrenheit = g_is_fahrenheit;
    }
}

void draw_lab_widget_mix_screen(bool screen_changed, bool sensor_data_changed) {
    const bool hum_valid = !isnan(g_ui_readings_snapshot.humidity);
    const int hum_key = hum_valid ? (int)lroundf(g_ui_readings_snapshot.humidity) : INT_MIN;
    const bool light_valid = !isnan(g_ui_readings_snapshot.ldr);
    const int light_key = light_valid ? (int)lroundf(g_ui_readings_snapshot.ldr) : INT_MIN;
    const bool sound_valid = !isnan(g_ui_readings_snapshot.mic);
    const int sound_key = sound_valid ? (int)lroundf(g_ui_readings_snapshot.mic) : INT_MIN;
    const bool hum_dirty = !g_mix_cache.valid || (g_mix_cache.hum_valid != hum_valid) || (g_mix_cache.hum != hum_key);
    const bool light_dirty = !g_mix_cache.valid || (g_mix_cache.light_valid != light_valid) || (g_mix_cache.light != light_key);
    const bool sound_dirty = !g_mix_cache.valid || (g_mix_cache.sound_valid != sound_valid) || (g_mix_cache.sound != sound_key);

    if (screen_changed) {
        draw_mix_shell();
        draw_mix_hum_dynamic();
        draw_mix_light_dynamic();
        draw_mix_sound_dynamic();
        g_mix_cache.valid = true;
        g_mix_cache.hum_valid = hum_valid;
        g_mix_cache.hum = hum_key;
        g_mix_cache.light_valid = light_valid;
        g_mix_cache.light = light_key;
        g_mix_cache.sound_valid = sound_valid;
        g_mix_cache.sound = sound_key;
        return;
    }
    if (sensor_data_changed) {
        if (hum_dirty) draw_mix_hum_dynamic();
        if (light_dirty) draw_mix_light_dynamic();
        if (sound_dirty) draw_mix_sound_dynamic();
        if (hum_dirty || light_dirty || sound_dirty) {
            g_mix_cache.valid = true;
            g_mix_cache.hum_valid = hum_valid;
            g_mix_cache.hum = hum_key;
            g_mix_cache.light_valid = light_valid;
            g_mix_cache.light = light_key;
            g_mix_cache.sound_valid = sound_valid;
            g_mix_cache.sound = sound_key;
        }
    }
}
