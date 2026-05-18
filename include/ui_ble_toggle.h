#pragma once
#include <Arduino.h>

void init_ble_toggle_screen();
void draw_ble_toggle_screen(bool screen_changed, bool data_changed);

int     get_ble_toggle_encoder_min();
int     get_ble_toggle_encoder_max();
int     get_ble_toggle_encoder_value();
void    set_ble_toggle_input_value(int value);
uint8_t handle_ble_toggle_button();
