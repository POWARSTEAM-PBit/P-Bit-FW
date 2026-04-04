#include "alert_engine.h"
#include "hw.h"
#include "led_control.h"

namespace {

portMUX_TYPE g_alert_engine_mux = portMUX_INITIALIZER_UNLOCKED;

constexpr uint32_t ALERT_ENTRY_NOTICE_MS = 1800;
constexpr uint32_t ALERT_AUDIO_COOLDOWN_MS = 1200;
constexpr uint32_t ALERT_SOIL_AUDIO_COOLDOWN_MS = 1800;

uint8_t g_current_alert_code[(size_t)AlertSensor::Count] = {
    ALERT_CODE_OFF,
    ALERT_CODE_OFF,
    ALERT_CODE_OFF,
    ALERT_CODE_OFF,
    ALERT_CODE_OFF,
    ALERT_CODE_OFF
};

GlobalAlertSummary g_global_alert_summary = {
    false,
    AlertSensor::Temp,
    ALERT_CODE_OFF,
    0,
    false,
    AlertSensor::Temp,
    ALERT_CODE_OFF,
    0,
    0
};

uint32_t g_last_audio_ms[(size_t)AlertSensor::Count] = {0};

constexpr ToneStep SOIL_DRY_MELODY[] = {
    {988, 38},
    {0,   12},
    {784, 42},
    {0,   12},
    {659, 54},
};

constexpr ToneStep SOIL_OK_MELODY[] = {
    {784,  34},
    {0,    10},
    {988,  38},
    {0,    10},
    {1318, 52},
};

constexpr ToneStep SOIL_WET_MELODY[] = {
    {698, 42},
    {0,   12},
    {554, 44},
    {0,   12},
    {440, 58},
};

static bool is_active_alert_code(uint8_t code) {
    return code == ALERT_CODE_LOW
        || code == ALERT_CODE_HIGH
        || code == ALERT_CODE_CRITICAL
        || code == ALERT_CODE_MOIST;
}

static uint8_t alert_severity_rank(uint8_t code) {
    switch (code) {
        case ALERT_CODE_CRITICAL: return 3;
        case ALERT_CODE_HIGH: return 2;
        case ALERT_CODE_LOW:
        case ALERT_CODE_MOIST:
            return 1;
        default:
            return 0;
    }
}

static uint8_t alert_sensor_rank(AlertSensor sensor) {
    switch (sensor) {
        case AlertSensor::Soil: return 0;
        case AlertSensor::Temp: return 1;
        case AlertSensor::Humidity: return 2;
        case AlertSensor::Ds18: return 3;
        case AlertSensor::Light: return 4;
        case AlertSensor::Sound: return 5;
        default: return 6;
    }
}

static bool higher_priority(AlertSensor lhs_sensor, uint8_t lhs_code, AlertSensor rhs_sensor, uint8_t rhs_code) {
    uint8_t lhs_severity = alert_severity_rank(lhs_code);
    uint8_t rhs_severity = alert_severity_rank(rhs_code);
    if (lhs_severity != rhs_severity) return lhs_severity > rhs_severity;
    return alert_sensor_rank(lhs_sensor) < alert_sensor_rank(rhs_sensor);
}

static uint32_t audio_cooldown_ms(AlertSensor sensor) {
    switch (sensor) {
        case AlertSensor::Soil:
            return ALERT_SOIL_AUDIO_COOLDOWN_MS;
        case AlertSensor::Sound:
            return 0;
        default:
            return ALERT_AUDIO_COOLDOWN_MS;
    }
}

static bool time_before(uint32_t now_ms, uint32_t until_ms) {
    return (int32_t)(now_ms - until_ms) < 0;
}

static const char* sensor_short_name(AlertSensor sensor) {
    switch (sensor) {
        case AlertSensor::Temp: return "TEMP";
        case AlertSensor::Humidity: return "HUM";
        case AlertSensor::Light: return "LUZ";
        case AlertSensor::Sound: return "SND";
        case AlertSensor::Soil: return "SUE";
        case AlertSensor::Ds18: return "DS18";
        default: return "--";
    }
}

} // namespace

