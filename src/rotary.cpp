#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include "rotary.h"
#include "tft_display.h"

RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC);

void knobCallback(uint8_t value) {
    // The rotary value will correspond to screen index (0–2)
    active_screen = static_cast<Screen>(value);

    Serial.printf("[Rotary] Switched to screen %d\n", active_screen);
}

void buttonCallback(unsigned long duration) {
    active_screen = static_cast<Screen>(3);
    Serial.printf("[Rotary] Button pressed for %lums\n", duration);
}

void init_rotary() {
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    rotaryEncoder.setBoundaries(0, 2, true); // 3 screens: 0–2
    rotaryEncoder.onTurned(&knobCallback);
    rotaryEncoder.onPressed(&buttonCallback);
    rotaryEncoder.begin();
    Serial.println("[Rotary] Initialized");
}
