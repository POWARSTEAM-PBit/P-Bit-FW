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

#define SERIAL_BAUD_RATE 115200

// --- DEFINICIONES DE VARIABLES GLOBALES ---
bool g_is_fahrenheit = false; // Estado C/F
bool g_sound_enabled = true; // El sonido está activado por defecto

// Variables de gestión de energía
volatile unsigned long g_last_activity_ms = 0; 
bool g_peripherals_sleeping = false; 
const unsigned long LIGHT_SLEEP_TIMEOUT_MS = 60000; // 1 Minuto
const unsigned long DEEP_SLEEP_TIMEOUT_MS = 600000; // 10 Minutos

// --- DECLARACIONES EXTERNAS ---
extern bool client_connected; // Desde ble.cpp
// IMPORTANTE: Accedemos a los datos globales para no bloquear el loop con lecturas lentas
extern Reading global_readings; 

// Variable para el timer de debug en el loop
static unsigned long last_debug_time = 0; 

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Inicializar temporizador de actividad
    g_last_activity_ms = millis();
    
    // Inicialización de módulos
    set_devicename();
    init_tft_display();
    init_ble();
    init_rotary();
    init_leds_and_buzzer();
    
    // 🛑 CORRECCIÓN CRÍTICA: Usamos init_hw() en lugar de init_sensors()
    // Esto inicializa los pines correctos definidos en hw.cpp y evita el error "undefined reference"
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
            g_peripherals_sleeping = false; 
            active_screen = TEMP_SCREEN;
            break;
            
        default : // Encendido normal (Power On)
            Serial.println("[Power] Cold Boot detected. Running boot sequence.");
            run_boot_sequence(); 
            active_screen = TEMP_SCREEN; 
            break;
    }
    // ---------------------------------------------------------


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
    // 0. DEBUG SENSORS (Imprimir cada 2 segundos si no está durmiendo)
    // ---------------------------------------------------------
    if (!g_peripherals_sleeping && (millis() - last_debug_time > 2000)) {
        last_debug_time = millis();
        
        Serial.println("\n--- [PRUEBA DE SENSORES (RAM)] ---");
        
        // 🟢 OPTIMIZACIÓN: Leemos de la estructura global (RAM) en lugar del sensor físico.
        // Esto evita que el loop se congele esperando al DS18B20 y los pitidos suenen lentos.
        
        // Temperatura
        float t = global_readings.temp_ds18b20;
        Serial.print("Temp DS18B20: "); 
        if(t == -999.0) Serial.println("ERROR / DESCONECTADO");
        else { Serial.print(t); Serial.println(" C"); }

        // Humedad Suelo
        Serial.print("Humedad Suelo: "); 
        Serial.print(global_readings.soil_humidity); 
        Serial.println("%");

        // Sonido
        Serial.print("Nivel Sonido:  "); 
        Serial.print(global_readings.mic); 
        Serial.println("%");
        
        Serial.println("----------------------------");
    }

    // ---------------------------------------------------------
    // 1. CASO DEEP SLEEP (10 Minutos Y SIN BLE)
    // ---------------------------------------------------------
    if (inactivity_time >= DEEP_SLEEP_TIMEOUT_MS && !client_connected) {
        Serial.println("[Power] 10 min inactivity AND BLE disconnected. Entering Deep Sleep.");
        
        tft.fillScreen(TFT_BLACK); 
        tft.writecommand(TFT_DISPOFF); 
        set_rgb(0, 0, 0); 

        // Despertar al presionar el botón (Nivel LOW)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW); 
        esp_deep_sleep_start();
    }
    
    // ---------------------------------------------------------
    // 2. CASO LIGHT SLEEP (1 Minuto, CUALQUIER ESTADO BLE)
    // ---------------------------------------------------------
    if (inactivity_time >= LIGHT_SLEEP_TIMEOUT_MS) {
        
        // Si no estamos ya en modo ahorro, apagamos los periféricos
        if (!g_peripherals_sleeping) { 
            Serial.println("[Power] 1 min inactivity. Entering Peripheral Sleep (TFT Black).");
            
            tft.fillScreen(TFT_BLACK); 
            set_rgb(0, 0, 0); 

            g_peripherals_sleeping = true;
        }

        // Entramos en Light Sleep (Ahorro de CPU)
        esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW); 
        esp_light_sleep_start();
        
        // ... (Al despertar por el botón, el rotary.cpp ejecutará wakeUpPeripherals()) ...
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); 
}