static AlertEvent update_alert_state(AlertSensor sensor, uint8_t code, bool screen_changed) {
    const size_t index = (size_t)sensor;
    portENTER_CRITICAL(&g_alert_engine_mux);
    uint8_t previous = g_current_alert_code[index];

    if (screen_changed) {
        g_current_alert_code[index] = code;
        portEXIT_CRITICAL(&g_alert_engine_mux);
        return {code, false, false, false};
    }

    bool changed = (previous != code);
    bool entered = changed && code != ALERT_CODE_OFF;
    bool exited = changed && previous != ALERT_CODE_OFF && code == ALERT_CODE_OFF;

    g_current_alert_code[index] = code;
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return {code, changed, entered, exited};
}

static GlobalAlertSummary build_global_summary_locked(uint32_t now_ms) {
    GlobalAlertSummary summary = {
        false,
        AlertSensor::Temp,
        ALERT_CODE_OFF,
        0,
        false,
        AlertSensor::Temp,
        ALERT_CODE_OFF,
        0,
        0
    };

    for (size_t i = 0; i < (size_t)AlertSensor::Count; ++i) {
        uint8_t code = g_current_alert_code[i];
        if (!is_active_alert_code(code)) continue;

        AlertSensor sensor = static_cast<AlertSensor>(i);
        if (!summary.active) {
            summary.active = true;
            summary.primary_sensor = sensor;
            summary.primary_code = code;
        } else {
            summary.additional_count++;
            if (higher_priority(sensor, code, summary.primary_sensor, summary.primary_code)) {
                summary.primary_sensor = sensor;
                summary.primary_code = code;
            }
        }
    }

    const GlobalAlertSummary previous = g_global_alert_summary;
    const bool primary_changed =
        summary.active != previous.active ||
        summary.primary_sensor != previous.primary_sensor ||
        summary.primary_code != previous.primary_code;

    if (summary.active) {
        if (primary_changed) {
            summary.entry_notice_active = true;
            summary.entry_notice_sensor = summary.primary_sensor;
            summary.entry_notice_code = summary.primary_code;
            summary.entry_notice_additional_count = summary.additional_count;
            summary.entry_notice_until_ms = now_ms + ALERT_ENTRY_NOTICE_MS;
        } else if (previous.entry_notice_active && time_before(now_ms, previous.entry_notice_until_ms)) {
            summary.entry_notice_active = true;
            summary.entry_notice_sensor = previous.entry_notice_sensor;
            summary.entry_notice_code = previous.entry_notice_code;
            summary.entry_notice_additional_count = previous.entry_notice_additional_count;
            summary.entry_notice_until_ms = previous.entry_notice_until_ms;
        }
    }

    return summary;
}

static void refresh_global_summary_locked(uint32_t now_ms) {
    g_global_alert_summary = build_global_summary_locked(now_ms);
}

static int classify_soil_category(float soil, bool no_sensor) {
    if (no_sensor) return -1;
    if (soil < (float)get_soil_threshold_dry()) return 0;
    if (soil < (float)get_soil_threshold_optimal()) return 1;
    if (soil < (float)get_soil_threshold_moist()) return 2;
    return 3;
}

uint8_t classify_temp_alert(float temp_c, bool no_sensor, bool alerts_enabled, int low_alarm, int high_alarm) {
    if (no_sensor || !alerts_enabled) return ALERT_CODE_OFF;
    if (temp_c <= (float)low_alarm) return ALERT_CODE_LOW;
    if (temp_c >= (float)high_alarm) return ALERT_CODE_HIGH;
    return ALERT_CODE_OK;
}

uint8_t classify_humidity_alert(float humidity, bool no_sensor, bool alerts_enabled, int dry_max, int comfort_max) {
    if (no_sensor || !alerts_enabled) return ALERT_CODE_OFF;
    if (humidity < (float)dry_max) return ALERT_CODE_LOW;
    if (humidity > (float)comfort_max) return ALERT_CODE_HIGH;
    return ALERT_CODE_OK;
}

uint8_t classify_light_alert(float lux, bool alerts_enabled, int dim_max, int bright_max) {
    if (!alerts_enabled) return ALERT_CODE_OFF;
    if (lux < (float)dim_max) return ALERT_CODE_LOW;
    if (lux >= (float)bright_max) return ALERT_CODE_HIGH;
    return ALERT_CODE_OK;
}

