// led_control.cpp
#include "led_control.h"
#include "hw.h" 

// --- Definición de Pines ---
const int RGB_R_PIN = 5;
const int RGB_G_PIN = 17;
const int RGB_B_PIN = 16;
const int BUZZER_PIN = 18; // Definición
// ---------------------------------

// --- Canales PWM (ledc) ---
const int RGB_R_CHANNEL = 0;
const int RGB_G_CHANNEL = 1;
const int RGB_B_CHANNEL = 2;
const int BUZZER_CHANNEL = 3; // 🟢 FIX: Definición del canal del Buzzer
const int PWM_FREQ = 5000;
const int BUZZER_RESOLUTION = 8;
// ---------------------------------

unsigned long buzzer_stop_time = 0;


/**
 * @brief Inicializa los pines del LED RGB y el Buzzer 
 */
void init_leds_and_buzzer() {
    // 1. Configurar los 3 canales para el LED RGB
    ledcSetup(RGB_R_CHANNEL, PWM_FREQ, 8); // 8-bit resolution
    ledcSetup(RGB_G_CHANNEL, PWM_FREQ, 8);
    ledcSetup(RGB_B_CHANNEL, PWM_FREQ, 8);

    // 2. "Atar" cada pin a su canal
    ledcAttachPin(RGB_R_PIN, RGB_R_CHANNEL);
    ledcAttachPin(RGB_G_PIN, RGB_G_CHANNEL);
    ledcAttachPin(RGB_B_PIN, RGB_B_CHANNEL);

    // 3. Configurar el canal del Buzzer Pasivo
    ledcSetup(BUZZER_CHANNEL, 1000, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWrite(BUZZER_CHANNEL, 0); // Apagado

    // 4. Apagar el LED RGB al inicio
    set_rgb(0, 0, 0);

    Serial.println("[Hardware] LEDs y Buzzer (Pasivo en Pin 18) inicializados.");
}

/**
 * @brief Establece el color del LED RGB.
 */
void set_rgb(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(RGB_R_CHANNEL, 255 - r); // Asumiendo Cátodo Común
    ledcWrite(RGB_G_CHANNEL, 255 - g);
    ledcWrite(RGB_B_CHANNEL, 255 - b);
}


// --- ¡FUNCIONES DE BUZZER CORREGIDAS (PASIVO)! ---

/**
 * @brief (Asíncrono) Toca una nota y la apaga en el loop.
 */
void beep(int freq_hz, int duration_ms) {
    ledcChangeFrequency(BUZZER_CHANNEL, freq_hz, BUZZER_RESOLUTION);
    ledcWrite(BUZZER_CHANNEL, 128); // 50% duty cycle
    
    buzzer_stop_time = millis() + duration_ms;
}

/**
 * @brief Apaga el buzzer (para el loop asíncrono).
 */
void stop_beep() {
    ledcWrite(BUZZER_CHANNEL, 0); // Apagado (0% duty cycle)
    buzzer_stop_time = 0;
}

/**
 * @brief (Asíncrono) Función de loop para apagar el buzzer.
 */
void loop_buzzer() {
    if (buzzer_stop_time == 0) {
        return;
    }
    if (millis() >= buzzer_stop_time) {
        stop_beep();
    }
}

/**
 * @brief (Síncrono/Bloqueante) Toca una nota para la secuencia de arranque.
 */
void play_tone_blocking(int freq_hz, int duration_ms) {
    if (freq_hz == 0) { // Silencio
        ledcWrite(BUZZER_CHANNEL, 0);
    } else {
        ledcChangeFrequency(BUZZER_CHANNEL, freq_hz, BUZZER_RESOLUTION);
        ledcWrite(BUZZER_CHANNEL, 128); // Tono ON
    }
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledcWrite(BUZZER_CHANNEL, 0); // Tono OFF
}