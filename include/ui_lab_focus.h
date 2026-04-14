#pragma once

#include <stdint.h>

// Temporary laboratory screen for focused sensor views.
// The integrator can hook this into the carousel and rotary later.
enum LabFocusSensor : uint8_t {
    LAB_FOCUS_HUMIDITY = 0,
    LAB_FOCUS_TEMP,
    LAB_FOCUS_LIGHT,
    LAB_FOCUS_SOUND,
    LAB_FOCUS_SOIL,
    LAB_FOCUS_DS18,
    LAB_FOCUS_COUNT
};

// Advance to the next sensor in the lab focus carousel.
void lab_focus_cycle_sensor();

// Optional helper for integrators that want to inspect or restore the state.
LabFocusSensor lab_focus_get_sensor();
void lab_focus_set_sensor(LabFocusSensor sensor);

// Draw the current lab focus screen.
void draw_lab_focus_screen(bool screen_changed, bool sensor_data_changed);