uint8_t classify_sound_alert(float level, bool alerts_enabled, int normal_max, int loud_max) {
    if (!alerts_enabled) return ALERT_CODE_OFF;
    if (level >= (float)loud_max) return ALERT_CODE_CRITICAL;
    if (level >= (float)normal_max) return ALERT_CODE_HIGH;
    return ALERT_CODE_OK;
}

uint8_t classify_ds18_alert(float temp_c, bool no_sensor, bool alerts_enabled, int low_alarm, int high_alarm) {
    if (no_sensor || !alerts_enabled) return ALERT_CODE_OFF;
    if (temp_c <= (float)low_alarm) return ALERT_CODE_LOW;
    if (temp_c >= (float)high_alarm) return ALERT_CODE_HIGH;
    return ALERT_CODE_OK;
}

uint8_t classify_soil_alert(int category_id, bool no_sensor, bool alerts_enabled) {
    if (no_sensor || !alerts_enabled) return ALERT_CODE_OFF;
    switch (category_id) {
        case 0: return ALERT_CODE_LOW;
        case 1: return ALERT_CODE_OK;
        case 2: return ALERT_CODE_MOIST;
        case 3: return ALERT_CODE_CRITICAL;
        default: return ALERT_CODE_OFF;
    }
}

void alert_engine_refresh_from_reading(const Reading& reading, bool sound_enabled) {
    uint32_t now_ms = millis();
    const bool temp_no_sensor = isnan(reading.temperature);
    const bool humidity_no_sensor = isnan(reading.humidity);
    const bool light_alerts_enabled = get_light_alerts_enabled();
    const bool sound_alerts_enabled = get_sound_alerts_enabled();
    const bool soil_no_sensor = isnan(reading.soil_humidity);
    const bool soil_alerts_enabled = get_soil_alerts_enabled();
    const bool ds18_no_sensor = reading.temp_ds18b20 < -100.0f;

    const uint8_t codes[(size_t)AlertSensor::Count] = {
        classify_temp_alert(reading.temperature,
                            temp_no_sensor,
                            get_temp_alerts_enabled(),
                            get_temp_alarm_low(),
                            get_temp_alarm_high()),
        classify_humidity_alert(reading.humidity,
                                humidity_no_sensor,
                                get_humidity_alerts_enabled(),
                                get_humidity_threshold_dry(),
                                get_humidity_threshold_comfort()),
        classify_light_alert(reading.ldr,
                             light_alerts_enabled,
                             get_light_threshold_dim(),
                             get_light_threshold_bright()),
        classify_sound_alert(reading.mic,
                             sound_alerts_enabled,
                             get_sound_threshold_quiet(),
                             get_sound_threshold_loud()),
        classify_soil_alert(classify_soil_category(reading.soil_humidity, soil_no_sensor),
                            soil_no_sensor,
                            soil_alerts_enabled),
        classify_ds18_alert(reading.temp_ds18b20,
                            ds18_no_sensor,
                            get_ds18_alerts_enabled(),
                            get_ds18_alarm_low(),
                            get_ds18_alarm_high())
    };

    for (size_t i = 0; i < (size_t)AlertSensor::Count; ++i) {
        AlertSensor sensor = static_cast<AlertSensor>(i);
        AlertEvent event = update_alert_state(sensor, codes[i], false);
        alert_engine_emit_audio(sensor, event, sound_enabled);
    }

    portENTER_CRITICAL(&g_alert_engine_mux);
    refresh_global_summary_locked(now_ms);
    portEXIT_CRITICAL(&g_alert_engine_mux);
}

uint8_t alert_engine_get_code(AlertSensor sensor) {
    const size_t index = (size_t)sensor;
    portENTER_CRITICAL(&g_alert_engine_mux);
    uint8_t code = g_current_alert_code[index];
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return code;
}

const char* alert_engine_sensor_short_name(AlertSensor sensor) {
    return sensor_short_name(sensor);
}

