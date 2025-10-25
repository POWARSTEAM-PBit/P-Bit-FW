#pragma once
#include <Arduino.h>

// Initialize ADC for the LDR input (call once in setup/init)
void  ldr_init();

// Read estimated light level in lux (float)
// Plug-and-play: no calibration required.
float ldr_read_lux();

// (Optional) Read averaged raw ADC (0..4095) for debugging
int   ldr_read_raw();
