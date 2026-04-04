#include <Arduino.h>
#include <esp_system.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "hw.h"
#include "config.h"
#include "settings_store.h"

// --- Global hardware identity and shared state ---
uint8_t mac[MAC_LEN];
char dev_name[MAX_DEVICE_NAME_LEN];
extern bool g_sound_enabled;

// Shared DS18B20 bus instance.
OneWire oneWire(PIN_TEMP_DS18B20);
DallasTemperature sensors(&oneWire);

constexpr int SOIL_DEFAULT_DRY = 3408;
constexpr int SOIL_DEFAULT_WET = 1904;
constexpr int SOIL_MIN_VALID_DELTA = 300;
constexpr int SOIL_THRESH_DEFAULT_DRY = 20;
constexpr int SOIL_THRESH_DEFAULT_OPTIMAL = 55;
constexpr int SOIL_THRESH_DEFAULT_MOIST = 80;
constexpr bool SOIL_ALERTS_DEFAULT_ENABLED = true;

static int g_soil_cal_dry = SOIL_DEFAULT_DRY;
static int g_soil_cal_wet = SOIL_DEFAULT_WET;
static int g_soil_thresh_dry = SOIL_THRESH_DEFAULT_DRY;
static int g_soil_thresh_optimal = SOIL_THRESH_DEFAULT_OPTIMAL;
static int g_soil_thresh_moist = SOIL_THRESH_DEFAULT_MOIST;
static bool g_soil_alerts_enabled = SOIL_ALERTS_DEFAULT_ENABLED;
constexpr int SOUND_THRESH_DEFAULT_QUIET_MAX = 20;
constexpr int SOUND_THRESH_DEFAULT_NORMAL_MAX = 60;
constexpr int SOUND_THRESH_DEFAULT_LOUD_MAX = 85;
constexpr bool SOUND_ALERTS_DEFAULT_ENABLED = false;
constexpr int TEMP_THRESH_DEFAULT_LOW = 18;
constexpr int TEMP_THRESH_DEFAULT_HIGH = 28;
constexpr bool TEMP_ALERTS_DEFAULT_ENABLED = false;
constexpr int LIGHT_THRESH_DEFAULT_DIM_MAX = 100;
constexpr int LIGHT_THRESH_DEFAULT_INDOOR_MAX = 500;
constexpr int LIGHT_THRESH_DEFAULT_BRIGHT_MAX = 2000;
constexpr uint8_t LIGHT_DISPLAY_MODE_DEFAULT = 0;
constexpr bool LIGHT_ALERTS_DEFAULT_ENABLED = true;
constexpr uint32_t SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS = DEEP_SLEEP_TIMEOUT_MS;
constexpr bool SYSTEM_DEFAULT_SOUND_ENABLED = true;

static int g_sound_thresh_quiet_max = SOUND_THRESH_DEFAULT_QUIET_MAX;
static int g_sound_thresh_normal_max = SOUND_THRESH_DEFAULT_NORMAL_MAX;
static int g_sound_thresh_loud_max = SOUND_THRESH_DEFAULT_LOUD_MAX;
static bool g_sound_alerts_enabled = SOUND_ALERTS_DEFAULT_ENABLED;
static int g_temp_thresh_low = TEMP_THRESH_DEFAULT_LOW;
static int g_temp_thresh_high = TEMP_THRESH_DEFAULT_HIGH;
static bool g_temp_alerts_enabled = TEMP_ALERTS_DEFAULT_ENABLED;
static int g_light_thresh_dim_max = LIGHT_THRESH_DEFAULT_DIM_MAX;
static int g_light_thresh_indoor_max = LIGHT_THRESH_DEFAULT_INDOOR_MAX;
static int g_light_thresh_bright_max = LIGHT_THRESH_DEFAULT_BRIGHT_MAX;
static uint8_t g_light_display_mode = LIGHT_DISPLAY_MODE_DEFAULT;
static bool g_light_alerts_enabled = LIGHT_ALERTS_DEFAULT_ENABLED;
static uint32_t g_system_sleep_timeout_ms = SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS;

