#pragma once
#include "tft_display.h"

// Internal states for the System screen.
typedef enum {
    SYS_MODE_NORMAL,
    SYS_MODE_MENU,
    SYS_MODE_EDIT_SOUND,
    SYS_MODE_EDIT_SLEEP,
    SYS_MODE_EDIT_LANG,
    SYS_MODE_CONFIRM_RESET,
    SYS_MODE_SAVED
} SysMenuState;

// Public API
void draw_system_screen(bool screen_changed, bool data_changed);
void start_system_menu();
bool system_menu_is_active();
SysMenuState get_system_menu_state();
uint8_t handle_system_button();
void set_system_input_value(int value);
int get_system_encoder_value();
int get_system_encoder_min();
int get_system_encoder_max();
