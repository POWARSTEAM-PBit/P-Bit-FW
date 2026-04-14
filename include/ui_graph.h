#pragma once

// Draw the graph screen (called from the UI router task).
void draw_graph_screen(bool screen_changed, bool sensor_data_changed);

// Cycle to the next sensor shown in the graph (called from rotary short press).
void graph_cycle_sensor();
