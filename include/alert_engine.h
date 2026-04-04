#pragma once

#include <Arduino.h>
#include "io.h"

// Shared alert code space used by the central alert engine.
// OFF means alerts are disabled or the sensor is unavailable.
constexpr uint8_t ALERT_CODE_OFF = 0;
constexpr uint8_t ALERT_CODE_LOW = 1;
constexpr uint8_t ALERT_CODE_HIGH = 2;
constexpr uint8_t ALERT_CODE_CRITICAL = 3;
constexpr uint8_t ALERT_CODE_OK = 4;
constexpr uint8_t ALERT_CODE_MOIST = 5;

enum class AlertSensor : uint8_t {
    Temp = 0,
    Humidity,
    Light,
    Sound,
    Soil,
    Ds18,
    Count
};

struct AlertEvent {
    uint8_t code;
    bool changed;
    bool entered;
    bool exited;
};

struct GlobalAlertSummary {
    bool active;
    AlertSensor primary_sensor;
    uint8_t primary_code;
    uint8_t additional_count;
    bool entry_notice_active;
    AlertSensor entry_notice_sensor;
    uint8_t entry_notice_code;
    uint8_t entry_notice_additional_count;
    uint32_t entry_notice_until_ms;
};

uint8_t classify_temp_alert(float temp_c, bool no_sensor, bool alerts_enabled, int low_alarm, int high_alarm);
uint8_t classify_humidity_alert(float humidity, bool no_sensor, bool alerts_enabled, int dry_max, int comfort_max);
uint8_t classify_light_alert(float lux, bool alerts_enabled, int dim_max, int bright_max);
uint8_t classify_sound_alert(float level, bool alerts_enabled, int normal_max, int loud_max);
uint8_t classify_ds18_alert(float temp_c, bool no_sensor, bool alerts_enabled, int low_alarm, int high_alarm);
uint8_t classify_soil_alert(int category_id, bool no_sensor, bool alerts_enabled);

// Refresh the shared alert state from the latest sensor snapshot.
// This runs from the sensor task so alerts no longer depend on screen draws.
void alert_engine_refresh_from_reading(const Reading& reading, bool sound_enabled);

// Return the last known stable alert code for a sensor.
uint8_t alert_engine_get_code(AlertSensor sensor);

// Short labels used by the global badge/toast renderer.
const char* alert_engine_sensor_short_name(AlertSensor sensor);

// Return the current global alert summary.
// The UI/router can use this to draw a single global badge and a short entry toast.
GlobalAlertSummary alert_engine_get_global_summary();

// Convenience helpers for callers that only need a quick check.
bool alert_engine_has_global_alert();
bool alert_engine_has_entry_notice();

AlertEvent alert_engine_update(AlertSensor sensor, uint8_t code, bool screen_changed);
void alert_engine_emit_audio(AlertSensor sensor, const AlertEvent& event, bool sound_enabled);
void alert_engine_reset();
