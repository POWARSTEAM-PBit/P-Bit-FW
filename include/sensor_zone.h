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

// Visualization modes. The first five are common to every sensor and keep
// their persisted numeric values; sound-only modes are append-only.
enum SzVizMode : uint8_t {
    SZ_VIZ_FOCUS = 0,   // Principal (ui_lab_focus)
    SZ_VIZ_VALOR,       // Dato      (ui_lab_widget_showcase)
    SZ_VIZ_GRAPH,       // Curva     (ui_graph)
    SZ_VIZ_GAUGE,       // Rango     (ui_lab_widget_showcase)
    SZ_VIZ_CARD,        // Ficha     (ui_lab_sensor_cards)
    SZ_VIZ_SOUND_VU_STACK, // Sonido VU   (solo SZ_SOUND)
    SZ_VIZ_SOUND_VU_WAVE,  // Sonido Onda (solo SZ_SOUND)
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

// Runtime-only jumps used by transient flows such as Demo Mode.
// These do not persist to NVS.
void sz_set_sensor_runtime(uint8_t sensor_id);
void sz_set_viz_runtime(uint8_t sensor_id, uint8_t viz_mode);

// Snapshot/restore de estado runtime de SENSOR_ZONE.
// Util para Demo Mode: el carrusel modifica sensor activo y modos de varios
// sensores, pero al salir debe devolver exactamente la seleccion del usuario.
// No toca NVS; solo estado en memoria.
struct SzRuntimeSnapshot {
    SzSensorId sensor;
    SzVizMode  viz[SZ_SENSOR_COUNT];
    uint8_t graph_sensor;
    uint8_t focus_sensor;
    uint8_t gauge_sensor;
    uint8_t value_sensor;
    uint8_t card_sensor;
};
void sz_snapshot_runtime(SzRuntimeSnapshot& out);
void sz_restore_runtime(const SzRuntimeSnapshot& in);

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

// Full visible header name (e.g. "TEMP. RANGO", "Humedad", "Luz Curva").
// Uses current sensor + current viz mode. Call from tft_display.cpp when drawing the header.
const char* sz_header_name();

// RGB color for a sensor (for LED feedback).
void sz_sensor_rgb(SzSensorId id, uint8_t& r, uint8_t& g, uint8_t& b);

// Sub-renderer active flag.
// Set true before calling any sub-renderer from SENSOR_ZONE_SCREEN so the
// sub-renderer knows to skip its own drawHeader call (sensor_zone draws it instead).
void sz_set_active(bool active);
bool sz_is_active();
