#include "settings_store.h"

#include <Preferences.h>
#include <cstring>

namespace {

constexpr char PREFS_NAMESPACE[] = "pbit";

class ScopedPrefs {
public:
    explicit ScopedPrefs(bool read_only) {
        prefs.begin(PREFS_NAMESPACE, read_only);
    }

    ~ScopedPrefs() {
        prefs.end();
    }

    Preferences prefs;
};

} // namespace

SoilCalibrationData load_soil_calibration_store(int default_dry, int default_wet) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("soil_dry", default_dry),
        prefs.prefs.getInt("soil_wet", default_wet),
    };
}

void save_soil_calibration_store(int dry_raw, int wet_raw) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("soil_dry", dry_raw);
    prefs.prefs.putInt("soil_wet", wet_raw);
}

SoilThresholdSettings load_soil_threshold_settings(int default_dry, int default_optimal, int default_moist, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("soil_thr_dry", default_dry),
        prefs.prefs.getInt("soil_thr_opt", default_optimal),
        prefs.prefs.getInt("soil_thr_moi", default_moist),
        prefs.prefs.getBool("soil_aen", default_alerts_enabled),
        prefs.prefs.getBool("soil_marks", default_range_marks_visible),
    };
}

void save_soil_threshold_settings(int dry_pct, int optimal_pct, int moist_pct) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("soil_thr_dry", dry_pct);
    prefs.prefs.putInt("soil_thr_opt", optimal_pct);
    prefs.prefs.putInt("soil_thr_moi", moist_pct);
}

void save_soil_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("soil_aen", enabled);
}

void save_soil_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("soil_marks", visible);
}

HumiditySettings load_humidity_settings_store(int default_dry, int default_comfort, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("hum_dry_max", default_dry),
        prefs.prefs.getInt("hum_comf_max", default_comfort),
        prefs.prefs.getBool("hum_alert_en", default_alerts_enabled),
        prefs.prefs.getBool("hum_marks", default_range_marks_visible),
    };
}

void save_humidity_thresholds_store(int dry_max, int comfort_max) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("hum_dry_max", dry_max);
    prefs.prefs.putInt("hum_comf_max", comfort_max);
}

void save_humidity_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("hum_alert_en", enabled);
}

void save_humidity_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("hum_marks", visible);
}

Ds18Settings load_ds18_settings_store(int default_alarm_low, int default_alarm_high, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("d18_alow", default_alarm_low),
        prefs.prefs.getInt("d18_ahigh", default_alarm_high),
        prefs.prefs.getBool("d18_aen", default_alerts_enabled),
        prefs.prefs.getBool("d18_marks", default_range_marks_visible),
    };
}

void save_ds18_settings_store(int alarm_low, int alarm_high) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("d18_alow", alarm_low);
    prefs.prefs.putInt("d18_ahigh", alarm_high);
}

void save_ds18_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("d18_aen", enabled);
}

void save_ds18_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("d18_marks", visible);
}

SoundSettings load_sound_settings_store(int default_quiet_max, int default_normal_max, int default_loud_max, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("snd_quiet", default_quiet_max),
        prefs.prefs.getInt("snd_norm", default_normal_max),
        prefs.prefs.getInt("snd_loud", default_loud_max),
        prefs.prefs.getBool("snd_aen", default_alerts_enabled),
        prefs.prefs.getBool("snd_marks", default_range_marks_visible),
    };
}

void save_sound_settings_store(int quiet_max, int normal_max, int loud_max) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("snd_quiet", quiet_max);
    prefs.prefs.putInt("snd_norm", normal_max);
    prefs.prefs.putInt("snd_loud", loud_max);
}

void save_sound_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("snd_aen", enabled);
}

void save_sound_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("snd_marks", visible);
}

TempSettings load_temp_settings_store(int default_low_alarm, int default_high_alarm, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("tmp_low", default_low_alarm),
        prefs.prefs.getInt("tmp_high", default_high_alarm),
        prefs.prefs.getBool("tmp_aen", default_alerts_enabled),
        prefs.prefs.getBool("tmp_marks", default_range_marks_visible),
    };
}

void save_temp_settings_store(int low_alarm, int high_alarm) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("tmp_low", low_alarm);
    prefs.prefs.putInt("tmp_high", high_alarm);
}

void save_temp_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("tmp_aen", enabled);
}

