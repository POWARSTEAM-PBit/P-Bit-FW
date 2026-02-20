#pragma once
#include <Arduino.h>

// --- ENUM DE PANTALLAS ---
// (Asegúrate de que este enum esté aquí o en un config.h global)
enum Screen {
    BOOT_SCREEN,
    TEMP_SCREEN,
    HUMIDITY_SCREEN,
    LIGHT_SCREEN,
    SOUND_SCREEN,
    SOIL_SCREEN,
    DS18B20_SCREEN,
    SYSTEM_SCREEN,
    TIMER_SCREEN,
    TEST_SCREEN 
};

extern Screen active_screen;

// --- TAREA PRINCIPAL DE PANTALLA ---
void init_tft_display();
void switch_screen(void *param);
void clear_screen();