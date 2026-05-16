#include "ui_lab_widget_showcase.h"

#include "fonts.h"
#include "graph_buffer.h"
#include "io.h"
#include "languages.h"
#include "layout.h"
#include "runtime_events.h"
#include "tft_display.h"
#include "ui_icons.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <climits>
#include <math.h>
#include <stdio.h>
#include <string.h>

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

static uint16_t mix565(uint16_t a, uint16_t b, uint8_t amount_b) {
    const uint16_t ar = (a >> 11) & 0x1F;
    const uint16_t ag = (a >> 5) & 0x3F;
    const uint16_t ab = a & 0x1F;
    const uint16_t br = (b >> 11) & 0x1F;
    const uint16_t bg = (b >> 5) & 0x3F;
    const uint16_t bb = b & 0x1F;
    const uint16_t amount_a = 255 - amount_b;
    const uint16_t r = (ar * amount_a + br * amount_b) / 255;
    const uint16_t g = (ag * amount_a + bg * amount_b) / 255;
    const uint16_t bl = (ab * amount_a + bb * amount_b) / 255;
    return (uint16_t)((r << 11) | (g << 5) | bl);
}

enum ValueLabSensor : uint8_t {
    VALUE_SENSOR_TEMP = 0,
    VALUE_SENSOR_HUM,
    VALUE_SENSOR_LIGHT,
    VALUE_SENSOR_SOUND,
    VALUE_SENSOR_SOIL,
    VALUE_SENSOR_DS18,
    VALUE_SENSOR_COUNT
};

struct ValueRenderCache {
    bool valid = false;
    ValueLabSensor sensor = VALUE_SENSOR_TEMP;
    bool sensor_valid = false;
    int value_key = INT_MIN;
    bool fahrenheit = false;
};

enum GaugeLabSensor : uint8_t {
    GAUGE_SENSOR_TEMP = 0,
    GAUGE_SENSOR_HUM,
    GAUGE_SENSOR_LIGHT,
    GAUGE_SENSOR_SOUND,
    GAUGE_SENSOR_SOIL,
    GAUGE_SENSOR_DS18,
    GAUGE_SENSOR_COUNT
};

struct GaugeRenderCache {
    bool valid = false;
    GaugeLabSensor sensor = GAUGE_SENSOR_TEMP;
    bool sensor_valid = false;
    int value_key = INT_MIN;
    bool fahrenheit = false;
};

struct TempLabCache {
    bool valid = false;
    bool ambient_valid = false;
    int ambient10 = INT_MIN;
    bool probe_valid = false;
    int probe10 = INT_MIN;
    int delta10 = INT_MIN;
    bool fahrenheit = false;
};

static GaugeLabSensor g_gauge_sensor = GAUGE_SENSOR_TEMP;
static GaugeRenderCache g_gauge_cache;
static ValueLabSensor g_value_sensor = VALUE_SENSOR_TEMP;
static ValueRenderCache g_value_cache;
static TempLabCache g_mix_cache;

struct GaugeSpec {
    GaugeLabSensor sensor;
    LangKey label_key;
    uint16_t primary;
    uint16_t secondary;
    float min_c;
    float max_c;
    float min_value;
    float max_value;
    bool uses_temperature_unit;
    bool decimal;
    void (*draw_icon)(int cx, int cy, uint16_t color);
};

static float display_temp(float temp_c) {
    return g_is_fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static int display_temp_key(bool* out_valid = nullptr) {
    const bool valid = !isnan(g_ui_readings_snapshot.temperature);
    if (out_valid) *out_valid = valid;
    if (!valid) return INT_MIN;
    return (int)lroundf(display_temp(g_ui_readings_snapshot.temperature) * 10.0f);
}

static int display_probe_key(bool* out_valid = nullptr) {
    const bool valid = g_ui_readings_snapshot.temp_ds18b20 >= -100.0f;
    if (out_valid) *out_valid = valid;
    if (!valid) return INT_MIN;
    return (int)lroundf(display_temp(g_ui_readings_snapshot.temp_ds18b20) * 10.0f);
}

static const char* temp_unit() {
    return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
}

static void draw_solid_temp_large(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 7, cy - 18, 14, 24, 6, color);
    tft.fillCircle(cx, cy + 8, 10, color);
    tft.fillRoundRect(cx - 2, cy - 13, 4, 16, 2, TFT_BLACK);
}

static void draw_solid_temp_xl(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 8, cy - 22, 16, 30, 7, color);
    tft.fillCircle(cx, cy + 10, 12, color);
    tft.fillRoundRect(cx - 2, cy - 16, 4, 20, 2, TFT_BLACK);
}

