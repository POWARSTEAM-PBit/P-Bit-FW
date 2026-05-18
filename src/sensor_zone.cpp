// sensor_zone.cpp
// Two-level navigation state for the sensor zone:
//   encoder turns  → cycle sensors
//   press short    → cycle viz mode for current sensor (persisted per-sensor)
//   press long     → open config menu for current sensor

#include "sensor_zone.h"
#include "settings_store.h"
#include "runtime_events.h"

// Sub-renderer headers (for sz_sync_renderer).
#include "ui_lab_sensor_cards.h"
#include "ui_lab_widget_showcase.h"
#include "ui_lab_focus.h"
#include "ui_graph.h"

// ---------------------------------------------------------------------------
// Mapping tables — convert SzSensorId to each sub-renderer's enum.
// ---------------------------------------------------------------------------

// LabSensorCardId order: TEMP=0, DS18=1, HUM=2, LIGHT=3, SOUND=4, SOIL=5
static const uint8_t kSzToCardId[SZ_SENSOR_COUNT] = {
    0, // SZ_TEMP  → CARD_TEMP
    2, // SZ_HUM   → CARD_HUM
    3, // SZ_LIGHT → CARD_LIGHT
    4, // SZ_SOUND → CARD_SOUND
    5, // SZ_SOIL  → CARD_SOIL
    1, // SZ_DS18  → CARD_DS18
};

// LabFocusSensor order: HUMIDITY=0, TEMP=1, LIGHT=2, SOUND=3, SOIL=4, DS18=5
static const uint8_t kSzToFocusId[SZ_SENSOR_COUNT] = {
    1, // SZ_TEMP  → LAB_FOCUS_TEMP
    0, // SZ_HUM   → LAB_FOCUS_HUMIDITY
    2, // SZ_LIGHT → LAB_FOCUS_LIGHT
    3, // SZ_SOUND → LAB_FOCUS_SOUND
    4, // SZ_SOIL  → LAB_FOCUS_SOIL
    5, // SZ_DS18  → LAB_FOCUS_DS18
};

// GraphSensor, GaugeLabSensor, ValueLabSensor share the same order as SzSensorId
// (TEMP=0, HUM=1, LIGHT=2, SOUND=3, SOIL=4, DS18=5) — cast directly.

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static SzSensorId g_sensor = SZ_TEMP;
static SzVizMode  g_viz[SZ_SENSOR_COUNT] = {
    SZ_VIZ_FOCUS, SZ_VIZ_FOCUS, SZ_VIZ_FOCUS,
    SZ_VIZ_FOCUS, SZ_VIZ_FOCUS, SZ_VIZ_FOCUS
};
static bool g_sz_active = false;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void sz_init() {
    g_sensor = (SzSensorId)load_sz_sensor_store();
    if (g_sensor >= SZ_SENSOR_COUNT) g_sensor = SZ_TEMP;

    for (uint8_t i = 0; i < SZ_SENSOR_COUNT; i++) {
        uint8_t v = load_sz_viz_store(i);
        g_viz[i] = (v < SZ_VIZ_COUNT) ? (SzVizMode)v : SZ_VIZ_FOCUS;
    }
}

SzSensorId sz_get_sensor() {
    return g_sensor;
}

SzVizMode sz_get_viz() {
    return g_viz[g_sensor];
}

void sz_set_sensor(uint8_t sensor_id) {
    if (sensor_id >= SZ_SENSOR_COUNT) return;
    g_sensor = (SzSensorId)sensor_id;
    save_sz_sensor_store(sensor_id);
    runtime_request_ui_full_redraw();
}

void sz_next_sensor() {
    g_sensor = (SzSensorId)((uint8_t)(g_sensor + 1) % SZ_SENSOR_COUNT);
    save_sz_sensor_store((uint8_t)g_sensor);
    runtime_request_ui_full_redraw();
}

