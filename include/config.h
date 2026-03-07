#pragma once

#include <Arduino.h>

// --- Intervalo de lectura de sensores ---
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 1000; // 1 segundo

// --- Gestión de energía ---
// IDLE es un modo lógico de reposo con BLE y sensores activos.
constexpr uint32_t IDLE_TIMEOUT_MS = 60000;       // 1 minuto
// DEEP_SLEEP es el único sleep real del ESP32 durante pruebas.
constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 120000; // 2 minutos
constexpr uint16_t IDLE_BEEP_HZ = 700;
constexpr uint16_t DEEP_SLEEP_BEEP_HZ = 900;

// --- Debug: descomenta para activar mensajes de diagnóstico por Serial ---
// #define FIRMWARE_DEBUG

#ifdef FIRMWARE_DEBUG
  #define DPRINT(fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__)
  #define DPRINTLN(msg)      Serial.println(msg)
#else
  #define DPRINT(fmt, ...)   ((void)0)
  #define DPRINTLN(msg)      ((void)0)
#endif
