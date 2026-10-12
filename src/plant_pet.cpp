#include "plant_pet.h"

#include <cmath>

namespace {
constexpr uint32_t kBandConfirmMs = 1200;
constexpr uint32_t kEvidenceMaxGapMs = 3000;
constexpr uint32_t kCulpritHoldMs = 3000;
constexpr uint32_t kTransitionMs = 1500;
constexpr uint32_t kRecoveryMs = 3000;
constexpr float kCulpritAdvantage = 0.12f;

const PlantPetProfile kGeneral = { PLANT_PET_PROFILE_VERSION,
    { {30.0f, 70.0f}, {15.0f, 30.0f}, {40.0f, 70.0f} } };
// Graphic scales are independent of measurement validity and editable references.
// LDR's maximum is instrumental, not a healthy maximum.
const PlantPetRange kRanges[4] = {
    { 0.0f, 100.0f, 30.0f, 70.0f, false },
    { 0.0f, 40.0f, 15.0f, 30.0f, false },
    { 0.0f, 100.0f, 40.0f, 70.0f, false },
    { 0.0f, PLANT_PET_LIGHT_MAX, 200.0f, PLANT_PET_LIGHT_MAX, true }
};
const PlantPetRange kNoRange = { 0.0f, 1.0f, 0.0f, 1.0f, false };
const float kValidLow[4] = { 0.0f, -20.0f, 0.0f, 0.0f };
const float kValidHigh[4] = { 100.0f, 80.0f, 100.0f, PLANT_PET_LIGHT_MAX };
const float kMinReferenceWidth[3] = { 5.0f, 2.0f, 5.0f };
// Positive, versioned UI amplitudes anchored to the active references. These
// describe gesture strength, not comparable physiological damage between plants.
const float kLowAmplitude[3] = { 30.0f, 7.0f, 40.0f };
const float kHighAmplitude[3] = { 30.0f, 6.0f, 30.0f };
const uint32_t kSampleMaxAgeMs[4] = { 1000, 3000, 3000, 1000 };

bool outside_reference(PlantBand band) {
    return band == PlantBand::Low || band == PlantBand::High;
}

bool adverse(PlantMood mood) {
    return mood != PlantMood::Content && mood != PlantMood::Dim && mood != PlantMood::Unknown;
}

bool calm(PlantMood mood) {
    return mood == PlantMood::Content || mood == PlantMood::Dim;
}

float clamp_unit(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

bool usable(const PlantPetInputs& inputs, uint8_t channel, uint32_t now_ms) {
    const float value = inputs.values[channel];
    if (!inputs.valid[channel] || !std::isfinite(value) ||
        value < kValidLow[channel] || value > kValidHigh[channel]) return false;
    if (!inputs.has_sample_metadata) return true;
    const PlantSampleStamp& sample = inputs.samples[channel];
    return sample.present && static_cast<uint32_t>(now_ms - sample.sampled_at_ms) <= kSampleMaxAgeMs[channel];
}

PlantBand classify(float value, uint8_t channel, PlantBand current, const PlantPetRange& range) {
    // Temperature's 0.5 C hysteresis prevents a 30/31 C alternation from
    // repeatedly switching the gesture. References never imply injury.
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

float reference_deviation(uint8_t channel, PlantBand band, float value, const PlantPetRange& range) {
    if (channel >= 3 || !outside_reference(band) || !std::isfinite(value)) return 0.0f;
    const float strength = band == PlantBand::Low
        ? (range.opt_low - value) / kLowAmplitude[channel]
        : (value - range.opt_high) / kHighAmplitude[channel];
    return clamp_unit(strength);
}

PlantMood mood_for(PlantChannel channel, PlantBand band) {
    const bool low = band == PlantBand::Low;
    switch (channel) {
        case PlantChannel::Soil: return low ? PlantMood::Thirsty : PlantMood::Soggy;
        case PlantChannel::Temperature: return low ? PlantMood::Cold : PlantMood::Hot;
        case PlantChannel::Air: return low ? PlantMood::AirDry : PlantMood::AirHumid;
        default: return PlantMood::Unknown;
    }
}

bool same_profile(const PlantPetProfile& a, const PlantPetProfile& b) {
    if (a.version != b.version) return false;
    for (uint8_t channel = 0; channel < 3; ++channel) {
        if (a.references[channel].low != b.references[channel].low ||
            a.references[channel].high != b.references[channel].high) return false;
    }
    return true;
}
} // namespace

const PlantPetProfile& plant_pet_general_profile() { return kGeneral; }

bool plant_pet_profile_valid(const PlantPetProfile& profile) {
    if (profile.version != PLANT_PET_PROFILE_VERSION) return false;
    for (uint8_t channel = 0; channel < 3; ++channel) {
        const PlantPetReference& ref = profile.references[channel];
        if (!std::isfinite(ref.low) || !std::isfinite(ref.high) ||
            ref.low < kValidLow[channel] || ref.high > kValidHigh[channel] ||
            ref.high - ref.low < kMinReferenceWidth[channel]) return false;
    }
    return true;
}

uint32_t plant_pet_sample_max_age_ms(PlantChannel channel) {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4 ? kSampleMaxAgeMs[index] : 0;
}

const PlantPetRange& plant_pet_range(PlantChannel channel) {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4 ? kRanges[index] : kNoRange;
}

PlantPetModel::PlantPetModel() : profile_(kGeneral) {
    for (uint8_t channel = 0; channel < 4; ++channel) ranges_[channel] = kRanges[channel];
}

const PlantPetRange& PlantPetModel::range(PlantChannel channel) const {
    const uint8_t index = static_cast<uint8_t>(channel);
    return index < 4 ? ranges_[index] : kNoRange;
}

bool PlantPetModel::setProfile(const PlantPetProfile& profile, uint32_t now_ms, bool force_reset) {
    if (!plant_pet_profile_valid(profile)) return false;
    if (!force_reset && same_profile(profile_, profile)) return true;
    const uint32_t next_revision = profile_revision_ + 1;
    // Recalculate the latest fresh observation under the new reference once;
    // old candidates/attention cannot masquerade as recovery.
    *this = PlantPetModel();
    profile_ = profile;
    profile_revision_ = next_revision;
    for (uint8_t channel = 0; channel < 3; ++channel) {
        ranges_[channel].opt_low = profile.references[channel].low;
        ranges_[channel].opt_high = profile.references[channel].high;
    }
    transition_since_ms_ = now_ms;
    return true;
}

void PlantPetModel::update(const PlantPetInputs& inputs, uint32_t now_ms) {
    const bool first_reading = !initialized_;
    uint16_t previous_pose[PLANT_PET_POSE_COUNT];
    pose_at(now_ms, previous_pose);
    const uint8_t current_thermal = intensity_at(1, now_ms);
    uint8_t affected_mask = 0;
    uint8_t unknown_mask = 0;

    for (uint8_t channel = 0; channel < 4; ++channel) {
        const bool known = usable(inputs, channel, now_ms);
        const PlantBand observed = known
            ? classify(inputs.values[channel], channel, bands_[channel], ranges_[channel])
            : PlantBand::Unknown;
        const PlantSampleStamp& sample = inputs.samples[channel];
        const uint32_t sampled_ms = inputs.has_sample_metadata ? sample.sampled_at_ms : now_ms;
        const bool new_sample = !inputs.has_sample_metadata || !sample_seen_[channel] ||
            sample.sequence != last_sequence_[channel] || sampled_ms != last_sample_ms_[channel];
        const bool evidence_interrupted = sample_seen_[channel] &&
            static_cast<uint32_t>(sampled_ms - last_sample_ms_[channel]) > kEvidenceMaxGapMs;

        if (observed == PlantBand::Unknown) {
            // Expiry/invalidation is immediate even when the acquisition stamp
            // stays unchanged (e.g. the second failed DHT attempt).
            bands_[channel] = candidates_[channel] = PlantBand::Unknown;
            candidate_count_[channel] = 0;
        } else if (new_sample || first_reading) {
            if (first_reading || !sample_seen_[channel]) {
                bands_[channel] = candidates_[channel] = observed;
                candidate_since_[channel] = sampled_ms;
                candidate_count_[channel] = 0;
            } else if (observed == bands_[channel]) {
                candidates_[channel] = observed;
                candidate_since_[channel] = sampled_ms;
                candidate_count_[channel] = 0;
            } else if (evidence_interrupted || observed != candidates_[channel] || candidate_count_[channel] == 0) {
                candidates_[channel] = observed;
                candidate_since_[channel] = sampled_ms;
                candidate_count_[channel] = 1;
            } else {
                if (candidate_count_[channel] < 255) ++candidate_count_[channel];
                if (candidate_count_[channel] >= 2 &&
                    static_cast<uint32_t>(sampled_ms - candidate_since_[channel]) >= kBandConfirmMs) {
                    bands_[channel] = observed;
                    candidate_count_[channel] = 0;
                }
            }
            sample_seen_[channel] = true;
            last_sequence_[channel] = sample.sequence;
            last_sample_ms_[channel] = sampled_ms;
        }

        const uint8_t bit = static_cast<uint8_t>(1u << channel);
        if (channel < 3 && outside_reference(bands_[channel])) affected_mask |= bit;
        if (bands_[channel] == PlantBand::Unknown) unknown_mask |= bit;
        severity_[channel] = reference_deviation(channel, bands_[channel], inputs.values[channel], ranges_[channel]);
        const uint8_t target = static_cast<uint8_t>(severity_[channel] * 255.0f + 0.5f);
        if (first_reading || bands_[channel] == PlantBand::Unknown) {
            intensity_from_[channel] = intensity_target_[channel] = target;
            intensity_since_ms_[channel] = now_ms;
        } else if (target != intensity_target_[channel] && new_sample) {
            intensity_from_[channel] = intensity_at(channel, now_ms);
            intensity_target_[channel] = target;
            intensity_since_ms_[channel] = now_ms;
        }
    }

    PlantChannel next_culprit = PlantChannel::None;
    float best_severity = -1.0f;
    for (uint8_t channel = 0; channel < 3; ++channel) {
        if ((affected_mask & (1u << channel)) == 0) continue;
        if (severity_[channel] > best_severity) {
            best_severity = severity_[channel];
            next_culprit = static_cast<PlantChannel>(channel);
        }
    }

    const uint8_t current = static_cast<uint8_t>(culprit_);
    if (current < 3 && (affected_mask & (1u << current)) != 0) {
        const bool hold_complete = static_cast<uint32_t>(now_ms - culprit_since_ms_) >= kCulpritHoldMs;
        const bool clearly_worse = best_severity > severity_[current] + kCulpritAdvantage;
        if (!hold_complete || !clearly_worse) next_culprit = culprit_;
    }

    const PlantMood next_mood = next_culprit == PlantChannel::None
        ? (unknown_mask != 0 ? PlantMood::Unknown
            : bands_[3] == PlantBand::Low ? PlantMood::Dim : PlantMood::Content)
        : mood_for(next_culprit, bands_[static_cast<uint8_t>(next_culprit)]);
    if (first_reading) {
        mood_ = previous_mood_ = next_mood;
        culprit_ = next_culprit;
        culprit_since_ms_ = transition_since_ms_ = now_ms;
        transition_duration_ms_ = 0;
        previous_thermal_intensity_ = intensity_target_[1];
        initialized_ = true;
    } else if (next_mood != mood_ || next_culprit != culprit_) {
        for (uint8_t pose = 0; pose < PLANT_PET_POSE_COUNT; ++pose) pose_from_[pose] = previous_pose[pose];
        previous_thermal_intensity_ = current_thermal;
        previous_mood_ = mood_;
        mood_ = next_mood;
        culprit_ = next_culprit;
        culprit_since_ms_ = transition_since_ms_ = now_ms;
        transition_duration_ms_ = adverse(previous_mood_) && calm(mood_) ? kRecoveryMs : kTransitionMs;
    }
}

uint8_t PlantPetModel::intensity_at(uint8_t channel, uint32_t now_ms) const {
    const uint32_t elapsed = static_cast<uint32_t>(now_ms - intensity_since_ms_[channel]);
    if (elapsed >= kTransitionMs) return intensity_target_[channel];
    const int delta = static_cast<int>(intensity_target_[channel]) - intensity_from_[channel];
    return static_cast<uint8_t>(intensity_from_[channel] + delta * static_cast<int>(elapsed) / static_cast<int>(kTransitionMs));
}

void PlantPetModel::pose_at(uint32_t now_ms, uint16_t* weights) const {
    uint16_t target[PLANT_PET_POSE_COUNT] = {};
    const uint8_t channel = static_cast<uint8_t>(culprit_);
    const uint16_t strength = channel < 3
        ? static_cast<uint16_t>(intensity_at(channel, now_ms) * 257u) : PLANT_PET_POSE_WEIGHT_MAX;
    const uint8_t mood = static_cast<uint8_t>(mood_);
    target[mood] = strength;
    target[static_cast<uint8_t>(PlantMood::Content)] += PLANT_PET_POSE_WEIGHT_MAX - strength;
    const uint32_t elapsed = static_cast<uint32_t>(now_ms - transition_since_ms_);
    const uint32_t progress = transition_duration_ms_ == 0 || elapsed >= transition_duration_ms_
        ? PLANT_PET_POSE_WEIGHT_MAX : elapsed * PLANT_PET_POSE_WEIGHT_MAX / transition_duration_ms_;
    uint32_t sum = 0;
    for (uint8_t pose = 0; pose < PLANT_PET_POSE_COUNT; ++pose) {
        weights[pose] = static_cast<uint16_t>((static_cast<uint32_t>(pose_from_[pose]) * (PLANT_PET_POSE_WEIGHT_MAX - progress)
            + static_cast<uint32_t>(target[pose]) * progress) / PLANT_PET_POSE_WEIGHT_MAX);
        sum += weights[pose];
    }
    // Quantization loses at most eight units. Keep normalization through
    // repeated interrupted transitions, using a fixed-size mixture.
    weights[static_cast<uint8_t>(PlantMood::Content)] += static_cast<uint16_t>(PLANT_PET_POSE_WEIGHT_MAX - sum);
}

PlantPetSnapshot PlantPetModel::snapshot(uint32_t now_ms) const {
    PlantPetSnapshot result = {};
    result.mood = mood_;
    result.previous_mood = previous_mood_;
    result.culprit = culprit_;
    result.thermal_intensity = intensity_at(1, now_ms);
    result.previous_thermal_intensity = previous_thermal_intensity_;
    result.profile_revision = profile_revision_;
    result.low_light = bands_[3] == PlantBand::Low;
    for (uint8_t channel = 0; channel < 4; ++channel) {
        result.bands[channel] = bands_[channel];
        result.intensity[channel] = intensity_at(channel, now_ms);
        const uint8_t bit = static_cast<uint8_t>(1u << channel);
        if (outside_reference(bands_[channel])) result.reference_mask |= bit;
        if (channel < 3 && outside_reference(bands_[channel])) result.affected_mask |= bit;
        if (bands_[channel] == PlantBand::Unknown) result.unknown_mask |= bit;
    }
    result.attention_mask = result.affected_mask;
    pose_at(now_ms, result.pose_weights);
    const uint32_t elapsed = static_cast<uint32_t>(now_ms - transition_since_ms_);
    result.transition = transition_duration_ms_ == 0 || elapsed >= transition_duration_ms_
        ? 255 : static_cast<uint8_t>(elapsed * 255u / transition_duration_ms_);
    result.recovering = result.transition < 255 && calm(mood_) && adverse(previous_mood_);
    return result;
}
