#pragma once

#include <stdint.h>

// Fixed educational profile for Planta Lab; not a species-specific diagnosis.
enum class PlantChannel : uint8_t { Soil = 0, Temperature = 1, Air = 2, Light = 3, None = 255 };
enum class PlantBand : uint8_t { Unknown, Normal, Low, High };
enum class PlantMood : uint8_t { Content, Thirsty, Soggy, Cold, Hot, AirDry, AirHumid, Dim, Unknown };

struct PlantPetInputs {
    float values[4];
    bool valid[4];
};

struct PlantPetRange {
    float min_value;
    float max_value;
    float opt_low;
    float opt_high;
    bool logarithmic;
};

constexpr float PLANT_PET_LIGHT_MAX = 8000.0f;
const PlantPetRange& plant_pet_range(PlantChannel channel);

struct PlantPetSnapshot {
    PlantMood mood;
    PlantMood previous_mood;
    PlantChannel culprit;
    PlantBand bands[4];
    uint8_t affected_mask;
    uint8_t unknown_mask;
    uint8_t transition; // 0..255: previous_mood -> mood; 255 means settled.
    uint8_t thermal_intensity; // Smoothed 0..255 pose strength, not measured damage.
    uint8_t previous_thermal_intensity; // Frozen strength when the pose changed.
    bool recovering;   // Only while transitioning from an adverse mood to Content.
};

// Call update once per newly copied sensor reading, regardless of the visible
// screen. snapshot is read-only: frames advance transitions, never band evidence.
// A changed band needs >=2 readings spanning 1200 ms, with no gap >3000 ms.
// Invalid data becomes Unknown immediately; startup adopts the current reading.
// Temperature's 15..30 C product band has +/-0.5 C hysteresis after startup.
// Thermal pose intensity eases over 1500 ms; no elapsed time implies plant damage.
// A valid adverse culprit is held for 3000 ms, then another needs a severity
// advantage >0.12 to replace it. Resolving/invalidating the culprit ends the hold.
// Transitions last 1500 ms, or 3000 ms when recovering to Content. No NVS/history.
class PlantPetModel {
public:
    void update(const PlantPetInputs& inputs, uint32_t now_ms);
    PlantPetSnapshot snapshot(uint32_t now_ms) const;

private:
    uint8_t thermal_at(uint32_t now_ms) const;
    bool initialized_ = false;
    PlantBand bands_[4] = { PlantBand::Unknown, PlantBand::Unknown,
                            PlantBand::Unknown, PlantBand::Unknown };
    PlantBand candidates_[4] = { PlantBand::Unknown, PlantBand::Unknown,
                                 PlantBand::Unknown, PlantBand::Unknown };
    uint32_t candidate_since_[4] = { 0, 0, 0, 0 };
    uint8_t candidate_count_[4] = { 0, 0, 0, 0 };
    float severity_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    uint32_t last_update_ms_ = 0;
    uint32_t culprit_since_ms_ = 0;
    uint32_t transition_since_ms_ = 0;
    uint32_t transition_duration_ms_ = 0;
    uint32_t thermal_since_ms_ = 0;
    uint8_t thermal_from_ = 0;
    uint8_t thermal_target_ = 0;
    uint8_t previous_thermal_intensity_ = 0;
    PlantMood mood_ = PlantMood::Unknown;
    PlantMood previous_mood_ = PlantMood::Unknown;
    PlantChannel culprit_ = PlantChannel::None;
};
