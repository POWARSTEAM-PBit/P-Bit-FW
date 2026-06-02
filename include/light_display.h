#pragma once

#include "io.h"
#include <stddef.h>
#include <stdint.h>

constexpr uint8_t LIGHT_DISPLAY_LUX = 0;
constexpr uint8_t LIGHT_DISPLAY_FC = 1;
constexpr uint8_t LIGHT_DISPLAY_RAW_ADC = 2;

constexpr float LIGHT_LUX_MAX = 8000.0f;
constexpr float LIGHT_RAW_ADC_MAX = 4095.0f;
constexpr float LIGHT_LUX_TO_FC = 1.0f / 10.764f;

struct LightDisplayReading {
    uint8_t mode;
    bool valid;
    float value;
    const char* unit;
    int key;
};

uint8_t light_display_mode();
const char* light_display_mode_name(uint8_t mode);
const char* light_display_unit(uint8_t mode);

bool light_lux_valid(float lux);
bool light_raw_valid(float raw);

float light_display_lux_to_unit(float lux, uint8_t mode);
float light_display_max(uint8_t mode);
float light_display_min_span(uint8_t mode);
float light_display_ratio(float value, uint8_t mode, bool valid);

LightDisplayReading light_display_from_values(float lux, float raw);
LightDisplayReading light_display_from_reading(const Reading& reading);
void light_format_value(char* out, size_t out_size, const LightDisplayReading& display, bool compact_lux_k);
