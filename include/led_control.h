#pragma once
#include <Arduino.h>

// --- Prototipos de Funciones ---
void init_leds_and_buzzer();
void set_rgb(uint8_t r, uint8_t g, uint8_t b);

// --- Funciones de Sonido ---

// (Asíncrono) Toca una nota y la apaga en el loop principal.
void beep(int freq_hz, int duration_ms);
void loop_buzzer();
void stop_beep();

// 🟢 FIX: (Síncrono/Bloqueante) Prototipo de la función de tono para el arranque.
void play_tone_blocking(int freq_hz, int duration_ms);