#pragma once

#include <Arduino.h>

// --- Sensor read interval ---
constexpr uint32_t SENSOR_READ_INTERVAL_MS = 1000; // 1 segundo

// --- Runtime synchronization ---
// Core tasks share a small set of redraw / overlay flags. The runtime events
// helpers own the "set / consume" logic so the behavior stays explicit.

// --- Optional runtime features ---
// Enable the CSV stream used by Arduino Serial Plotter / lab mode.
// Keep this on during classroom experiments and disable it for quieter production runs.
#define PBIT_ENABLE_SERIAL_PLOTTER 1

// --- Power management ---
// IDLE is the product's visible sleep state.
// On this hardware revision we keep the "ZZZ" overlay because automatic
// deep sleep blanks the TFT.
constexpr uint32_t SLEEP_WARNING_MS = 5000;       // 5 segundos
// Keep the same base timeout for configuration compatibility.
constexpr uint32_t DEEP_SLEEP_TIMEOUT_MS = 120000; // 2 minutos
constexpr uint16_t IDLE_BEEP_HZ = 700;
constexpr uint16_t DEEP_SLEEP_BEEP_HZ = 900;

// --- Debug: uncomment to enable Serial diagnostics ---
// #define FIRMWARE_DEBUG

#ifdef FIRMWARE_DEBUG
  #define DPRINT(fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__)
  #define DPRINTLN(msg)      Serial.println(msg)
#else
  #define DPRINT(fmt, ...)   ((void)0)
  #define DPRINTLN(msg)      ((void)0)
#endif
