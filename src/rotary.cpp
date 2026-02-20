// rotary.cpp
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include "rotary.h"
#include "tft_display.h" 
#include "timer.h"      
#include "led_control.h"
#include "io.h"         
#include "ui_test.h"    

// DECLARACIONES EXTERNAS REQUERIDAS
extern volatile unsigned long g_last_activity_ms; 
extern bool g_peripherals_sleeping;             
extern bool g_is_fahrenheit;                    
extern volatile bool g_sensor_data_ready;        
extern bool g_sound_enabled; 

// DECLARACIONES EXTERNAS DE LA UI
#include "ui_widgets.h" 
extern TFT_eSPI tft;    


RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC);

/**
 * @brief (Función de Despertar) Enciende los periféricos si estaban dormidos.
 */
void wakeUpPeripherals() {
    if (g_peripherals_sleeping) {
        Serial.println("[Power] Waking up peripherals (TFT/LED On).");
        
        // 🟢 FIX: No usamos TFT_DISPON, solo encendemos la UI
        
        // 1. Marcamos que ya no estamos durmiendo
        g_peripherals_sleeping = false; 
        
        // 2. Forzamos la pantalla principal
        active_screen = TEMP_SCREEN; 
        
        // 🟢 FIX CRÍTICO (Soluciona Título Faltante):
        // Forzamos un redibujo completo (incluyendo estáticos)
        // al decirle a tft_display que los datos "han cambiado"
        // (Aunque esto normalmente solo redibuja datos, 
        // tft_display.cpp lo usará junto con g_peripherals_sleeping=false 
        // para forzar el redibujo).
        g_sensor_data_ready = true; 
        
        // 3. Reiniciamos el timer de actividad
        g_last_activity_ms = millis();
        
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}


/**
 * @brief Callback function for rotary encoder knob turn
 */
void knobCallback(uint8_t value) {
    g_last_activity_ms = millis(); 
    wakeUpPeripherals();           
    
    active_screen = static_cast<Screen>(value);
    Serial.printf("[Rotary] Switched to screen %d\n", active_screen);

    // FIX: Sonido "clic" suave (800Hz, 15ms)
    if (g_sound_enabled) {
        beep(800, 15); 
    }

    // --- Control del LED RGB por pantalla ---
    switch (active_screen) {
        case TEMP_SCREEN:
            set_rgb(255, 69, 0); 
            break;
        case HUMIDITY_SCREEN:
            set_rgb(0, 0, 255); 
            break;
        case LIGHT_SCREEN:
            set_rgb(255, 255, 0); 
            break;
        case SOUND_SCREEN:
            set_rgb(255, 0, 255); 
            break;
        case SOIL_SCREEN:
            set_rgb(180, 80, 0); 
            break;
        case DS18B20_SCREEN:
            set_rgb(255, 255, 255); 
            break;
        case SYSTEM_SCREEN:
            set_rgb(0, 255, 0); 
            break;
        case TIMER_SCREEN:
            if (userTimerRunning) {
                set_rgb(0, 255, 0); 
            } else if (userTimerElapsed > 0) {
                set_rgb(255, 200, 0); 
            } else {
                set_rgb(0, 0, 255); 
            }
            break;
        case TEST_SCREEN:
            set_rgb(100, 100, 100); 
            break;
        default:
            set_rgb(0, 0, 0); 
            break;
    }
}

/**
 * @brief Callback llamado DESPUÉS de SOLTAR el botón.
 */
void buttonCallback(unsigned long duration) {
    g_last_activity_ms = millis(); 
    wakeUpPeripherals();           
    
    // ------------------------------------------------
    // Lógica de Mute en SYSTEM_SCREEN
    // ------------------------------------------------
    if (active_screen == SYSTEM_SCREEN) {
        g_sound_enabled = !g_sound_enabled; // Alternar Mute
        g_sensor_data_ready = true; // Forzar redibujo
        
        if (g_sound_enabled) {
            beep(1200, 70); 
        } else {
            beep(800, 70); 
        }
        return; 
    }

    // ------------------------------------------------
    // LÓGICA DE ALTERNANCIA C/F
    // ------------------------------------------------
    if (active_screen == TEMP_SCREEN || active_screen == TEST_SCREEN || active_screen == DS18B20_SCREEN) {
        g_is_fahrenheit = !g_is_fahrenheit;
        g_sensor_data_ready = true; 
        return; 
    }
    
    // ------------------------------------------------
    // LÓGICA EXISTENTE DE TIMER_SCREEN
    // ------------------------------------------------
    
    // Solo suena si el sonido está habilitado
    if (g_sound_enabled) {
        beep(2000, 100); // Tono de acción del Timer
    }

    if (active_screen == TIMER_SCREEN) {
        
        if (duration > 1000) { // Pulsación larga = Reset
            resetUserTimer(); 
            
            set_rgb(255, 0, 255); 
            delay(150);          
            set_rgb(0, 0, 255);  
        
        } else { // Pulsación corta = Start/Stop
            if (!userTimerRunning) {
                startUserTimer();
                set_rgb(0, 255, 0); 
            } else {
                stopUserTimer();
                set_rgb(255, 200, 0); 
            }
        }
    }
}

void init_rotary() {
    rotaryEncoder.setEncoderType(EncoderType::FLOATING);
    
    // FIX: Los límites ahora empiezan en 1 (TEMP_SCREEN) y excluyen BOOT_SCREEN
    rotaryEncoder.setBoundaries(TEMP_SCREEN, TEST_SCREEN, true); 
    
    rotaryEncoder.onTurned(&knobCallback);
    rotaryEncoder.onPressed(&buttonCallback); 
    
    rotaryEncoder.begin();
    Serial.println("[Rotary] Initialized.");
}