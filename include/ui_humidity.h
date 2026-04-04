#pragma once
#include "tft_display.h"

// Internal states for the Humidity screen.
typedef enum {
    HUM_MODE_NORMAL,
    HUM_MODE_MENU,
    HUM_MODE_EDIT_DRY,
    HUM_MODE_EDIT_COMFORT,
    HUM_MODE_EDIT_ALERTS,
    HUM_MODE_CONFIRM_RESET,
    HUM_MODE_SAVED
} HumidityMenuState;

// Public API for the humidity screen and menu.
void draw_humidity_screen(bool screen_changed, bool data_changed);
void start_humidity_menu();
bool humidity_menu_is_active();
HumidityMenuState get_humidity_menu_state();
uint8_t handle_humidity_button();
void set_humidity_input_value(int value);
int get_humidity_encoder_value();
int get_humidity_encoder_min();
int get_humidity_encoder_max();
