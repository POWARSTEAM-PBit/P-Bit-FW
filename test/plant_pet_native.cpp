// Standalone native checks; no Arduino, PlatformIO environment or device needed.
// Example: g++ -std=c++11 -Wall -Wextra -Werror -Iinclude src/plant_pet.cpp
//          test/plant_pet_native.cpp -o <temporary-output> && <temporary-output>
#include "plant_pet.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <initializer_list>

namespace {
int failures = 0;
#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); ++failures; \
} } while (false)

PlantPetInputs healthy() {
    PlantPetInputs inputs = {};
    const float values[4] = {50.0f, 22.0f, 55.0f, 1000.0f};
    for (uint8_t channel = 0; channel < 4; ++channel) {
        inputs.values[channel] = values[channel];
        inputs.valid[channel] = true;
    }
    return inputs;
}

void inclusive_thresholds_and_startup() {
    PlantPetModel untouched;
    CHECK(untouched.snapshot(0).mood == PlantMood::Unknown);
    CHECK(untouched.snapshot(0).unknown_mask == 0x0f);
    CHECK(untouched.snapshot(0).transition == 255);
    const PlantMood low_moods[4] = { PlantMood::Thirsty, PlantMood::Cold, PlantMood::AirDry, PlantMood::Dim };
    const PlantMood high_moods[4] = { PlantMood::Soggy, PlantMood::Hot, PlantMood::AirHumid, PlantMood::Unknown };
    for (uint8_t channel = 0; channel < 4; ++channel) {
        const PlantPetRange& range = plant_pet_range(static_cast<PlantChannel>(channel));
        PlantPetInputs inputs = healthy();
        inputs.values[channel] = range.opt_low;
        PlantPetModel low_boundary;
        low_boundary.update(inputs, 0);
        CHECK(low_boundary.snapshot(0).bands[channel] == PlantBand::Normal);
        CHECK(low_boundary.snapshot(0).mood == PlantMood::Content);
        inputs.values[channel] = range.opt_high;
        PlantPetModel high_boundary;
        high_boundary.update(inputs, 0);
        CHECK(high_boundary.snapshot(0).bands[channel] == PlantBand::Normal);
        inputs.values[channel] = range.opt_low - 0.01f;
        PlantPetModel below;
        below.update(inputs, 0);
        CHECK(below.snapshot(0).bands[channel] == PlantBand::Low);
        CHECK(below.snapshot(0).mood == low_moods[channel]);
        CHECK(below.snapshot(0).affected_mask == (channel == 3 ? 0 : (1u << channel)));
        CHECK(below.snapshot(0).reference_mask == (1u << channel));
        CHECK(below.snapshot(0).culprit == (channel == 3 ? PlantChannel::None : static_cast<PlantChannel>(channel)));
        CHECK(below.snapshot(0).transition == 255); // Startup has no false wait.
        inputs.values[channel] = range.opt_high + 0.01f;
        PlantPetModel above;
        above.update(inputs, 0);
        CHECK(above.snapshot(0).bands[channel] ==
            (channel == 3 ? PlantBand::Unknown : PlantBand::High));
        CHECK(above.snapshot(0).mood == high_moods[channel]);
    }
    CHECK(plant_pet_range(PlantChannel::Light).max_value == 8000.0f);
    CHECK(plant_pet_range(PlantChannel::Light).logarithmic);
}

void noisy_thresholds_need_recent_evidence() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    model.update(inputs, 0);
    inputs.values[0] = 29.9f;
    model.update(inputs, 100);
    CHECK(model.snapshot(100).bands[0] == PlantBand::Normal);
    inputs.values[0] = 30.0f;
    model.update(inputs, 700);
    inputs.values[0] = 29.9f;
    model.update(inputs, 1300);
    inputs.values[0] = 30.0f;
    model.update(inputs, 1900);
    inputs.values[0] = 29.9f;
    model.update(inputs, 2200);
    CHECK(model.snapshot(3300).bands[0] == PlantBand::Normal);
    model.update(inputs, 3399);
    CHECK(model.snapshot(3399).bands[0] == PlantBand::Normal);
    model.update(inputs, 3400);
    CHECK(model.snapshot(3400).bands[0] == PlantBand::Low);
    CHECK(model.snapshot(3400).mood == PlantMood::Thirsty);
    CHECK(model.snapshot(3400).transition == 0);
    CHECK(model.snapshot(4150).transition == 127);
    CHECK(model.snapshot(4900).transition == 255);
}