static void draw_solid_temp_xxl(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 10, cy - 30, 20, 42, 8, color);
    tft.fillCircle(cx, cy + 17, 16, color);
    tft.fillRoundRect(cx - 3, cy - 23, 6, 30, 3, TFT_BLACK);
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

static void draw_solid_plant_large(int cx, int cy, uint16_t color) {
    tft.fillRect(cx - 2, cy - 8, 4, 23, color);
    tft.fillTriangle(cx, cy - 4, cx - 19, cy - 1, cx - 8, cy - 21, color);
    tft.fillTriangle(cx, cy - 5, cx + 19, cy - 2, cx + 8, cy - 22, color);
    tft.fillRoundRect(cx - 21, cy + 15, 42, 7, 3, color);
}

static void draw_solid_probe_large(int cx, int cy, uint16_t color) {
    tft.fillRoundRect(cx - 18, cy - 7, 31, 13, 5, color);
    tft.fillRoundRect(cx + 12, cy - 5, 9, 9, 3, color);
    tft.drawLine(cx + 21, cy - 1, cx + 32, cy - 6, color);
    tft.drawLine(cx + 21, cy, cx + 32, cy - 5, color);
    tft.drawLine(cx - 18, cy, cx - 28, cy + 8, color);
    tft.drawLine(cx - 28, cy + 8, cx - 33, cy + 17, color);
}

static void draw_section_label(const char* text, int x, int y, uint16_t color) {
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, kBg);
    tft.drawString(text, x, y);
    tft.setTextFont(0);
}

static void draw_compact_footer() {
    tft.setTextFont(0);
}

