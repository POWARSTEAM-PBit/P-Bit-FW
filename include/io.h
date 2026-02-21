#pragma once
#include <Arduino.h>

typedef struct {
    float humidity;
    float temperature; 
    float ldr;
    float mic;

    // --- Sensores Adicionales ---
    float soil_humidity; 
    float temp_ds18b20; 
    // ----------------------------
} Reading;

extern Reading global_readings;
extern volatile bool g_sensor_data_ready;

// --- Funciones de Sensores (Declaraciones) ---
void init_sensors(); 
void read_fast_sensors(Reading &r); 

// 🟢 Prototipos necesarios para io.cpp
void read_slow_sensors(Reading &r); 
void read_dht11(Reading &r); 
void read_ds18b20(Reading &r); 

void sensor_reading_task(void *param); 
// ------------------------------------------