void recovery_is_gradual_but_not_persistent_growth() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    inputs.values[0] = 10.0f;
    model.update(inputs, 1000);
    inputs.values[0] = 50.0f;
    model.update(inputs, 2000);
    CHECK(model.snapshot(3199).mood == PlantMood::Thirsty);
    model.update(inputs, 3200);
    PlantPetSnapshot start = model.snapshot(3200);
    CHECK(start.mood == PlantMood::Content);
    CHECK(start.previous_mood == PlantMood::Thirsty);
    CHECK(start.transition == 0 && start.recovering);
    const PlantPetSnapshot half = model.snapshot(4700);
    CHECK(half.transition == 127 && half.recovering);
    CHECK(model.snapshot(4700).transition == half.transition);
    CHECK(model.snapshot(6199).transition == 254);
    CHECK(model.snapshot(6200).transition == 255);
    CHECK(!model.snapshot(6200).recovering);
    CHECK(model.snapshot(6200).affected_mask == 0);
}

void severity_and_culprit_hold_show_concurrent_needs() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    inputs.values[0] = 15.0f; // 0.50 severity.
    inputs.values[1] = 11.5f; // 0.50 authored cold strength: deterministic startup tie.
    model.update(inputs, 0);
    CHECK(model.snapshot(0).culprit == PlantChannel::Soil);
    CHECK(model.snapshot(0).affected_mask == 0x03);
    inputs.values[1] = 0.0f; // 1.00 severity, but current cause held 3 seconds.
    model.update(inputs, 1000);
    CHECK(model.snapshot(1000).culprit == PlantChannel::Soil);
    model.update(inputs, 2999);
    CHECK(model.snapshot(2999).culprit == PlantChannel::Soil);
    model.update(inputs, 3000);
    CHECK(model.snapshot(3000).culprit == PlantChannel::Temperature);
    CHECK(model.snapshot(3000).mood == PlantMood::Cold);
    CHECK(model.snapshot(3000).affected_mask == 0x03);
    inputs.values[0] = 0.0f;
    inputs.values[1] = 1.0f; // 0.9333; <0.12 advantage should not flap back.
    model.update(inputs, 5000);
    model.update(inputs, 6000);
    CHECK(model.snapshot(6000).culprit == PlantChannel::Temperature);
    inputs.values[1] = 22.0f;
    model.update(inputs, 6100);
    model.update(inputs, 7300);
    CHECK(model.snapshot(7300).culprit == PlantChannel::Soil);
    CHECK(model.snapshot(7300).affected_mask == 0x01);

    PlantPetModel no_soil_priority;
    inputs = healthy();
    inputs.values[0] = 29.0f; // 0.0333 severity.
    inputs.values[2] = 0.0f;  // 1.00 severity.
    no_soil_priority.update(inputs, 0);
    CHECK(no_soil_priority.snapshot(0).culprit == PlantChannel::Air);
    CHECK(no_soil_priority.snapshot(0).mood == PlantMood::AirDry);
    CHECK(no_soil_priority.snapshot(0).affected_mask == 0x05);

    PlantPetModel logarithmic_light;
    inputs = healthy();
    inputs.values[0] = 15.0f; // 0.50 severity.
    inputs.values[3] = 100.0f; // log1p gap ~0.13, not a linear 0.50.
    logarithmic_light.update(inputs, 0);
    CHECK(logarithmic_light.snapshot(0).culprit == PlantChannel::Soil);
    CHECK(logarithmic_light.snapshot(0).affected_mask == 0x01);
    CHECK(logarithmic_light.snapshot(0).reference_mask == 0x09);
}

