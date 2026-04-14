// ui_graph.cpp
// Graph screen: shows the recent history of a selected sensor as a line chart.
// Short press cycles between Temperature and Humidity.

#include "ui_graph.h"
#include "graph_buffer.h"
#include "tft_display.h"
#include "ui_widgets.h"
#include "layout.h"
#include "languages.h"
#include "fonts.h"
#include "runtime_events.h"
#include "io.h"
#include <TFT_eSPI.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

extern TFT_eSPI tft;
extern bool g_is_fahrenheit;

// ── Sensor selection ──────────────────────────────────────────────────────────

enum GraphSensor : uint8_t { GRAPH_TEMP = 0, GRAPH_HUM = 1, GRAPH_COUNT = 2 };
static GraphSensor g_graph_sensor = GRAPH_TEMP;

void graph_cycle_sensor() {
    g_graph_sensor = (GraphSensor)(((uint8_t)g_graph_sensor + 1) % (uint8_t)GRAPH_COUNT);
    runtime_request_ui_full_redraw();
}

// ── Sprite ────────────────────────────────────────────────────────────────────

static TFT_eSprite g_sprite(&tft);
static bool        g_sprite_ready = false;

static void ensure_sprite() {
    if (g_sprite_ready) return;
    g_sprite.setColorDepth(16);
    g_sprite.createSprite(LG_GRAPH_W, LG_GRAPH_H);
    g_sprite_ready = true;
}

static const char* graph_sensor_label() {
    return (g_graph_sensor == GRAPH_TEMP) ? L(LAB_TEMP_SHORT) : L(LAB_HUM_SHORT);
}

static uint16_t graph_bg_color() {
    return tft.color565(4, 8, 18);
}

static uint16_t graph_grid_h_color() {
    return tft.color565(22, 26, 34);
}

static uint16_t graph_grid_v_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(110, 52, 148); // darker purple, less competitive
        case GRAPH_HUM:  return tft.color565(92, 82, 156); // softer violet so humidity grid competes less
        default:         return tft.color565(96, 54, 140);
    }
}

static uint16_t graph_grid_h_color(GraphSensor sensor, int row) {
    if (sensor == GRAPH_TEMP) {
        static const uint16_t temp_rows[] = {
            tft.color565(120, 28, 34),   // hot red (top), darker
            tft.color565(136, 62, 18),   // orange
            tft.color565(132, 102, 26),  // warm yellow
            tft.color565(20, 86, 110),   // cyan-blue
            tft.color565(18, 44, 98),    // cold blue (bottom)
        };
        return temp_rows[(row < 5) ? row : 4];
    }

    static const uint16_t hum_rows[] = {
        tft.color565(14, 38, 92),    // very humid / top
        tft.color565(18, 62, 118),
        tft.color565(28, 88, 150),
        tft.color565(108, 176, 220),
        TFT_WHITE,                   // drier / bottom
    };
    return hum_rows[(row < 5) ? row : 4];
}

static uint16_t graph_shadow_color() {
    return tft.color565(10, 12, 18);
}

static uint16_t graph_line_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(88, 255, 96);
        case GRAPH_HUM:  return tft.color565(80, 255, 255);
        default:         return TFT_WHITE;
    }
}

static uint16_t graph_band_label_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(90, 240, 255);
        case GRAPH_HUM:  return tft.color565(255, 110, 230);
        default:         return TFT_CYAN;
    }
}

static uint16_t graph_border_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(132, 74, 0);
        case GRAPH_HUM:  return tft.color565(0, 84, 130);
        default:         return TFT_DARKGREY;
    }
}

static uint16_t graph_max_label_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(255, 214, 56);
        case GRAPH_HUM:  return TFT_WHITE;
        default:         return TFT_WHITE;
    }
}

static uint16_t graph_min_label_color(GraphSensor sensor) {
    switch (sensor) {
        case GRAPH_TEMP: return tft.color565(54, 202, 255);
        case GRAPH_HUM:  return TFT_WHITE;
        default:         return TFT_LIGHTGREY;
    }
}

// ── Graph rendering ───────────────────────────────────────────────────────────