static bool is_valid_sleep_timeout(uint32_t timeout_ms) {
    switch (timeout_ms) {
        case 0:
        case 30000:
        case 60000:
        case 120000:
        case 300000:
        case 600000:
            return true;
        default:
            return false;
    }
}

// --- Initialization ---

void init_hw() {
    // 1. Configure the analog sensor inputs.
    pinMode(PIN_SENSOR_SONIDO, INPUT);
    pinMode(PIN_SENSOR_HUMEDAD, INPUT);

    // 2. Force 11dB attenuation on the ADC inputs so the full 0-3.3V range is usable.
    //    Without this the ESP32 may stay around 0dB (~0-0.8V) and clip the signal.
    analogSetPinAttenuation(PIN_SENSOR_SONIDO,   ADC_11db);
    analogSetPinAttenuation(PIN_SENSOR_HUMEDAD,  ADC_11db);
    analogSetPinAttenuation(PIN_LDR_SIGNAL,      ADC_11db);

    // 3. Initialize the DS18B20 bus in the right order:
    //    - set INPUT_PULLUP first so the 1-Wire line is high before scanning,
    //    - wait briefly for the bus to settle,
    //    - then call sensors.begin() so the library scans a stable bus.
    pinMode(PIN_TEMP_DS18B20, INPUT_PULLUP);
    delay(10);
    sensors.begin();
    sensors.setResolution(9);
    load_soil_calibration();
    load_soil_thresholds();
    load_humidity_thresholds();
    load_ds18_settings();
    load_sound_settings();
    load_temp_settings();
    load_light_settings();
    load_system_settings();
    Serial.printf("[DS18B20] init: %d dispositivo(s) en bus\n", sensors.getDeviceCount());
}

void set_devicename() {
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(dev_name, sizeof(dev_name), "PBIT-%02X%02X", mac[4], mac[5]);
}

void load_soil_calibration() {
    SoilCalibrationData stored = load_soil_calibration_store(SOIL_DEFAULT_DRY, SOIL_DEFAULT_WET);
    int dry = stored.dry_raw;
    int wet = stored.wet_raw;

    if (dry > wet && (dry - wet) >= SOIL_MIN_VALID_DELTA) {
        g_soil_cal_dry = dry;
        g_soil_cal_wet = wet;
    } else {
        g_soil_cal_dry = SOIL_DEFAULT_DRY;
        g_soil_cal_wet = SOIL_DEFAULT_WET;
    }
}

bool save_soil_calibration(int dry_raw, int wet_raw) {
    if (dry_raw <= wet_raw || (dry_raw - wet_raw) < SOIL_MIN_VALID_DELTA) {
        return false;
    }

    save_soil_calibration_store(dry_raw, wet_raw);

    g_soil_cal_dry = dry_raw;
    g_soil_cal_wet = wet_raw;
    return true;
}

void load_soil_thresholds() {
    SoilThresholdSettings stored = load_soil_threshold_settings(
        SOIL_THRESH_DEFAULT_DRY,
        SOIL_THRESH_DEFAULT_OPTIMAL,
        SOIL_THRESH_DEFAULT_MOIST,
        SOIL_ALERTS_DEFAULT_ENABLED);
    int dry = stored.dry_pct;
    int optimal = stored.optimal_pct;
    int moist = stored.moist_pct;
    bool alerts_enabled = stored.alerts_enabled;

    if (dry >= 0 && moist <= 100 && dry < optimal && optimal < moist) {
        g_soil_thresh_dry = dry;
        g_soil_thresh_optimal = optimal;
        g_soil_thresh_moist = moist;
    } else {
        g_soil_thresh_dry = SOIL_THRESH_DEFAULT_DRY;
        g_soil_thresh_optimal = SOIL_THRESH_DEFAULT_OPTIMAL;
        g_soil_thresh_moist = SOIL_THRESH_DEFAULT_MOIST;
    }
    g_soil_alerts_enabled = alerts_enabled;
}

bool save_soil_thresholds(int dry_pct, int optimal_pct, int moist_pct) {
    if (dry_pct < 0 || moist_pct > 100 || dry_pct >= optimal_pct || optimal_pct >= moist_pct) {
        return false;
    }

    save_soil_threshold_settings(dry_pct, optimal_pct, moist_pct);

    g_soil_thresh_dry = dry_pct;
    g_soil_thresh_optimal = optimal_pct;
    g_soil_thresh_moist = moist_pct;
    return true;
}