static void draw_gauge_ring(int cx, int cy, int radius, int thickness, float ratio, uint16_t primary, uint16_t secondary) {
    const int segs = 54;
    const float start_deg = 135.0f;
    const float sweep_deg = 270.0f;
    const int active_until = (int)roundf(ratio * (float)segs);

    for (int i = 0; i < segs; ++i) {
        const float a0 = (start_deg + ((float)i * sweep_deg / (float)segs)) * DEG_TO_RAD;
        const float a1 = (start_deg + ((float)(i + 1) * sweep_deg / (float)segs)) * DEG_TO_RAD;
        const uint8_t blend = (uint8_t)roundf((float)i * 255.0f / (float)max(1, segs - 1));
        const uint16_t color = (i < active_until)
            ? mix565(primary, secondary, blend)
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

static void draw_sparkline(int x, int y, int w, int h, const GraphBuffer& buf, uint16_t line_color, uint16_t grid_color, uint16_t bg_color = kBg) {
    float data[GRAPH_BUFFER_SIZE];
    size_t count = 0;
    portENTER_CRITICAL(&g_graph_mux);
    count = graph_buffer_get(buf, data, GRAPH_BUFFER_SIZE);
    portEXIT_CRITICAL(&g_graph_mux);

    tft.fillRoundRect(x, y, w, h, 3, bg_color);
    tft.drawRoundRect(x, y, w, h, 3, grid_color);
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

struct GaugeSensorSpec {
    LangKey label_key;
    uint16_t primary;
    uint16_t secondary;
    bool decimal;
};

static const GaugeSensorSpec& gauge_spec(GaugeLabSensor sensor) {
    static const GaugeSensorSpec specs[GAUGE_SENSOR_COUNT] = {
        { LAB_TEMP_SHORT,  kWarmOrange, kHotPink,     true  },
        { LAB_HUM_SHORT,   kCoolBlue,   kElectricBlue, false },
        { LAB_LIGHT_SHORT, kNeonYellow, kCoolBlue,    false },
        { LAB_SOUND_SHORT, kHotPink,    kNeonGreen,   false },
        { LAB_SOIL_SHORT,  kNeonGreen,  kCoolBlue,    false },
        { LAB_PROBE_SHORT, kElectricBlue, kHotPink,   true  },
    };
    if ((uint8_t)sensor >= (uint8_t)GAUGE_SENSOR_COUNT) {
        return specs[0];
    }
    return specs[(uint8_t)sensor];
}

static bool gauge_sensor_valid(GaugeLabSensor sensor) {
    const Reading& r = g_ui_readings_snapshot;
    switch (sensor) {
        case GAUGE_SENSOR_TEMP:  return !isnan(r.temperature);
        case GAUGE_SENSOR_HUM:   return !isnan(r.humidity);
        case GAUGE_SENSOR_LIGHT: return !isnan(r.ldr);
        case GAUGE_SENSOR_SOUND: return !isnan(r.mic);
        case GAUGE_SENSOR_SOIL:  return !isnan(r.soil_humidity);
        case GAUGE_SENSOR_DS18:  return r.temp_ds18b20 >= -100.0f;
        default:                 return false;
    }
}

static float gauge_sensor_value(GaugeLabSensor sensor) {
    const Reading& r = g_ui_readings_snapshot;
    switch (sensor) {
        case GAUGE_SENSOR_TEMP:  return display_temp(r.temperature);
        case GAUGE_SENSOR_HUM:   return r.humidity;
        case GAUGE_SENSOR_LIGHT: return r.ldr;
        case GAUGE_SENSOR_SOUND: return r.mic;
        case GAUGE_SENSOR_SOIL:  return r.soil_humidity;
        case GAUGE_SENSOR_DS18:  return display_temp(r.temp_ds18b20);
        default:                 return 0.0f;
    }
}

static void gauge_sensor_range(GaugeLabSensor sensor, float& min_value, float& max_value) {
    switch (sensor) {
        case GAUGE_SENSOR_TEMP:
        case GAUGE_SENSOR_DS18:
            min_value = g_is_fahrenheit ? 32.0f : 0.0f;
            max_value = g_is_fahrenheit ? 122.0f : 50.0f;
            break;
        case GAUGE_SENSOR_HUM:
        case GAUGE_SENSOR_SOUND:
        case GAUGE_SENSOR_SOIL:
            min_value = 0.0f;
            max_value = 100.0f;
            break;
        case GAUGE_SENSOR_LIGHT:
            min_value = 0.0f;
            max_value = 1023.0f;
            break;
        default:
            min_value = 0.0f;
            max_value = 1.0f;
            break;
    }
}

static const char* gauge_sensor_unit(GaugeLabSensor sensor) {
    switch (sensor) {
        case GAUGE_SENSOR_TEMP:
        case GAUGE_SENSOR_DS18:
            return temp_unit();
        case GAUGE_SENSOR_LIGHT:
            return L(ST_LUX_UNIT);
        case GAUGE_SENSOR_HUM:
        case GAUGE_SENSOR_SOUND:
        case GAUGE_SENSOR_SOIL:
            return "%";
        default:
            return "";
    }
}

static int gauge_sensor_key(GaugeLabSensor sensor, bool* out_valid = nullptr) {
    const bool valid = gauge_sensor_valid(sensor);
    if (out_valid) *out_valid = valid;
    if (!valid) return INT_MIN;
    const GaugeSensorSpec& spec = gauge_spec(sensor);
    const float value = gauge_sensor_value(sensor);
    return spec.decimal ? (int)lroundf(value * 10.0f) : (int)lroundf(value);
}

static void format_gauge_value(char* out, size_t out_len, GaugeLabSensor sensor, bool valid) {
    if (!valid) {
        snprintf(out, out_len, "--");
        return;
    }
    const GaugeSensorSpec& spec = gauge_spec(sensor);
    const float value = gauge_sensor_value(sensor);
    if (spec.decimal) {
        snprintf(out, out_len, "%.1f", value);
    } else {
        snprintf(out, out_len, "%.0f", value);
    }
}

static void format_gauge_limit(char* out, size_t out_len, float value, const char* unit) {
    snprintf(out, out_len, "%.0f%s", value, unit ? unit : "");
}

static void draw_gauge_icon(GaugeLabSensor sensor, int cx, int cy, uint16_t color) {
    switch (sensor) {
        case GAUGE_SENSOR_TEMP:
            draw_solid_temp_xxl(cx, cy, color);
            break;
        case GAUGE_SENSOR_HUM:
            draw_solid_drop_large(cx, cy, color);
            break;
        case GAUGE_SENSOR_LIGHT:
            draw_solid_light_large(cx, cy, color);
            break;
        case GAUGE_SENSOR_SOUND:
            draw_solid_mic_large(cx, cy, color);
            break;
        case GAUGE_SENSOR_SOIL:
            draw_solid_plant_large(cx, cy, color);
            break;
        case GAUGE_SENSOR_DS18:
            draw_solid_probe_large(cx, cy, color);
            break;
        default:
            tft.drawRoundRect(cx - 10, cy - 10, 20, 20, 3, color);
            break;
    }
}

static void draw_lab_gauge_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_GAUGE));
    draw_compact_footer();
}

