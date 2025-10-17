#pragma once

#include <Arduino.h>

extern QueueHandle_t reading_queue;

void init_queue();