void reset_soil_settings() {
    save_soil_calibration_store(SOIL_DEFAULT_DRY, SOIL_DEFAULT_WET);
    save_soil_threshold_settings(SOIL_THRESH_DEFAULT_DRY, SOIL_THRESH_DEFAULT_OPTIMAL, SOIL_THRESH_DEFAULT_MOIST);
    save_soil_alerts_enabled_store(SOIL_ALERTS_DEFAULT_ENABLED);

    g_soil_cal_dry = SOIL_DEFAULT_DRY;
    g_soil_cal_wet = SOIL_DEFAULT_WET;
    g_soil_thresh_dry = SOIL_THRESH_DEFAULT_DRY;
    g_soil_thresh_optimal = SOIL_THRESH_DEFAULT_OPTIMAL;
    g_soil_thresh_moist = SOIL_THRESH_DEFAULT_MOIST;
    g_soil_alerts_enabled = SOIL_ALERTS_DEFAULT_ENABLED;
}

int get_soil_threshold_dry() {
    return g_soil_thresh_dry;
}

int get_soil_threshold_optimal() {
    return g_soil_thresh_optimal;
}

int get_soil_threshold_moist() {
    return g_soil_thresh_moist;
}

bool get_soil_alerts_enabled() {
    return g_soil_alerts_enabled;
}

void set_soil_alerts_enabled(bool enabled) {
    save_soil_alerts_enabled_store(enabled);
    g_soil_alerts_enabled = enabled;
}

// --- Ambient humidity thresholds and alerts ---

// Default thresholds for ambient humidity.
constexpr int HUM_THRESH_DEFAULT_DRY_MAX = 30;
constexpr int HUM_THRESH_DEFAULT_COMFORT_MAX = 70;
constexpr bool HUM_ALERTS_DEFAULT_ENABLED = true;

// Cached copies of the persisted values.
static int g_hum_thresh_dry_max = HUM_THRESH_DEFAULT_DRY_MAX;
static int g_hum_thresh_comfort_max = HUM_THRESH_DEFAULT_COMFORT_MAX;
static bool g_hum_alerts_enabled = HUM_ALERTS_DEFAULT_ENABLED;

void load_humidity_thresholds() {
    HumiditySettings stored = load_humidity_settings_store(
        HUM_THRESH_DEFAULT_DRY_MAX,
        HUM_THRESH_DEFAULT_COMFORT_MAX,
        HUM_ALERTS_DEFAULT_ENABLED);
    g_hum_thresh_dry_max = stored.dry_max;
    g_hum_thresh_comfort_max = stored.comfort_max;
    g_hum_alerts_enabled = stored.alerts_enabled;
}

bool save_humidity_thresholds(int dry_max, int comfort_max) {
    if (dry_max < 0 || comfort_max > 100 || dry_max >= comfort_max) {
        return false; // Basic range validation.
    }

    save_humidity_thresholds_store(dry_max, comfort_max);

    // Update the in-memory cache.
    g_hum_thresh_dry_max = dry_max;
    g_hum_thresh_comfort_max = comfort_max;
    return true;
}

void reset_humidity_settings() {
    save_humidity_thresholds_store(HUM_THRESH_DEFAULT_DRY_MAX, HUM_THRESH_DEFAULT_COMFORT_MAX);
    save_humidity_alerts_enabled_store(HUM_ALERTS_DEFAULT_ENABLED);

    g_hum_thresh_dry_max = HUM_THRESH_DEFAULT_DRY_MAX;
    g_hum_thresh_comfort_max = HUM_THRESH_DEFAULT_COMFORT_MAX;
    g_hum_alerts_enabled = HUM_ALERTS_DEFAULT_ENABLED;
}

int get_humidity_threshold_dry() {
    return g_hum_thresh_dry_max;
}

int get_humidity_threshold_comfort() {
    return g_hum_thresh_comfort_max;
}

bool get_humidity_alerts_enabled() {
    return g_hum_alerts_enabled;
}

// --- DS18B20 offset and alarms ---

