#pragma once
#include <Arduino.h>

// --- Core hardware helpers ---
void init_leds_and_buzzer();
void set_rgb(uint8_t r, uint8_t g, uint8_t b);

// --- Sound helpers ---
struct ToneStep {
    int freq_hz;
    int duration_ms;
};

// Schedule a tone and let loop_buzzer() stop it later.
void beep(int freq_hz, int duration_ms);
void play_tone_sequence(const ToneStep* steps, size_t count);
void loop_buzzer();
void stop_beep();

// Blocking tone helper used by the boot sequence and short melodies.
void play_tone_blocking(int freq_hz, int duration_ms);
