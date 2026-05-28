#pragma once
#include "tft_display.h"

// Public API for the startup logo sequence.
// When detect_encoder_hold=true, returns true if the encoder button is held
// at any sampled point during the logo/tone sequence.
bool run_boot_sequence(bool detect_encoder_hold = false);