static void draw_lab_gauge_dynamic() {
    const GaugeSensorSpec& spec = gauge_spec(g_gauge_sensor);
    const bool valid = gauge_sensor_valid(g_gauge_sensor);
    const float shown_value = valid ? gauge_sensor_value(g_gauge_sensor) : 0.0f;
    float min_value = 0.0f;
    float max_value = 1.0f;
    gauge_sensor_range(g_gauge_sensor, min_value, max_value);
    const float ratio = valid ? constrain((shown_value - min_value) / (max_value - min_value), 0.0f, 1.0f) : 0.0f;
    const uint16_t primary = valid ? spec.primary : TFT_DARKGREY;
    const uint16_t secondary = valid ? spec.secondary : TFT_DARKGREY;

    constexpr int kGaugeCx = 102;
    constexpr int kGaugeCy = 76;
    constexpr int kGaugeR = 31;
    constexpr int kIconCx = 34;
    constexpr int kIconCy = 83;

    tft.fillRect(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, LC_SCREEN_BOTTOM - LC_CARD_TOP + 1, kBg);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(primary, kBg);
    tft.drawString(L(spec.label_key), kIconCx, 31);
    tft.setTextFont(0);
    draw_gauge_icon(g_gauge_sensor, kIconCx, kIconCy, primary);
    draw_gauge_ring(kGaugeCx, kGaugeCy, kGaugeR, 6, ratio, primary, secondary);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    tft.setFreeFont(FONT_MENU);
    char value_buf[16];
    format_gauge_value(value_buf, sizeof(value_buf), g_gauge_sensor, valid);
    if (tft.textWidth(value_buf) > 56) {
        tft.setFreeFont(FONT_BODY);
    }
    tft.drawString(value_buf, kGaugeCx, kGaugeCy + 1);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(valid ? kWarmOrange : TFT_DARKGREY, kBg);
    tft.setTextDatum(TR_DATUM);
    tft.drawString(gauge_sensor_unit(g_gauge_sensor), 148, 34);
    tft.setTextFont(0);

    char min_buf[16];
    char max_buf[16];
    format_gauge_limit(min_buf, sizeof(min_buf), min_value, gauge_sensor_unit(g_gauge_sensor));
    format_gauge_limit(max_buf, sizeof(max_buf), max_value, gauge_sensor_unit(g_gauge_sensor));
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(primary, kBg);
    tft.drawString(min_buf, kGaugeCx - kGaugeR, 108);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(secondary, kBg);
    tft.drawString(max_buf, kGaugeCx + kGaugeR, 108);
    tft.setTextFont(0);
}

static const GraphBuffer& value_sensor_graph(ValueLabSensor s) {
    switch (s) {
        case VALUE_SENSOR_HUM:   return g_graph_humidity;
        case VALUE_SENSOR_LIGHT: return g_graph_light;
        case VALUE_SENSOR_SOUND: return g_graph_sound;
        case VALUE_SENSOR_SOIL:  return g_graph_soil;
        case VALUE_SENSOR_DS18:  return g_graph_ds18;
        default:                 return g_graph_temp;
    }
}

static void draw_lab_value_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_VALUE));
    draw_compact_footer();

    // Colors from gauge_spec: primary for icon/bar, secondary for border/device label
    const GaugeSensorSpec& vspec = gauge_spec((GaugeLabSensor)g_value_sensor);

    void (*icon_fn)(int, int, uint16_t);
    LangKey label_key;
    const char* device_str;
    uint16_t icon_badge_bg;

    switch (g_value_sensor) {
        case VALUE_SENSOR_HUM:
            icon_fn = pbit_draw_humidity_icon; label_key = LAB_HUM_SHORT;
            device_str = "DHT11";    icon_badge_bg = 0x0863;
            break;
        case VALUE_SENSOR_LIGHT:
            icon_fn = pbit_draw_light_icon; label_key = LAB_LIGHT_SHORT;
            device_str = "LDR";      icon_badge_bg = 0x10A0;
            break;
        case VALUE_SENSOR_SOUND:
            icon_fn = pbit_draw_sound_icon; label_key = LAB_SOUND_SHORT;
            device_str = "MIC";      icon_badge_bg = 0x1822;
            break;
        case VALUE_SENSOR_SOIL:
            icon_fn = pbit_draw_plant_icon; label_key = LAB_SOIL_SHORT;
            device_str = "SOIL";     icon_badge_bg = 0x00A0;
            break;
        case VALUE_SENSOR_DS18:
            icon_fn = pbit_draw_probe_icon; label_key = LAB_PROBE_SHORT;
            device_str = "DS18B20";  icon_badge_bg = 0x0063;
            break;
        default:
            icon_fn = pbit_draw_temp_icon;  label_key = LAB_TEMP_SHORT;
            device_str = "DHT11";    icon_badge_bg = 0x1863;
            break;
    }

    // secondary → card border + device label color; primary → icon color
    drawCard(LC_SCREEN_X, LC_CARD_TOP, LC_SCREEN_W, LC_SCREEN_BOTTOM - LC_CARD_TOP + 1, vspec.secondary);
    tft.fillRoundRect(12, 36, 64, 18, 4, icon_badge_bg);
    icon_fn(22, 45, vspec.primary);
    draw_section_label(L(label_key), 34, 38, TFT_WHITE);
    tft.fillRoundRect(94, 36, 54, 18, 4, 0x0883);
    draw_section_label(device_str, 104, 37, vspec.secondary);
}

