#pragma once

#include <ESP32RotaryEncoder.h>

constexpr uint8_t DI_ENCODER_A = 14;
constexpr uint8_t DI_ENCODER_B = 12;
constexpr int8_t  DI_ENCODER_SW = 13;
constexpr int8_t  DO_ENCODER_VCC = -1;

extern RotaryEncoder rotaryEncoder;

/**
 * Initialize the rotary encoder, callbacks, and default boundaries.
 */
void init_rotary();

/**
 * Handle debounce, short presses on release, and long-press actions while held.
 */
void poll_rotary_aux();
