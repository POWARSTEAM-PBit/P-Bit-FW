// rotary.cpp
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include <Preferences.h>
#include <esp_system.h>
#include <esp_timer.h>
#include "rotary.h"
#include "config.h"
#include "tft_display.h"
#include "timer.h"
#include "led_control.h"
#include "io.h"

// DECLARACIONES EXTERNAS REQUERIDAS
extern volatile unsigned long g_last_activity_ms;
extern bool g_is_fahrenheit;
extern volatile bool g_sensor_data_ready;
extern bool g_sound_enabled;

static bool isRestorableScreen(Screen screen) {
    return screen >= TEMP_SCREEN && screen <= TIMER_SCREEN;
}

static unsigned long now_ms() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}


RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC);

/**
 * @brief Sale del modo IDLE y restaura el contexto visible de la app.
 */
static void exitIdleModeIfNeeded() {
    if (g_power_mode == POWER_IDLE) {
        Serial.println("[Power] Leaving IDLE mode.");

        g_power_mode = POWER_ACTIVE;
        g_ui_overlay_state = UI_OVERLAY_NONE;

        if (isRestorableScreen((Screen)g_last_active_screen_before_sleep)) {
            active_screen = (Screen)g_last_active_screen_before_sleep;
        } else {
            active_screen = TEMP_SCREEN;
        }

        g_sensor_data_ready = true;
        g_ui_force_full_redraw = true;
        g_last_activity_ms = now_ms();
    }
}


/**
 * @brief Callback function for rotary encoder knob turn
 */
void knobCallback(uint8_t value) {
    g_last_activity_ms = now_ms(); 
    exitIdleModeIfNeeded();
    
    if (value < TEMP_SCREEN || value > TIMER_SCREEN) return;
    active_screen = static_cast<Screen>(value);
    DPRINT("[Rotary] Switched to screen %d\n", (int)active_screen);

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
            set_rgb(0, 0, 0); // LED apagado: evita incidencia en el LDR
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
        default:
            set_rgb(0, 0, 0); 
            break;
    }
}

/**
 * @brief Callback llamado DESPUÉS de SOLTAR el botón.
 */
void buttonCallback(unsigned long duration) {
    g_last_activity_ms = now_ms(); 
    exitIdleModeIfNeeded();
    
    // ------------------------------------------------
    // Lógica de Mute / Cambio de idioma en SYSTEM_SCREEN
    // ------------------------------------------------
    if (active_screen == SYSTEM_SCREEN) {
        if (duration > 2000) {
            // Pulsación larga (>2s): borrar preferencia de idioma y reiniciar
            Preferences prefs;
            prefs.begin("pbit", false);
            prefs.remove("lang");
            prefs.end();
            g_ui_overlay_state = UI_OVERLAY_RESTARTING;
            vTaskDelay(pdMS_TO_TICKS(1500));
            esp_restart();
        } else {
            // Pulsación corta: alternar mute
            g_sound_enabled = !g_sound_enabled;
            g_sensor_data_ready = true;
            if (g_sound_enabled) {
                beep(1200, 70);
            } else {
                beep(800, 70);
            }
        }
        return;
    }

    // ------------------------------------------------
    // LÓGICA DE ALTERNANCIA C/F
    // ------------------------------------------------
    if (active_screen == TEMP_SCREEN || active_screen == DS18B20_SCREEN) {
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
            vTaskDelay(pdMS_TO_TICKS(150));
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
    rotaryEncoder.setBoundaries(TEMP_SCREEN, TIMER_SCREEN, true);
    
    rotaryEncoder.onTurned(&knobCallback);
    rotaryEncoder.onPressed(&buttonCallback); 
    
    rotaryEncoder.begin();
    Serial.println("[Rotary] Initialized.");
}