void unknown_is_immediate_and_not_an_adverse_state() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    model.update(inputs, 0);
    inputs.valid[0] = false;
    model.update(inputs, 100);
    CHECK(model.snapshot(100).bands[0] == PlantBand::Unknown);
    CHECK(model.snapshot(100).unknown_mask == 0x01);
    CHECK(model.snapshot(100).affected_mask == 0);
    CHECK(model.snapshot(100).mood == PlantMood::Unknown);
    CHECK(!model.snapshot(100).recovering);
    inputs.values[1] = 40.0f;
    model.update(inputs, 200);
    model.update(inputs, 1400);
    CHECK(model.snapshot(1400).mood == PlantMood::Hot);
    CHECK(model.snapshot(1400).unknown_mask == 0x01);
    CHECK(model.snapshot(1400).affected_mask == 0x02);
    for (uint8_t channel = 0; channel < 4; ++channel) inputs.valid[channel] = false;
    model.update(inputs, 1500);
    CHECK(model.snapshot(1500).mood == PlantMood::Unknown);
    CHECK(model.snapshot(1500).unknown_mask == 0x0f);
    CHECK(model.snapshot(1500).affected_mask == 0);
    CHECK(model.snapshot(1500).culprit == PlantChannel::None);
    inputs = healthy();
    inputs.values[1] = std::numeric_limits<float>::quiet_NaN();
    inputs.values[2] = std::numeric_limits<float>::infinity();
    inputs.values[3] = 8001.0f;
    model.update(inputs, 1600);
    model.update(inputs, 2800);
    CHECK(model.snapshot(2800).bands[0] == PlantBand::Normal);
    CHECK(model.snapshot(2800).unknown_mask == 0x0e);
    CHECK(model.snapshot(2800).mood == PlantMood::Unknown);
    PlantPetModel negative_light;
    inputs = healthy();
    inputs.values[3] = -1.0f;
    negative_light.update(inputs, 0);
    CHECK(negative_light.snapshot(0).bands[3] == PlantBand::Unknown);
    CHECK(negative_light.snapshot(0).affected_mask == 0);
}

void update_works_without_a_visible_screen() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    model.update(inputs, 0);
    inputs.values[3] = 0.0f;
    model.update(inputs, 1000);
    model.update(inputs, 2200);
    model.update(inputs, 3400);
    // First rendered snapshot happens later; behavior was already updated.
    CHECK(model.snapshot(5000).mood == PlantMood::Dim);
    CHECK(model.snapshot(5000).affected_mask == 0);
    CHECK(model.snapshot(5000).low_light);
    CHECK(model.snapshot(5000).transition == 255);
}

void long_pauses_do_not_confirm_old_candidates() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    model.update(inputs, 0);
    inputs.values[0] = 10.0f;
    model.update(inputs, 100);
    CHECK(model.snapshot(100000).bands[0] == PlantBand::Normal);
    model.update(inputs, 100000); // Long pause resets, cannot confirm thirst.
    CHECK(model.snapshot(100000).bands[0] == PlantBand::Normal);
    model.update(inputs, 101199);
    CHECK(model.snapshot(101199).bands[0] == PlantBand::Normal);
    model.update(inputs, 101200);
    CHECK(model.snapshot(101200).bands[0] == PlantBand::Low);
    inputs.values[0] = 50.0f;
    model.update(inputs, 101300);
    model.update(inputs, 200000); // Nor can a pause confirm recovery.
    CHECK(model.snapshot(200000).mood == PlantMood::Thirsty);
    model.update(inputs, 201200);
    CHECK(model.snapshot(201200).mood == PlantMood::Content);
    CHECK(model.snapshot(201200).recovering);
}

void rollover_preserves_evidence_and_transitions() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    const uint32_t start = UINT32_MAX - 500u;
    model.update(inputs, start - 100u);
    inputs.values[0] = 10.0f;
    model.update(inputs, start);
    model.update(inputs, start + 1200u);
    CHECK(model.snapshot(start + 1200u).mood == PlantMood::Thirsty);
    CHECK(model.snapshot(start + 1950u).transition == 127);
    CHECK(model.snapshot(start + 2700u).transition == 255);

    PlantPetModel recovery;
    inputs.values[0] = 10.0f;
    recovery.update(inputs, start - 1400u);
    inputs.values[0] = 50.0f;
    recovery.update(inputs, start - 1300u);
    recovery.update(inputs, start - 100u);
    CHECK(recovery.snapshot(start - 100u).recovering);
    CHECK(recovery.snapshot(start + 1400u).transition == 127);
    CHECK(!recovery.snapshot(start + 2900u).recovering);
}