constexpr int DS18_DEFAULT_OFFSET_X10 = 0;
constexpr int DS18_DEFAULT_ALARM_LOW = 0;
constexpr int DS18_DEFAULT_ALARM_HIGH = 40;
constexpr bool DS18_DEFAULT_ALERTS_EN = false;

static int g_ds18_offset_x10 = DS18_DEFAULT_OFFSET_X10;
static int g_ds18_alarm_low = DS18_DEFAULT_ALARM_LOW;
static int g_ds18_alarm_high = DS18_DEFAULT_ALARM_HIGH;
static bool g_ds18_alerts_enabled = DS18_DEFAULT_ALERTS_EN;

void load_ds18_settings() {
    Ds18Settings stored = load_ds18_settings_store(
        DS18_DEFAULT_OFFSET_X10,
        DS18_DEFAULT_ALARM_LOW,
        DS18_DEFAULT_ALARM_HIGH,
        DS18_DEFAULT_ALERTS_EN);
    g_ds18_offset_x10 = stored.offset_x10;
    g_ds18_alarm_low = stored.alarm_low;
    g_ds18_alarm_high = stored.alarm_high;
    g_ds18_alerts_enabled = stored.alerts_enabled;
}

bool save_ds18_settings(int offset_x10, int alarm_low, int alarm_high) {
    if (alarm_low >= alarm_high) return false;
    save_ds18_settings_store(offset_x10, alarm_low, alarm_high);
    g_ds18_offset_x10 = offset_x10;
    g_ds18_alarm_low = alarm_low;
    g_ds18_alarm_high = alarm_high;
    return true;
}

bool save_ds18_settings(float offset, float alarm_low, float alarm_high) {
    return save_ds18_settings((int)lroundf(offset * 10.0f), (int)lroundf(alarm_low), (int)lroundf(alarm_high));
}

void reset_ds18_settings() {
    save_ds18_settings_store(DS18_DEFAULT_OFFSET_X10, DS18_DEFAULT_ALARM_LOW, DS18_DEFAULT_ALARM_HIGH);
    save_ds18_alerts_enabled_store(DS18_DEFAULT_ALERTS_EN);

    g_ds18_offset_x10 = DS18_DEFAULT_OFFSET_X10;
    g_ds18_alarm_low = DS18_DEFAULT_ALARM_LOW;
    g_ds18_alarm_high = DS18_DEFAULT_ALARM_HIGH;
    g_ds18_alerts_enabled = DS18_DEFAULT_ALERTS_EN;
}

int get_ds18_offset_x10() { return g_ds18_offset_x10; }
int get_ds18_alarm_low() { return g_ds18_alarm_low; }
int get_ds18_alarm_high() { return g_ds18_alarm_high; }
float get_ds18_offset() { return (float)g_ds18_offset_x10 / 10.0f; }
float get_ds18_low_alarm() { return (float)g_ds18_alarm_low; }
float get_ds18_high_alarm() { return (float)g_ds18_alarm_high; }
bool get_ds18_alerts_enabled() { return g_ds18_alerts_enabled; }

void set_humidity_alerts_enabled(bool enabled) {
    save_humidity_alerts_enabled_store(enabled);
    g_hum_alerts_enabled = enabled;
}

void set_ds18_alerts_enabled(bool enabled) {
    save_ds18_alerts_enabled_store(enabled);
    g_ds18_alerts_enabled = enabled;
}

void load_sound_settings() {
    SoundSettings stored = load_sound_settings_store(
        SOUND_THRESH_DEFAULT_QUIET_MAX,
        SOUND_THRESH_DEFAULT_NORMAL_MAX,
        SOUND_THRESH_DEFAULT_LOUD_MAX,
        SOUND_ALERTS_DEFAULT_ENABLED);
    int quiet_max = stored.quiet_max;
    int normal_max = stored.normal_max;
    int loud_max = stored.loud_max;
    bool alerts_enabled = stored.alerts_enabled;

    if (quiet_max >= 0 && loud_max <= 100 && quiet_max < normal_max && normal_max < loud_max) {
        g_sound_thresh_quiet_max = quiet_max;
        g_sound_thresh_normal_max = normal_max;
        g_sound_thresh_loud_max = loud_max;
    } else {
        g_sound_thresh_quiet_max = SOUND_THRESH_DEFAULT_QUIET_MAX;
        g_sound_thresh_normal_max = SOUND_THRESH_DEFAULT_NORMAL_MAX;
        g_sound_thresh_loud_max = SOUND_THRESH_DEFAULT_LOUD_MAX;
    }
    g_sound_alerts_enabled = alerts_enabled;
}

