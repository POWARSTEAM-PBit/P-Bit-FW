// led_control.cpp
// Shared RGB LED and passive buzzer driver.
#include "led_control.h"
#include "hw.h"
#include "config.h"
#include <freertos/semphr.h>

// --- Pin mapping ---
constexpr int RGB_R_PIN = 5;
constexpr int RGB_G_PIN = 17;
constexpr int RGB_B_PIN = 16;
constexpr int BUZZER_PIN = 18;
// ---------------------------------

// --- PWM channels ---
constexpr int RGB_R_CHANNEL = 0;
constexpr int RGB_G_CHANNEL = 1;
constexpr int RGB_B_CHANNEL = 2;
constexpr int BUZZER_CHANNEL = 3;
constexpr int PWM_FREQ = 5000;
constexpr int BUZZER_RESOLUTION = 8;
// ---------------------------------

unsigned long buzzer_stop_time = 0;
constexpr size_t MAX_TONE_SEQUENCE_STEPS = 12;
ToneStep g_tone_sequence[MAX_TONE_SEQUENCE_STEPS];
size_t g_tone_sequence_count = 0;
size_t g_tone_sequence_index = 0;
SemaphoreHandle_t g_buzzer_mutex = nullptr;

static bool take_buzzer_mutex(TickType_t timeout_ticks = pdMS_TO_TICKS(5)) {
    return !g_buzzer_mutex || xSemaphoreTake(g_buzzer_mutex, timeout_ticks) == pdTRUE;
}

static void give_buzzer_mutex() {
    if (g_buzzer_mutex) {
        xSemaphoreGive(g_buzzer_mutex);
    }
}

static void start_tone_step(const ToneStep& step) {
    if (step.freq_hz <= 0) {
        ledcWrite(BUZZER_CHANNEL, 0);
    } else {
        ledcChangeFrequency(BUZZER_CHANNEL, step.freq_hz, BUZZER_RESOLUTION);
        ledcWrite(BUZZER_CHANNEL, 128);
    }
    buzzer_stop_time = millis() + (unsigned long)step.duration_ms;
}

static void stop_beep_unlocked() {
    ledcWrite(BUZZER_CHANNEL, 0); // Apagado (0% duty cycle)
    buzzer_stop_time = 0;
    g_tone_sequence_count = 0;
    g_tone_sequence_index = 0;
}


/**
 * Initialize the RGB LED and buzzer PWM channels.
 */
void init_leds_and_buzzer() {
    if (!g_buzzer_mutex) {
        g_buzzer_mutex = xSemaphoreCreateMutex();
    }

    // 1. Configure the RGB channels.
    ledcSetup(RGB_R_CHANNEL, PWM_FREQ, 8); // 8-bit resolution
    ledcSetup(RGB_G_CHANNEL, PWM_FREQ, 8);
    ledcSetup(RGB_B_CHANNEL, PWM_FREQ, 8);

    // 2. Attach each pin to its channel.
    ledcAttachPin(RGB_R_PIN, RGB_R_CHANNEL);
    ledcAttachPin(RGB_G_PIN, RGB_G_CHANNEL);
    ledcAttachPin(RGB_B_PIN, RGB_B_CHANNEL);

    // 3. Configure the passive buzzer channel.
    ledcSetup(BUZZER_CHANNEL, 1000, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0); // Apagado

    // 4. Start with the RGB LED off.
    set_rgb(0, 0, 0);

    DPRINTLN("[Hardware] RGB LED and passive buzzer initialized.");
}

/**
 * Set the RGB LED color.
 */
void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(RGB_R_CHANNEL, 255 - r); // Common-cathode assumption.
    ledcWrite(RGB_G_CHANNEL, 255 - g);
    ledcWrite(RGB_B_CHANNEL, 255 - b);
}


// --- Passive buzzer helpers ---

/**
 * Schedule a buzzer tone and let loop_buzzer() stop it later.
 */
void beep(int freq_hz, int duration_ms) {
    if (!take_buzzer_mutex()) return;
    g_tone_sequence_count = 0;
    g_tone_sequence_index = 0;
    ledcChangeFrequency(BUZZER_CHANNEL, freq_hz, BUZZER_RESOLUTION);
    ledcWrite(BUZZER_CHANNEL, 128); // 50% duty cycle
    buzzer_stop_time = millis() + duration_ms;
    give_buzzer_mutex();
}

void play_tone_sequence(const ToneStep* steps, size_t count) {
    if (!steps || count == 0) {
        stop_beep();
        return;
    }

    if (!take_buzzer_mutex()) return;
    g_tone_sequence_count = (count > MAX_TONE_SEQUENCE_STEPS) ? MAX_TONE_SEQUENCE_STEPS : count;
    g_tone_sequence_index = 0;

    for (size_t i = 0; i < g_tone_sequence_count; ++i) {
        g_tone_sequence[i] = steps[i];
    }

    start_tone_step(g_tone_sequence[0]);
    give_buzzer_mutex();
}

/**
 * Stop the buzzer and clear the pending timeout.
 */
void stop_beep() {
    if (!take_buzzer_mutex()) return;
    stop_beep_unlocked();
    give_buzzer_mutex();
}

/**
 * Poll the buzzer timeout from the main loop.
 */
void loop_buzzer() {
    if (!take_buzzer_mutex(0)) return;
    if (buzzer_stop_time == 0) {
        give_buzzer_mutex();
        return;
    }
    if (millis() >= buzzer_stop_time) {
        if (g_tone_sequence_count > 0 && (g_tone_sequence_index + 1) < g_tone_sequence_count) {
            ++g_tone_sequence_index;
            start_tone_step(g_tone_sequence[g_tone_sequence_index]);
        } else {
            stop_beep_unlocked();
        }
    }
    give_buzzer_mutex();
}

/**
 * Blocking tone helper used by the boot sequence and short melodies.
 */
void play_tone_blocking(int freq_hz, int duration_ms) {
    if (!take_buzzer_mutex(pdMS_TO_TICKS(20))) return;
    if (freq_hz == 0) { // Silencio
        ledcWrite(BUZZER_CHANNEL, 0);
    } else {
        ledcChangeFrequency(BUZZER_CHANNEL, freq_hz, BUZZER_RESOLUTION);
        ledcWrite(BUZZER_CHANNEL, 128); // Tono ON
    }
    give_buzzer_mutex();
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    if (!take_buzzer_mutex(pdMS_TO_TICKS(20))) {
        ledcWrite(BUZZER_CHANNEL, 0);
        return;
    }
    ledcWrite(BUZZER_CHANNEL, 0); // Tono OFF
    give_buzzer_mutex();
}