static void draw_lab_value_dynamic() {
    const GaugeLabSensor gs = (GaugeLabSensor)g_value_sensor;
    const bool valid = gauge_sensor_valid(gs);
    const float shown = valid ? gauge_sensor_value(gs) : 0.0f;
    float min_v = 0.0f, max_v = 1.0f;
    gauge_sensor_range(gs, min_v, max_v);
    const GaugeSensorSpec& spec = gauge_spec(gs);

    tft.fillRect(12, 56, 136, 61, kBg);
    tft.fillRect(18, 98, 124, 16, kBg);

    char val_buf[16];
    format_gauge_value(val_buf, sizeof(val_buf), gs, valid);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_MENU);
    tft.setTextColor(valid ? TFT_WHITE : TFT_DARKGREY, kBg);
    tft.drawString(val_buf, 18, 63);
    const int value_w = tft.textWidth(val_buf);

    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(valid ? spec.primary : TFT_DARKGREY, kBg);
    tft.drawString(gauge_sensor_unit(gs), 18 + value_w + 4, 67);
    tft.setTextFont(0);

    draw_segment_bar(18, 99, 124, 14, 9,
                     valid ? constrain((shown - min_v) / (max_v - min_v), 0.0f, 1.0f) : 0.0f,
                     valid ? spec.primary : TFT_DARKGREY,
                     tft.color565(30, 24, 32));

    draw_sparkline(88, 58, 56, 31, value_sensor_graph(g_value_sensor),
                   spec.secondary, tft.color565(32, 44, 64), tft.color565(8, 16, 28));
}

constexpr uint16_t kCardBg = 0x0841;
constexpr uint16_t kAmbientCardBg = 0x1882;
constexpr uint16_t kProbeCardBg = 0x10A4;
constexpr uint16_t kDeltaCardBg = 0x1063;
constexpr uint16_t kBadgeBg = 0x10C3;
constexpr uint16_t kProbeAccent = 0xC71F;
constexpr int kTopCardY = LC_CARD_TOP;
constexpr int kTopCardW = 76;
constexpr int kTopCardH = 50;
constexpr int kAmbientCardX = LC_MASTER_CARD_X0;
constexpr int kProbeCardX = LC_MASTER_CARD_X1;
constexpr int kDeltaCardX = LC_SCREEN_X;
constexpr int kDeltaCardY = kTopCardY + kTopCardH + LC_GAP;
constexpr int kDeltaCardW = LC_SCREEN_W;
constexpr int kDeltaCardH = LC_SCREEN_BOTTOM - kDeltaCardY + 1;

static uint16_t ambient_accent(bool valid, float temp_c) {
    return valid ? getTempColor(temp_c) : TFT_DARKGREY;
}

static uint16_t probe_accent(bool valid, float temp_c) {
    if (!valid) return TFT_DARKGREY;
    if (temp_c < 0.0f) return TFT_CYAN;
    return kProbeAccent;
}

static uint16_t delta_accent(bool valid, float delta_value) {
    if (!valid) return TFT_DARKGREY;
    if (delta_value > 0.25f) return kWarmOrange;
    if (delta_value < -0.25f) return kCoolBlue;
    return kNeonGreen;
}

static void fill_lab_card(int x, int y, int w, int h, uint16_t accent, uint16_t bg = kCardBg) {
    tft.fillRoundRect(x, y, w, h, LC_CARD_RADIUS, bg);
    tft.drawRoundRect(x, y, w, h, LC_CARD_RADIUS, accent);
}

static void draw_badge(int x, int y, int w, const char* label, uint16_t accent) {
    tft.fillRoundRect(x, y, w, 12, 3, kBadgeBg);
    tft.drawRoundRect(x, y, w, 12, 3, accent);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kBadgeBg);
    tft.drawString(label, x + (w / 2), y + 7);
    tft.setTextFont(0);
}

