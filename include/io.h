#pragma once

#include <Arduino.h>

typedef struct {
    float humidity;
    float temperature;
    float ldr;
    float mic;
    float batt;
} Reading;

/**
 * @brief Read sensor data and populate the Reading struct
 */
void read_sensors(Reading &r);