void save_temp_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("tmp_marks", visible);
}

LightSettings load_light_settings_store(int default_dim_max, int default_indoor_max, int default_bright_max, uint8_t default_display_mode, bool default_alerts_enabled, bool default_range_marks_visible) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getInt("lgt_dim", default_dim_max),
        prefs.prefs.getInt("lgt_ind", default_indoor_max),
        prefs.prefs.getInt("lgt_bri", default_bright_max),
        prefs.prefs.getUChar("lgt_mode", default_display_mode),
        prefs.prefs.getBool("lgt_aen", default_alerts_enabled),
        prefs.prefs.getBool("lgt_marks", default_range_marks_visible),
    };
}

void save_light_thresholds_store(int dim_max, int indoor_max, int bright_max) {
    ScopedPrefs prefs(false);
    prefs.prefs.putInt("lgt_dim", dim_max);
    prefs.prefs.putInt("lgt_ind", indoor_max);
    prefs.prefs.putInt("lgt_bri", bright_max);
}

void save_light_display_mode_store(uint8_t mode) {
    ScopedPrefs prefs(false);
    prefs.prefs.putUChar("lgt_mode", mode);
}

void save_light_alerts_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("lgt_aen", enabled);
}

void save_light_range_marks_visible_store(bool visible) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("lgt_marks", visible);
}

SystemSettings load_system_settings_store(uint32_t default_sleep_timeout_ms, bool default_sound_enabled, bool default_alarm_sound_enabled, bool default_fahrenheit) {
    ScopedPrefs prefs(true);
    return {
        prefs.prefs.getUInt("sys_sleep", default_sleep_timeout_ms),
        prefs.prefs.getBool("sys_sound", default_sound_enabled),
        prefs.prefs.getBool("sys_alarm", default_alarm_sound_enabled),
        prefs.prefs.getBool("sys_unit_f", default_fahrenheit),
    };
}

void save_system_sound_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("sys_sound", enabled);
}

void save_system_alarm_sound_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("sys_alarm", enabled);
}

void save_system_sleep_timeout_store(uint32_t timeout_ms) {
    ScopedPrefs prefs(false);
    prefs.prefs.putUInt("sys_sleep", timeout_ms);
}

void save_system_unit_fahrenheit_store(bool fahrenheit) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("sys_unit_f", fahrenheit);
}

void clear_all_settings_store() {
    ScopedPrefs prefs(false);
    prefs.prefs.clear();
}

// Sensor zone — keys: "sz_sen", "sz_v0".."sz_v5"
uint8_t load_sz_sensor_store() {
    ScopedPrefs prefs(true);
    return prefs.prefs.getUChar("sz_sen", 0);
}

void save_sz_sensor_store(uint8_t sensor_id) {
    ScopedPrefs prefs(false);
    prefs.prefs.putUChar("sz_sen", sensor_id);
}

uint8_t load_sz_viz_store(uint8_t sensor_id) {
    char key[6];
    snprintf(key, sizeof(key), "sz_v%u", (unsigned)sensor_id);
    ScopedPrefs prefs(true);
    return prefs.prefs.getUChar(key, 0);
}

void save_sz_viz_store(uint8_t sensor_id, uint8_t viz_mode) {
    char key[6];
    snprintf(key, sizeof(key), "sz_v%u", (unsigned)sensor_id);
    ScopedPrefs prefs(false);
    prefs.prefs.putUChar(key, viz_mode);
}

uint8_t load_language_store() {
    ScopedPrefs prefs(true);
    return prefs.prefs.getUChar("lang", 0); // 0 = LANG_ES default
}

bool has_language_store() {
    ScopedPrefs prefs(true);
    return prefs.prefs.isKey("lang");
}

void save_language_store(uint8_t lang) {
    ScopedPrefs prefs(false);
    prefs.prefs.putUChar("lang", lang);
}

bool load_ble_enabled_store() {
    ScopedPrefs prefs(true);
    return prefs.prefs.getBool("ble_en", false);
}

void save_ble_enabled_store(bool enabled) {
    ScopedPrefs prefs(false);
    prefs.prefs.putBool("ble_en", enabled);
}

uint32_t load_fw_build_stamp_store() {
    ScopedPrefs prefs(true);
    return prefs.prefs.getUInt("fw_stamp", 0);
}

