// LAB_SENSOR_FOCUS_SCREEN
// Temporary lab screen for experimenting with more polished sensor views.
// This module is self-contained so it can be tested without wiring the
// carousel or shared graph buffers yet.

#include "ui_lab_focus.h"

#include "tft_display.h"
#include "ui_widgets.h"
#include "graph_buffer.h"
#include "layout.h"
#include "languages.h"
#include "fonts.h"
#include "ui_icons.h"
#include "io.h"
#include "runtime_events.h"

#include <TFT_eSPI.h>
#include <climits>
#include <math.h>
#include <stdio.h>

extern bool g_is_fahrenheit;

namespace {

constexpr int LF_SUMMARY_X = LC_SCREEN_X;
constexpr int LF_SUMMARY_Y = LC_CARD_TOP;
constexpr int LF_SUMMARY_W = LC_SCREEN_W;
constexpr int LF_SUMMARY_H = 41;

constexpr int LF_GRAPH_X = LC_SCREEN_X;
constexpr int LF_GRAPH_Y = LF_SUMMARY_Y + LF_SUMMARY_H + LC_GAP;
constexpr int LF_GRAPH_W = LC_SCREEN_W;
constexpr int LF_GRAPH_H = 41;
constexpr int LF_GRAPH_INSET = 2;
constexpr int LF_GRAPH_INNER_W = LF_GRAPH_W - (LF_GRAPH_INSET * 2);
constexpr int LF_GRAPH_INNER_H = LF_GRAPH_H - (LF_GRAPH_INSET * 2);

constexpr int LF_ICON_CX = LF_SUMMARY_X + 14;
constexpr int LF_ICON_CY = LF_SUMMARY_Y + 20;
constexpr int LF_TITLE_X = LF_SUMMARY_X + 27;
constexpr int LF_TITLE_Y = LF_SUMMARY_Y + 10;
constexpr int LF_HINT_Y = 120;

static LabFocusSensor g_sensor = LAB_FOCUS_HUMIDITY;
static bool g_force_full_redraw = true;
static LabFocusSensor g_last_summary_sensor = LAB_FOCUS_COUNT;
static bool g_last_summary_valid = false;
static int g_last_summary_key = INT_MIN;
static bool g_last_summary_unit_mode = false;

static TFT_eSprite g_graph_sprite(&tft);
static bool g_graph_sprite_ready = false;

static void ensure_graph_sprite() {
    if (g_graph_sprite_ready) return;
    g_graph_sprite.setColorDepth(16);
    g_graph_sprite.createSprite(LF_GRAPH_INNER_W, LF_GRAPH_INNER_H);
    g_graph_sprite_ready = true;
}

static uint16_t blend565(uint16_t a, uint16_t b) {
    const uint8_t ar = (uint8_t)((a >> 11) & 0x1F);
    const uint8_t ag = (uint8_t)((a >> 5) & 0x3F);
    const uint8_t ab = (uint8_t)(a & 0x1F);
    const uint8_t br = (uint8_t)((b >> 11) & 0x1F);
    const uint8_t bg = (uint8_t)((b >> 5) & 0x3F);
    const uint8_t bb = (uint8_t)(b & 0x1F);
    return (uint16_t)(((uint8_t)((ar + br) / 2) << 11) |
                      ((uint8_t)((ag + bg) / 2) << 5) |
                      (uint8_t)((ab + bb) / 2));
}

struct SensorPalette {
    uint16_t primary;
    uint16_t secondary;
};

static SensorPalette sensor_palette(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:     return { TFT_ORANGE, TFT_MAGENTA };
        case LAB_FOCUS_HUMIDITY: return { TFT_CYAN, tft.color565(168, 96, 255) };
        case LAB_FOCUS_DS18:     return { tft.color565(180, 100, 255), TFT_CYAN };
        case LAB_FOCUS_LIGHT:    return { TFT_YELLOW, TFT_CYAN };
        case LAB_FOCUS_SOUND:    return { TFT_MAGENTA, TFT_GREEN };
        case LAB_FOCUS_SOIL:     return { TFT_GREEN, TFT_CYAN };
        default:                 return { TFT_DARKGREY, TFT_DARKGREY };
    }
}

static uint16_t sensor_primary_color(LabFocusSensor sensor) {
    return sensor_palette(sensor).primary;
}

static uint16_t sensor_secondary_color(LabFocusSensor sensor) {
    return sensor_palette(sensor).secondary;
}

static int summary_value_y() {
    return 36;
}

static int summary_no_sensor_y(LabFocusSensor sensor) {
    return (sensor == LAB_FOCUS_SOIL || sensor == LAB_FOCUS_DS18) ? 48 : 52;
}

