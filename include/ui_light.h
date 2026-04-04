#pragma once
#include "tft_display.h"

// Internal states for the Light screen.
typedef enum {
    LIGHT_MODE_NORMAL,
    LIGHT_MODE_MENU,
    LIGHT_MODE_EDIT_DIM,
    LIGHT_MODE_EDIT_INDOOR,
    LIGHT_MODE_EDIT_BRIGHT,
    LIGHT_MODE_EDIT_DISPLAY,
    LIGHT_MODE_EDIT_ALERTS,
    LIGHT_MODE_CONFIRM_RESET,
    LIGHT_MODE_SAVED
} LightMenuState;

// Public API
void draw_light_screen(bool screen_changed, bool data_changed);
void start_light_menu();
bool light_menu_is_active();
LightMenuState get_light_menu_state();
uint8_t handle_light_button();
void set_light_input_value(int value);
int get_light_encoder_value();
int get_light_encoder_min();
int get_light_encoder_max();