void save_fw_build_stamp_store(uint32_t stamp) {
    ScopedPrefs prefs(false);
    prefs.prefs.putUInt("fw_stamp", stamp);
}

namespace {

constexpr char PLANT_SETTINGS_KEY[] = "plant_cfg";
// Fixed schema: 4-byte header, six IEEE754 references, RAW calibration pair,
// checksum. One NVS blob prevents a partially saved selection/reference set.
struct PlantSettingsWire {
    uint8_t schema;
    uint8_t profile_version;
    uint8_t mode;
    uint8_t reserved;
    float references[6];
    uint16_t soil_dry;
    uint16_t soil_wet;
    uint32_t checksum;
};
static_assert(sizeof(PlantSettingsWire) == 36, "Planta Lab NVS schema must be 36 bytes");

uint32_t plant_settings_checksum(const PlantSettingsWire& wire) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&wire);
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < sizeof(wire) - sizeof(wire.checksum); ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

PlantSettingsData plant_settings_defaults() {
    return {PLANT_SETTINGS_SCHEMA_VERSION, PlantProfileMode::General,
            plant_pet_general_profile(), 0, 0};
}

bool plant_settings_data_valid(const PlantSettingsData& settings) {
    const bool calibration_valid = settings.soil_dry_raw >= 0
        && settings.soil_dry_raw <= 4095 && settings.soil_wet_raw >= 0
        && settings.soil_wet_raw <= 4095
        && ((settings.soil_dry_raw == 0 && settings.soil_wet_raw == 0)
            || settings.soil_dry_raw - settings.soil_wet_raw >= 300);
    return settings.schema_version == PLANT_SETTINGS_SCHEMA_VERSION
        && (settings.mode == PlantProfileMode::General || settings.mode == PlantProfileMode::Custom)
        && plant_pet_profile_valid(settings.custom_profile) && calibration_valid
        && (settings.mode == PlantProfileMode::General || settings.soil_dry_raw != 0);
}

} // namespace

PlantSettingsData load_plant_settings_store() {
    ScopedPrefs prefs(true);
    if (prefs.prefs.getBytesLength(PLANT_SETTINGS_KEY) != sizeof(PlantSettingsWire)) {
        return plant_settings_defaults();
    }
    PlantSettingsWire wire{};
    if (prefs.prefs.getBytes(PLANT_SETTINGS_KEY, &wire, sizeof(wire)) != sizeof(wire)
        || wire.schema != PLANT_SETTINGS_SCHEMA_VERSION || wire.reserved != 0
        || wire.checksum != plant_settings_checksum(wire)) {
        return plant_settings_defaults();
    }
    PlantSettingsData settings = plant_settings_defaults();
    settings.schema_version = wire.schema;
    settings.mode = static_cast<PlantProfileMode>(wire.mode);
    settings.custom_profile.version = wire.profile_version;
    for (uint8_t i = 0; i < 3; ++i) {
        settings.custom_profile.references[i] = {wire.references[i * 2], wire.references[i * 2 + 1]};
    }
    settings.soil_dry_raw = wire.soil_dry;
    settings.soil_wet_raw = wire.soil_wet;
    return plant_settings_data_valid(settings) ? settings : plant_settings_defaults();
}

bool save_plant_settings_store(const PlantSettingsData& settings) {
    if (!plant_settings_data_valid(settings)) return false;
    PlantSettingsWire wire{};
    wire.schema = settings.schema_version;
    wire.profile_version = settings.custom_profile.version;
    wire.mode = static_cast<uint8_t>(settings.mode);
    for (uint8_t i = 0; i < 3; ++i) {
        wire.references[i * 2] = settings.custom_profile.references[i].low;
        wire.references[i * 2 + 1] = settings.custom_profile.references[i].high;
    }
    wire.soil_dry = static_cast<uint16_t>(settings.soil_dry_raw);
    wire.soil_wet = static_cast<uint16_t>(settings.soil_wet_raw);
    wire.checksum = plant_settings_checksum(wire);
    ScopedPrefs prefs(false);
    return prefs.prefs.putBytes(PLANT_SETTINGS_KEY, &wire, sizeof(wire)) == sizeof(wire);
}

bool reset_plant_settings_store() {
    ScopedPrefs prefs(false);
    return !prefs.prefs.isKey(PLANT_SETTINGS_KEY) || prefs.prefs.remove(PLANT_SETTINGS_KEY);
}
