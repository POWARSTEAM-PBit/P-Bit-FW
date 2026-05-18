#pragma once

#include <stdint.h>

void lab_sensor_card_cycle();
void lab_sensor_card_set_sensor(uint8_t card_id); // jump to specific LabSensorCardId
void draw_lab_sensor_card_screen(bool screen_changed, bool sensor_data_changed);
void draw_lab_temp_card_screen(bool screen_changed, bool sensor_data_changed);
void draw_lab_ds18_card_screen(bool screen_changed, bool sensor_data_changed);