void temperature_is_flexible_and_hysteretic() {
    for (float temp : {15.0f, 18.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f}) {
        PlantPetInputs inputs = healthy();
        inputs.values[1] = temp;
        PlantPetModel model;
        model.update(inputs, 0);
        CHECK(model.snapshot(0).mood == PlantMood::Content);
        CHECK(model.snapshot(0).thermal_intensity == 0);
    }
    PlantPetInputs inputs = healthy();
    inputs.values[1] = 30.0f;
    PlantPetModel model;
    model.update(inputs, 0);
    inputs.values[1] = 30.4f;
    model.update(inputs, 100);
    model.update(inputs, 1300);
    CHECK(model.snapshot(1300).mood == PlantMood::Content);
    inputs.values[1] = 30.6f;
    model.update(inputs, 1400);
    model.update(inputs, 2600);
    CHECK(model.snapshot(2600).mood == PlantMood::Hot);
    CHECK(model.snapshot(2600).thermal_intensity == 0);
    CHECK(model.snapshot(3350).thermal_intensity == 13);
    CHECK(model.snapshot(4100).thermal_intensity == 26);
    inputs.values[1] = 30.0f;
    model.update(inputs, 4200);
    model.update(inputs, 5400);
    CHECK(model.snapshot(5400).bands[1] == PlantBand::High);
    CHECK(model.snapshot(5700).thermal_intensity == 0);
    inputs.values[1] = 29.4f;
    model.update(inputs, 5800);
    model.update(inputs, 7000);
    CHECK(model.snapshot(7000).mood == PlantMood::Content);
    CHECK(model.snapshot(7000).recovering);

    PlantPetModel cool;
    inputs.values[1] = 15.0f;
    cool.update(inputs, 0);
    inputs.values[1] = 14.6f;
    cool.update(inputs, 100);
    cool.update(inputs, 1300);
    CHECK(cool.snapshot(1300).mood == PlantMood::Content);
    inputs.values[1] = 14.4f;
    cool.update(inputs, 1400);
    cool.update(inputs, 2600);
    CHECK(cool.snapshot(2600).mood == PlantMood::Cold);
    inputs.values[1] = 15.0f;
    cool.update(inputs, 2700);
    cool.update(inputs, 3900);
    CHECK(cool.snapshot(3900).bands[1] == PlantBand::Low);
    inputs.values[1] = 15.6f;
    cool.update(inputs, 4000);
    cool.update(inputs, 5200);
    CHECK(cool.snapshot(5200).mood == PlantMood::Content);
}

void thermal_strength_progresses_without_counting_frames_as_evidence() {
    PlantPetInputs inputs = healthy();
    inputs.values[1] = 31.0f;
    PlantPetModel model;
    model.update(inputs, 0);
    CHECK(model.snapshot(0).thermal_intensity == 43);
    inputs.values[1] = 36.0f;
    model.update(inputs, 1000);
    CHECK(model.snapshot(1000).thermal_intensity == 43);
    CHECK(model.snapshot(1750).thermal_intensity == 149);
    CHECK(model.snapshot(1750).thermal_intensity == 149);
    model.update(inputs, 1800); // Same target must not restart the ramp.
    CHECK(model.snapshot(2500).thermal_intensity == 255);
    inputs.values[1] = 29.0f;
    model.update(inputs, 2600);
    model.update(inputs, 3800);
    const PlantPetSnapshot recovery = model.snapshot(3800);
    CHECK(recovery.mood == PlantMood::Content && recovery.recovering);
    CHECK(recovery.previous_thermal_intensity == 51);
    CHECK(model.snapshot(5300).transition == 127);
    inputs.valid[1] = false;
    model.update(inputs, 3900);
    CHECK(model.snapshot(3900).thermal_intensity == 0);
    CHECK(model.snapshot(3900).mood == PlantMood::Unknown);

    const uint32_t start = UINT32_MAX - 500u;
    PlantPetModel rollover;
    inputs = healthy();
    inputs.values[1] = 31.0f;
    rollover.update(inputs, start - 100u);
    inputs.values[1] = 36.0f;
    rollover.update(inputs, start);
    CHECK(rollover.snapshot(start + 750u).thermal_intensity == 149);
    CHECK(rollover.snapshot(start + 1500u).thermal_intensity == 255);
}

