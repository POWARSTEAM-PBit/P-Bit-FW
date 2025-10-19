#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include "rotary.h"
#include "tft_display.h"
#include "timer.h"

RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC);

/**
 * @brief Callback function for rotary encoder knob turn
 */
void knobCallback(uint8_t value) {
    active_screen = static_cast<Screen>(value);
    Serial.printf("[Rotary] Switched to screen %d\n", active_screen);
}

/**
 * @brief Callback function for rotary encoder button press
 */
void buttonCallback(unsigned long duration) {
    if (active_screen == TIMER_SCREEN) {
        if (!userTimerRunning) {
            startUserTimer();
        } else {
            stopUserTimer();
        }
    }
}

void init_rotary() {
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    rotaryEncoder.setBoundaries(0, TIMER_SCREEN, true); // 6 screens: 0–5
    rotaryEncoder.onTurned(&knobCallback);
    rotaryEncoder.onPressed(&buttonCallback);
    rotaryEncoder.begin();
    Serial.println("[Rotary] Initialized");
}