void sz_prev_sensor() {
    g_sensor = (SzSensorId)((g_sensor == SZ_TEMP)
        ? (uint8_t)(SZ_SENSOR_COUNT - 1)
        : (uint8_t)(g_sensor - 1));
    save_sz_sensor_store((uint8_t)g_sensor);
    runtime_request_ui_full_redraw();
}

void sz_next_viz() {
    SzVizMode& v = g_viz[g_sensor];
    v = (SzVizMode)((uint8_t)(v + 1) % SZ_VIZ_COUNT);
    save_sz_viz_store((uint8_t)g_sensor, (uint8_t)v);
    runtime_request_ui_full_redraw();
}

void sz_sync_renderer(bool force) {
    // Track last-synced state so we only push to the sub-renderer when something changed,
    // never on every sensor-data tick (which would cause constant cache invalidation / flicker).
    static SzSensorId last_sensor = (SzSensorId)0xFF;
    static SzVizMode  last_viz    = (SzVizMode)0xFF;

    const SzVizMode v = g_viz[g_sensor];
    if (!force && g_sensor == last_sensor && v == last_viz) return;

    const uint8_t s = (uint8_t)g_sensor;

    switch (v) {
        case SZ_VIZ_CARD:
            lab_sensor_card_set_sensor(kSzToCardId[s]);
            break;
        case SZ_VIZ_VALOR:
            lab_value_set_sensor(s);
            break;
        case SZ_VIZ_FOCUS:
            lab_focus_set_sensor((LabFocusSensor)kSzToFocusId[s]);
            break;
        case SZ_VIZ_GRAPH:
            graph_set_sensor(s);
            break;
        case SZ_VIZ_GAUGE:
            lab_gauge_set_sensor(s);
            break;
        default:
            break;
    }

    last_sensor = g_sensor;
    last_viz    = v;
}

const char* sz_sensor_name(SzSensorId id) {
    switch (id) {
        case SZ_TEMP:  return "TEMPERATURA";
        case SZ_HUM:   return "HUMEDAD";
        case SZ_LIGHT: return "LUZ";
        case SZ_SOUND: return "SONIDO";
        case SZ_SOIL:  return "SUELO";
        case SZ_DS18:  return "TERMOMETRO";
        default:       return "?";
    }
}

const char* sz_header_name() {
    static const char* const kShort[SZ_SENSOR_COUNT] = {
        "TEMP", "HUM", "LUZ", "SONIDO", "SUELO", "TERMO"
    };
    const uint8_t s = (uint8_t)g_sensor;
    switch (g_viz[g_sensor]) {
        case SZ_VIZ_FOCUS: return sz_sensor_name(g_sensor);
        case SZ_VIZ_CARD:  { static char buf[24]; snprintf(buf, sizeof(buf), "%s CARD",  kShort[s]); return buf; }
        case SZ_VIZ_VALOR: { static char buf[24]; snprintf(buf, sizeof(buf), "%s LAB",   kShort[s]); return buf; }
        case SZ_VIZ_GRAPH: { static char buf[24]; snprintf(buf, sizeof(buf), "%s GRAF",  kShort[s]); return buf; }
        case SZ_VIZ_GAUGE: { static char buf[24]; snprintf(buf, sizeof(buf), "%s DIAL",  kShort[s]); return buf; }
        default:           return sz_sensor_name(g_sensor);
    }
}

void sz_set_active(bool active) { g_sz_active = active; }
bool sz_is_active()             { return g_sz_active; }

void sz_sensor_rgb(SzSensorId id, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (id) {
        case SZ_TEMP:  r=255; g=69;  b=0;   return;
        case SZ_HUM:   r=0;   g=0;   b=255; return;
        case SZ_LIGHT: r=0;   g=0;   b=0;   return; // LED apagado: evita incidir en LDR
        case SZ_SOUND: r=255; g=0;   b=255; return;
        case SZ_SOIL:  r=180; g=80;  b=0;   return;
        case SZ_DS18:  r=255; g=255; b=255; return;
        default:       r=0;   g=0;   b=0;   return;
    }
}
