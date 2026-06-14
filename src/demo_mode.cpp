#include "demo_mode.h"

#include "config.h"
#include "light_display.h"
#include "runtime_events.h"
#include "tft_display.h"
#include <math.h>

#if PBIT_ENABLE_GRAPH_LAB
#include "sensor_zone.h"
#endif

namespace {

struct DemoScene {
    Screen screen;
    int8_t sensor;
    uint8_t viz;
    uint16_t dwell_s;
};

#if PBIT_ENABLE_GRAPH_LAB
constexpr DemoScene kDemoScenes[] = {
    { LAB_HOME_CARDS_SCREEN,     -1,       0,            8 },
    { LAB_DUAL_TH_SCREEN,        -1,       0,            8 },
    { LAB_WIDGET_MIX_SCREEN,     -1,       0,            9 },
    { SENSOR_ZONE_SCREEN,        SZ_SOUND, SZ_VIZ_SOUND_VU_STACK, 7 },
    { SENSOR_ZONE_SCREEN,        SZ_SOUND, SZ_VIZ_SOUND_VU_WAVE,  7 },
    { SENSOR_ZONE_SCREEN,        SZ_TEMP,  SZ_VIZ_GAUGE, 8 },
    { SENSOR_ZONE_SCREEN,        SZ_HUM,   SZ_VIZ_CARD,  7 },
    { SENSOR_ZONE_SCREEN,        SZ_LIGHT, SZ_VIZ_GRAPH, 10 },
    { SENSOR_ZONE_SCREEN,        SZ_SOUND, SZ_VIZ_CARD,  7 },
    { SENSOR_ZONE_SCREEN,        SZ_SOIL,  SZ_VIZ_GAUGE, 8 },
    { SENSOR_ZONE_SCREEN,        SZ_DS18,  SZ_VIZ_VALOR, 8 },
    { TIMER_SCREEN,              -1,       0,            6 },
};
#else
constexpr DemoScene kDemoScenes[] = {
    { TEMP_SCREEN,    -1, 0, 8 },
    { HUMIDITY_SCREEN,-1, 0, 7 },
    { LIGHT_SCREEN,   -1, 0, 9 },
    { SOUND_SCREEN,   -1, 0, 7 },
    { SOIL_SCREEN,    -1, 0, 8 },
    { DS18B20_SCREEN, -1, 0, 8 },
    { TIMER_SCREEN,   -1, 0, 6 },
};
#endif

constexpr uint8_t kDemoSceneCount = sizeof(kDemoScenes) / sizeof(kDemoScenes[0]);
constexpr bool kDemoSimulatedReadings = true;
constexpr uint16_t kDemoValueRefreshMs = 220;
constexpr float kTwoPi = 6.2831853f;
constexpr uint8_t kGraphSensorTemp = 0;
constexpr uint8_t kGraphSensorHum = 1;
constexpr uint8_t kGraphSensorLight = 2;
constexpr uint8_t kGraphSensorSound = 3;
constexpr uint8_t kGraphSensorSoil = 4;
constexpr uint8_t kGraphSensorDs18 = 5;

bool g_demo_active = false;
bool g_wait_boot_release = false;
uint8_t g_scene_index = 0;
uint32_t g_next_change_ms = 0;
bool g_pending_first_scene = false;
uint32_t g_splash_until_ms = 0;
bool g_pre_demo_saved = false;
Screen g_pre_demo_active_screen = BOOT_SCREEN;
#if PBIT_ENABLE_GRAPH_LAB
SzRuntimeSnapshot g_pre_demo_sz_snapshot;
#endif
uint32_t g_demo_started_ms = 0;

float smoothstep(float x) {
    x = constrain(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

float smooth_wave(uint32_t now_ms, float period_ms, float phase) {
    const float t = fmodf((float)now_ms, period_ms) / period_ms;
    return smoothstep(0.5f + 0.5f * sinf((t * kTwoPi) + phase));
}

float demo_lux_to_raw(float lux) {
    const float safe_lux = constrain(lux, 0.0f, LIGHT_LUX_MAX);
    const float ratio = sqrtf(safe_lux / 10.0f);
    const float raw = (4095.0f - (150.0f * ratio)) / (ratio + 1.0f);
    return constrain(raw, 0.0f, LIGHT_RAW_ADC_MAX);
}

bool current_scene_is_graph_focus(uint8_t graph_sensor) {
    const DemoScene& scene = kDemoScenes[g_scene_index];
#if PBIT_ENABLE_GRAPH_LAB
    if (scene.screen == SENSOR_ZONE_SCREEN && scene.sensor == (int8_t)graph_sensor) return true;
    if (graph_sensor == kGraphSensorTemp && scene.screen == LAB_DUAL_TH_SCREEN) return true;
    if (graph_sensor == kGraphSensorDs18 && scene.screen == LAB_WIDGET_MIX_SCREEN) return true;
#else
    if (graph_sensor == kGraphSensorTemp && scene.screen == TEMP_SCREEN) return true;
    if (graph_sensor == kGraphSensorHum && scene.screen == HUMIDITY_SCREEN) return true;
    if (graph_sensor == kGraphSensorLight && scene.screen == LIGHT_SCREEN) return true;
    if (graph_sensor == kGraphSensorSound && scene.screen == SOUND_SCREEN) return true;
    if (graph_sensor == kGraphSensorSoil && scene.screen == SOIL_SCREEN) return true;
    if (graph_sensor == kGraphSensorDs18 && scene.screen == DS18B20_SCREEN) return true;
#endif
    return false;
}

void compute_demo_reading(uint32_t now_ms, Reading& reading) {
    const bool light_focus = current_scene_is_graph_focus(kGraphSensorLight);
    const bool sound_focus = current_scene_is_graph_focus(kGraphSensorSound);
    const bool soil_focus = current_scene_is_graph_focus(kGraphSensorSoil);
    const bool temp_focus = current_scene_is_graph_focus(kGraphSensorTemp);
    const bool hum_focus = current_scene_is_graph_focus(kGraphSensorHum);
    const bool ds18_focus = current_scene_is_graph_focus(kGraphSensorDs18);

    const float temp_wave = smooth_wave(now_ms, temp_focus ? 18000.0f : 24000.0f, 0.2f);
    const float hum_wave = smooth_wave(now_ms, hum_focus ? 22000.0f : 30000.0f, 2.1f);
    const float light_wave = smooth_wave(now_ms, light_focus ? 16000.0f : 26000.0f, 1.4f);
    const float light_glint = smooth_wave(now_ms, 6500.0f, 0.3f);
    const float sound_wave = smooth_wave(now_ms, sound_focus ? 4200.0f : 9000.0f, 0.8f);
    const float sound_peak = smooth_wave(now_ms, 1700.0f, 2.4f);
    const float soil_wave = smooth_wave(now_ms, soil_focus ? 21000.0f : 32000.0f, 4.0f);
    const float probe_wave = smooth_wave(now_ms, ds18_focus ? 20000.0f : 28000.0f, 1.0f);

    reading.temperature = 18.8f + ((temp_focus ? 9.0f : 7.0f) * temp_wave);
    reading.humidity = 40.0f + ((hum_focus ? 35.0f : 28.0f) * hum_wave);

    const float light_span = light_focus ? 6800.0f : 3600.0f;
    const float light_extra = light_focus ? (600.0f * light_glint) : 0.0f;
    reading.ldr = constrain(70.0f + (light_span * light_wave * light_wave) + light_extra, 0.0f, LIGHT_LUX_MAX);

    const float raw_jitter = 24.0f * (smooth_wave(now_ms, 5200.0f, 2.2f) - 0.5f);
    reading.ldr_raw = constrain(demo_lux_to_raw(reading.ldr) + raw_jitter, 0.0f, LIGHT_RAW_ADC_MAX);

    const float sound_energy = sound_focus
        ? ((sound_wave > sound_peak * 0.82f) ? sound_wave : sound_peak * 0.82f)
        : sound_wave;
    reading.mic = constrain(12.0f + ((sound_focus ? 78.0f : 52.0f) * sound_energy), 0.0f, 100.0f);
    reading.soil_humidity = 20.0f + ((soil_focus ? 68.0f : 52.0f) * soil_wave);
    reading.temp_ds18b20 = 16.8f + ((ds18_focus ? 15.5f : 12.0f) * probe_wave);
}

void apply_scene(uint8_t index) {
    if (index >= kDemoSceneCount) index = 0;
    g_scene_index = index;
    const DemoScene& scene = kDemoScenes[g_scene_index];

#if PBIT_ENABLE_GRAPH_LAB
    if (scene.screen == SENSOR_ZONE_SCREEN && scene.sensor >= 0) {
        sz_set_sensor_runtime((uint8_t)scene.sensor);
        sz_set_viz_runtime((uint8_t)scene.sensor, scene.viz);
    }
#endif

    active_screen = scene.screen;
    runtime_mark_sensor_data_ready();
    runtime_request_ui_full_redraw();
    g_next_change_ms = millis() + (uint32_t)scene.dwell_s * 1000UL;
}

} // namespace

void demo_mode_start(bool consume_current_release) {
    if (!g_pre_demo_saved) {
        g_pre_demo_active_screen = active_screen;
#if PBIT_ENABLE_GRAPH_LAB
        sz_snapshot_runtime(g_pre_demo_sz_snapshot);
#endif
        g_pre_demo_saved = true;
    }
    g_demo_started_ms = millis();

    g_demo_active = true;
    g_wait_boot_release = consume_current_release;
    g_scene_index = 0;
    g_pending_first_scene = true;
    g_splash_until_ms = millis() + 550UL;
    runtime_request_ui_full_redraw();
}

void demo_mode_stop() {
    if (!g_demo_active) return;
    g_demo_active = false;
    g_wait_boot_release = false;
    g_pending_first_scene = false;
    g_splash_until_ms = 0;

    if (g_pre_demo_saved) {
        active_screen = g_pre_demo_active_screen;
#if PBIT_ENABLE_GRAPH_LAB
        sz_restore_runtime(g_pre_demo_sz_snapshot);
#endif
        g_pre_demo_saved = false;
    }

    runtime_mark_sensor_data_ready();
    runtime_request_ui_full_redraw();
}

void demo_mode_service() {
    if (!g_demo_active) return;
    const uint32_t now = millis();
    if (g_pending_first_scene) {
        if ((int32_t)(now - g_splash_until_ms) < 0) return;
        g_pending_first_scene = false;
        apply_scene(0);
        return;
    }
    if ((int32_t)(now - g_next_change_ms) < 0) return;
    apply_scene((uint8_t)((g_scene_index + 1) % kDemoSceneCount));
}

bool demo_mode_is_active() {
    return g_demo_active;
}

bool demo_mode_splash_active() {
    return g_demo_active
        && g_pending_first_scene
        && ((int32_t)(millis() - g_splash_until_ms) < 0);
}

void demo_mode_apply_simulated_readings(Reading& reading) {
    if (!kDemoSimulatedReadings || !g_demo_active || g_pending_first_scene) return;

    compute_demo_reading(millis(), reading);
}

uint16_t demo_mode_value_refresh_ms() {
    return kDemoValueRefreshMs;
}

bool demo_mode_graph_values(uint8_t sensor, uint8_t light_mode, float* out, size_t out_size, size_t* out_count) {
    if (!kDemoSimulatedReadings || !g_demo_active || g_pending_first_scene || !out || out_size == 0) {
        if (out_count) *out_count = 0;
        return false;
    }

    const size_t count = (out_size < 96) ? out_size : 96;
    const uint32_t now = millis();
    constexpr uint32_t kStepMs = 260;

    for (size_t i = 0; i < count; ++i) {
        Reading sample;
        const uint32_t age = (uint32_t)(count - 1 - i) * kStepMs;
        const uint32_t sample_ms = (now >= age) ? (now - age) : 0;
        compute_demo_reading(sample_ms, sample);
        switch (sensor) {
            case kGraphSensorTemp:
                out[i] = sample.temperature;
                break;
            case kGraphSensorHum:
                out[i] = sample.humidity;
                break;
            case kGraphSensorLight:
                out[i] = (light_mode == LIGHT_DISPLAY_RAW_ADC) ? sample.ldr_raw : sample.ldr;
                break;
            case kGraphSensorSound:
                out[i] = sample.mic;
                break;
            case kGraphSensorSoil:
                out[i] = sample.soil_humidity;
                break;
            case kGraphSensorDs18:
                out[i] = sample.temp_ds18b20;
                break;
            default:
                out[i] = 0.0f;
                break;
        }
    }

    if (out_count) *out_count = count;
    return count > 0;
}

bool demo_mode_consume_boot_release() {
    if (!g_demo_active || !g_wait_boot_release) return false;
    g_wait_boot_release = false;
    return true;
}

uint32_t demo_mode_simulated_timer_ms() {
    if (!g_demo_active) return 0;
    return (millis() - g_demo_started_ms) % 60000UL;
}