void stamp_all(PlantPetInputs& inputs, uint32_t sequence, uint32_t sampled_ms) {
    inputs.has_sample_metadata = true;
    for (uint8_t channel = 0; channel < 4; ++channel)
        inputs.samples[channel] = { sequence, sampled_ms, true };
}

void duplicate_copies_cannot_confirm_a_band() {
    PlantPetInputs inputs = healthy();
    stamp_all(inputs, 1, 0);
    PlantPetModel model;
    model.update(inputs, 0);
    inputs.values[1] = 31.0f;
    stamp_all(inputs, 2, 1000);
    model.update(inputs, 1000);
    for (uint32_t now = 1100; now <= 2200; now += 100) model.update(inputs, now);
    CHECK(model.snapshot(2200).bands[1] == PlantBand::Normal);
    CHECK(model.snapshot(2200).thermal_intensity == 0);
    // The identical numerical value is a new physical acquisition and counts.
    stamp_all(inputs, 3, 3000);
    model.update(inputs, 3000);
    CHECK(model.snapshot(3000).bands[1] == PlantBand::High);
    CHECK(model.snapshot(3000).mood == PlantMood::Hot);

    PlantPetModel delayed_copies;
    inputs = healthy();
    stamp_all(inputs, 1, 0);
    delayed_copies.update(inputs, 0);
    inputs.values[1] = 31.0f;
    stamp_all(inputs, 2, 1000);
    delayed_copies.update(inputs, 1000);
    stamp_all(inputs, 3, 1100);
    delayed_copies.update(inputs, 2500); // Delivery lag is not sustained exposure.
    CHECK(delayed_copies.snapshot(2500).bands[1] == PlantBand::Normal);
    stamp_all(inputs, 4, 2200);
    delayed_copies.update(inputs, 2600);
    CHECK(delayed_copies.snapshot(2600).bands[1] == PlantBand::High);
}

void acquisition_quality_and_expiry_are_independent_of_identity() {
    PlantPetInputs inputs = healthy();
    stamp_all(inputs, 1, 0);
    PlantPetModel model;
    model.update(inputs, 0);
    model.update(inputs, 1000);
    CHECK(model.snapshot(1000).unknown_mask == 0);
    model.update(inputs, 1001);
    CHECK(model.snapshot(1001).unknown_mask == 0x09); // Soil and LDR expire first.
    model.update(inputs, 3000);
    CHECK(model.snapshot(3000).bands[1] == PlantBand::Normal);
    model.update(inputs, 3001);
    CHECK(model.snapshot(3001).unknown_mask == 0x0f);
    CHECK(model.snapshot(3001).affected_mask == 0);

    PlantPetModel quality;
    inputs = healthy();
    inputs.values[1] = 36.0f;
    stamp_all(inputs, 1, 100);
    quality.update(inputs, 100);
    CHECK(quality.snapshot(100).mood == PlantMood::Hot);
    inputs.valid[1] = false; // Invalid quality; stamp intentionally unchanged.
    quality.update(inputs, 101);
    CHECK(quality.snapshot(101).bands[1] == PlantBand::Unknown);
    CHECK(quality.snapshot(101).thermal_intensity == 0);
    CHECK(quality.snapshot(101).affected_mask == 0);
    CHECK(!quality.snapshot(101).recovering);

    const uint32_t start = UINT32_MAX - 500u;
    PlantPetModel rollover;
    inputs = healthy();
    stamp_all(inputs, UINT32_MAX, start);
    rollover.update(inputs, start);
    rollover.update(inputs, start + 1000u);
    CHECK(rollover.snapshot(start + 1000u).unknown_mask == 0);
    rollover.update(inputs, start + 1001u);
    CHECK(rollover.snapshot(start + 1001u).unknown_mask == 0x09);
    stamp_all(inputs, 1, start + 1200u); // Acquisition sequence and millis wrap.
    rollover.update(inputs, start + 1200u);
    CHECK(rollover.snapshot(start + 1200u).bands[1] == PlantBand::Normal);
    CHECK(plant_pet_sample_max_age_ms(PlantChannel::None) == 0);
}