static void draw_card_value(int cx, int y, bool valid, float shown_value, const char* footer_text, uint16_t footer_color, uint16_t bg, uint16_t value_color) {
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_MENU);
    tft.setTextColor(valid ? value_color : TFT_DARKGREY, bg);
    if (valid) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f", shown_value);
        tft.drawString(buf, cx, y);
    } else {
        tft.drawString("--", cx, y);
    }

    if (!valid && footer_text && footer_text[0]) {
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(footer_color, bg);
        tft.drawString(footer_text, cx, y + 17);
    }
    tft.setTextFont(0);
}

static void draw_temp_lab_card(int x,
                               int y,
                               int w,
                               bool valid,
                               float raw_temp_c,
                               float shown_temp,
                               const char* title,
                               const char* badge,
                               const char* invalid_text,
                               SensorIconDrawFn icon_fn,
                               uint16_t accent) {
    (void)icon_fn;
    (void)raw_temp_c;
    const bool is_probe = badge && (strncmp(badge, "DS", 2) == 0);
    const uint16_t card_bg = is_probe ? kProbeCardBg : kAmbientCardBg;
    const uint16_t name_color = is_probe ? kCoolBlue : kWarmOrange;
    const uint16_t unit_color = is_probe ? kHotPink : kElectricBlue;
    const uint16_t value_color = is_probe ? kElectricBlue : TFT_WHITE;
    fill_lab_card(x, y, w, kTopCardH, accent, card_bg);

    const char* card_title = (badge && badge[0]) ? badge : title;

    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(name_color, card_bg);
    const int title_y = y + 4;
    tft.drawString(card_title, x + 7, title_y);
    if (valid) {
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(unit_color, card_bg);
        tft.drawString(temp_unit(), x + w - 7, title_y);
    }
    tft.setTextFont(0);

    const char* footer_text = valid ? "" : invalid_text;
    const uint16_t footer_color = valid ? accent : TFT_DARKGREY;
    draw_card_value(x + (w / 2), y + 19, valid, shown_temp, footer_text, footer_color, card_bg, value_color);
}

static uint16_t delta_bar_color(float ratio, bool dht_hotter) {
    ratio = constrain(ratio, 0.0f, 1.0f);
    if (dht_hotter) {
        const uint8_t mix = (uint8_t)roundf((ratio < 0.55f ? ratio / 0.55f : (ratio - 0.55f) / 0.45f) * 255.0f);
        return (ratio < 0.55f)
            ? mix565(kHotPink, kWarmOrange, mix)
            : mix565(kWarmOrange, kNeonYellow, mix);
    }
    const uint8_t mix = (uint8_t)roundf((ratio < 0.55f ? ratio / 0.55f : (ratio - 0.55f) / 0.45f) * 255.0f);
    return (ratio < 0.55f)
        ? mix565(kHotPink, kDeepPurple, mix)
        : mix565(kDeepPurple, kCoolBlue, mix);
}

static void draw_delta_bar(int x, int y, int w, int h, bool valid, float delta_value) {
    const int center_x = x + (w / 2);
    const uint16_t track_col = tft.color565(18, 18, 28);
    const uint16_t tick_col = tft.color565(54, 50, 76);

    tft.fillRoundRect(x, y, w, h, 3, track_col);
    tft.drawRoundRect(x, y, w, h, 3, tft.color565(80, 72, 104));
    tft.drawFastVLine(center_x, y - 2, h + 4, TFT_WHITE);

    for (int tx = 12; tx < w / 2; tx += 18) {
        tft.drawFastVLine(center_x - tx, y + 2, h - 4, tick_col);
        tft.drawFastVLine(center_x + tx, y + 2, h - 4, tick_col);
    }

    if (!valid) return;

    const float base_scale = g_is_fahrenheit ? 18.0f : 10.0f;
    const float full_scale = max(base_scale, fabsf(delta_value));
    const float ratio = constrain(fabsf(delta_value) / full_scale, 0.0f, 1.0f);
    const int half_w = (w / 2) - 3;
    const int fill_w = max(1, (int)roundf((float)half_w * ratio));
    const bool dht_hotter = delta_value >= 0.0f;
    const int step = 3;

    for (int i = 0; i < fill_w; i += step) {
        const int seg_w = min(step, fill_w - i);
        const float seg_ratio = (float)(i + seg_w) / (float)max(1, fill_w);
        const uint16_t color = delta_bar_color(seg_ratio, dht_hotter);
        if (dht_hotter) {
            tft.fillRect(center_x - i - seg_w, y + 2, seg_w, h - 4, color);
        } else {
            tft.fillRect(center_x + 1 + i, y + 2, seg_w, h - 4, color);
        }
    }

    const int end_x = dht_hotter ? center_x - fill_w : center_x + fill_w;
    tft.fillCircle(end_x, y + (h / 2), 2, TFT_WHITE);
}