static int graph_no_sensor_y(LabFocusSensor sensor) {
    (void)sensor;
    return (LF_GRAPH_H / 2) + 1;
}

static uint16_t summary_bg_color() {
    return tft.color565(8, 12, 18);
}

static uint16_t graph_bg_color() {
    return tft.color565(4, 8, 20);
}

static uint16_t graph_border_color(LabFocusSensor sensor) {
    return blend565(sensor_primary_color(sensor), sensor_secondary_color(sensor));
}

static uint16_t graph_line_color(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:     return TFT_ORANGE;
        case LAB_FOCUS_HUMIDITY: return TFT_CYAN;
        case LAB_FOCUS_DS18:     return tft.color565(180, 100, 255);
        case LAB_FOCUS_LIGHT:    return TFT_YELLOW;
        case LAB_FOCUS_SOUND:    return TFT_MAGENTA;
        case LAB_FOCUS_SOIL:     return TFT_GREEN;
        default:                 return TFT_WHITE;
    }
}

static const char* sensor_title(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:     return L(LAB_TEMP_SHORT);
        case LAB_FOCUS_HUMIDITY: return L(LAB_HUM_SHORT);
        case LAB_FOCUS_DS18:     return L(LAB_PROBE_SHORT);
        case LAB_FOCUS_LIGHT:    return L(LAB_LIGHT_SHORT);
        case LAB_FOCUS_SOUND:    return L(LAB_SOUND_SHORT);
        case LAB_FOCUS_SOIL:     return L(LAB_SOIL_SHORT);
        default:                 return L(TIT_GRAPH);
    }
}

static const char* sensor_footer() {
    return L(GRAPH_PUSH_SENSOR);
}

static bool sensor_uses_decimal(LabFocusSensor sensor);

static bool sensor_has_value(LabFocusSensor sensor) {
    const Reading& r = g_ui_readings_snapshot;
    switch (sensor) {
        case LAB_FOCUS_TEMP:
            return !isnan(r.temperature);
        case LAB_FOCUS_HUMIDITY:
            return !isnan(r.humidity);
        case LAB_FOCUS_DS18:
            return r.temp_ds18b20 >= -100.0f;
        case LAB_FOCUS_LIGHT:
            return !isnan(r.ldr);
        case LAB_FOCUS_SOUND:
            return !isnan(r.mic);
        case LAB_FOCUS_SOIL:
            return !isnan(r.soil_humidity);
        default:
            return false;
    }
}

static float sensor_value(LabFocusSensor sensor) {
    const Reading& r = g_ui_readings_snapshot;
    switch (sensor) {
        case LAB_FOCUS_TEMP:
            return g_is_fahrenheit ? (r.temperature * 1.8f + 32.0f) : r.temperature;
        case LAB_FOCUS_HUMIDITY:
            return r.humidity;
        case LAB_FOCUS_DS18:
            return g_is_fahrenheit ? (r.temp_ds18b20 * 1.8f + 32.0f) : r.temp_ds18b20;
        case LAB_FOCUS_LIGHT:
            return r.ldr;
        case LAB_FOCUS_SOUND:
            return r.mic;
        case LAB_FOCUS_SOIL:
            return r.soil_humidity;
        default:
            return NAN;
    }
}

static int sensor_visible_key(LabFocusSensor sensor) {
    if (!sensor_has_value(sensor)) return INT_MIN;
    const float value = sensor_value(sensor);
    if (sensor_uses_decimal(sensor)) {
        return (int)lroundf(value * 10.0f);
    }
    return (int)lroundf(value);
}

static const char* sensor_unit(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:
        case LAB_FOCUS_DS18:
            return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
        case LAB_FOCUS_HUMIDITY:
        case LAB_FOCUS_SOIL:
        case LAB_FOCUS_SOUND:
            return "%";
        case LAB_FOCUS_LIGHT:
            return L(ST_LUX_UNIT);
        default:
            return "";
    }
}

static bool sensor_uses_decimal(LabFocusSensor sensor) {
    return sensor == LAB_FOCUS_TEMP || sensor == LAB_FOCUS_DS18;
}

static float sensor_min_span(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:
        case LAB_FOCUS_DS18:
            return 4.0f;
        case LAB_FOCUS_HUMIDITY:
        case LAB_FOCUS_SOIL:
            return 8.0f;
        case LAB_FOCUS_LIGHT:
            return 100.0f;
        case LAB_FOCUS_SOUND:
            return 10.0f;
        default:
            return 4.0f;
    }
}

