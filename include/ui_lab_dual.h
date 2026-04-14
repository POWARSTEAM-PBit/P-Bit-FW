#pragma once

// Temporary lab screen: dual Temp + Humidity view.
// Self-contained module so it can be integrated or removed without touching
// the rest of the UI family.
void draw_lab_dual_th_screen(bool screen_changed, bool sensor_data_changed);
