#include "runtime_events.h"
#include "io.h"

namespace {

portMUX_TYPE g_runtime_events_mux = portMUX_INITIALIZER_UNLOCKED;

} // namespace

void runtime_mark_sensor_data_ready() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    g_sensor_data_ready = true;
    portEXIT_CRITICAL(&g_runtime_events_mux);
}

bool runtime_take_sensor_data_ready() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    bool pending = g_sensor_data_ready;
    g_sensor_data_ready = false;
    portEXIT_CRITICAL(&g_runtime_events_mux);
    return pending;
}

void runtime_request_ui_refresh(bool force_full) {
    portENTER_CRITICAL(&g_runtime_events_mux);
    g_sensor_data_ready = true;
    if (force_full) {
        g_ui_force_full_redraw = true;
    }
    portEXIT_CRITICAL(&g_runtime_events_mux);
}

void runtime_request_ui_full_redraw() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    g_ui_force_full_redraw = true;
    portEXIT_CRITICAL(&g_runtime_events_mux);
}

bool runtime_take_ui_full_redraw() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    bool pending = g_ui_force_full_redraw;
    g_ui_force_full_redraw = false;
    portEXIT_CRITICAL(&g_runtime_events_mux);
    return pending;
}

void runtime_set_ui_overlay(UiOverlayState state) {
    portENTER_CRITICAL(&g_runtime_events_mux);
    g_ui_overlay_state = state;
    portEXIT_CRITICAL(&g_runtime_events_mux);
}

UiOverlayState runtime_get_ui_overlay() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    UiOverlayState state = g_ui_overlay_state;
    portEXIT_CRITICAL(&g_runtime_events_mux);
    return state;
}

void runtime_set_last_active_screen_before_sleep(Screen screen) {
    portENTER_CRITICAL(&g_runtime_events_mux);
    g_last_active_screen_before_sleep = screen;
    portEXIT_CRITICAL(&g_runtime_events_mux);
}

Screen runtime_get_last_active_screen_before_sleep() {
    portENTER_CRITICAL(&g_runtime_events_mux);
    Screen screen = static_cast<Screen>(g_last_active_screen_before_sleep);
    portEXIT_CRITICAL(&g_runtime_events_mux);
    return screen;
}