GlobalAlertSummary alert_engine_get_global_summary() {
    portENTER_CRITICAL(&g_alert_engine_mux);
    GlobalAlertSummary summary = g_global_alert_summary;
    const uint32_t now_ms = millis();
    if (summary.entry_notice_active && !time_before(now_ms, summary.entry_notice_until_ms)) {
        summary.entry_notice_active = false;
    }
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return summary;
}

bool alert_engine_has_global_alert() {
    portENTER_CRITICAL(&g_alert_engine_mux);
    bool active = g_global_alert_summary.active;
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return active;
}

bool alert_engine_has_entry_notice() {
    portENTER_CRITICAL(&g_alert_engine_mux);
    const uint32_t now_ms = millis();
    bool active = g_global_alert_summary.entry_notice_active &&
                  time_before(now_ms, g_global_alert_summary.entry_notice_until_ms);
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return active;
}

AlertEvent alert_engine_update(AlertSensor sensor, uint8_t code, bool screen_changed) {
    AlertEvent event = update_alert_state(sensor, code, screen_changed);
    uint32_t now_ms = millis();
    portENTER_CRITICAL(&g_alert_engine_mux);
    refresh_global_summary_locked(now_ms);
    portEXIT_CRITICAL(&g_alert_engine_mux);
    return event;
}

void alert_engine_emit_audio(AlertSensor sensor, const AlertEvent& event, bool sound_enabled) {
    if (!sound_enabled || !event.changed) return;
    if (sensor == AlertSensor::Sound) return;

    const uint32_t now_ms = millis();
    const size_t index = (size_t)sensor;
    const uint32_t cooldown_ms = audio_cooldown_ms(sensor);
    if (cooldown_ms == 0) return;
    if ((uint32_t)(now_ms - g_last_audio_ms[index]) < cooldown_ms) return;
    g_last_audio_ms[index] = now_ms;

    switch (sensor) {
        case AlertSensor::Temp:
            if (event.code == ALERT_CODE_LOW) {
                beep(760, 110);
            } else if (event.code == ALERT_CODE_HIGH) {
                beep(1680, 80);
            }
            break;

        case AlertSensor::Humidity:
            if (event.code == ALERT_CODE_LOW) {
                beep(1700, 90);
            } else if (event.code == ALERT_CODE_HIGH) {
                beep(760, 140);
            }
            break;

        case AlertSensor::Light:
            if (event.code == ALERT_CODE_LOW) {
                beep(860, 70);
            } else if (event.code == ALERT_CODE_HIGH) {
                beep(1560, 55);
            }
            break;

        case AlertSensor::Sound:
            // Keep sound alerts visual-only so the buzzer does not contaminate the microphone.
            break;

        case AlertSensor::Soil:
            if (event.code == ALERT_CODE_LOW) {
                play_tone_sequence(SOIL_DRY_MELODY, sizeof(SOIL_DRY_MELODY) / sizeof(SOIL_DRY_MELODY[0]));
            } else if (event.code == ALERT_CODE_OK) {
                play_tone_sequence(SOIL_OK_MELODY, sizeof(SOIL_OK_MELODY) / sizeof(SOIL_OK_MELODY[0]));
            } else if (event.code == ALERT_CODE_CRITICAL) {
                play_tone_sequence(SOIL_WET_MELODY, sizeof(SOIL_WET_MELODY) / sizeof(SOIL_WET_MELODY[0]));
            }
            break;

        case AlertSensor::Ds18:
            if (event.code == ALERT_CODE_LOW) {
                beep(720, 110);
            } else if (event.code == ALERT_CODE_HIGH) {
                beep(1760, 80);
            }
            break;

        default:
            break;
    }
}

void alert_engine_reset() {
    portENTER_CRITICAL(&g_alert_engine_mux);
    for (size_t i = 0; i < (size_t)AlertSensor::Count; ++i) {
        g_current_alert_code[i] = ALERT_CODE_OFF;
        g_last_audio_ms[i] = 0;
    }
    g_global_alert_summary = {
        false,
        AlertSensor::Temp,
        ALERT_CODE_OFF,
        0,
        false,
        AlertSensor::Temp,
        ALERT_CODE_OFF,
        0,
        0
    };
    portEXIT_CRITICAL(&g_alert_engine_mux);
    stop_beep();
}
