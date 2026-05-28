#include "demo_mode.h"

#include "config.h"
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
    { LAB_HOME_CARDS_SCREEN,     -1,       0,            6 },
    { LAB_DUAL_TH_SCREEN,        -1,       0,            6 },
    { LAB_WIDGET_MIX_SCREEN,     -1,       0,            6 },
    { LAB_SOUND_VU_STACK_SCREEN, -1,       0,            6 },
    { SENSOR_ZONE_SCREEN,        SZ_TEMP,  SZ_VIZ_GAUGE, 6 },
    { SENSOR_ZONE_SCREEN,        SZ_HUM,   SZ_VIZ_CARD,  6 },
    { SENSOR_ZONE_SCREEN,        SZ_LIGHT, SZ_VIZ_GRAPH, 6 },
    { SENSOR_ZONE_SCREEN,        SZ_SOUND, SZ_VIZ_CARD,  6 },
    { SENSOR_ZONE_SCREEN,        SZ_SOIL,  SZ_VIZ_GAUGE, 6 },
    { SENSOR_ZONE_SCREEN,        SZ_DS18,  SZ_VIZ_VALOR, 6 },
    { TIMER_SCREEN,              -1,       0,            6 },
};
#else
constexpr DemoScene kDemoScenes[] = {
    { TEMP_SCREEN,    -1, 0, 6 },
    { HUMIDITY_SCREEN,-1, 0, 6 },
    { LIGHT_SCREEN,   -1, 0, 6 },
    { SOUND_SCREEN,   -1, 0, 6 },
    { SOIL_SCREEN,    -1, 0, 6 },
    { DS18B20_SCREEN, -1, 0, 6 },
    { TIMER_SCREEN,   -1, 0, 6 },
};
#endif

constexpr uint8_t kDemoSceneCount = sizeof(kDemoScenes) / sizeof(kDemoScenes[0]);
constexpr bool kDemoSimulatedReadings = true;

bool g_demo_active = false;
bool g_wait_boot_release = false;
uint8_t g_scene_index = 0;
uint32_t g_next_change_ms = 0;
bool g_pending_first_scene = false;
uint32_t g_splash_until_ms = 0;

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
    g_demo_active = true;
    g_wait_boot_release = consume_current_release;
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

    const float t = (float)(millis() % 12000UL) / 12000.0f;
    const float wave = 0.5f + 0.5f * sinf(t * 6.2831853f);
    const float wave_fast = 0.5f + 0.5f * sinf(t * 18.849556f + 0.8f);
    const float wave_slow = 0.5f + 0.5f * sinf(t * 6.2831853f + 2.2f);

    reading.temperature = 18.5f + (9.5f * wave);
    reading.humidity = 38.0f + (34.0f * wave_slow);
    reading.ldr = 60.0f + (4200.0f * wave * wave);
    reading.ldr_raw = 350.0f + (3000.0f * wave);
    reading.mic = 12.0f + (76.0f * wave_fast);
    reading.soil_humidity = 18.0f + (70.0f * (0.5f + 0.5f * sinf(t * 6.2831853f + 4.0f)));
    reading.temp_ds18b20 = 17.0f + (15.0f * (0.5f + 0.5f * sinf(t * 6.2831853f + 1.4f)));
}

bool demo_mode_consume_boot_release() {
    if (!g_demo_active || !g_wait_boot_release) return false;
    g_wait_boot_release = false;
    return true;
}
