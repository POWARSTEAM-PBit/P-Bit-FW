#pragma once
#include <Arduino.h>

// Identity belongs to acquisition, not to UI snapshots. A retained DHT value
// keeps its last successful timestamp and never supplies new evidence.
enum class ReadingSampleQuality : uint8_t {
    Uninitialized = 0,
    Valid,
    Retained,
    Invalid
};

struct ReadingSampleStamp {
    uint32_t sequence;       // 0: no successful acquisition yet; skips 0 on wrap.
    uint32_t sampled_at_ms;  // millis() at last successful acquisition, may be 0.
    ReadingSampleQuality quality;
};

inline void reading_note_sample(ReadingSampleStamp& stamp, uint32_t now_ms,
                                bool valid = true) {
    if (!valid) {
        stamp.quality = ReadingSampleQuality::Invalid;
        return;
    }
    ++stamp.sequence;
    if (stamp.sequence == 0) stamp.sequence = 1;
    stamp.sampled_at_ms = now_ms;
    stamp.quality = ReadingSampleQuality::Valid;
}

inline void reading_note_retained(ReadingSampleStamp& stamp, bool valid) {
    stamp.quality = valid ? ReadingSampleQuality::Retained
                          : ReadingSampleQuality::Invalid;
}

typedef struct {
    float humidity;
    float temperature; 
    float ldr;
    float ldr_raw;
    float mic;

    // --- Sensores Adicionales ---
    float soil_humidity; 
    float temp_ds18b20; 
    // ----------------------------

    // Planta Lab temporal contract. Copy atomically with the values.
    ReadingSampleStamp soil_sample;
    ReadingSampleStamp temperature_sample;
    ReadingSampleStamp humidity_sample;
    ReadingSampleStamp light_sample;
} Reading;

extern Reading global_readings;
extern volatile bool g_sensor_data_ready;
extern portMUX_TYPE readings_mux;
extern portMUX_TYPE timerMux;

void sensor_reading_task(void *param);
// ------------------------------------------