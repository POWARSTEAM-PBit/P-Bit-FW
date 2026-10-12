// Native: g++ -std=c++11 -Wall -Wextra -Werror -Itest/native_plant_settings -Iinclude
// src/plant_pet.cpp src/settings_store.cpp src/ui_plant_settings.cpp test/plant_settings_native.cpp
#include "ui_plant_settings.h"
#include "fonts.h"
#include "languages.h"
#include "ui_widgets.h"
#include <Preferences.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

std::map<std::string, std::vector<uint8_t>> native_prefs;
unsigned native_writes = 0;
unsigned native_full_clears = 0;
bool native_write_failure = false;
bool g_is_fahrenheit = false;
TFT_eSPI tft;
const GFXfont Roboto_Regular7pt8b{};
const GFXfont Roboto_Light6pt8b{};
const GFXfont Roboto_Medium10pt8b{};
const GFXfont IBMPlexSans_Regular9pt8b{};
const GFXfont IBMPlexMono_Regular12pt8b{};
const GFXfont IBMPlexMono_Regular24pt8b{};
SoilCalibrationData native_calibration{3550, 1855};
SoilCalibrationData get_current_soil_calibration() { return native_calibration; }
const char* L(LangKey) { return "key"; }
void runtime_request_ui_refresh(bool) {}
void drawHeader(const char*) {}
void drawFooterHint(const char*,int,int,uint16_t) {}
void drawSettingsGridMenu(const char* const*,uint8_t,uint8_t,const char*,const char*,bool,int) {}
void drawCenteredMenuFrame(const char*,uint16_t,const char*,uint16_t) {}
void drawCenteredMenuValueScreen(const char*,const char*,uint16_t,MenuValueFont,const char*,bool,uint16_t) {}
void drawCenteredMenuSavedScreen(const char*,const char*,uint16_t,MenuValueFont,const char*,uint16_t) {}
void drawCenteredMenuBodyLines(const char* const*,const uint16_t*,uint8_t,MenuTextFont,int,int) {}
void drawResetChoicePromptShell(const char*,const char*,const char*,const char*,uint16_t) {}
void updateResetChoiceButtons(const char*,const char*,uint8_t) {}

void press() { handle_plant_settings_button(); }
void choose(int value) { set_plant_settings_encoder_value(value); }
void open_personalize() { start_plant_settings_menu(); choose(1); press(); }
void review_remaining_fields(unsigned count) { while (count--) press(); }
void save_draft() { assert(get_plant_settings_menu_state() == PLANT_SETTINGS_CONFIRM_SAVE); choose(1); press(); }
void switch_profile(int mode) { start_plant_settings_menu(); press(); choose(mode); press(); save_draft(); }

void rewrite_checksum(std::vector<uint8_t>& bytes) {
    uint32_t hash = 2166136261u;
    for (size_t i=0; i<bytes.size()-4; ++i) { hash ^= bytes[i]; hash *= 16777619u; }
    std::memcpy(bytes.data()+bytes.size()-4, &hash, 4);
}