static GraphBuffer* sensor_buffer(LabFocusSensor sensor) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:     return &g_graph_temp;
        case LAB_FOCUS_HUMIDITY: return &g_graph_humidity;
        case LAB_FOCUS_DS18:     return &g_graph_ds18;
        case LAB_FOCUS_LIGHT:    return &g_graph_light;
        case LAB_FOCUS_SOUND:    return &g_graph_sound;
        case LAB_FOCUS_SOIL:     return &g_graph_soil;
        default:                 return nullptr;
    }
}

static void draw_icon(LabFocusSensor sensor, int cx, int cy, uint16_t color) {
    switch (sensor) {
        case LAB_FOCUS_TEMP:
            pbit_draw_temp_icon(cx, cy, color);
            break;
        case LAB_FOCUS_DS18:
            pbit_draw_probe_icon(cx, cy, color);
            break;

        case LAB_FOCUS_HUMIDITY:
            pbit_draw_humidity_icon(cx, cy, color);
            break;

        case LAB_FOCUS_LIGHT:
            pbit_draw_light_icon(cx, cy, color);
            break;

        case LAB_FOCUS_SOUND:
            pbit_draw_sound_icon(cx, cy, color);
            break;

        case LAB_FOCUS_SOIL:
            pbit_draw_plant_icon(cx, cy, color);
            break;

        default:
            tft.drawRoundRect(cx - 6, cy - 6, 12, 12, 2, color);
            break;
    }
}

static void draw_right_aligned_pair(int right_x,
                                    int y,
                                    const char* main_text,
                                    const char* unit_text,
                                    const GFXfont* main_font,
                                    const GFXfont* unit_font,
                                    uint16_t main_color,
                                    uint16_t unit_color,
                                    uint16_t bg_color) {
    tft.setFreeFont(main_font);
    const int main_w = tft.textWidth(main_text);
    int rendered_unit_w = 0;
    if (unit_text && unit_text[0]) {
        tft.setFreeFont(unit_font);
        rendered_unit_w = tft.textWidth(unit_text);
    }
    const int total_w = main_w + ((rendered_unit_w > 0) ? 3 + rendered_unit_w : 0);
    const int start_x = right_x - total_w;

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(main_color, bg_color);
    tft.setFreeFont(main_font);
    tft.drawString(main_text, start_x, y);

    if (rendered_unit_w > 0) {
        tft.setFreeFont(unit_font);
        tft.setTextColor(unit_color, bg_color);
        tft.drawString(unit_text, start_x + main_w + 3, y);
    }
    tft.setTextFont(0);
}

static void clear_summary_content(LabFocusSensor sensor, bool valid) {
    const uint16_t bg = summary_bg_color();
    const int clear_x = LF_SUMMARY_X + 72;
    const int clear_y = LF_SUMMARY_Y + 6;
    const int clear_w = LF_SUMMARY_W - 76;
    const int clear_h = LF_SUMMARY_H - 12;
    tft.fillRect(clear_x, clear_y, clear_w, clear_h, bg);
    if (!valid) {
        tft.fillRect(clear_x, LF_SUMMARY_Y + 12, clear_w, 14, bg);
    }
}

static void draw_summary_shell(LabFocusSensor sensor, uint16_t primary, uint16_t secondary) {
    const uint16_t bg = summary_bg_color();
    tft.fillRoundRect(LF_SUMMARY_X, LF_SUMMARY_Y, LF_SUMMARY_W, LF_SUMMARY_H, 4, bg);
    tft.drawRoundRect(LF_SUMMARY_X, LF_SUMMARY_Y, LF_SUMMARY_W, LF_SUMMARY_H, 4, blend565(primary, secondary));
    draw_icon(sensor, LF_ICON_CX, LF_ICON_CY, primary);

    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(primary, bg);
    tft.drawString(sensor_title(sensor), LF_TITLE_X, LF_TITLE_Y);
    tft.setTextFont(0);
}

static void draw_summary_content(LabFocusSensor sensor, bool valid, uint16_t primary, uint16_t secondary) {
    const char* unit = sensor_unit(sensor);
    const uint16_t value_color = valid ? secondary : TFT_DARKGREY;
    const uint16_t unit_color = valid ? primary : TFT_DARKGREY;
    const uint16_t bg = summary_bg_color();

    clear_summary_content(sensor, valid);

    if (!valid) {
        tft.setTextDatum(TR_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, bg);
        tft.drawString(L(ST_NO_SENSOR),
                       LF_SUMMARY_X + LF_SUMMARY_W - 6,
                       summary_no_sensor_y(sensor));
        tft.setTextFont(0);
        return;
    }

    char value_buf[24];
    if (sensor_uses_decimal(sensor)) {
        snprintf(value_buf, sizeof(value_buf), "%.1f", sensor_value(sensor));
    } else {
        snprintf(value_buf, sizeof(value_buf), "%.0f", sensor_value(sensor));
    }

    draw_right_aligned_pair(LF_SUMMARY_X + LF_SUMMARY_W - 8,
                            summary_value_y(),
                            value_buf,
                            unit,
                            FONT_MENU,
                            FONT_SMALL,
                            value_color,
                             unit_color,
                             bg);
}

