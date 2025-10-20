#pragma once

#include <ESP32RotaryEncoder.h>

constexpr uint8_t DI_ENCODER_A = 14;
constexpr uint8_t DI_ENCODER_B = 12;
constexpr int8_t  DI_ENCODER_SW = 13;
constexpr int8_t  DO_ENCODER_VCC = -1;

extern RotaryEncoder rotaryEncoder;

/**
 * @brief Initialize the rotary encoder
 */
void init_rotary();
