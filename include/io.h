#pragma once
#include <Arduino.h>

typedef struct {
    float humidity;
    float temperature; 
    float ldr;
    float ldr_raw;
    float mic;

    // --- Sensores Adicionales ---
    float soil_humidity; 
    float temp_ds18b20; 
    // ----------------------------
} Reading;

extern Reading global_readings;
extern volatile bool g_sensor_data_ready;
extern portMUX_TYPE readings_mux;
extern portMUX_TYPE timerMux;

void sensor_reading_task(void *param);
// ------------------------------------------