static void render_graph(const float* data, size_t n, uint16_t line_color) {
    ensure_sprite();
    g_sprite.fillSprite(graph_bg_color());

    // Balanced grid: verticals lead, horizontals stay subtler so the curve stays dominant.
    const int grid_step = 12;
    int row = 0;
    for (int gy = 0; gy < LG_GRAPH_H; gy += grid_step, ++row) {
        g_sprite.drawFastHLine(0, gy, LG_GRAPH_W, graph_grid_h_color(g_graph_sensor, row));
    }
    for (int gx = 0; gx < LG_GRAPH_W; gx += grid_step) {
        g_sprite.drawFastVLine(gx, 0, LG_GRAPH_H, graph_grid_v_color(g_graph_sensor));
    }

    // Determine the visible window (rightmost LG_GRAPH_W samples)
    const size_t start_i = (n > (size_t)LG_GRAPH_W) ? n - (size_t)LG_GRAPH_W : 0;
    const size_t visible  = n - start_i;

    // Auto-scale: find min/max over the visible window
    float vmin = data[start_i], vmax = data[start_i];
    for (size_t i = start_i + 1; i < n; ++i) {
        if (data[i] < vmin) vmin = data[i];
        if (data[i] > vmax) vmax = data[i];
    }

    // Enforce a minimum visible span so tiny fluctuations don't look gigantic
    const float min_span = (g_graph_sensor == GRAPH_TEMP) ? 4.0f : 8.0f;
    float span = vmax - vmin;
    if (span < min_span) {
        float center = (vmin + vmax) * 0.5f;
        vmin = center - min_span * 0.5f;
        vmax = center + min_span * 0.5f;
    }
    // 5% padding above and below so the line doesn't hug the border
    const float pad = (vmax - vmin) * 0.05f;
    vmin -= pad;
    vmax += pad;

    // Value → sprite Y pixel (0 = top, LG_GRAPH_H-1 = bottom)
    auto val_to_py = [&](float v) -> int {
        float ratio = (v - vmin) / (vmax - vmin);
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        return (int)((1.0f - ratio) * (float)(LG_GRAPH_H - 1) + 0.5f);
    };

    // Samples are right-aligned: newest sample always at the rightmost column
    const int x_off = (visible < (size_t)LG_GRAPH_W) ? (int)(LG_GRAPH_W - visible) : 0;

    const uint16_t shadow_col = graph_shadow_color();
    for (size_t i = start_i + 1; i < n; ++i) {
        const int xi = x_off + (int)(i - 1 - start_i);
        const int xj = x_off + (int)(i     - start_i);
        const int yi = val_to_py(data[i - 1]);
        const int yj = val_to_py(data[i]);
        g_sprite.drawLine(xi, yi + 1, xj, yj + 1, shadow_col);
        g_sprite.drawLine(xi, yi, xj, yj, line_color);
        // Extra pixel for crisp look on small screen
        g_sprite.drawPixel(xj, yj, line_color);
    }

    // Min / max labels in the corners, color-coded to improve readability.
    char buf[10];
    g_sprite.setTextFont(1);

    if (g_graph_sensor == GRAPH_TEMP) {
        const char* unit = g_is_fahrenheit ? "F" : "C";
        const float shown_max = g_is_fahrenheit ? (vmax * 1.8f + 32.0f) : vmax;
        const float shown_min = g_is_fahrenheit ? (vmin * 1.8f + 32.0f) : vmin;
        snprintf(buf, sizeof(buf), "%.0f%s", shown_max, unit);
        g_sprite.setTextDatum(TL_DATUM);
        g_sprite.setTextColor(graph_max_label_color(g_graph_sensor), TFT_BLACK);
        g_sprite.drawString(buf, 2, 2);
        snprintf(buf, sizeof(buf), "%.0f%s", shown_min, unit);
        g_sprite.setTextDatum(BL_DATUM);
        g_sprite.setTextColor(graph_min_label_color(g_graph_sensor), TFT_BLACK);
        g_sprite.drawString(buf, 2, LG_GRAPH_H - 1);
    } else {
        snprintf(buf, sizeof(buf), "%.0f%%", vmax);
        g_sprite.setTextDatum(TL_DATUM);
        g_sprite.setTextColor(graph_max_label_color(g_graph_sensor), TFT_BLACK);
        g_sprite.drawString(buf, 2, 2);
        snprintf(buf, sizeof(buf), "%.0f%%", vmin);
        g_sprite.setTextDatum(BL_DATUM);
        g_sprite.setTextColor(graph_min_label_color(g_graph_sensor), TFT_BLACK);
        g_sprite.drawString(buf, 2, LG_GRAPH_H - 1);
    }

    g_sprite.setTextFont(0);
    g_sprite.pushSprite(LG_GRAPH_X + 1, LG_GRAPH_Y + 1);
}

