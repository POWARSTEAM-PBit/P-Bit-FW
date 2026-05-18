#pragma once

#include <stdint.h>

void draw_lab_gauge_temp_screen(bool screen_changed, bool sensor_data_changed);
void lab_gauge_cycle_sensor();
void lab_gauge_set_sensor(uint8_t sensor_id); // jump to specific GaugeLabSensor
void draw_lab_value_modern_screen(bool screen_changed, bool sensor_data_changed);
void lab_value_cycle_sensor();
void lab_value_set_sensor(uint8_t sensor_id); // jump to specific ValueLabSensor
void draw_lab_widget_mix_screen(bool screen_changed, bool sensor_data_changed);