bool save_sound_settings(int quiet_max, int normal_max, int loud_max) {
    if (quiet_max < 0 || loud_max > 100 || quiet_max >= normal_max || normal_max >= loud_max) {
        return false;
    }

    save_sound_settings_store(quiet_max, normal_max, loud_max);

    g_sound_thresh_quiet_max = quiet_max;
    g_sound_thresh_normal_max = normal_max;
    g_sound_thresh_loud_max = loud_max;
    return true;
}

void reset_sound_settings() {
    save_sound_settings_store(
        SOUND_THRESH_DEFAULT_QUIET_MAX,
        SOUND_THRESH_DEFAULT_NORMAL_MAX,
        SOUND_THRESH_DEFAULT_LOUD_MAX);
    save_sound_alerts_enabled_store(SOUND_ALERTS_DEFAULT_ENABLED);

    g_sound_thresh_quiet_max = SOUND_THRESH_DEFAULT_QUIET_MAX;
    g_sound_thresh_normal_max = SOUND_THRESH_DEFAULT_NORMAL_MAX;
    g_sound_thresh_loud_max = SOUND_THRESH_DEFAULT_LOUD_MAX;
    g_sound_alerts_enabled = SOUND_ALERTS_DEFAULT_ENABLED;
}

int get_sound_threshold_quiet() { return g_sound_thresh_quiet_max; }
int get_sound_threshold_normal() { return g_sound_thresh_normal_max; }
int get_sound_threshold_loud() { return g_sound_thresh_loud_max; }
bool get_sound_alerts_enabled() { return g_sound_alerts_enabled; }

void set_sound_alerts_enabled(bool enabled) {
    save_sound_alerts_enabled_store(enabled);
    g_sound_alerts_enabled = enabled;
}

void load_temp_settings() {
    TempSettings stored = load_temp_settings_store(
        TEMP_THRESH_DEFAULT_LOW,
        TEMP_THRESH_DEFAULT_HIGH,
        TEMP_ALERTS_DEFAULT_ENABLED);
    int low_alarm = stored.low_alarm;
    int high_alarm = stored.high_alarm;
    bool alerts_enabled = stored.alerts_enabled;

    if (low_alarm < high_alarm) {
        g_temp_thresh_low = low_alarm;
        g_temp_thresh_high = high_alarm;
    } else {
        g_temp_thresh_low = TEMP_THRESH_DEFAULT_LOW;
        g_temp_thresh_high = TEMP_THRESH_DEFAULT_HIGH;
    }
    g_temp_alerts_enabled = alerts_enabled;
}

bool save_temp_settings(int low_alarm, int high_alarm) {
    if (low_alarm >= high_alarm) {
        return false;
    }

    save_temp_settings_store(low_alarm, high_alarm);

    g_temp_thresh_low = low_alarm;
    g_temp_thresh_high = high_alarm;
    return true;
}

void reset_temp_settings() {
    save_temp_settings_store(TEMP_THRESH_DEFAULT_LOW, TEMP_THRESH_DEFAULT_HIGH);
    save_temp_alerts_enabled_store(TEMP_ALERTS_DEFAULT_ENABLED);

    g_temp_thresh_low = TEMP_THRESH_DEFAULT_LOW;
    g_temp_thresh_high = TEMP_THRESH_DEFAULT_HIGH;
    g_temp_alerts_enabled = TEMP_ALERTS_DEFAULT_ENABLED;
}

int get_temp_alarm_low() { return g_temp_thresh_low; }
int get_temp_alarm_high() { return g_temp_thresh_high; }
bool get_temp_alerts_enabled() { return g_temp_alerts_enabled; }

void set_temp_alerts_enabled(bool enabled) {
    save_temp_alerts_enabled_store(enabled);
    g_temp_alerts_enabled = enabled;
}

