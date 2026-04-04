#pragma once
#include "tft_display.h"

// Internal states for the DS18B20 screen.
typedef enum {
    DS18_MODE_NORMAL,
    DS18_MODE_MENU,
    DS18_MODE_EDIT_OFFSET,
    DS18_MODE_EDIT_LOW,
    DS18_MODE_EDIT_HIGH,
    DS18_MODE_EDIT_UNIT,
    DS18_MODE_EDIT_ALERTS,
    DS18_MODE_CONFIRM_RESET,
    DS18_MODE_SAVED
} Ds18MenuState;

// Public API
void draw_ds18_screen(bool screen_changed, bool data_changed);
void start_ds18_menu();
bool ds18_menu_is_active();
Ds18MenuState get_ds18_menu_state();
uint8_t handle_ds18_button();
void set_ds18_input_value(int value);
int get_ds18_encoder_value();
int get_ds18_encoder_min();
int get_ds18_encoder_max();

