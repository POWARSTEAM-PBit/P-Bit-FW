#pragma once

#include <stdint.h>

// Educational references for Planta Lab; never a species-specific diagnosis.
enum class PlantChannel : uint8_t { Soil = 0, Temperature = 1, Air = 2, Light = 3, None = 255 };
enum class PlantBand : uint8_t { Unknown, Normal, Low, High };
enum class PlantMood : uint8_t { Content, Thirsty, Soggy, Cold, Hot, AirDry, AirHumid, Dim, Unknown };

struct PlantSampleStamp {
    uint32_t sequence;
    uint32_t sampled_at_ms;
    bool present;
};

struct PlantPetInputs {
    float values[4];
    bool valid[4];
    PlantSampleStamp samples[4];
    // Production always supplies stamps. False is for standalone synthetic
    // readings/tests: each update is a newly acquired sample at now_ms.
    bool has_sample_metadata;
};

struct PlantPetReference {
    float low;
    float high;
};

constexpr uint8_t PLANT_PET_PROFILE_VERSION = 1;
struct PlantPetProfile {
    uint8_t version;
    PlantPetReference references[3]; // Soil %, temperature C, relative air %.
};
const PlantPetProfile& plant_pet_general_profile();
bool plant_pet_profile_valid(const PlantPetProfile& profile);
uint32_t plant_pet_sample_max_age_ms(PlantChannel channel);

struct PlantPetRange {
    float min_value;
    float max_value;
    float opt_low;
    float opt_high;
    bool logarithmic;
};

constexpr float PLANT_PET_LIGHT_MAX = 8000.0f;
const PlantPetRange& plant_pet_range(PlantChannel channel);
constexpr uint8_t PLANT_PET_POSE_COUNT = 9;
constexpr uint16_t PLANT_PET_POSE_WEIGHT_MAX = 65535;

struct PlantPetSnapshot {
    PlantMood mood;
    PlantMood previous_mood;
    PlantChannel culprit;
    PlantBand bands[4];
    uint8_t affected_mask;
    uint8_t attention_mask; // Same as affected_mask; only Soil/Temperature/Air.
    uint8_t reference_mask; // Outside reference, including contextual low light.
    uint8_t unknown_mask;
    uint8_t transition; // 0..255: previous_mood -> mood; 255 means settled.
    uint8_t thermal_intensity; // Smoothed 0..255 pose strength, not measured damage.
    uint8_t previous_thermal_intensity; // Frozen strength when the pose changed.
    uint8_t intensity[4]; // Smoothed relative gesture strength; light always zero.
    // Effective mixture of the authored poses, in PlantMood enum order. The
    // total is 65535. A new transition freezes this mixture, avoiding jumps
    // when recovery is interrupted by another sensor condition.
    uint16_t pose_weights[PLANT_PET_POSE_COUNT];
    uint32_t profile_revision;
    bool low_light;    // Illumination context, not darkness-related damage.
    bool recovering;  // Adverse gesture -> Content or calm low-light rest.
};

// update can receive repeated UI copies: stamps identify actual acquisitions.
// snapshot is read-only: frames advance transitions, never band evidence.
// A changed band needs >=2 distinct samples spanning 1200 ms, no gap >3000 ms.
// Invalid/stale data becomes Unknown immediately; startup adopts fresh data.
// Temperature has +/-0.5 C hysteresis around the active reference after startup.
// Gesture intensity eases over 1500 ms; elapsed time never implies plant damage.
// A valid adverse culprit is held for 3000 ms, then another needs a severity
// advantage >0.12 to replace it. Resolving/invalidating the culprit ends the hold.
// Transitions last 1500 ms, or 3000 ms when recovering to Content. No NVS/history.
class PlantPetModel {
public:
    PlantPetModel();
    // Explicit activation/reconfirmation also clears evidence when references
    // are numerically unchanged (e.g. after reviewing soil calibration).
    bool setProfile(const PlantPetProfile& profile, uint32_t now_ms, bool force_reset = false);
    const PlantPetProfile& profile() const { return profile_; }
    const PlantPetRange& range(PlantChannel channel) const;
    void update(const PlantPetInputs& inputs, uint32_t now_ms);
    PlantPetSnapshot snapshot(uint32_t now_ms) const;

private:
    uint8_t intensity_at(uint8_t channel, uint32_t now_ms) const;
    void pose_at(uint32_t now_ms, uint16_t* weights) const;
    PlantPetProfile profile_;
    PlantPetRange ranges_[4];
    uint32_t profile_revision_ = 0;
    bool initialized_ = false;
    PlantBand bands_[4] = { PlantBand::Unknown, PlantBand::Unknown,
                            PlantBand::Unknown, PlantBand::Unknown };
    PlantBand candidates_[4] = { PlantBand::Unknown, PlantBand::Unknown,
                                 PlantBand::Unknown, PlantBand::Unknown };
    uint32_t candidate_since_[4] = { 0, 0, 0, 0 };
    uint8_t candidate_count_[4] = { 0, 0, 0, 0 };
    float severity_[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    uint32_t last_sequence_[4] = { 0, 0, 0, 0 };
    uint32_t last_sample_ms_[4] = { 0, 0, 0, 0 };
    bool sample_seen_[4] = { false, false, false, false };
    uint32_t culprit_since_ms_ = 0;
    uint32_t transition_since_ms_ = 0;
    uint32_t transition_duration_ms_ = 0;
    uint32_t intensity_since_ms_[4] = { 0, 0, 0, 0 };
    uint8_t intensity_from_[4] = { 0, 0, 0, 0 };
    uint8_t intensity_target_[4] = { 0, 0, 0, 0 };
    uint16_t pose_from_[PLANT_PET_POSE_COUNT] = { 0, 0, 0, 0, 0, 0, 0, 0, PLANT_PET_POSE_WEIGHT_MAX };
    uint8_t previous_thermal_intensity_ = 0;
    PlantMood mood_ = PlantMood::Unknown;
    PlantMood previous_mood_ = PlantMood::Unknown;
    PlantChannel culprit_ = PlantChannel::None;
};
