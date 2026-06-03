#include "light_display.h"

#include "hw.h"
#include "languages.h"

#include <Arduino.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

uint8_t light_display_mode() {
    const uint8_t mode = get_light_display_mode();
    return (mode <= LIGHT_DISPLAY_RAW_ADC) ? mode : LIGHT_DISPLAY_LUX;
}

const char* light_display_mode_name(uint8_t mode) {
    switch (mode) {
        case LIGHT_DISPLAY_FC:
            return L(ST_FC_UNIT);
        case LIGHT_DISPLAY_RAW_ADC:
            return L(ST_RAW_ADC);
        case LIGHT_DISPLAY_LUX:
        default:
            return L(ST_LUX_UNIT);
    }
}

const char* light_display_unit(uint8_t mode) {
    switch (mode) {
        case LIGHT_DISPLAY_FC:
            return L(ST_FC_UNIT);
        case LIGHT_DISPLAY_RAW_ADC:
            return L(ST_RAW_UNIT);
        case LIGHT_DISPLAY_LUX:
        default:
            return L(ST_LUX_UNIT);
    }
}

bool light_lux_valid(float lux) {
    return !isnan(lux) && isfinite(lux) && lux >= 0.0f && lux <= LIGHT_LUX_MAX;
}

bool light_raw_valid(float raw) {
    return !isnan(raw) && isfinite(raw) && raw >= 0.0f && raw <= LIGHT_RAW_ADC_MAX;
}

float light_display_lux_to_unit(float lux, uint8_t mode) {
    if (mode == LIGHT_DISPLAY_FC) {
        return lux * LIGHT_LUX_TO_FC;
    }
    return lux;
}

float light_display_max(uint8_t mode) {
    switch (mode) {
        case LIGHT_DISPLAY_FC:
            return LIGHT_LUX_MAX * LIGHT_LUX_TO_FC;
        case LIGHT_DISPLAY_RAW_ADC:
            return LIGHT_RAW_ADC_MAX;
        case LIGHT_DISPLAY_LUX:
        default:
            return LIGHT_LUX_MAX;
    }
}

float light_display_min_span(uint8_t mode) {
    switch (mode) {
        case LIGHT_DISPLAY_FC:
            return 15.0f;
        case LIGHT_DISPLAY_RAW_ADC:
            return 80.0f;
        case LIGHT_DISPLAY_LUX:
        default:
            return 150.0f;
    }
}

float light_display_ratio(float value, uint8_t mode, bool valid) {
    if (!valid) return 0.0f;
    const float max_value = light_display_max(mode);
    if (max_value <= 0.0f) return 0.0f;
    const float safe_value = constrain(value, 0.0f, max_value);
    if (mode == LIGHT_DISPLAY_RAW_ADC) {
        return safe_value / max_value;
    }
    return constrain(log1pf(safe_value) / log1pf(max_value), 0.0f, 1.0f);
}

LightDisplayReading light_display_from_values(float lux, float raw) {
    LightDisplayReading display;
    display.mode = light_display_mode();
    display.unit = light_display_unit(display.mode);
    display.value = 0.0f;
    display.valid = false;
    display.key = INT_MIN;

    if (display.mode == LIGHT_DISPLAY_RAW_ADC) {
        display.valid = light_raw_valid(raw);
        display.value = raw;
    } else {
        display.valid = light_lux_valid(lux);
        display.value = light_display_lux_to_unit(lux, display.mode);
    }

    if (display.valid) {
        display.key = (int)lroundf(display.value);
    }
    return display;
}

LightDisplayReading light_display_from_reading(const Reading& reading) {
    return light_display_from_values(reading.ldr, reading.ldr_raw);
}

void light_format_value(char* out, size_t out_size, const LightDisplayReading& display, bool compact_lux_k) {
    if (!out || out_size == 0) return;
    if (!display.valid) {
        snprintf(out, out_size, "---");
        return;
    }
    if (compact_lux_k && display.mode == LIGHT_DISPLAY_LUX && display.value >= 10000.0f) {
        snprintf(out, out_size, "%.0fk", display.value / 1000.0f);
        return;
    }
    snprintf(out, out_size, "%.0f", display.value);
}
