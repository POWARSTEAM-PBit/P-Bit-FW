#pragma once
#include "tft_display.h"

// Internal states for the Sound screen.
typedef enum {
    SOUND_MODE_NORMAL,
    SOUND_MODE_MENU,
    SOUND_MODE_EDIT_QUIET,
    SOUND_MODE_EDIT_NORMAL,
    SOUND_MODE_EDIT_LOUD,
    SOUND_MODE_EDIT_ALERTS,
    SOUND_MODE_CONFIRM_RESET,
    SOUND_MODE_SAVED
} SoundMenuState;

// Public API
void draw_sound_screen(bool screen_changed, bool data_changed);
void start_sound_menu();
bool sound_menu_is_active();
SoundMenuState get_sound_menu_state();
uint8_t handle_sound_button();
void set_sound_input_value(int value);
int get_sound_encoder_value();
int get_sound_encoder_min();
int get_sound_encoder_max();

