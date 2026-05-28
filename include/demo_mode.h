#pragma once

#include <Arduino.h>
#include "io.h"

// Runtime-only presentation carousel. It never writes NVS preferences.
void demo_mode_start(bool consume_current_release = true);
void demo_mode_stop();
void demo_mode_service();
bool demo_mode_is_active();
bool demo_mode_splash_active();
void demo_mode_apply_simulated_readings(Reading& reading);

// Consumes the release of the encoder button that was already held during boot.
bool demo_mode_consume_boot_release();