int main() {
    load_plant_settings();
    assert(get_plant_settings_snapshot().settings.mode == PlantProfileMode::General);
    start_plant_settings_menu();
    assert(get_plant_settings_encoder_max() == 3);
    draw_plant_settings_menu(true);
    const unsigned clears = native_full_clears;
    choose(1); draw_plant_settings_menu(false);
    assert(native_full_clears == clears); // Ordinary encoder navigation keeps shell.
    const unsigned baseline_writes = native_writes;
    press(); choose(35);
    assert(get_plant_settings_snapshot().settings.mode == PlantProfileMode::General);
    assert(native_writes == baseline_writes);
    assert(cancel_plant_settings_menu());
    assert(native_writes == baseline_writes);
    assert(get_active_plant_profile().references[0].low == 30.0f);

    open_personalize(); choose(35); review_remaining_fields(6);
    // Confirmation defaults to Cancel, so a plain press discards the draft.
    assert(get_plant_settings_encoder_value() == 0); press();
    assert(native_writes == baseline_writes);
    open_personalize(); choose(35); review_remaining_fields(6); save_draft();
    assert(get_plant_settings_menu_state() == PLANT_SETTINGS_SAVED);
    assert(native_writes == baseline_writes + 1);
    assert(native_prefs.at("plant_cfg").size() == 36);
    assert(get_active_plant_profile().references[0].low == 35.0f);
    assert(!plant_settings_soil_needs_review());
    auto custom = load_plant_settings_store();
    assert(custom.mode == PlantProfileMode::Custom && custom.custom_profile.references[0].low == 35.0f);

    switch_profile(0);
    assert(get_active_plant_profile().references[0].low == 30.0f);
    assert(get_plant_settings_snapshot().settings.custom_profile.references[0].low == 35.0f);
    switch_profile(1);
    assert(get_active_plant_profile().references[0].low == 35.0f);

    const uint32_t old_revision = plant_settings_revision();
    native_calibration = {3600,1900};
    assert(plant_settings_soil_needs_review());
    assert(plant_settings_revision() > old_revision);
    switch_profile(0); switch_profile(1);
    assert(plant_settings_soil_needs_review()); // Selecting Custom does not rebind.
    open_personalize(); review_remaining_fields(6); save_draft();
    assert(!plant_settings_soil_needs_review());
    assert(load_plant_settings_store().soil_dry_raw == 3600);

    // Moving an interval above its old high reference must take only one
    // wizard pass, and must not publish/write the automatically raised partner.
    const unsigned range_writes = native_writes;
    const auto before_range = get_active_plant_profile();
    open_personalize();
    assert(get_plant_settings_encoder_max() == 95);
    choose(90); press();
    assert(get_plant_settings_encoder_value() == 95);
    assert(get_plant_settings_encoder_min() == 95);
    choose(95); press();
    assert(get_plant_settings_encoder_max() == 78);
    choose(32); press();
    assert(get_plant_settings_encoder_value() == 34);
    choose(35); press(); press(); press();
    assert(native_writes == range_writes);
    assert(get_active_plant_profile().references[0].low == before_range.references[0].low);
    assert(get_active_plant_profile().references[1].high == before_range.references[1].high);
    save_draft();
    assert(native_writes == range_writes + 1);
    assert(get_active_plant_profile().references[0].low == 90.0f);
    assert(get_active_plant_profile().references[0].high == 95.0f);
    assert(get_active_plant_profile().references[1].low == 32.0f);
    assert(get_active_plant_profile().references[1].high == 35.0f);
    assert(plant_pet_profile_valid(get_active_plant_profile()));

    g_is_fahrenheit = true;
    open_personalize(); press(); press(); // Temperature low.
    assert(get_plant_settings_encoder_min() == -4);
    choose(60); press(); // 60 F stores exactly (60-32)/1.8 C.
    assert(get_plant_settings_encoder_min() >= 64);
    choose(88); press(); press(); press(); save_draft();
    const PlantPetProfile fahrenheit_profile = get_active_plant_profile();
    assert(std::fabs(fahrenheit_profile.references[1].low - 28.0f/1.8f) < 0.0001f);
    assert(std::fabs(fahrenheit_profile.references[1].high - 56.0f/1.8f) < 0.0001f);
    g_is_fahrenheit = false;

    // A failed NVS write must leave the active profile unchanged.
    const auto prior = get_plant_settings_snapshot();
    native_write_failure = true;
    open_personalize(); choose(40); review_remaining_fields(6); save_draft();
    assert(get_plant_settings_menu_state() == PLANT_SETTINGS_ERROR);
    assert(get_plant_settings_snapshot().revision == prior.revision);
    assert(get_active_plant_profile().references[0].low == prior.settings.custom_profile.references[0].low);
    native_write_failure = false;

    auto valid_blob = native_prefs.at("plant_cfg");
    native_prefs["plant_cfg"][35] ^= 1;
    assert(load_plant_settings_store().mode == PlantProfileMode::General);
    native_prefs["plant_cfg"] = valid_blob; native_prefs["plant_cfg"][0] = 2; rewrite_checksum(native_prefs["plant_cfg"]);
    assert(load_plant_settings_store().mode == PlantProfileMode::General);
    native_prefs["plant_cfg"] = valid_blob; native_prefs["plant_cfg"][1] = 2; rewrite_checksum(native_prefs["plant_cfg"]);
    assert(load_plant_settings_store().mode == PlantProfileMode::General);
    native_prefs["plant_cfg"] = valid_blob; native_prefs["plant_cfg"].pop_back();
    assert(load_plant_settings_store().mode == PlantProfileMode::General);
    native_prefs["plant_cfg"] = valid_blob;
    auto invalid = load_plant_settings_store();
    invalid.custom_profile.references[1].high = invalid.custom_profile.references[1].low;
    assert(!save_plant_settings_store(invalid));
    invalid = load_plant_settings_store(); invalid.custom_profile.references[0].low = NAN;
    assert(!save_plant_settings_store(invalid));

    // Reset is isolated: soil RAW calibration and sensor alarm keys survive.
    save_soil_calibration_store(3600,1900); save_temp_settings_store(18,30);
    start_plant_settings_menu(); choose(2); press(); choose(1); press();
    assert(get_active_plant_profile().references[0].low == 30.0f);
    assert(get_plant_settings_snapshot().settings.mode == PlantProfileMode::General);
    assert(native_prefs.count("plant_cfg") == 0);
    assert(load_soil_calibration_store(0,0).dry_raw == 3600);
    assert(load_temp_settings_store(0,0,false,false).low_alarm == 18);
    start_plant_settings_menu(); choose(3); assert(handle_plant_settings_button() == 1);
    assert(!plant_settings_menu_is_active());
    std::puts("Planta Lab settings: draft/save/cancel, F conversion, calibration review, corruption/failure and isolated Reset PASS");
}
