#pragma once

#include <Arduino.h>

typedef struct {
    float humidity;
    float temperature;
    float ldr;
    float mic;
    float batt;
} Reading;

void read_sensors(Reading &r);