void darkness_is_calm_context_and_never_competes_for_attention() {
    PlantPetInputs inputs = healthy();
    inputs.values[3] = 0.0f;
    PlantPetModel resting;
    resting.update(inputs, 0);
    const PlantPetSnapshot rest = resting.snapshot(0);
    CHECK(rest.mood == PlantMood::Dim);
    CHECK(rest.culprit == PlantChannel::None);
    CHECK(rest.affected_mask == 0 && rest.attention_mask == 0);
    CHECK(rest.reference_mask == 0x08 && rest.low_light);
    CHECK(rest.intensity[3] == 0);
    CHECK(rest.pose_weights[static_cast<uint8_t>(PlantMood::Dim)] == PLANT_PET_POSE_WEIGHT_MAX);

    inputs.values[0] = 15.0f;
    inputs.values[1] = 35.0f;
    inputs.values[2] = 20.0f;
    PlantPetModel needs;
    needs.update(inputs, 0);
    const PlantPetSnapshot need = needs.snapshot(0);
    CHECK(need.mood == PlantMood::Hot);
    CHECK(need.culprit == PlantChannel::Temperature);
    CHECK(need.affected_mask == 0x07 && need.reference_mask == 0x0f);
    CHECK(need.low_light);

    inputs = healthy();
    inputs.values[0] = 0.0f;
    inputs.values[3] = 0.0f;
    PlantPetModel recovery;
    recovery.update(inputs, 0);
    inputs.values[0] = 50.0f;
    recovery.update(inputs, 100);
    recovery.update(inputs, 1300);
    CHECK(recovery.snapshot(1300).mood == PlantMood::Dim);
    CHECK(recovery.snapshot(1300).recovering);
    CHECK(recovery.snapshot(2800).transition == 127);
    CHECK(!recovery.snapshot(4300).recovering);
}

void soil_and_air_gestures_are_gentle_and_progressive() {
    for (uint8_t channel : {uint8_t(0), uint8_t(2)}) {
        PlantPetInputs inputs = healthy();
        const float low = plant_pet_range(static_cast<PlantChannel>(channel)).opt_low;
        inputs.values[channel] = low - 0.01f;
        PlantPetModel gentle;
        gentle.update(inputs, 0);
        const PlantPetSnapshot initial = gentle.snapshot(0);
        CHECK(initial.bands[channel] == PlantBand::Low);
        CHECK(initial.intensity[channel] == 0);
        CHECK(initial.pose_weights[static_cast<uint8_t>(PlantMood::Content)] == PLANT_PET_POSE_WEIGHT_MAX);
        inputs.values[channel] = 0.0f;
        gentle.update(inputs, 1000);
        CHECK(gentle.snapshot(1000).intensity[channel] == 0);
        CHECK(gentle.snapshot(1750).intensity[channel] == 127);
        CHECK(gentle.snapshot(2500).intensity[channel] == 255);
        const uint8_t pose = static_cast<uint8_t>(channel == 0 ? PlantMood::Thirsty : PlantMood::AirDry);
        CHECK(gentle.snapshot(2500).pose_weights[pose] == PLANT_PET_POSE_WEIGHT_MAX);
        gentle.update(inputs, 2501);
        CHECK(gentle.snapshot(3000).intensity[channel] == 255); // Identical target holds.
    }
}

uint32_t weight_sum(const PlantPetSnapshot& snapshot) {
    uint32_t sum = 0;
    for (uint8_t pose = 0; pose < PLANT_PET_POSE_COUNT; ++pose) sum += snapshot.pose_weights[pose];
    return sum;
}

