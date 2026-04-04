#pragma once
#include "tft_display.h"

// Internal states for the Temperature screen.
typedef enum {
    TEMP_MODE_NORMAL,
    TEMP_MODE_MENU,
    TEMP_MODE_EDIT_LOW,
    TEMP_MODE_EDIT_HIGH,
    TEMP_MODE_EDIT_UNIT,
    TEMP_MODE_EDIT_ALERTS,
    TEMP_MODE_CONFIRM_RESET,
    TEMP_MODE_SAVED
} TempMenuState;

// Public API
void draw_temp_screen(bool screen_changed, bool data_changed);
void start_temp_menu();
bool temp_menu_is_active();
TempMenuState get_temp_menu_state();
uint8_t handle_temp_button();
void set_temp_input_value(int value);
int get_temp_encoder_value();
int get_temp_encoder_min();
int get_temp_encoder_max();
