#pragma once

#include <stdint.h>

// Draw the graph screen (called from the UI router task).
void draw_graph_screen(bool screen_changed, bool sensor_data_changed);

// Cycle to the next sensor shown in the graph (called from rotary short press).
// The graph carousel covers Temp, Hum, Light, Sound, Soil and DS18.
void graph_cycle_sensor();

// Jump directly to a sensor (called from sensor zone sync).
// GraphSensor enum order matches SzSensorId (TEMP=0..DS18=5) — cast directly.
void graph_set_sensor(uint8_t sensor_id);
