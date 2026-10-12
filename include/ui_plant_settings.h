#pragma once

#include <stdint.h>
#include "settings_store.h"

enum PlantSettingsMenuState : uint8_t {
    PLANT_SETTINGS_NORMAL = 0,
    PLANT_SETTINGS_MENU,
    PLANT_SETTINGS_PROFILE,
    PLANT_SETTINGS_EDIT_REFERENCE,
    PLANT_SETTINGS_CONFIRM_SAVE,
    PLANT_SETTINGS_CONFIRM_RESET,
    PLANT_SETTINGS_SAVED,
    PLANT_SETTINGS_ERROR
};

struct PlantSettingsSnapshot {
    PlantSettingsData settings;
    uint32_t revision;
    bool soil_needs_review;
};

// Load once after hardware/NVS initialization, before UI/encoder tasks start.
void load_plant_settings();
PlantSettingsSnapshot get_plant_settings_snapshot();
PlantPetProfile get_active_plant_profile();
uint32_t plant_settings_revision();
bool plant_settings_soil_needs_review();

void start_plant_settings_menu();
bool plant_settings_menu_is_active();
PlantSettingsMenuState get_plant_settings_menu_state();
bool cancel_plant_settings_menu();
void draw_plant_settings_menu(bool screen_changed);
// Returns 1 only when leaving the menu, otherwise 0.
uint8_t handle_plant_settings_button();
void set_plant_settings_encoder_value(int value);
int get_plant_settings_encoder_value();
int get_plant_settings_encoder_min();
int get_plant_settings_encoder_max();
bool plant_settings_encoder_is_circular();
