#pragma once

// Two-page icon-size gallery.
// ENV page: temperature, humidity, light  (environment sensors)
// EXT page: sound, soil, probe            (activity / external sensors)
void draw_lab_icon_sizes_env_screen(bool screen_changed, bool sensor_data_changed);
void draw_lab_icon_sizes_ext_screen(bool screen_changed, bool sensor_data_changed);
