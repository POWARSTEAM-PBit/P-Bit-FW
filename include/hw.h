#pragma once

#include <Arduino.h>

constexpr size_t MAC_LEN = 6;
constexpr size_t MAX_DEVICE_NAME_LEN = 20;

// --- Pin configuration ---

#define PIN_LDR_SIGNAL     39   // LDR (SENSOR_VN)
#define PIN_SENSOR_SONIDO  36   // Microphone (SENSOR_VP)
#define PIN_SENSOR_HUMEDAD 35   // Soil moisture sensor (external J6)
#define PIN_TEMP_DS18B20   33   // DS18B20 (external J3)
#define PIN_DHT            4    // DHT11

/**
 * @brief MAC address of the device
 */
extern uint8_t mac[MAC_LEN];

/**
 * @brief Device name based on MAC address
 */
extern char dev_name[MAX_DEVICE_NAME_LEN];

// --- Function declarations ---

void set_devicename();

/**
 * Initialize hardware pins and shared sensor drivers.
 */
void init_hw(); 

// --- Sensor helpers and persistence ---

/**
 * Load soil calibration from NVS, or fall back to defaults.
 */
void load_soil_calibration();

/**
 * Save a new soil calibration into NVS.
 */
bool save_soil_calibration(int dry_raw, int wet_raw);

/**
 * Load the soil percentage thresholds from NVS.
 */
void load_soil_thresholds();

/**
 * Save the soil percentage thresholds into NVS.
 */
bool save_soil_thresholds(int dry_pct, int optimal_pct, int moist_pct);
void reset_soil_settings();

int get_soil_threshold_dry();
int get_soil_threshold_optimal();
int get_soil_threshold_moist();
bool get_soil_alerts_enabled();
void set_soil_alerts_enabled(bool enabled);

// --- Ambient humidity thresholds and alerts ---
void load_humidity_thresholds();
bool save_humidity_thresholds(int dry_max, int comfort_max);
void reset_humidity_settings();
int get_humidity_threshold_dry();
int get_humidity_threshold_comfort();
bool get_humidity_alerts_enabled();
void set_humidity_alerts_enabled(bool enabled);

// --- DS18B20 thresholds and alerts ---
void load_ds18_settings();
bool save_ds18_settings(int offset_x10, int alarm_low, int alarm_high);
bool save_ds18_settings(float offset, float low_alarm, float high_alarm);
void reset_ds18_settings();
int get_ds18_offset_x10();
int get_ds18_alarm_low();
int get_ds18_alarm_high();
float get_ds18_offset();
float get_ds18_low_alarm();
float get_ds18_high_alarm();
bool get_ds18_alerts_enabled();
void set_ds18_alerts_enabled(bool enabled);

// --- Sound thresholds and alerts ---
void load_sound_settings();
bool save_sound_settings(int quiet_max, int normal_max, int loud_max);
void reset_sound_settings();
int get_sound_threshold_quiet();
int get_sound_threshold_normal();
int get_sound_threshold_loud();
bool get_sound_alerts_enabled();
void set_sound_alerts_enabled(bool enabled);

// --- DHT temperature thresholds and alerts ---
void load_temp_settings();
bool save_temp_settings(int low_alarm, int high_alarm);
void reset_temp_settings();
int get_temp_alarm_low();
int get_temp_alarm_high();
bool get_temp_alerts_enabled();
void set_temp_alerts_enabled(bool enabled);

// --- Light thresholds and display modes ---
void load_light_settings();
bool save_light_settings(int dim_max, int indoor_max, int bright_max);
void reset_light_settings();
int get_light_threshold_dim();
int get_light_threshold_indoor();
int get_light_threshold_bright();
uint8_t get_light_display_mode();
void set_light_display_mode(uint8_t mode);
bool get_light_alerts_enabled();
void set_light_alerts_enabled(bool enabled);

// --- Global system settings ---
void load_system_settings();
void save_sound_enabled(bool enabled);
void save_sleep_timeout(uint32_t timeout_ms);
uint32_t get_sleep_timeout();
void reset_all_settings();






/**
 * Return a stable raw average from the soil sensor ADC.
 */
int read_soil_raw_average();

/**
 * Read the sound level and return a percentage (0-100).
 */
int read_sound_level();

/**
 * Read calibrated soil moisture as a percentage (0-100).
 * Returns NAN if the pin appears floating and the sensor is likely disconnected.
 */
float read_soil_moisture();

/**
 * Read temperature in Celsius from the DS18B20 sensor.
 */
float read_ds18b20_temp();