void load_light_settings() {
    LightSettings stored = load_light_settings_store(
        LIGHT_THRESH_DEFAULT_DIM_MAX,
        LIGHT_THRESH_DEFAULT_INDOOR_MAX,
        LIGHT_THRESH_DEFAULT_BRIGHT_MAX,
        LIGHT_DISPLAY_MODE_DEFAULT,
        LIGHT_ALERTS_DEFAULT_ENABLED);
    int dim_max = stored.dim_max;
    int indoor_max = stored.indoor_max;
    int bright_max = stored.bright_max;
    uint8_t display_mode = stored.display_mode;
    bool alerts_enabled = stored.alerts_enabled;

    if (dim_max >= 10 && bright_max <= 10000 && dim_max < indoor_max && indoor_max < bright_max) {
        g_light_thresh_dim_max = dim_max;
        g_light_thresh_indoor_max = indoor_max;
        g_light_thresh_bright_max = bright_max;
    } else {
        g_light_thresh_dim_max = LIGHT_THRESH_DEFAULT_DIM_MAX;
        g_light_thresh_indoor_max = LIGHT_THRESH_DEFAULT_INDOOR_MAX;
        g_light_thresh_bright_max = LIGHT_THRESH_DEFAULT_BRIGHT_MAX;
    }
    g_light_display_mode = (display_mode <= 2) ? display_mode : LIGHT_DISPLAY_MODE_DEFAULT;
    g_light_alerts_enabled = alerts_enabled;
}

bool save_light_settings(int dim_max, int indoor_max, int bright_max) {
    if (dim_max < 10 || bright_max > 10000 || dim_max >= indoor_max || indoor_max >= bright_max) {
        return false;
    }

    save_light_thresholds_store(dim_max, indoor_max, bright_max);

    g_light_thresh_dim_max = dim_max;
    g_light_thresh_indoor_max = indoor_max;
    g_light_thresh_bright_max = bright_max;
    return true;
}

void reset_light_settings() {
    save_light_thresholds_store(
        LIGHT_THRESH_DEFAULT_DIM_MAX,
        LIGHT_THRESH_DEFAULT_INDOOR_MAX,
        LIGHT_THRESH_DEFAULT_BRIGHT_MAX);
    save_light_display_mode_store(LIGHT_DISPLAY_MODE_DEFAULT);
    save_light_alerts_enabled_store(LIGHT_ALERTS_DEFAULT_ENABLED);

    g_light_thresh_dim_max = LIGHT_THRESH_DEFAULT_DIM_MAX;
    g_light_thresh_indoor_max = LIGHT_THRESH_DEFAULT_INDOOR_MAX;
    g_light_thresh_bright_max = LIGHT_THRESH_DEFAULT_BRIGHT_MAX;
    g_light_display_mode = LIGHT_DISPLAY_MODE_DEFAULT;
    g_light_alerts_enabled = LIGHT_ALERTS_DEFAULT_ENABLED;
}

int get_light_threshold_dim() { return g_light_thresh_dim_max; }
int get_light_threshold_indoor() { return g_light_thresh_indoor_max; }
int get_light_threshold_bright() { return g_light_thresh_bright_max; }
uint8_t get_light_display_mode() { return g_light_display_mode; }

void set_light_display_mode(uint8_t mode) {
    g_light_display_mode = (mode <= 2) ? mode : LIGHT_DISPLAY_MODE_DEFAULT;
    save_light_display_mode_store(g_light_display_mode);
}

bool get_light_alerts_enabled() { return g_light_alerts_enabled; }

void set_light_alerts_enabled(bool enabled) {
    save_light_alerts_enabled_store(enabled);
    g_light_alerts_enabled = enabled;
}

void load_system_settings() {
    SystemSettings stored = load_system_settings_store(
        SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS,
        SYSTEM_DEFAULT_SOUND_ENABLED);
    uint32_t sleep_timeout_ms = stored.sleep_timeout_ms;
    bool sound_enabled = stored.sound_enabled;

    g_system_sleep_timeout_ms = is_valid_sleep_timeout(sleep_timeout_ms)
        ? sleep_timeout_ms
        : SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS;
    g_sound_enabled = sound_enabled;
}

