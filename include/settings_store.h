#pragma once

#include <Arduino.h>

// Typed snapshots loaded from / persisted to NVS.
// The hardware layer stays responsible for validation and in-memory caches,
// while this module only owns raw storage concerns.

struct SoilCalibrationData {
    int dry_raw;
    int wet_raw;
};

struct SoilThresholdSettings {
    int dry_pct;
    int optimal_pct;
    int moist_pct;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct HumiditySettings {
    int dry_max;
    int comfort_max;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct Ds18Settings {
    int alarm_low;
    int alarm_high;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct SoundSettings {
    int quiet_max;
    int normal_max;
    int loud_max;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct TempSettings {
    int low_alarm;
    int high_alarm;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct LightSettings {
    int dim_max;
    int indoor_max;
    int bright_max;
    uint8_t display_mode;
    bool alerts_enabled;
    bool range_marks_visible;
};

struct SystemSettings {
    uint32_t sleep_timeout_ms;
    bool sound_enabled;
    bool alarm_sound_enabled;
    bool fahrenheit;
};

SoilCalibrationData load_soil_calibration_store(int default_dry, int default_wet);
void save_soil_calibration_store(int dry_raw, int wet_raw);

SoilThresholdSettings load_soil_threshold_settings(int default_dry, int default_optimal, int default_moist, bool default_alerts_enabled, bool default_range_marks_visible);
void save_soil_threshold_settings(int dry_pct, int optimal_pct, int moist_pct);
void save_soil_alerts_enabled_store(bool enabled);
void save_soil_range_marks_visible_store(bool visible);

HumiditySettings load_humidity_settings_store(int default_dry, int default_comfort, bool default_alerts_enabled, bool default_range_marks_visible);
void save_humidity_thresholds_store(int dry_max, int comfort_max);
void save_humidity_alerts_enabled_store(bool enabled);
void save_humidity_range_marks_visible_store(bool visible);

Ds18Settings load_ds18_settings_store(int default_alarm_low, int default_alarm_high, bool default_alerts_enabled, bool default_range_marks_visible);
void save_ds18_settings_store(int alarm_low, int alarm_high);
void save_ds18_alerts_enabled_store(bool enabled);
void save_ds18_range_marks_visible_store(bool visible);

SoundSettings load_sound_settings_store(int default_quiet_max, int default_normal_max, int default_loud_max, bool default_alerts_enabled, bool default_range_marks_visible);
void save_sound_settings_store(int quiet_max, int normal_max, int loud_max);
void save_sound_alerts_enabled_store(bool enabled);
void save_sound_range_marks_visible_store(bool visible);

TempSettings load_temp_settings_store(int default_low_alarm, int default_high_alarm, bool default_alerts_enabled, bool default_range_marks_visible);
void save_temp_settings_store(int low_alarm, int high_alarm);
void save_temp_alerts_enabled_store(bool enabled);
void save_temp_range_marks_visible_store(bool visible);

LightSettings load_light_settings_store(int default_dim_max, int default_indoor_max, int default_bright_max, uint8_t default_display_mode, bool default_alerts_enabled, bool default_range_marks_visible);
void save_light_thresholds_store(int dim_max, int indoor_max, int bright_max);
void save_light_display_mode_store(uint8_t mode);
void save_light_alerts_enabled_store(bool enabled);
void save_light_range_marks_visible_store(bool visible);

SystemSettings load_system_settings_store(uint32_t default_sleep_timeout_ms, bool default_sound_enabled, bool default_alarm_sound_enabled, bool default_fahrenheit);
void save_system_sound_enabled_store(bool enabled);
void save_system_alarm_sound_enabled_store(bool enabled);
void save_system_sleep_timeout_store(uint32_t timeout_ms);
void save_system_unit_fahrenheit_store(bool fahrenheit);

void clear_all_settings_store();

// Sensor zone navigation persistence
uint8_t load_sz_sensor_store();
void    save_sz_sensor_store(uint8_t sensor_id);
uint8_t load_sz_viz_store(uint8_t sensor_id);
void    save_sz_viz_store(uint8_t sensor_id, uint8_t viz_mode);

// BLE feature gate — factory-disabled. Cleared on every new firmware image.
bool load_ble_enabled_store();
void save_ble_enabled_store(bool enabled);

// Language — UI display language persisted across reboots.
// Stored as uint8_t (casted Language enum). Returns LANG_ES (0) if key absent.
bool    has_language_store();
uint8_t load_language_store();
void    save_language_store(uint8_t lang);

// Firmware stamp — resets NVS whenever a new binary is flashed.
// Stores/loads a compact hash derived from the running ELF SHA256.
uint32_t load_fw_build_stamp_store();
void     save_fw_build_stamp_store(uint32_t stamp);