void recovery_interruption_preserves_the_effective_pose() {
    PlantPetInputs inputs = healthy();
    inputs.values[0] = 0.0f;
    PlantPetModel model;
    model.update(inputs, 0);
    inputs.values[0] = 50.0f;
    model.update(inputs, 100);
    model.update(inputs, 1300);
    inputs.values[0] = 100.0f;
    model.update(inputs, 1400);
    const PlantPetSnapshot before = model.snapshot(2600);
    CHECK(before.mood == PlantMood::Content && before.recovering);
    CHECK(before.pose_weights[static_cast<uint8_t>(PlantMood::Thirsty)] > 0);
    model.update(inputs, 2600);
    const PlantPetSnapshot after = model.snapshot(2600);
    CHECK(after.mood == PlantMood::Soggy && after.transition == 0);
    CHECK(!after.recovering);
    for (uint8_t pose = 0; pose < PLANT_PET_POSE_COUNT; ++pose)
        CHECK(after.pose_weights[pose] == before.pose_weights[pose]);
    CHECK(weight_sum(after) == PLANT_PET_POSE_WEIGHT_MAX);
    CHECK(model.snapshot(3350).pose_weights[static_cast<uint8_t>(PlantMood::Soggy)] > 0);
    CHECK(model.snapshot(5600).pose_weights[static_cast<uint8_t>(PlantMood::Soggy)] == PLANT_PET_POSE_WEIGHT_MAX);

    // Rapid reversals never lose normalization or restart from a logical pose.
    for (uint32_t now = 6000; now < 24000; now += 1300) {
        inputs.values[0] = inputs.values[0] == 0.0f ? 100.0f : 0.0f;
        model.update(inputs, now);
        const PlantPetSnapshot old = model.snapshot(now + 1200);
        model.update(inputs, now + 1200);
        const PlantPetSnapshot changed = model.snapshot(now + 1200);
        CHECK(weight_sum(changed) == PLANT_PET_POSE_WEIGHT_MAX);
        if (changed.transition == 0)
            for (uint8_t pose = 0; pose < PLANT_PET_POSE_COUNT; ++pose)
                CHECK(changed.pose_weights[pose] == old.pose_weights[pose]);
    }
}

void profile_validation_activation_and_scale_are_independent() {
    CHECK(plant_pet_profile_valid(plant_pet_general_profile()));
    PlantPetProfile custom = plant_pet_general_profile();
    custom.references[0] = {10.0f, 60.0f};
    custom.references[1] = {8.0f, 36.0f}; // Both used to be zero ramp denominators.
    custom.references[2] = {25.0f, 85.0f};
    PlantPetModel model;
    CHECK(model.setProfile(custom, 0));
    CHECK(model.range(PlantChannel::Temperature).min_value == 0.0f);
    CHECK(model.range(PlantChannel::Temperature).max_value == 40.0f);
    CHECK(model.range(PlantChannel::Temperature).opt_low == 8.0f);
    CHECK(model.range(PlantChannel::Temperature).opt_high == 36.0f);
    CHECK(model.snapshot(0).profile_revision == 1);
    PlantPetInputs inputs = healthy();
    inputs.values[1] = 40.0f;
    model.update(inputs, 0);
    CHECK(model.snapshot(0).mood == PlantMood::Hot);
    CHECK(model.snapshot(0).thermal_intensity == 170);
    inputs.values[1] = 1.0f;
    model.update(inputs, 100);
    model.update(inputs, 1300);
    CHECK(model.snapshot(1300).mood == PlantMood::Cold);
    CHECK(model.snapshot(2800).thermal_intensity == 255);
    CHECK(model.setProfile(plant_pet_general_profile(), 3000));
    CHECK(model.snapshot(3000).profile_revision == 2);
    model.update(healthy(), 3000);
    CHECK(model.snapshot(3000).mood == PlantMood::Content);
    CHECK(!model.snapshot(3000).recovering);
    CHECK(model.setProfile(plant_pet_general_profile(), 3100));
    CHECK(model.snapshot(3100).profile_revision == 2);

    custom.references[1] = {50.0f, 60.0f}; // References outside the graphic scale.
    CHECK(model.setProfile(custom, 4000));
    inputs = healthy();
    inputs.values[1] = 55.0f;
    model.update(inputs, 4000);
    CHECK(model.snapshot(4000).bands[1] == PlantBand::Normal);
    CHECK(model.snapshot(4000).mood == PlantMood::Content);
    CHECK(model.range(PlantChannel::Temperature).max_value == 40.0f);

    for (uint8_t channel = 0; channel < 3; ++channel) {
        custom = plant_pet_general_profile();
        custom.references[channel].low = std::numeric_limits<float>::quiet_NaN();
        CHECK(!plant_pet_profile_valid(custom));
        CHECK(!model.setProfile(custom, 5000));
        custom = plant_pet_general_profile();
        custom.references[channel].high = std::numeric_limits<float>::infinity();
        CHECK(!plant_pet_profile_valid(custom));
        custom = plant_pet_general_profile();
        custom.references[channel].high = custom.references[channel].low + 1.0f;
        CHECK(!plant_pet_profile_valid(custom));
        custom = plant_pet_general_profile();
        custom.references[channel].low = -21.0f;
        CHECK(!plant_pet_profile_valid(custom));
        custom = plant_pet_general_profile();
        custom.references[channel].high = 101.0f;
        CHECK(!plant_pet_profile_valid(custom));
    }
    custom = plant_pet_general_profile();
    custom.version = PLANT_PET_PROFILE_VERSION + 1;
    CHECK(!plant_pet_profile_valid(custom));
}