void save_sound_enabled(bool enabled) {
    save_system_sound_enabled_store(enabled);
    g_sound_enabled = enabled;
}

void save_sleep_timeout(uint32_t timeout_ms) {
    if (!is_valid_sleep_timeout(timeout_ms)) {
        timeout_ms = SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS;
    }

    save_system_sleep_timeout_store(timeout_ms);
    g_system_sleep_timeout_ms = timeout_ms;
}

uint32_t get_sleep_timeout() {
    return g_system_sleep_timeout_ms;
}

void reset_all_settings() {
    clear_all_settings_store();

    g_soil_cal_dry = SOIL_DEFAULT_DRY;
    g_soil_cal_wet = SOIL_DEFAULT_WET;
    g_soil_thresh_dry = SOIL_THRESH_DEFAULT_DRY;
    g_soil_thresh_optimal = SOIL_THRESH_DEFAULT_OPTIMAL;
    g_soil_thresh_moist = SOIL_THRESH_DEFAULT_MOIST;
    g_soil_alerts_enabled = SOIL_ALERTS_DEFAULT_ENABLED;
    g_hum_thresh_dry_max = HUM_THRESH_DEFAULT_DRY_MAX;
    g_hum_thresh_comfort_max = HUM_THRESH_DEFAULT_COMFORT_MAX;
    g_hum_alerts_enabled = HUM_ALERTS_DEFAULT_ENABLED;
    g_ds18_offset_x10 = DS18_DEFAULT_OFFSET_X10;
    g_ds18_alarm_low = DS18_DEFAULT_ALARM_LOW;
    g_ds18_alarm_high = DS18_DEFAULT_ALARM_HIGH;
    g_ds18_alerts_enabled = DS18_DEFAULT_ALERTS_EN;
    g_sound_thresh_quiet_max = SOUND_THRESH_DEFAULT_QUIET_MAX;
    g_sound_thresh_normal_max = SOUND_THRESH_DEFAULT_NORMAL_MAX;
    g_sound_thresh_loud_max = SOUND_THRESH_DEFAULT_LOUD_MAX;
    g_sound_alerts_enabled = SOUND_ALERTS_DEFAULT_ENABLED;
    g_temp_thresh_low = TEMP_THRESH_DEFAULT_LOW;
    g_temp_thresh_high = TEMP_THRESH_DEFAULT_HIGH;
    g_temp_alerts_enabled = TEMP_ALERTS_DEFAULT_ENABLED;
    g_light_thresh_dim_max = LIGHT_THRESH_DEFAULT_DIM_MAX;
    g_light_thresh_indoor_max = LIGHT_THRESH_DEFAULT_INDOOR_MAX;
    g_light_thresh_bright_max = LIGHT_THRESH_DEFAULT_BRIGHT_MAX;
    g_light_display_mode = LIGHT_DISPLAY_MODE_DEFAULT;
    g_light_alerts_enabled = LIGHT_ALERTS_DEFAULT_ENABLED;
    g_system_sleep_timeout_ms = SYSTEM_DEFAULT_SLEEP_TIMEOUT_MS;
    g_sound_enabled = SYSTEM_DEFAULT_SOUND_ENABLED;
}



int read_soil_raw_average() {
    const uint8_t SAMPLE_COUNT = 16;
    uint32_t raw_sum = 0;
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
        raw_sum += (uint32_t)analogRead(PIN_SENSOR_HUMEDAD);
        delayMicroseconds(250);
    }
    return (int)(raw_sum / SAMPLE_COUNT);
}

// --- LECTURA DE SENSORES ---

