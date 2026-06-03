#pragma once

#include <stdint.h>

#include "io.h"
#include "languages.h"
#include "sensor_zone.h"

bool pbit_external_sensor_missing(SzSensorId sensor, const Reading& reading);
bool pbit_external_sensor_has_port_hint(SzSensorId sensor);
LangKey pbit_external_sensor_check_key(SzSensorId sensor);

uint16_t pbit_color_dim(uint16_t color);
uint16_t pbit_external_dim_primary(SzSensorId sensor);
uint16_t pbit_external_dim_secondary(SzSensorId sensor);
uint16_t pbit_external_dim_bg(SzSensorId sensor);
