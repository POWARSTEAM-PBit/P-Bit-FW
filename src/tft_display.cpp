// tft_display.cpp
// GESTOR DE TAREAS DE UI (UI ROUTER)

#include "tft_display.h"
#include "config.h"     
#include "ui_widgets.h" 
#include "timer.h"      

// --- INCLUSIONES DE TODOS LOS MÓDULOS DE PANTALLA ---
#include "ui_boot.h"
#include "ui_temp.h"
#include "ui_humidity.h"
#include "ui_light.h"
#include "ui_sound.h"
#include "ui_soil.h"
#include "ui_ds18.h"
#include "ui_system.h"
#include "ui_timer.h"
// ----------------------------------------------------

// --- VARIABLES GLOBALES DE TFT ---
Screen active_screen;

// --- DECLARACIONES EXTERNAS ---
extern volatile bool g_sensor_data_ready;
extern bool userTimerRunning;
extern volatile bool g_timer_just_reset;
extern volatile bool g_peripherals_sleeping;
extern volatile bool g_sleep_warning_active; // Congela el dibujo durante el aviso pre-sleep


// --- FUNCIONES DE INICIALIZACIÓN Y LIMPIEZA ---

void init_tft_display() {
    tft.init();
    tft.setRotation(1); // Paisaje
    tft.fillScreen(TFT_BLACK); 
}

// --- TAREA PRINCIPAL DE PANTALLA (FreeRTOS) ---

void switch_screen(void *param) {
    Serial.println("[Display] Tarea de Pantalla (Router) iniciada en Núcleo 1.");
    
    bool screen_changed = true; 
    Screen last_drawn = BOOT_SCREEN; 
    
    unsigned long last_timer_update_ms = 0;
    
    static uint16_t last_drawn_timer_state = 0; 
    
    // 🟢 FIX: Variable para rastrear el estado de sleep anterior
    bool last_sleep_state = g_peripherals_sleeping;
    
    while (1) {

        // Detectar si acabamos de despertar — evaluado ANTES del guard de sleep_warning
        // para no perder la señal si g_sleep_warning_active sigue activo un ciclo más tras el wake.
        bool just_woke_up = (last_sleep_state == true) && (g_peripherals_sleeping == false);

        // Si el aviso pre-sleep está activo, ceder CPU sin dibujar nada.
        // last_sleep_state NO se actualiza aquí para no perder la señal de wake.
        if (g_sleep_warning_active) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // Solo actualizar last_sleep_state cuando realmente podemos procesar el evento
        last_sleep_state = g_peripherals_sleeping;

        bool sensor_data_changed = g_sensor_data_ready;
        bool timer_needs_update = false;
        
        
        // El cronómetro se actualiza a 100Hz (10ms) para fluidez.
        if (millis() - last_timer_update_ms >= 10) {
            timer_needs_update = true;
            last_timer_update_ms = millis();
        }

        if (last_drawn != active_screen) {
            screen_changed = true;
            if (active_screen != BOOT_SCREEN) sensor_data_changed = true; 
        } else {
            screen_changed = false;
        }

        if (g_timer_just_reset) timer_needs_update = true;

        // ------------------------------------------------------------------
        // LÓGICA DE DIBUJO CENTRAL
        // ------------------------------------------------------------------
        
        // 🟢 FIX: Forzar redibujo completo si acabamos de despertar
        if (screen_changed || sensor_data_changed || just_woke_up || (active_screen == TIMER_SCREEN && (timer_needs_update || g_timer_just_reset))) {
            
            // Si acabamos de despertar, forzamos 'screen_changed' para redibujar estáticos
            if (just_woke_up) {
                screen_changed = true; 
            }
            
            if (screen_changed) {
                last_drawn = active_screen;
            }
            
            // --- ENRUTADOR DE UI ---
            switch (active_screen) {
                
                case BOOT_SCREEN:
                    // (Esta pantalla solo se ejecuta en el setup)
                    break; 

                case TEMP_SCREEN: 
                    draw_temp_screen(screen_changed, sensor_data_changed); 
                    break;
                
                case HUMIDITY_SCREEN: 
                    draw_humidity_screen(screen_changed, sensor_data_changed); 
                    break;

                case LIGHT_SCREEN: 
                    draw_light_screen(screen_changed, sensor_data_changed); 
                    break;

                case SOUND_SCREEN:
                    draw_sound_screen(screen_changed, sensor_data_changed);
                    break;

                case SOIL_SCREEN:
                    draw_soil_screen(screen_changed, sensor_data_changed);
                    break;

                case DS18B20_SCREEN:
                    draw_ds18_screen(screen_changed, sensor_data_changed);
                    break;

                case SYSTEM_SCREEN: 
                    draw_system_screen(screen_changed, sensor_data_changed);
                    break;
                
                case TIMER_SCREEN: 
                    draw_timer_screen(screen_changed, sensor_data_changed, timer_needs_update);
                    break;
                    
            } // fin del switch
            
            if (g_timer_just_reset) g_timer_just_reset = false;
            if (sensor_data_changed) g_sensor_data_ready = false;

        } // fin del if(screen_changed...)

#ifdef FIRMWARE_DEBUG
        static bool _hwm_reported = false;
        if (!_hwm_reported) { _hwm_reported = true; DPRINT("[Stack] DisplayTask HWM: %u words\n", uxTaskGetStackHighWaterMark(NULL)); }
#endif
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}