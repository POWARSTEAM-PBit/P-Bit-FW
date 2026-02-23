#pragma once

#include <Arduino.h>

// --- Intervalo de lectura de sensores ---
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 1000; // 1 segundo

// --- Debug: descomenta para activar mensajes de diagnóstico por Serial ---
// #define FIRMWARE_DEBUG

#ifdef FIRMWARE_DEBUG
  #define DPRINT(fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__)
  #define DPRINTLN(msg)      Serial.println(msg)
#else
  #define DPRINT(fmt, ...)   ((void)0)
  #define DPRINTLN(msg)      ((void)0)
#endif