static void draw_temp_lab_shell() {
    tft.fillScreen(kBg);
    drawHeader(L(TIT_LAB_WIDGETS));
}

static void draw_temp_lab_dynamic() {
    const bool ambient_valid = !isnan(g_ui_readings_snapshot.temperature);
    const bool probe_valid = g_ui_readings_snapshot.temp_ds18b20 >= -100.0f;
    const float ambient_c = ambient_valid ? g_ui_readings_snapshot.temperature : 0.0f;
    const float probe_c = probe_valid ? g_ui_readings_snapshot.temp_ds18b20 : 0.0f;
    const float ambient_display = ambient_valid ? display_temp(ambient_c) : 0.0f;
    const float probe_display = probe_valid ? display_temp(probe_c) : 0.0f;
    const bool delta_valid = ambient_valid && probe_valid;
    const float delta_value = delta_valid ? (ambient_display - probe_display) : 0.0f;

    const uint16_t ambient_color = ambient_accent(ambient_valid, ambient_c);
    const uint16_t probe_color = probe_accent(probe_valid, probe_c);
    const uint16_t delta_color = delta_accent(delta_valid, delta_value);

    tft.fillRect(0, L_CONTENT_TOP, tft.width(), tft.height() - L_CONTENT_TOP, kBg);

    draw_temp_lab_card(kAmbientCardX,
                       kTopCardY,
                       kTopCardW,
                       ambient_valid,
                       ambient_c,
                       ambient_display,
                       L(LAB_TEMP_SHORT),
                       "DHT11",
                       L(ST_NO_SENSOR),
                       pbit_draw_temp_icon,
                       ambient_color);

    draw_temp_lab_card(kProbeCardX,
                       kTopCardY,
                       kTopCardW,
                       probe_valid,
                       probe_c,
                       probe_display,
                       L(LAB_PROBE_SHORT),
                       "DS18B20",
                       L(ST_CHECK_DS18),
                       pbit_draw_probe_icon,
                       probe_color);

    fill_lab_card(kDeltaCardX, kDeltaCardY, kDeltaCardW, kDeltaCardH, delta_color, kDeltaCardBg);

    const int bar_x = kDeltaCardX + 8;
    const int bar_y = kDeltaCardY + 16;
    const int bar_w = kDeltaCardW - 16;
    draw_delta_bar(bar_x, bar_y, bar_w, 14, delta_valid, delta_value);

    tft.setTextFont(1);
    const char* scale_label = g_is_fahrenheit ? "+18F" : "+10C";
    tft.setTextColor(kWarmOrange, kDeltaCardBg);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(scale_label, bar_x, bar_y + 16);
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(TFT_WHITE, kDeltaCardBg);
    tft.drawString("0", bar_x + (bar_w / 2), bar_y - 1);
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(kCoolBlue, kDeltaCardBg);
    tft.drawString(scale_label, bar_x + bar_w, bar_y + 16);
    tft.setTextFont(0);

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(delta_valid ? delta_color : TFT_DARKGREY, kDeltaCardBg);
    if (delta_valid) {
        char delta_buf[16];
        snprintf(delta_buf, sizeof(delta_buf), "%+.1f%s", delta_value, temp_unit());
        tft.drawString(delta_buf, kDeltaCardX + (kDeltaCardW / 2), kDeltaCardY + kDeltaCardH - 10);
    } else {
        tft.drawString("--", kDeltaCardX + (kDeltaCardW / 2), kDeltaCardY + kDeltaCardH - 10);
    }
    tft.setTextFont(0);
}

} // namespace

void lab_gauge_cycle_sensor() {
    g_gauge_sensor = (GaugeLabSensor)(((uint8_t)g_gauge_sensor + 1) % (uint8_t)GAUGE_SENSOR_COUNT);
    g_gauge_cache.valid = false;
    runtime_request_ui_full_redraw();
}

