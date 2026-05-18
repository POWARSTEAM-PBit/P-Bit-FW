// ui_graph.cpp
// Graph screen: shows the recent history of a selected sensor as a line chart.
// Short press cycles through all six available sensors.

#include "ui_graph.h"
#include "sensor_zone.h"
#include "palette.h"

#include "fonts.h"
#include "graph_buffer.h"
#include "io.h"
#include "languages.h"
#include "layout.h"
#include "runtime_events.h"
#include "tft_display.h"
#include "ui_widgets.h"

#include <TFT_eSPI.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern TFT_eSPI tft;
extern bool g_is_fahrenheit;

namespace {

enum GraphSensor : uint8_t {
    GRAPH_TEMP = 0,
    GRAPH_HUM,
    GRAPH_LIGHT,
    GRAPH_SOUND,
    GRAPH_SOIL,
    GRAPH_DS18,
    GRAPH_COUNT
};

static GraphSensor g_graph_sensor = GRAPH_TEMP;
static TFT_eSprite g_sprite(&tft);
static bool g_sprite_ready = false;

static void ensure_sprite() {
    if (g_sprite_ready) return;
    g_sprite.setColorDepth(16);
    g_sprite.createSprite(LG_GRAPH_W, LG_GRAPH_H);
    g_sprite_ready = true;
}

static uint16_t graph_bg_color() {
    return tft.color565(4, 8, 18);
}

static uint16_t graph_shadow_color() {
    return tft.color565(10, 12, 18);
}

static bool graph_uses_decimal(GraphSensor sensor) {
    return sensor == GRAPH_TEMP || sensor == GRAPH_DS18;
}

static float graph_display_value(GraphSensor sensor, float raw_value) {
    if (sensor == GRAPH_TEMP || sensor == GRAPH_DS18) {
        return g_is_fahrenheit ? (raw_value * 1.8f + 32.0f) : raw_value;
    }
    return raw_value;
}

static const char* graph_sensor_label(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP:  return L(GRAPH_LABEL_TEMP_AIR);
        case GRAPH_HUM:   return L(GRAPH_LABEL_HUM_AIR);
        case GRAPH_LIGHT: return L(GRAPH_LABEL_LIGHT);
        case GRAPH_SOUND: return L(GRAPH_LABEL_SOUND);
        case GRAPH_SOIL:  return L(GRAPH_LABEL_SOIL_HUM);
        case GRAPH_DS18:  return L(GRAPH_LABEL_DS18);
        default:          return L(TIT_GRAPH);
    }
}

static const char* graph_sensor_unit(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP:
        case GRAPH_DS18:
            return g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT);
        case GRAPH_HUM:
        case GRAPH_SOUND:
        case GRAPH_SOIL:
            return "%";
        case GRAPH_LIGHT:
            return L(ST_LUX_UNIT);
        default:
            return "";
    }
}

static GraphBuffer* graph_sensor_buffer(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP:  return &g_graph_temp;
        case GRAPH_HUM:   return &g_graph_humidity;
        case GRAPH_LIGHT: return &g_graph_light;
        case GRAPH_SOUND: return &g_graph_sound;
        case GRAPH_SOIL:  return &g_graph_soil;
        case GRAPH_DS18:  return &g_graph_ds18;
        default:          return nullptr;
    }
}

static float graph_min_span(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP:
        case GRAPH_DS18:
            return 4.0f;
        case GRAPH_HUM:
        case GRAPH_SOUND:
        case GRAPH_SOIL:
            return 8.0f;
        case GRAPH_LIGHT:
            return 150.0f;
        default:
            return 4.0f;
    }
}

