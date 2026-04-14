#pragma once

// Layout A: 2×2 card grid.
// Each card: icon + sensor label + large value + unit + colour-state dot.
// Live sensor data: temperature, humidity, light, sound.
void draw_lab_home_cards_screen(bool screen_changed, bool sensor_data_changed);