void draw_lab_gauge_temp_screen(bool screen_changed, bool sensor_data_changed) {
    bool gauge_valid = false;
    const int gauge_key = gauge_sensor_key(g_gauge_sensor, &gauge_valid);
    const bool dynamic_dirty = !g_gauge_cache.valid
        || (g_gauge_cache.sensor != g_gauge_sensor)
        || (g_gauge_cache.sensor_valid != gauge_valid)
        || (g_gauge_cache.value_key != gauge_key)
        || (g_gauge_cache.fahrenheit != g_is_fahrenheit);

    if (screen_changed) {
        draw_lab_gauge_shell();
        draw_lab_gauge_dynamic();
        g_gauge_cache.valid = true;
        g_gauge_cache.sensor = g_gauge_sensor;
        g_gauge_cache.sensor_valid = gauge_valid;
        g_gauge_cache.value_key = gauge_key;
        g_gauge_cache.fahrenheit = g_is_fahrenheit;
        return;
    }
    if (sensor_data_changed && dynamic_dirty) {
        draw_lab_gauge_dynamic();
        g_gauge_cache.valid = true;
        g_gauge_cache.sensor = g_gauge_sensor;
        g_gauge_cache.sensor_valid = gauge_valid;
        g_gauge_cache.value_key = gauge_key;
        g_gauge_cache.fahrenheit = g_is_fahrenheit;
    }
}

void draw_lab_value_modern_screen(bool screen_changed, bool sensor_data_changed) {
    const GaugeLabSensor gs = (GaugeLabSensor)g_value_sensor;
    bool sv = false;
    const int vk = gauge_sensor_key(gs, &sv);
    const bool dynamic_dirty = !g_value_cache.valid
        || (g_value_cache.sensor != g_value_sensor)
        || (g_value_cache.sensor_valid != sv)
        || (g_value_cache.value_key != vk)
        || (g_value_cache.fahrenheit != g_is_fahrenheit);

    if (screen_changed) {
        draw_lab_value_shell();
        draw_lab_value_dynamic();
        g_value_cache.valid = true;
        g_value_cache.sensor = g_value_sensor;
        g_value_cache.sensor_valid = sv;
        g_value_cache.value_key = vk;
        g_value_cache.fahrenheit = g_is_fahrenheit;
        return;
    }
    if (sensor_data_changed && dynamic_dirty) {
        draw_lab_value_dynamic();
        g_value_cache.valid = true;
        g_value_cache.sensor = g_value_sensor;
        g_value_cache.sensor_valid = sv;
        g_value_cache.value_key = vk;
        g_value_cache.fahrenheit = g_is_fahrenheit;
    }
}

void lab_value_cycle_sensor() {
    g_value_sensor = (ValueLabSensor)(((uint8_t)g_value_sensor + 1) % (uint8_t)VALUE_SENSOR_COUNT);
    g_value_cache.valid = false;
    runtime_request_ui_full_redraw();
}

void draw_lab_widget_mix_screen(bool screen_changed, bool sensor_data_changed) {
    bool ambient_valid = false;
    bool probe_valid = false;
    const int ambient_key = display_temp_key(&ambient_valid);
    const int probe_key = display_probe_key(&probe_valid);
    const int delta_key = (ambient_valid && probe_valid) ? (ambient_key - probe_key) : INT_MIN;
    const bool dynamic_dirty = !g_mix_cache.valid
        || (g_mix_cache.ambient_valid != ambient_valid)
        || (g_mix_cache.ambient10 != ambient_key)
        || (g_mix_cache.probe_valid != probe_valid)
        || (g_mix_cache.probe10 != probe_key)
        || (g_mix_cache.delta10 != delta_key)
        || (g_mix_cache.fahrenheit != g_is_fahrenheit);

    if (screen_changed) {
        draw_temp_lab_shell();
        draw_temp_lab_dynamic();
        g_mix_cache.valid = true;
        g_mix_cache.ambient_valid = ambient_valid;
        g_mix_cache.ambient10 = ambient_key;
        g_mix_cache.probe_valid = probe_valid;
        g_mix_cache.probe10 = probe_key;
        g_mix_cache.delta10 = delta_key;
        g_mix_cache.fahrenheit = g_is_fahrenheit;
        return;
    }
    if (sensor_data_changed && dynamic_dirty) {
        draw_temp_lab_dynamic();
        g_mix_cache.valid = true;
        g_mix_cache.ambient_valid = ambient_valid;
        g_mix_cache.ambient10 = ambient_key;
        g_mix_cache.probe_valid = probe_valid;
        g_mix_cache.probe10 = probe_key;
        g_mix_cache.delta10 = delta_key;
        g_mix_cache.fahrenheit = g_is_fahrenheit;
    }
}
