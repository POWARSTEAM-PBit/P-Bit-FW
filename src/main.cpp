// main.cpp
// Núcleo principal del P-Bit.
// Gestiona la inicialización de módulos, las variables globales y la lógica de energía.

#include <Arduino.h>
#include "tft_display.h" // Cabecera principal de la UI
#include "io.h"         
#include "ble.h"
#include "rotary.h"
#include "hw.h"
#include "led_control.h" 
#include <esp_sleep.h> // Para Light/Deep Sleep
#include "ui_widgets.h" // Para tft
#include "ui_boot.h"    // Para la secuencia de arranque
#include "lang_select.h" // Para selección de idioma

#define SERIAL_BAUD_RATE 115200

// --- DEFINICIONES DE VARIABLES GLOBALES ---
bool g_is_fahrenheit = false; // Estado C/F
bool g_sound_enabled = true; // El sonido está activado por defecto

// Variables de gestión de energía
volatile unsigned long g_last_activity_ms = 0;
volatile bool g_peripherals_sleeping = false;
const unsigned long LIGHT_SLEEP_TIMEOUT_MS  = 60000;  // 1 Minuto
const unsigned long DEEP_SLEEP_TIMEOUT_MS   = 600000; // 10 Minutos
const unsigned long SLEEP_WARNING_MS        = LIGHT_SLEEP_TIMEOUT_MS - 5000; // 55s — aviso previo al sleep

// Flag que congela la tarea de UI durante la secuencia de aviso pre-sleep
volatile bool g_sleep_warning_active = false;

// --- DECLARACIONES EXTERNAS ---
extern bool client_connected; // Desde ble.cpp

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Inicializar temporizador de actividad
    g_last_activity_ms = millis();
    
    // Inicialización de módulos
    set_devicename();
    init_tft_display();
    init_ble();
    init_leds_and_buzzer();
    init_hw();

    // ---------------------------------------------------------
    // LÓGICA DE ARRANQUE SILENCIOSO (Evita animación al despertar)
    // ---------------------------------------------------------

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();

    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : // Despertar por Botón (GPIO 13)
            Serial.println("[Power] Waking up from Deep Sleep (Button).");
            loadLanguage();           // Cargar idioma desde NVS sin mostrar menú
            g_peripherals_sleeping = false;
            active_screen = TEMP_SCREEN;
            break;

        default : // Encendido normal (Power On)
            Serial.println("[Power] Cold Boot detected. Running boot sequence.");
            run_boot_sequence();
            active_screen = TEMP_SCREEN;
            showLanguageMenu();       // Siempre en cold boot — usa rotaryEncoder internamente
            break;
    }
    // ---------------------------------------------------------

    // init_rotary() reconfigura el encoder con límites y callbacks para la app principal
    init_rotary();

    // --- FreeRTOS Tasks ---

    // Tarea de Pantalla (UI): Núcleo 1
    xTaskCreatePinnedToCore(
        switch_screen,   
        "SwitchScreen",  
        4096,            
        NULL,            
        1,               
        NULL,            
        1                
    );
    
    // Tarea de Sensores (Núcleo 0)
    xTaskCreatePinnedToCore(
        sensor_reading_task, 
        "SensorTask",        
        4096,                
        NULL,                
        1,                   // Prioridad 1
        NULL,                
        0                    // Asignar al Núcleo 0
    );
}

void loop() {
    // El loop principal (Núcleo 1) solo gestiona tareas rápidas, debug y energía
    rotaryEncoder.loop();
    loop_buzzer(); 
    
    unsigned long inactivity_time = millis() - g_last_activity_ms;

    // ---------------------------------------------------------
    // AVISO PRE-SLEEP (últimos 5 segundos antes del Light Sleep)
    // ---------------------------------------------------------
    {
        static bool warning_beeps_done = false;
        static bool warning_zzz_done   = false;

        // Reiniciar flags cuando hay actividad o no aplican las condiciones de sleep
        if (inactivity_time < SLEEP_WARNING_MS || client_connected || g_peripherals_sleeping) {
            warning_beeps_done = false;
            warning_zzz_done   = false;
            g_sleep_warning_active = false;
        }

        if (inactivity_time >= SLEEP_WARNING_MS && !client_connected && !g_peripherals_sleeping) {

            // ONE-SHOT en t=55s: 3 destellos ámbar + 3 pitidos
            if (!warning_beeps_done) {
                warning_beeps_done = true;
                for (int i = 0; i < 3; i++) {
                    set_rgb(255, 80, 0); // Ámbar
                    if (g_sound_enabled) play_tone_blocking(700, 80);
                    else                 vTaskDelay(pdMS_TO_TICKS(80));
                    set_rgb(0, 0, 0);
                    vTaskDelay(pdMS_TO_TICKS(250));
                }
            }

            // ONE-SHOT en t=58s: congelar UI y mostrar pantalla ZZZ
            if (inactivity_time >= LIGHT_SLEEP_TIMEOUT_MS - 2000 && !warning_zzz_done) {
                warning_zzz_done       = true;
                g_sleep_warning_active = true; // Congela la tarea de UI

                tft.fillScreen(TFT_BLACK);
                tft.setTextDatum(MC_DATUM);
                tft.setTextColor(TFT_CYAN, TFT_BLACK);
                tft.drawString("ZZZ", tft.width() / 2, tft.height() / 2 - 14, 4);
                tft.setTextColor(TFT_DARKCYAN, TFT_BLACK);
                tft.drawString("sleeping...", tft.width() / 2, tft.height() / 2 + 14, 2);
            }
        }
    }

    // ---------------------------------------------------------
    // 1. CASO DEEP SLEEP (10 Minutos Y SIN BLE)
    // ---------------------------------------------------------
    if (inactivity_time >= DEEP_SLEEP_TIMEOUT_MS && !client_connected) {
        Serial.println("[Power] 10 min inactivity AND BLE disconnected. Entering Deep Sleep.");

        tft.fillScreen(TFT_BLACK);
        set_rgb(0, 0, 0);

        // Mantener pull-up del botón activo durante el sleep para evitar falsas activaciones
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        // Despertar al presionar el botón (Nivel LOW)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
        esp_deep_sleep_start();
    }

    // ---------------------------------------------------------
    // 2. CASO LIGHT SLEEP (1 Minuto, SIN BLE — igual que Deep Sleep)
    // ---------------------------------------------------------
    if (inactivity_time >= LIGHT_SLEEP_TIMEOUT_MS && !client_connected) {

        // Si no estamos ya en modo ahorro, apagamos los periféricos
        if (!g_peripherals_sleeping) {
            Serial.println("[Power] 1 min inactivity. Entering Peripheral Sleep (TFT Black).");

            tft.fillScreen(TFT_BLACK);
            set_rgb(0, 0, 0);

            g_peripherals_sleeping = true;
        }

        // Mantener pull-up del botón activo durante el sleep para evitar falsas activaciones
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        // Entramos en Light Sleep (Ahorro de CPU)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
        esp_light_sleep_start();

        // ... (Al despertar por el botón, el rotary.cpp ejecutará wakeUpPeripherals()) ...
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); 
}