static uint16_t graph_grid_v_color(GraphSensor sensor) {
    // ~35% of P2 — vivid retro grid; visible sensor tint without competing with the line
    const uint16_t p2 = pb_secondary((uint8_t)sensor);
    const uint8_t r = (uint8_t)(((p2 >> 11) & 0x1F) * 9 / 25);
    const uint8_t g = (uint8_t)(((p2 >> 5)  & 0x3F) * 9 / 25);
    const uint8_t b = (uint8_t)((p2         & 0x1F) * 9 / 25);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

static uint16_t graph_grid_h_color(GraphSensor sensor, int row) {
    // Rows 0=bottom .. 4=top. Each sensor gets 5 dark shades that evoke its palette.
    // Ranges: dim (bottom) → slightly brighter (top) to add vertical depth.
    switch (sensor) {
        case GRAPH_TEMP: {
            // Orange-red tones (fire, heat) — dark at bottom, hotter toward top
            static const uint16_t rows[] = {
                tft.color565(60,  14,  0),
                tft.color565(80,  22,  0),
                tft.color565(100, 30,  4),
                tft.color565(80,  10, 30),
                tft.color565(60,   6, 48),
            };
            return rows[(row < 5) ? row : 4];
        }
        case GRAPH_HUM: {
            // Cyan-blue tones (water, rain) — deeper at bottom, electric at top
            static const uint16_t rows[] = {
                tft.color565(0,  18,  60),
                tft.color565(0,  28,  80),
                tft.color565(0,  44, 100),
                tft.color565(4,  60, 120),
                tft.color565(8,  80, 140),
            };
            return rows[(row < 5) ? row : 4];
        }
        case GRAPH_LIGHT: {
            // Yellow-amber tones (sun, energy) — dark gold at bottom, warmer top
            static const uint16_t rows[] = {
                tft.color565(50,  40,  0),
                tft.color565(70,  54,  0),
                tft.color565(90,  70,  0),
                tft.color565(110, 86,  4),
                tft.color565(130, 100, 8),
            };
            return rows[(row < 5) ? row : 4];
        }
        case GRAPH_SOUND: {
            // Magenta-purple tones (VU, waves) — dark purple at bottom, hot pink top
            static const uint16_t rows[] = {
                tft.color565(30,  0,  50),
                tft.color565(50,  0,  70),
                tft.color565(70,  0,  90),
                tft.color565(90,  0,  70),
                tft.color565(110, 0,  50),
            };
            return rows[(row < 5) ? row : 4];
        }
        case GRAPH_SOIL: {
            // Green tones (plant, growth) — dark earth at bottom, lime at top
            static const uint16_t rows[] = {
                tft.color565(8,  40,  8),
                tft.color565(12, 56, 12),
                tft.color565(18, 72, 18),
                tft.color565(24, 90, 24),
                tft.color565(30, 110, 30),
            };
            return rows[(row < 5) ? row : 4];
        }
        case GRAPH_DS18:
        default: {
            // Violet-blue tones (crystal, precision) — dark indigo at bottom, amethyst top
            static const uint16_t rows[] = {
                tft.color565(40,  0, 70),
                tft.color565(54,  0, 90),
                tft.color565(40, 20, 110),
                tft.color565(30, 40, 120),
                tft.color565(20, 60, 130),
            };
            return rows[(row < 5) ? row : 4];
        }
    }
}

static uint16_t graph_line_color(GraphSensor sensor) {
    // GraphSensor order matches SzSensorId — cast directly.
    return pb_primary((uint8_t)sensor);
}

static uint16_t graph_band_label_color(GraphSensor sensor) {
    // Secondary color provides contrast against the primary graph line.
    return pb_secondary((uint8_t)sensor);
}

static uint16_t graph_border_color(GraphSensor sensor) {
    // ~55% of P1 — vivid sensor border without competing with the data line
    const uint16_t p1 = pb_primary((uint8_t)sensor);
    const uint8_t r = (uint8_t)(((p1 >> 11) & 0x1F) * 14 / 25);
    const uint8_t g = (uint8_t)(((p1 >> 5)  & 0x3F) * 14 / 25);
    const uint8_t b = (uint8_t)((p1         & 0x1F) * 14 / 25);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

static uint16_t graph_max_label_color(GraphSensor sensor) {
    // P3: acento cálido — resalta el máximo/pico de la gráfica.
    return pb_accent_warm((uint8_t)sensor);
}

static uint16_t graph_min_label_color(GraphSensor sensor) {
    // P4: contraste frío — referencia del mínimo.
    return pb_contrast_cool((uint8_t)sensor);
}

static void format_graph_value(char* out, size_t out_size, GraphSensor sensor, float raw_value) {
    const float shown = graph_display_value(sensor, raw_value);
    const char* unit = graph_sensor_unit(sensor);

    if (sensor == GRAPH_LIGHT) {
        snprintf(out, out_size, "%.0f %s", shown, unit);
        return;
    }

    if (graph_uses_decimal(sensor)) {
        snprintf(out, out_size, "%.1f%s", shown, unit);
        return;
    }

    snprintf(out, out_size, "%.0f%s", shown, unit);
}

static void format_graph_corner_value(char* out, size_t out_size, GraphSensor sensor, float raw_value) {
    const float shown = graph_display_value(sensor, raw_value);
    const char* unit = graph_sensor_unit(sensor);
    snprintf(out, out_size, "%.0f%s", shown, unit);
}

static void render_graph(const float* data, size_t n, GraphSensor sensor) {
    ensure_sprite();
    g_sprite.fillSprite(graph_bg_color());

    const int grid_step = 12;
    int row = 0;
    for (int gy = 0; gy < LG_GRAPH_H; gy += grid_step, ++row) {
        g_sprite.drawFastHLine(0, gy, LG_GRAPH_W, graph_grid_h_color(sensor, row));
    }
    for (int gx = 0; gx < LG_GRAPH_W; gx += grid_step) {
        g_sprite.drawFastVLine(gx, 0, LG_GRAPH_H, graph_grid_v_color(sensor));
    }

    const size_t start_i = (n > (size_t)LG_GRAPH_W) ? (n - (size_t)LG_GRAPH_W) : 0;
    const size_t visible = n - start_i;

    float vmin = data[start_i];
    float vmax = data[start_i];
    for (size_t i = start_i + 1; i < n; ++i) {
        if (data[i] < vmin) vmin = data[i];
        if (data[i] > vmax) vmax = data[i];
    }

    const float min_span = graph_min_span(sensor);
    float span = vmax - vmin;
    if (span < min_span) {
        const float center = (vmin + vmax) * 0.5f;
        vmin = center - min_span * 0.5f;
        vmax = center + min_span * 0.5f;
    }

    const float pad = (vmax - vmin) * 0.05f;
    vmin -= pad;
    vmax += pad;

    auto val_to_py = [&](float value) -> int {
        float ratio = (value - vmin) / (vmax - vmin);
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        return (int)((1.0f - ratio) * (float)(LG_GRAPH_H - 1) + 0.5f);
    };

    const int x_off = (visible < (size_t)LG_GRAPH_W) ? (int)(LG_GRAPH_W - visible) : 0;
    const uint16_t shadow_col = graph_shadow_color();
    const uint16_t line_col = graph_line_color(sensor);
    for (size_t i = start_i + 1; i < n; ++i) {
        const int xi = x_off + (int)(i - 1 - start_i);
        const int xj = x_off + (int)(i - start_i);
        const int yi = val_to_py(data[i - 1]);
        const int yj = val_to_py(data[i]);
        g_sprite.drawLine(xi, yi + 1, xj, yj + 1, shadow_col);
        g_sprite.drawLine(xi, yi, xj, yj, line_col);
        g_sprite.drawPixel(xj, yj, line_col);
    }

    char buf[16];
    g_sprite.setTextFont(1);

    format_graph_corner_value(buf, sizeof(buf), sensor, vmax);
    g_sprite.setTextDatum(TL_DATUM);
    g_sprite.setTextColor(graph_max_label_color(sensor), TFT_BLACK);
    g_sprite.drawString(buf, 2, 2);

    format_graph_corner_value(buf, sizeof(buf), sensor, vmin);
    g_sprite.setTextDatum(BL_DATUM);
    g_sprite.setTextColor(graph_min_label_color(sensor), TFT_BLACK);
    g_sprite.drawString(buf, 2, LG_GRAPH_H - 1);

    g_sprite.setTextFont(0);
    g_sprite.pushSprite(LG_GRAPH_X + 1, LG_GRAPH_Y + 1);
}

static void draw_graph_band(bool valid,
                            GraphSensor sensor,
                            const char* value) {
    const int band_y = L_CONTENT_TOP;
    const int band_h = LG_GRAPH_Y - L_CONTENT_TOP - 2;
    tft.fillRect(0, band_y, tft.width(), band_h, TFT_BLACK);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(valid ? graph_band_label_color(sensor) : TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    const char* label = graph_sensor_label(sensor);
    if (tft.textWidth(label) > 104) {
        tft.setTextFont(1);
    }
    tft.drawString(label, LG_GRAPH_X + 4, LG_SENSOR_Y);
    tft.setFreeFont(FONT_SMALL);
    if (valid && value && value[0]) {
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(graph_line_color(sensor), TFT_BLACK);
        tft.drawString(value, LG_GRAPH_X + LG_GRAPH_W - 4, LG_SENSOR_Y);
    }
    tft.setTextFont(0);
}

} // namespace

void graph_cycle_sensor() {
    g_graph_sensor = (GraphSensor)(((uint8_t)g_graph_sensor + 1) % (uint8_t)GRAPH_COUNT);
    runtime_request_ui_full_redraw();
}

void graph_set_sensor(uint8_t sensor_id) {
    if (sensor_id >= (uint8_t)GRAPH_COUNT) return;
    g_graph_sensor = (GraphSensor)sensor_id;
}

void draw_graph_screen(bool screen_changed, bool sensor_data_changed) {
    static GraphSensor last_sensor = (GraphSensor)0xFF;
    static float data_buf[GRAPH_BUFFER_SIZE];
    static char last_band_value[24] = "";
    static bool last_band_valid = false;
    static GraphSensor last_band_sensor = (GraphSensor)0xFF;

    const bool sensor_switched = (last_sensor != g_graph_sensor);
    const bool need_full = screen_changed || sensor_switched;

    if (need_full) {
        tft.fillScreen(TFT_BLACK);
        if (!sz_is_active()) drawHeader(L(TIT_GRAPH));
    }

    if (need_full) {
        tft.fillRect(0, L_CONTENT_TOP, tft.width(), LG_GRAPH_Y - L_CONTENT_TOP - 1, TFT_BLACK);
        last_sensor = g_graph_sensor;
    }

    if (need_full || sensor_data_changed) {
        size_t n = 0;
        GraphBuffer* buffer = graph_sensor_buffer(g_graph_sensor);
        portENTER_CRITICAL(&g_graph_mux);
        if (buffer) {
            n = graph_buffer_get(*buffer, data_buf, GRAPH_BUFFER_SIZE);
        }
        portEXIT_CRITICAL(&g_graph_mux);

        if (n == 0) {
            tft.fillRect(LG_GRAPH_X + 1, LG_GRAPH_Y + 1, LG_GRAPH_W, LG_GRAPH_H, TFT_BLACK);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString(L(ST_WAITING), LG_GRAPH_X + 1 + LG_GRAPH_W / 2,
                           LG_GRAPH_Y + 1 + LG_GRAPH_H / 2);
            tft.setTextFont(0);
        } else {
            render_graph(data_buf, n, g_graph_sensor);
        }
        // Border drawn after sprite/content so rounded corners aren't overwritten.
        tft.drawRoundRect(LG_GRAPH_X,
                          LG_GRAPH_Y,
                          LG_GRAPH_W + 2,
                          LG_GRAPH_H + 2,
                          LC_CARD_RADIUS,
                          graph_border_color(g_graph_sensor));

        if (n > 0) {
            char value_buf[24];
            format_graph_value(value_buf, sizeof(value_buf), g_graph_sensor, data_buf[n - 1]);
            const bool band_changed = need_full
                || last_band_sensor != g_graph_sensor
                || !last_band_valid
                || strcmp(last_band_value, value_buf) != 0;
            if (band_changed) {
                draw_graph_band(true, g_graph_sensor, value_buf);
                strncpy(last_band_value, value_buf, sizeof(last_band_value) - 1);
                last_band_value[sizeof(last_band_value) - 1] = '\0';
                last_band_valid = true;
                last_band_sensor = g_graph_sensor;
            }
        } else {
            const bool band_changed = need_full || last_band_sensor != g_graph_sensor || last_band_valid;
            if (band_changed) {
                draw_graph_band(false, g_graph_sensor, "");
                last_band_value[0] = '\0';
                last_band_valid = false;
                last_band_sensor = g_graph_sensor;
            }
        }
    }

    if (need_full) {
        tft.fillRect(0, 112, tft.width(), 16, TFT_BLACK);
        drawFooterHint(L(GRAPH_PUSH_SENSOR), 80, LG_HINT_Y, TFT_DARKGREY);
    }
}
