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

void sensor_reading_task(void *param);
// ------------------------------------------