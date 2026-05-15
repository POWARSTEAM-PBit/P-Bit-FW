#pragma once

// Draw the graph screen (called from the UI router task).
void draw_graph_screen(bool screen_changed, bool sensor_data_changed);

// Cycle to the next sensor shown in the graph (called from rotary short press).
// The graph carousel covers Temp, Hum, Light, Sound, Soil and DS18.
void graph_cycle_sensor();
