#pragma once

#include "io.h"
#include <stdint.h>

// UI-task service: feed the appropriate real/demo model on each new snapshot.
void update_lab_plant_readings(const Reading& reading, uint32_t now_ms, bool demo);
void draw_lab_plant_screen(bool screen_changed, bool sensor_data_changed);