static void draw_graph_band(bool valid,
                            const char* label,
                            const char* value,
                            uint16_t label_color,
                            uint16_t value_color) {
    const int band_y = 30;
    const int band_h = 19;
    tft.fillRect(0, band_y, tft.width(), band_h, TFT_BLACK);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(valid ? label_color : TFT_DARKGREY, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(label, LG_GRAPH_X + 4, LG_SENSOR_Y);
    if (valid && value && value[0]) {
        tft.setTextDatum(TR_DATUM);
        tft.setTextColor(value_color, TFT_BLACK);
        tft.drawString(value, LG_GRAPH_X + LG_GRAPH_W - 4, LG_SENSOR_Y);
    }
    tft.setTextFont(0);
}

// ── Public draw function ──────────────────────────────────────────────────────

void draw_graph_screen(bool screen_changed, bool sensor_data_changed) {
    static GraphSensor last_sensor = (GraphSensor)0xFF;
    static float       data_buf[GRAPH_BUFFER_SIZE];
    static char        last_band_value[24] = "";
    static bool        last_band_valid = false;
    static GraphSensor last_band_sensor = (GraphSensor)0xFF;

    const bool sensor_switched = (last_sensor != g_graph_sensor);
    const bool need_full       = screen_changed || sensor_switched;

    // ── Header (title + divider) ──────────────────────────────────────────────
    if (need_full) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_GRAPH), TFT_WHITE);
    }

    // ── Sensor label + current value band ────────────────────────────────────
    if (need_full) {
        tft.fillRect(0, L_CONTENT_TOP, tft.width(), LG_GRAPH_Y - L_CONTENT_TOP - 1, TFT_BLACK);

        // Graph border (color matches the sensor's line color)
        const uint16_t border_col = graph_border_color(g_graph_sensor);
        tft.drawRect(LG_GRAPH_X, LG_GRAPH_Y, LG_GRAPH_W + 2, LG_GRAPH_H + 2, border_col);

        last_sensor = g_graph_sensor;
    }

    // ── Fetch data and render graph ───────────────────────────────────────────
    if (need_full || sensor_data_changed) {
        size_t n;
        portENTER_CRITICAL(&g_graph_mux);
        if (g_graph_sensor == GRAPH_TEMP) {
            n = graph_buffer_get(g_graph_temp, data_buf, GRAPH_BUFFER_SIZE);
        } else {
            n = graph_buffer_get(g_graph_humidity, data_buf, GRAPH_BUFFER_SIZE);
        }
        portEXIT_CRITICAL(&g_graph_mux);

        if (n == 0) {
            // Clear graph area and show waiting message
            tft.fillRect(LG_GRAPH_X + 1, LG_GRAPH_Y + 1, LG_GRAPH_W, LG_GRAPH_H, TFT_BLACK);
            tft.setFreeFont(FONT_BODY);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString(L(ST_WAITING), LG_GRAPH_X + 1 + LG_GRAPH_W / 2,
                           LG_GRAPH_Y + 1 + LG_GRAPH_H / 2);
            tft.setTextFont(0);
        } else {
            const uint16_t line_col = graph_line_color(g_graph_sensor);
            render_graph(data_buf, n, line_col);
        }

        if (n > 0) {
            char value_buf[24];
            if (g_graph_sensor == GRAPH_TEMP) {
                snprintf(value_buf, sizeof(value_buf), "%.1f%s",
                         g_is_fahrenheit ? data_buf[n - 1] * 1.8f + 32.0f : data_buf[n - 1],
                         g_is_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT));
            } else {
                snprintf(value_buf, sizeof(value_buf), "%.0f%%", data_buf[n - 1]);
            }
            const bool band_changed = need_full
                || last_band_sensor != g_graph_sensor
                || last_band_valid != true
                || strcmp(last_band_value, value_buf) != 0;
            if (band_changed) {
                draw_graph_band(true,
                                graph_sensor_label(),
                                value_buf,
                                graph_band_label_color(g_graph_sensor),
                                graph_line_color(g_graph_sensor));
                strncpy(last_band_value, value_buf, sizeof(last_band_value) - 1);
                last_band_value[sizeof(last_band_value) - 1] = '\0';
                last_band_valid = true;
                last_band_sensor = g_graph_sensor;
            }
        } else {
            const bool band_changed = need_full || last_band_sensor != g_graph_sensor || last_band_valid;
            if (band_changed) {
                draw_graph_band(false,
                                graph_sensor_label(),
                                "",
                                graph_band_label_color(g_graph_sensor),
                                graph_line_color(g_graph_sensor));
                last_band_value[0] = '\0';
                last_band_valid = false;
                last_band_sensor = g_graph_sensor;
            }
        }
    }

    // ── Footer hint ───────────────────────────────────────────────────────────
    if (need_full) {
        tft.fillRect(0, 112, tft.width(), 16, TFT_BLACK);
        drawFooterHint(L(GRAPH_PUSH_SENSOR), 80, LG_HINT_Y, TFT_DARKGREY);
    }
}
