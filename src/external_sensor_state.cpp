#include "external_sensor_state.h"

#include <Arduino.h>
#include <math.h>

#include "palette.h"

namespace {

uint16_t blend565(uint16_t a, uint16_t b, uint8_t amount_b) {
    const uint16_t ar = (a >> 11) & 0x1F;
    const uint16_t ag = (a >> 5) & 0x3F;
    const uint16_t ab = a & 0x1F;
    const uint16_t br = (b >> 11) & 0x1F;
    const uint16_t bg = (b >> 5) & 0x3F;
    const uint16_t bb = b & 0x1F;
    const uint16_t amount_a = 255 - amount_b;
    const uint16_t r = (ar * amount_a + br * amount_b) / 255;
    const uint16_t g = (ag * amount_a + bg * amount_b) / 255;
    const uint16_t bl = (ab * amount_a + bb * amount_b) / 255;
    return (uint16_t)((r << 11) | (g << 5) | bl);
}

} // namespace

bool pbit_external_sensor_missing(SzSensorId sensor, const Reading& reading) {
    switch (sensor) {
        case SZ_SOIL:
            return isnan(reading.soil_humidity);
        case SZ_DS18:
            return reading.temp_ds18b20 < -100.0f;
        default:
            return false;
    }
}

bool pbit_external_sensor_has_port_hint(SzSensorId sensor) {
    return sensor == SZ_SOIL || sensor == SZ_DS18;
}

LangKey pbit_external_sensor_check_key(SzSensorId sensor) {
    switch (sensor) {
        case SZ_SOIL:
            return ST_CHECK_SOIL;
        case SZ_DS18:
            return ST_CHECK_DS18;
        default:
            return ST_NO_SENSOR;
    }
}

uint16_t pbit_color_dim(uint16_t color) {
    return blend565(color, 0x0000, 112);
}

uint16_t pbit_external_dim_primary(SzSensorId sensor) {
    return pbit_color_dim(pb_primary((uint8_t)sensor));
}

uint16_t pbit_external_dim_secondary(SzSensorId sensor) {
    return pbit_color_dim(pb_secondary((uint8_t)sensor));
}

uint16_t pbit_external_dim_bg(SzSensorId sensor) {
    return blend565(0x0000, pb_primary((uint8_t)sensor), 28);
}