static void draw_summary_panel(LabFocusSensor sensor, bool valid, bool shell_redraw) {
    const uint16_t primary = sensor_primary_color(sensor);
    const uint16_t secondary = sensor_secondary_color(sensor);
    if (shell_redraw) {
        draw_summary_shell(sensor, primary, secondary);
    }
    draw_summary_content(sensor, valid, primary, secondary);
}

static void render_graph_sprite(LabFocusSensor sensor, const float* data, size_t n) {
    ensure_graph_sprite();
    g_graph_sprite.fillSprite(graph_bg_color());

    const uint16_t grid_h_col = tft.color565(12, 18, 34);
    const uint16_t grid_v_col = blend565(sensor_secondary_color(sensor), tft.color565(28, 44, 88));
    for (int gy = 0; gy < LF_GRAPH_INNER_H; gy += 10) {
        g_graph_sprite.drawFastHLine(0, gy, LF_GRAPH_INNER_W, grid_h_col);
    }
    for (int gx = 0; gx < LF_GRAPH_INNER_W; gx += 10) {
        g_graph_sprite.drawFastVLine(gx, 0, LF_GRAPH_INNER_H, grid_v_col);
    }

    if (n == 0) {
        g_graph_sprite.setFreeFont(FONT_SMALL);
        g_graph_sprite.setTextDatum(MC_DATUM);
        g_graph_sprite.setTextColor(TFT_DARKGREY, TFT_BLACK);
        g_graph_sprite.drawString(L(ST_WAITING), LF_GRAPH_INNER_W / 2, LF_GRAPH_INNER_H / 2);
        g_graph_sprite.setTextFont(0);
        g_graph_sprite.pushSprite(LF_GRAPH_X + LF_GRAPH_INSET, LF_GRAPH_Y + LF_GRAPH_INSET);
        return;
    }

    float vmin = data[0];
    float vmax = data[0];
    for (size_t i = 1; i < n; ++i) {
        if (data[i] < vmin) vmin = data[i];
        if (data[i] > vmax) vmax = data[i];
    }

    const float min_span = sensor_min_span(sensor);
    float span = vmax - vmin;
    if (span < min_span) {
        const float center = (vmin + vmax) * 0.5f;
        vmin = center - min_span * 0.5f;
        vmax = center + min_span * 0.5f;
    }

    const float pad = (vmax - vmin) * 0.05f;
    vmin -= pad;
    vmax += pad;

    auto value_to_y = [&](float v) -> int {
        const float ratio = (v - vmin) / (vmax - vmin);
        const float clamped = (ratio < 0.0f) ? 0.0f : (ratio > 1.0f ? 1.0f : ratio);
        return (int)((1.0f - clamped) * (float)(LF_GRAPH_INNER_H - 1) + 0.5f);
    };

    const size_t max_visible = (size_t)LF_GRAPH_INNER_W;
    const size_t start_i = (n > max_visible) ? (n - max_visible) : 0;
    const size_t visible = n - start_i;
    const int x_off = (visible < max_visible) ? (int)(max_visible - visible) : 0;
    const uint16_t line_col = graph_line_color(sensor);
    const uint16_t shadow_col = blend565(sensor_secondary_color(sensor), tft.color565(10, 12, 18));

    if (visible == 1) {
        const int y = value_to_y(data[n - 1]);
        g_graph_sprite.drawPixel(x_off + 1, y + 1, shadow_col);
        g_graph_sprite.drawPixel(x_off, y, line_col);
    } else {
        for (size_t i = start_i + 1; i < n; ++i) {
            const int xi = x_off + (int)(i - 1 - start_i);
            const int xj = x_off + (int)(i - start_i);
            const int yi = value_to_y(data[i - 1]);
            const int yj = value_to_y(data[i]);
            g_graph_sprite.drawLine(xi, yi + 1, xj, yj + 1, shadow_col);
            g_graph_sprite.drawLine(xi, yi, xj, yj, line_col);
            g_graph_sprite.drawPixel(xj, yj, line_col);
        }
    }

    g_graph_sprite.pushSprite(LF_GRAPH_X + LF_GRAPH_INSET, LF_GRAPH_Y + LF_GRAPH_INSET);
}

