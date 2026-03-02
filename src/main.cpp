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
#include <esp_timer.h>
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
const unsigned long DEEP_SLEEP_TIMEOUT_MS   = 120000; // 2 Minutos (pruebas)
const unsigned long SLEEP_WARNING_MS        = LIGHT_SLEEP_TIMEOUT_MS - 5000; // 55s — aviso previo al sleep

// Flag que congela la tarea de UI durante la secuencia de aviso pre-sleep
volatile bool g_sleep_warning_active = false;

// --- DECLARACIONES EXTERNAS ---
extern bool client_connected; // Desde ble.cpp
RTC_DATA_ATTR uint8_t g_rtc_last_active_screen = TEMP_SCREEN;

static bool isRestorableScreen(Screen screen) {
    return screen >= TEMP_SCREEN && screen <= TIMER_SCREEN;
}

static Screen getPersistedSleepScreen() {
    Screen persisted = static_cast<Screen>(g_rtc_last_active_screen);
    return isRestorableScreen(persisted) ? persisted : TEMP_SCREEN;
}

static unsigned long now_ms() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

static void saveCurrentScreenForSleep() {
    if (isRestorableScreen(active_screen)) {
        g_last_active_screen_before_sleep = active_screen;
        g_rtc_last_active_screen = static_cast<uint8_t>(active_screen);
    }
}

static void playSleepSignal(uint8_t flashes, uint8_t r, uint8_t g, uint8_t b, int tone_hz) {
    for (uint8_t i = 0; i < flashes; i++) {
        set_rgb(r, g, b);
        if (g_sound_enabled) play_tone_blocking(tone_hz, 80);
        else                 vTaskDelay(pdMS_TO_TICKS(80));
        set_rgb(0, 0, 0);
        if (i + 1 < flashes) vTaskDelay(pdMS_TO_TICKS(120));
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Inicializar temporizador de actividad
    g_last_activity_ms = now_ms();
    
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
            active_screen = getPersistedSleepScreen();
            g_last_active_screen_before_sleep = active_screen;
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
    
    unsigned long inactivity_time = now_ms() - g_last_activity_ms;
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (g_peripherals_sleeping && wakeup_reason == ESP_SLEEP_WAKEUP_TIMER && !client_connected) {
        Serial.println("[Power] Light Sleep timer wake. Escalating directly to Deep Sleep.");
        saveCurrentScreenForSleep();
        playSleepSignal(3, 255, 0, 0, 900);
        g_ui_overlay_state = UI_OVERLAY_BLACKOUT;
        vTaskDelay(pdMS_TO_TICKS(30));
        set_rgb(0, 0, 0);
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
        esp_deep_sleep_start();
    }

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
            g_ui_overlay_state = UI_OVERLAY_NONE;
        }

        if (inactivity_time >= SLEEP_WARNING_MS && !client_connected && !g_peripherals_sleeping) {
            // ONE-SHOT en t=55s: 2 destellos ámbar + 2 pitidos y overlay ZZZ
            if (!warning_beeps_done) {
                warning_beeps_done = true;
                playSleepSignal(2, 255, 80, 0, 700);
            }

            if (!warning_zzz_done) {
                warning_zzz_done       = true;
                g_sleep_warning_active = true; // Congela la tarea de UI
                g_ui_overlay_state = UI_OVERLAY_SLEEP_WARNING;
            }
        }
    }

    // ---------------------------------------------------------
    // 1. CASO DEEP SLEEP (2 Minutos Y SIN BLE)
    // ---------------------------------------------------------
    if (inactivity_time >= DEEP_SLEEP_TIMEOUT_MS && !client_connected) {
        Serial.println("[Power] 2 min inactivity AND BLE disconnected. Entering Deep Sleep.");

        saveCurrentScreenForSleep();
        playSleepSignal(3, 255, 0, 0, 900);
        g_ui_overlay_state = UI_OVERLAY_BLACKOUT;
        vTaskDelay(pdMS_TO_TICKS(30));
        set_rgb(0, 0, 0);

        // Mantener pull-up del botón activo durante el sleep para evitar falsas activaciones
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        // Despertar al presionar el botón (Nivel LOW)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
        esp_deep_sleep_start();
    }

    // ---------------------------------------------------------
    // 2. CASO LIGHT SLEEP (1 Minuto, SIN BLE — mantiene overlay ZZZ)
    // ---------------------------------------------------------
    if (inactivity_time >= LIGHT_SLEEP_TIMEOUT_MS && !client_connected) {

        // Si no estamos ya en modo ahorro, apagamos los periféricos
        if (!g_peripherals_sleeping) {
            Serial.println("[Power] 1 min inactivity. Entering Light Sleep.");

            saveCurrentScreenForSleep();
            set_rgb(0, 0, 0);

            g_peripherals_sleeping = true;
        }

        // Mantener pull-up del botón activo durante el sleep para evitar falsas activaciones
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        // Entramos en Light Sleep (Ahorro de CPU)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
        if (inactivity_time < DEEP_SLEEP_TIMEOUT_MS) {
            uint64_t remaining_us = (uint64_t)(DEEP_SLEEP_TIMEOUT_MS - inactivity_time) * 1000ULL;
            esp_sleep_enable_timer_wakeup(remaining_us);
        }
        esp_light_sleep_start();

        // ... (Al despertar por el botón, el rotary.cpp ejecutará wakeUpPeripherals()) ...
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); 
}