int read_sound_level() {
    // GM19767P: AC-coupled signal centered near 1.65V
    // (inverting LM358 stage, 0-20x gain, RV2 bias).
    // Measure peak-to-peak amplitude over 50 ms to capture more complete audio cycles.
    const uint32_t WINDOW_US = 50000;
    const uint16_t YIELD_EVERY_SAMPLES = 32;
    uint32_t t0 = micros();
    int hi = 0, lo = 4095;
    uint16_t sample_count = 0;
    while ((uint32_t)(micros() - t0) < WINDOW_US) {
        int s = analogRead(PIN_SENSOR_SONIDO);
        if (s > hi) hi = s;
        if (s < lo) lo = s;
        if (++sample_count >= YIELD_EVERY_SAMPLES) {
            sample_count = 0;
            taskYIELD();
        }
    }

    // PEAK_MAX: valor pico a pico que equivale al 100%.
    // With 20x gain, about 45 mV at the mic produces roughly 900 ADC counts of swing.
    // Raise this if the meter saturates too easily, lower it if it never reaches 100%.
    const int PEAK_MAX = 900;
    int raw = constrain(map(hi - lo, 0, PEAK_MAX, 0, 100), 0, 100);

    // EMA (Exponential Moving Average) to suppress random spikes.
    // 0.4 new + 0.6 old: responds in about 3-4 reads (300-400 ms).
    static float ema = 0.0f;
    ema = 0.6f * ema + 0.4f * (float)raw;
    return (int)ema;
}

float read_soil_moisture() {
    // Capacitive Soil Moisture Sensor V2.
    // Inverted logic: wetter soil -> lower voltage -> lower ADC.
    //
    // *** CALIBRATION: adjust with the real values from your sensor:
    //     1. Leave the sensor in dry air  -> note the raw value from Serial -> SOIL_DRY
    //     2. Submerge the sensor in water  -> note the raw value from Serial -> SOIL_WET
    //
    // Measured values on P-Bit (previous revision, 3.3V sensor, 11dB attenuation):
    //   Dry   raw ~= 3408  (~2746 mV, correct for a Capacitive V2 at 3.3V)
    //   Wet   raw ~= 1904  (~1534 mV, 1504-count delta, good dynamic range)
    const uint8_t SAMPLE_COUNT = 12;
    const int DISCONNECT_LOW_THRESHOLD = 80;
    const int DISCONNECT_AVG_THRESHOLD = 1400;

    int raw_min = 4095;
    int raw_max = 0;
    uint32_t raw_sum = 0;
    for (uint8_t i = 0; i < SAMPLE_COUNT; ++i) {
        int raw = analogRead(PIN_SENSOR_HUMEDAD);
        raw_sum += (uint32_t)raw;
        if (raw < raw_min) raw_min = raw;
        if (raw > raw_max) raw_max = raw;
        delayMicroseconds(200);
    }

    int raw_avg = (int)(raw_sum / SAMPLE_COUNT);
    // GPIO35 no tiene pull-up/down interno. Con el sensor desconectado en esta
    // placa, el ADC ha mostrado valores flotantes muy por debajo del rango real
    // del sensor. Usamos un umbral conservador basado en mediciones reales.
    bool likely_disconnected =
        raw_avg <= DISCONNECT_LOW_THRESHOLD ||
        raw_avg < DISCONNECT_AVG_THRESHOLD;

    if (likely_disconnected) {
        return NAN;
    }

    int percent = map(raw_avg, g_soil_cal_dry, g_soil_cal_wet, 0, 100);

    // Moderate EMA (0.80/0.20): with a 1504-count span the noise stays under 1% per count.
    static float ema = -1.0f;
    int clamped = constrain(percent, 0, 100);
    if (ema < 0.0f) ema = (float)clamped;
    ema = 0.80f * ema + 0.20f * (float)clamped;
    return ema;
}

float read_ds18b20_temp() {
    // If sensors.begin() did not detect the sensor at boot (bus still unstable),
    // try scanning again. This covers hot-plugged sensors or a startup that was
    // too fast for the 1-Wire bus to settle.
    if (sensors.getDeviceCount() == 0) {
        sensors.begin();
        if (sensors.getDeviceCount() == 0) {
            DPRINTLN("[DS18B20] Sin dispositivos en bus (GPIO33/J4)");
            return -999.0f;
        }
        sensors.setResolution(9);
        Serial.printf("[DS18B20] Re-detectado: %d dispositivo(s)\n", sensors.getDeviceCount());
    }

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC == DEVICE_DISCONNECTED_C || tempC < -55.0f || tempC > 125.0f) {
        return -999.0f;
    }
    
    return tempC + (g_ds18_offset_x10 / 10.0f);
}