static void draw_graph_panel(LabFocusSensor sensor, bool valid, bool shell_redraw) {
    const uint16_t border = valid ? graph_border_color(sensor) : TFT_DARKGREY;
    float data[GRAPH_BUFFER_SIZE];
    size_t n = 0;

    if (shell_redraw) {
        tft.fillRoundRect(LF_GRAPH_X, LF_GRAPH_Y, LF_GRAPH_W, LF_GRAPH_H, 4, graph_bg_color());
    } else {
        tft.fillRect(LF_GRAPH_X + 1, LF_GRAPH_Y + 1, LF_GRAPH_W - 2, LF_GRAPH_H - 2, graph_bg_color());
    }
    tft.drawRoundRect(LF_GRAPH_X, LF_GRAPH_Y, LF_GRAPH_W, LF_GRAPH_H, 4, border);

    if (!valid) {
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(L(ST_NO_SENSOR), LF_GRAPH_X + (LF_GRAPH_W / 2), LF_GRAPH_Y + graph_no_sensor_y(sensor));
        tft.setTextFont(0);
        return;
    }

    GraphBuffer* buffer = sensor_buffer(sensor);
    if (buffer) {
        portENTER_CRITICAL(&g_graph_mux);
        n = graph_buffer_get(*buffer, data, GRAPH_BUFFER_SIZE);
        portEXIT_CRITICAL(&g_graph_mux);
    }

    if (n == 0) {
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(L(ST_WAITING), LF_GRAPH_X + (LF_GRAPH_W / 2), LF_GRAPH_Y + (LF_GRAPH_H / 2) + 1);
        tft.setTextFont(0);
        return;
    }

    render_graph_sprite(sensor, data, n);
}

} // namespace

LabFocusSensor lab_focus_get_sensor() {
    return g_sensor;
}

void lab_focus_set_sensor(LabFocusSensor sensor) {
    if (sensor >= LAB_FOCUS_COUNT) return;
    if (g_sensor == sensor) return;
    g_sensor = sensor;
    g_force_full_redraw = true;
    runtime_request_ui_full_redraw();
}

void lab_focus_cycle_sensor() {
    switch (g_sensor) {
        case LAB_FOCUS_HUMIDITY: g_sensor = LAB_FOCUS_TEMP; break;
        case LAB_FOCUS_TEMP:     g_sensor = LAB_FOCUS_LIGHT; break;
        case LAB_FOCUS_LIGHT:    g_sensor = LAB_FOCUS_SOUND; break;
        case LAB_FOCUS_SOUND:    g_sensor = LAB_FOCUS_SOIL; break;
        case LAB_FOCUS_SOIL:     g_sensor = LAB_FOCUS_DS18; break;
        case LAB_FOCUS_DS18:     g_sensor = LAB_FOCUS_HUMIDITY; break;
        default:                 g_sensor = LAB_FOCUS_HUMIDITY; break;
    }
    g_force_full_redraw = true;
    runtime_request_ui_full_redraw();
}

void draw_lab_focus_screen(bool screen_changed, bool sensor_data_changed) {
    const bool sensor_switched = g_force_full_redraw;
    const bool need_full = screen_changed || sensor_switched;
    const bool valid = sensor_has_value(g_sensor);
    const int summary_key = sensor_visible_key(g_sensor);
    const bool summary_dirty = need_full
        || (g_sensor != g_last_summary_sensor)
        || (valid != g_last_summary_valid)
        || (summary_key != g_last_summary_key)
        || (g_last_summary_unit_mode != g_is_fahrenheit
            && (g_sensor == LAB_FOCUS_TEMP || g_sensor == LAB_FOCUS_DS18));

    if (need_full) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_LAB_FOCUS));
        draw_summary_panel(g_sensor, valid, true);
        draw_graph_panel(g_sensor, valid, true);
        drawFooterHint(sensor_footer(), tft.width() / 2, LF_HINT_Y, TFT_DARKGREY);
    } else if (sensor_data_changed) {
        if (summary_dirty) {
            draw_summary_panel(g_sensor, valid, false);
        }
        draw_graph_panel(g_sensor, valid, false);
    }

    g_last_summary_sensor = g_sensor;
    g_last_summary_valid = valid;
    g_last_summary_key = summary_key;
    g_last_summary_unit_mode = g_is_fahrenheit;
    g_force_full_redraw = false;
}
