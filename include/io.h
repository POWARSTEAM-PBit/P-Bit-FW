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
 * @brief Initialize sensor interfaces.
 */
void init_io();

/**
 * @brief Read sensor data and populate the Reading struct
 */
void read_sensors(Reading &r);
/**
 * @brief Get a cached sensor reading, refreshing if stale or when forced.
 */
void get_sensor_reading(Reading &r, bool forceRefresh = false);
