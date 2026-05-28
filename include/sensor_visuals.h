#pragma once

#include <Arduino.h>
#include <stdint.h>

#include "io.h"
#include "sensor_zone.h"

uint16_t pbit_mix565(uint16_t a, uint16_t b, uint8_t amount_b);
uint16_t pbit_mix3_565(uint16_t low, uint16_t mid, uint16_t high, uint8_t amount);
uint8_t pbit_ratio_to_amount(float value, float min_value, float max_value);
void pbit_rgb565_to_rgb888(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b);

uint16_t pbit_sensor_gauge_arc_color(SzSensorId sensor, uint8_t amount);
uint16_t pbit_sensor_visual_color(SzSensorId sensor,
                                  const Reading& reading,
                                  bool fahrenheit,
                                  bool* out_valid = nullptr);
uint16_t pbit_timer_visual_color(bool finished_active,
                                 bool running,
                                 unsigned long elapsed_ms,
                                 bool flash_on);
