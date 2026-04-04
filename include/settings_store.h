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
};

struct HumiditySettings {
    int dry_max;
    int comfort_max;
    bool alerts_enabled;
};

struct Ds18Settings {
    int offset_x10;
    int alarm_low;
    int alarm_high;
    bool alerts_enabled;
};

struct SoundSettings {
    int quiet_max;
    int normal_max;
    int loud_max;
    bool alerts_enabled;
};

struct TempSettings {
    int low_alarm;
    int high_alarm;
    bool alerts_enabled;
};

struct LightSettings {
    int dim_max;
    int indoor_max;
    int bright_max;
    uint8_t display_mode;
    bool alerts_enabled;
};

struct SystemSettings {
    uint32_t sleep_timeout_ms;
    bool sound_enabled;
};

SoilCalibrationData load_soil_calibration_store(int default_dry, int default_wet);
void save_soil_calibration_store(int dry_raw, int wet_raw);

SoilThresholdSettings load_soil_threshold_settings(int default_dry, int default_optimal, int default_moist, bool default_alerts_enabled);
void save_soil_threshold_settings(int dry_pct, int optimal_pct, int moist_pct);
void save_soil_alerts_enabled_store(bool enabled);

HumiditySettings load_humidity_settings_store(int default_dry, int default_comfort, bool default_alerts_enabled);
void save_humidity_thresholds_store(int dry_max, int comfort_max);
void save_humidity_alerts_enabled_store(bool enabled);

Ds18Settings load_ds18_settings_store(int default_offset_x10, int default_alarm_low, int default_alarm_high, bool default_alerts_enabled);
void save_ds18_settings_store(int offset_x10, int alarm_low, int alarm_high);
void save_ds18_alerts_enabled_store(bool enabled);

SoundSettings load_sound_settings_store(int default_quiet_max, int default_normal_max, int default_loud_max, bool default_alerts_enabled);
void save_sound_settings_store(int quiet_max, int normal_max, int loud_max);
void save_sound_alerts_enabled_store(bool enabled);

TempSettings load_temp_settings_store(int default_low_alarm, int default_high_alarm, bool default_alerts_enabled);
void save_temp_settings_store(int low_alarm, int high_alarm);
void save_temp_alerts_enabled_store(bool enabled);

LightSettings load_light_settings_store(int default_dim_max, int default_indoor_max, int default_bright_max, uint8_t default_display_mode, bool default_alerts_enabled);
void save_light_thresholds_store(int dim_max, int indoor_max, int bright_max);
void save_light_display_mode_store(uint8_t mode);
void save_light_alerts_enabled_store(bool enabled);

SystemSettings load_system_settings_store(uint32_t default_sleep_timeout_ms, bool default_sound_enabled);
void save_system_sound_enabled_store(bool enabled);
void save_system_sleep_timeout_store(uint32_t timeout_ms);

void clear_all_settings_store();
