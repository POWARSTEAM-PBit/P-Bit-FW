#pragma once
// sensor_zone.h
// Sensor zone navigation state — two-level model:
//   Level 1 (carousel): multi-sensor lab screens + sensor zone + timer + system
//   Level 2 (sensor zone): encoder cycles sensors, press cycles viz per sensor

#include <stdint.h>

// Canonical sensor IDs for the sensor zone (order matches GraphSensor, GaugeLabSensor,
// ValueLabSensor — cast directly).
enum SzSensorId : uint8_t {
    SZ_TEMP  = 0,
    SZ_HUM,
    SZ_LIGHT,
    SZ_SOUND,
    SZ_SOIL,
    SZ_DS18,
    SZ_SENSOR_COUNT
};

// Visualization modes available for every sensor, in cycle order.
enum SzVizMode : uint8_t {
    SZ_VIZ_FOCUS = 0,   // Sensor Lab  (ui_lab_focus)
    SZ_VIZ_VALOR,        // Valor Lab   (ui_lab_widget_showcase)
    SZ_VIZ_GRAPH,        // Gráfica     (ui_graph)
    SZ_VIZ_GAUGE,        // Gauge Lab   (ui_lab_widget_showcase)
    SZ_VIZ_CARD,         // Sensor Card (ui_lab_sensor_cards)
    SZ_VIZ_COUNT
};

// --- State access ---------------------------------------------------------

// Initialize from NVS (call once at startup, after settings_store is ready).
void sz_init();

// Current sensor (0 .. SZ_SENSOR_COUNT-1).
SzSensorId sz_get_sensor();

// Current visualization for the active sensor.
SzVizMode  sz_get_viz();

// Jump directly to a sensor by index. Persists and requests redraw.
void sz_set_sensor(uint8_t sensor_id);

// Advance to the next sensor (wraps around). Persists and requests redraw.
void sz_next_sensor();

// Retreat to the previous sensor (wraps around). Persists and requests redraw.
void sz_prev_sensor();

// Cycle to the next viz mode for the current sensor. Persists and requests redraw.
void sz_next_viz();

// Sync the active sub-renderer's internal sensor to match the current sensor zone state.
// Tracks its own last-synced state; only notifies the sub-renderer when something changed.
// Pass force=true when the screen just became active to guarantee a sync on the first frame.
void sz_sync_renderer(bool force = false);

// Display name for a sensor in FOCUS mode (short base name).
const char* sz_sensor_name(SzSensorId id);

// Full viz-aware header name (e.g. "Temp Dial", "Humedad", "Luz Graf").
// Uses current sensor + current viz mode. Call from tft_display.cpp when drawing the header.
const char* sz_header_name();

// RGB color for a sensor (for LED feedback).
void sz_sensor_rgb(SzSensorId id, uint8_t& r, uint8_t& g, uint8_t& b);

// Sub-renderer active flag.
// Set true before calling any sub-renderer from SENSOR_ZONE_SCREEN so the
// sub-renderer knows to skip its own drawHeader call (sensor_zone draws it instead).
void sz_set_active(bool active);
bool sz_is_active();