void explicit_reconfirmation_resets_evidence_without_false_recovery() {
    PlantPetModel model;
    PlantPetInputs inputs = healthy();
    stamp_all(inputs, 1, 100);
    model.update(inputs, 100);
    inputs.valid[0] = false; // Custom soil reference requires calibration review.
    model.update(inputs, 200);
    CHECK(model.snapshot(200).mood == PlantMood::Unknown);
    inputs.valid[0] = true;
    inputs.values[0] = 10.0f;
    stamp_all(inputs, 2, 300);
    model.update(inputs, 300);
    CHECK(model.snapshot(300).bands[0] == PlantBand::Unknown);
    const uint32_t old_revision = model.snapshot(300).profile_revision;
    CHECK(model.setProfile(plant_pet_general_profile(), 400, true));
    CHECK(model.snapshot(400).profile_revision == old_revision + 1);
    model.update(inputs, 400); // Same fresh acquisition, re-evaluated once.
    CHECK(model.snapshot(400).bands[0] == PlantBand::Low);
    CHECK(model.snapshot(400).mood == PlantMood::Thirsty);
    CHECK(!model.snapshot(400).recovering);
    CHECK(model.snapshot(400).transition == 255);
    CHECK(model.setProfile(plant_pet_general_profile(), 500));
    CHECK(model.snapshot(500).profile_revision == old_revision + 1);
    CHECK(model.setProfile(plant_pet_general_profile(), 600, true));
    inputs.values[0] = 50.0f;
    stamp_all(inputs, 3, 600);
    model.update(inputs, 600);
    CHECK(model.snapshot(600).mood == PlantMood::Content);
    CHECK(!model.snapshot(600).recovering);
}
} // namespace

int main() {
    inclusive_thresholds_and_startup();
    noisy_thresholds_need_recent_evidence();
    recovery_is_gradual_but_not_persistent_growth();
    severity_and_culprit_hold_show_concurrent_needs();
    unknown_is_immediate_and_not_an_adverse_state();
    update_works_without_a_visible_screen();
    long_pauses_do_not_confirm_old_candidates();
    rollover_preserves_evidence_and_transitions();
    temperature_is_flexible_and_hysteretic();
    thermal_strength_progresses_without_counting_frames_as_evidence();
    duplicate_copies_cannot_confirm_a_band();
    acquisition_quality_and_expiry_are_independent_of_identity();
    darkness_is_calm_context_and_never_competes_for_attention();
    soil_and_air_gestures_are_gentle_and_progressive();
    recovery_interruption_preserves_the_effective_pose();
    profile_validation_activation_and_scale_are_independent();
    explicit_reconfirmation_resets_evidence_without_false_recovery();
    if (failures != 0) {
        std::fprintf(stderr, "%d Planta Lab model checks failed\n", failures);
        return 1;
    }
    std::puts("Planta Lab model: all native checks passed");
    return 0;
}
