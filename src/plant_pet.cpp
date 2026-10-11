#include "plant_pet.h"

#include <cmath>

namespace {
constexpr uint32_t kBandConfirmMs = 1200;
constexpr uint32_t kEvidenceMaxGapMs = 3000;
constexpr uint32_t kCulpritHoldMs = 3000;
constexpr uint32_t kTransitionMs = 1500;
constexpr uint32_t kRecoveryMs = 3000;
constexpr float kCulpritAdvantage = 0.12f;

const PlantPetRange kRanges[4] = {
    { 0.0f, 100.0f, 30.0f, 70.0f, false },
    { 0.0f, 40.0f, 15.0f, 30.0f, false },
    { 0.0f, 100.0f, 40.0f, 70.0f, false },
    { 0.0f, PLANT_PET_LIGHT_MAX, 200.0f, PLANT_PET_LIGHT_MAX, true }
};
const PlantPetRange kNoRange = { 0.0f, 1.0f, 0.0f, 1.0f, false };

bool adverse(PlantBand band) {
    return band == PlantBand::Low || band == PlantBand::High;
}

bool adverse(PlantMood mood) {
    return mood != PlantMood::Content && mood != PlantMood::Unknown;
}

PlantBand classify(const PlantPetInputs& inputs, uint8_t channel, PlantBand current) {
    const float value = inputs.values[channel];
    if (!inputs.valid[channel] || !std::isfinite(value)) return PlantBand::Unknown;
    // The firmware's ADC light conversion cannot measure outside this interval.
    // Do not turn corrupt/unsupported light readings into an invented mood.
    if (channel == static_cast<uint8_t>(PlantChannel::Light) &&
        (value < 0.0f || value > PLANT_PET_LIGHT_MAX)) return PlantBand::Unknown;
    const PlantPetRange& range = kRanges[channel];
    // Temperature has a 0.5 C hysteresis after startup. A 30/31 C alternation
    // cannot repeatedly switch the pose; these are UI limits, not plant injury.
    if (channel == static_cast<uint8_t>(PlantChannel::Temperature) && current != PlantBand::Unknown) {
        const float low = range.opt_low + (current == PlantBand::Low ? 0.5f : -0.5f);
        const float high = range.opt_high + (current == PlantBand::High ? -0.5f : 0.5f);
        if (value < low) return PlantBand::Low;
        if (value > high) return PlantBand::High;
        return PlantBand::Normal;
    }
    if (value < range.opt_low) return PlantBand::Low;
    if (value > range.opt_high) return PlantBand::High;
    return PlantBand::Normal;
}

float clamp_unit(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

uint8_t thermal_target(const PlantPetInputs& inputs, PlantBand band) {
    const float value = inputs.values[1];
    if (!inputs.valid[1] || !std::isfinite(value)) return 0;
    // Authored pose strength: full heat at 36 C / cold at 8 C. The broad
    // 15..30 C content band and these ramps are provisional product choices.
    const PlantPetRange& range = kRanges[1];
    const float strength = band == PlantBand::High ? (value - range.opt_high) / (36.0f - range.opt_high)
        : band == PlantBand::Low ? (range.opt_low - value) / (range.opt_low - 8.0f) : 0.0f;
    return static_cast<uint8_t>(clamp_unit(strength) * 255.0f + 0.5f);
}

float normalized_severity(uint8_t channel, PlantBand band, float value) {
    if (!adverse(band) || !std::isfinite(value)) return 0.0f;
    const PlantPetRange& range = kRanges[channel];
    float boundary = band == PlantBand::Low ? range.opt_low : range.opt_high;
    float extreme = band == PlantBand::Low ? range.min_value : range.max_value;
    if (range.logarithmic) {
        value = std::log1p(value < 0.0f ? 0.0f : value);
        boundary = std::log1p(boundary);
        extreme = std::log1p(extreme);
    }
    const float margin = band == PlantBand::Low ? boundary - extreme : extreme - boundary;
    if (margin <= 0.0f) return 0.0f;
    const float deviation = band == PlantBand::Low ? boundary - value : value - boundary;
    return clamp_unit(deviation / margin);
}

PlantMood mood_for(PlantChannel channel, PlantBand band) {
    const bool low = band == PlantBand::Low;
    switch (channel) {
        case PlantChannel::Soil: return low ? PlantMood::Thirsty : PlantMood::Soggy;
        case PlantChannel::Temperature: return low ? PlantMood::Cold : PlantMood::Hot;
        case PlantChannel::Air: return low ? PlantMood::AirDry : PlantMood::AirHumid;
        case PlantChannel::Light: return PlantMood::Dim; // Only Low is measurable.
        case PlantChannel::None: default: return PlantMood::Unknown;
    }
}
} // namespace

const PlantPetRange& plant_pet_range(PlantChannel channel) {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4 ? kRanges[index] : kNoRange;
}

void PlantPetModel::update(const PlantPetInputs& inputs, uint32_t now_ms) {
    const bool first_reading = !initialized_;
    // Unsigned differences also work across millis() rollover. A long pause is
    // not evidence that an unobserved candidate persisted while no readings came.
    const bool evidence_interrupted = !first_reading &&
        static_cast<uint32_t>(now_ms - last_update_ms_) > kEvidenceMaxGapMs;
    uint8_t affected_mask = 0;
    uint8_t unknown_mask = 0;

    for (uint8_t channel = 0; channel < 4; ++channel) {
        const PlantBand observed = classify(inputs, channel, bands_[channel]);
        if (first_reading || observed == PlantBand::Unknown) {
            bands_[channel] = observed;
            candidates_[channel] = observed;
            candidate_since_[channel] = now_ms;
            candidate_count_[channel] = 0;
        } else if (observed == bands_[channel]) {
            candidates_[channel] = observed;
            candidate_since_[channel] = now_ms;
            candidate_count_[channel] = 0;
        } else if (evidence_interrupted || observed != candidates_[channel] ||
                   candidate_count_[channel] == 0) {
            candidates_[channel] = observed;
            candidate_since_[channel] = now_ms;
            candidate_count_[channel] = 1;
        } else {
            if (candidate_count_[channel] < 255) ++candidate_count_[channel];
            if (candidate_count_[channel] >= 2 &&
                static_cast<uint32_t>(now_ms - candidate_since_[channel]) >= kBandConfirmMs) {
                bands_[channel] = observed;
                candidate_count_[channel] = 0;
            }
        }

        const uint8_t bit = static_cast<uint8_t>(1u << channel);
        if (adverse(bands_[channel])) affected_mask |= bit;
        if (bands_[channel] == PlantBand::Unknown) unknown_mask |= bit;
        severity_[channel] = normalized_severity(channel, bands_[channel], inputs.values[channel]);
    }
    last_update_ms_ = now_ms;

    PlantChannel next_culprit = PlantChannel::None;
    float best_severity = -1.0f;
    for (uint8_t channel = 0; channel < 4; ++channel) {
        if ((affected_mask & (1u << channel)) == 0) continue;
        if (severity_[channel] > best_severity) {
            best_severity = severity_[channel];
            next_culprit = static_cast<PlantChannel>(channel);
        }
    }

    const uint8_t current = static_cast<uint8_t>(culprit_);
    if (current < 4 && (affected_mask & (1u << current)) != 0) {
        const bool hold_complete =
            static_cast<uint32_t>(now_ms - culprit_since_ms_) >= kCulpritHoldMs;
        const bool clearly_worse = best_severity > severity_[current] + kCulpritAdvantage;
        if (!hold_complete || !clearly_worse) next_culprit = culprit_;
    }

    const PlantMood next_mood = next_culprit == PlantChannel::None
        ? (unknown_mask != 0 ? PlantMood::Unknown : PlantMood::Content)
        : mood_for(next_culprit, bands_[static_cast<uint8_t>(next_culprit)]);
    const uint8_t current_thermal = thermal_at(now_ms);
    const uint8_t target = thermal_target(inputs, bands_[1]);
    if (first_reading || bands_[1] == PlantBand::Unknown) {
        thermal_from_ = thermal_target_ = target;
        thermal_since_ms_ = now_ms;
    } else if (target != thermal_target_) {
        thermal_from_ = current_thermal;
        thermal_target_ = target;
        thermal_since_ms_ = now_ms;
    }
    if (first_reading) {
        mood_ = next_mood;
        previous_mood_ = next_mood;
        culprit_ = next_culprit;
        culprit_since_ms_ = now_ms;
        transition_since_ms_ = now_ms;
        transition_duration_ms_ = 0;
        previous_thermal_intensity_ = target;
        initialized_ = true;
    } else if (next_mood != mood_ || next_culprit != culprit_) {
        previous_thermal_intensity_ = current_thermal;
        previous_mood_ = mood_;
        mood_ = next_mood;
        culprit_ = next_culprit;
        culprit_since_ms_ = now_ms;
        transition_since_ms_ = now_ms;
        transition_duration_ms_ = adverse(previous_mood_) && mood_ == PlantMood::Content
            ? kRecoveryMs : kTransitionMs;
    }
}

uint8_t PlantPetModel::thermal_at(uint32_t now_ms) const {
    const uint32_t elapsed = static_cast<uint32_t>(now_ms - thermal_since_ms_);
    if (elapsed >= kTransitionMs) return thermal_target_;
    const int delta = static_cast<int>(thermal_target_) - thermal_from_;
    return static_cast<uint8_t>(thermal_from_ + delta * static_cast<int>(elapsed) / static_cast<int>(kTransitionMs));
}

PlantPetSnapshot PlantPetModel::snapshot(uint32_t now_ms) const {
    PlantPetSnapshot result = {};
    result.mood = mood_;
    result.previous_mood = previous_mood_;
    result.culprit = culprit_;
    result.thermal_intensity = thermal_at(now_ms);
    result.previous_thermal_intensity = previous_thermal_intensity_;
    for (uint8_t channel = 0; channel < 4; ++channel) {
        result.bands[channel] = bands_[channel];
        const uint8_t bit = static_cast<uint8_t>(1u << channel);
        if (adverse(bands_[channel])) result.affected_mask |= bit;
        if (bands_[channel] == PlantBand::Unknown) result.unknown_mask |= bit;
    }
    const uint32_t elapsed = static_cast<uint32_t>(now_ms - transition_since_ms_);
    result.transition = transition_duration_ms_ == 0 || elapsed >= transition_duration_ms_
        ? 255 : static_cast<uint8_t>((elapsed * 255u) / transition_duration_ms_);
    result.recovering = result.transition < 255 && mood_ == PlantMood::Content &&
        adverse(previous_mood_);
    return result;
}
