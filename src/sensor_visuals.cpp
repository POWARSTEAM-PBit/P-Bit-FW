#include "sensor_visuals.h"

#include <TFT_eSPI.h>
#include <math.h>

#include "palette.h"

namespace {

static bool valid_number(float value) {
    return !isnan(value) && isfinite(value);
}

static float to_display_temp(float temp_c, bool fahrenheit) {
    return fahrenheit ? (temp_c * 1.8f + 32.0f) : temp_c;
}

static uint16_t ds18_thermal_color(uint8_t amount) {
    if (amount <= 50) {
        return pbit_mix565(TFT_WHITE, PB_DS18_P4, (uint8_t)((uint16_t)amount * 255U / 50U));
    }
    if (amount <= 78) {
        return pbit_mix565(PB_DS18_P4, PB_DS18_P2, (uint8_t)(((uint16_t)amount - 50U) * 255U / 28U));
    }
    if (amount <= 113) {
        return pbit_mix565(PB_DS18_P2, PB_LUZ_P1, (uint8_t)(((uint16_t)amount - 78U) * 255U / 35U));
    }
    if (amount <= 177) {
        return pbit_mix565(PB_LUZ_P1, PB_LUZ_P2, (uint8_t)(((uint16_t)amount - 113U) * 255U / 64U));
    }
    return pbit_mix565(PB_LUZ_P2, TFT_RED, (uint8_t)(((uint16_t)amount - 177U) * 255U / 78U));
}

} // namespace

uint16_t pbit_mix565(uint16_t a, uint16_t b, uint8_t amount_b) {
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

uint16_t pbit_mix3_565(uint16_t low, uint16_t mid, uint16_t high, uint8_t amount) {
    if (amount <= 127) {
        return pbit_mix565(low, mid, (uint8_t)((uint16_t)amount * 2U));
    }
    return pbit_mix565(mid, high, (uint8_t)(((uint16_t)amount - 128U) * 2U + 1U));
}

uint8_t pbit_ratio_to_amount(float value, float min_value, float max_value) {
    if (max_value <= min_value || !valid_number(value)) return 0;
    const float ratio = constrain((value - min_value) / (max_value - min_value), 0.0f, 1.0f);
    return (uint8_t)roundf(ratio * 255.0f);
}

void pbit_rgb565_to_rgb888(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
    const uint8_t r5 = (color >> 11) & 0x1F;
    const uint8_t g6 = (color >> 5) & 0x3F;
    const uint8_t b5 = color & 0x1F;
    r = (uint8_t)((r5 * 255U + 15U) / 31U);
    g = (uint8_t)((g6 * 255U + 31U) / 63U);
    b = (uint8_t)((b5 * 255U + 15U) / 31U);
}

uint16_t pbit_sensor_gauge_arc_color(SzSensorId sensor, uint8_t amount) {
    switch (sensor) {
        case SZ_SOUND:
            return pbit_mix565(PB_SOUND_P2, PB_SOUND_P3, amount);
        case SZ_TEMP:
            return pbit_mix3_565(PB_TEMP_P4, TFT_YELLOW, TFT_RED, amount);
        case SZ_SOIL:
            return pbit_mix3_565(TFT_YELLOW, PB_SOIL_P1, PB_HUM_P2, amount);
        case SZ_HUM:
            return pbit_mix565(TFT_WHITE, PB_HUM_P2, amount);
        case SZ_DS18:
            return ds18_thermal_color(amount);
        case SZ_LIGHT:
            return pbit_mix3_565(PB_LUZ_P4, PB_LUZ_P1, PB_LUZ_P3, amount);
        default:
            return pbit_mix565(pb_contrast_cool((uint8_t)sensor),
                               pb_accent_warm((uint8_t)sensor),
                               amount);
    }
}

uint16_t pbit_sensor_visual_color(SzSensorId sensor,
                                  const Reading& reading,
                                  bool fahrenheit,
                                  bool* out_valid) {
    bool valid = false;
    uint16_t color = TFT_DARKGREY;

    switch (sensor) {
        case SZ_TEMP: {
            valid = valid_number(reading.temperature);
            const float value = to_display_temp(reading.temperature, fahrenheit);
            const float min_v = to_display_temp(0.0f, fahrenheit);
            const float max_v = to_display_temp(50.0f, fahrenheit);
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(value, min_v, max_v));
            break;
        }
        case SZ_HUM:
            valid = valid_number(reading.humidity);
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(reading.humidity, 0.0f, 100.0f));
            break;
        case SZ_LIGHT:
            valid = valid_number(reading.ldr);
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(reading.ldr, 0.0f, 20000.0f));
            break;
        case SZ_SOUND:
            valid = valid_number(reading.mic);
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(reading.mic, 0.0f, 100.0f));
            break;
        case SZ_SOIL:
            valid = valid_number(reading.soil_humidity);
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(reading.soil_humidity, 0.0f, 100.0f));
            break;
        case SZ_DS18:
            valid = valid_number(reading.temp_ds18b20) && reading.temp_ds18b20 >= -100.0f;
            color = pbit_sensor_gauge_arc_color(sensor, pbit_ratio_to_amount(reading.temp_ds18b20, -55.0f, 125.0f));
            break;
        default:
            valid = false;
            color = TFT_DARKGREY;
            break;
    }

    if (out_valid) *out_valid = valid;
    return valid ? color : TFT_DARKGREY;
}

uint16_t pbit_timer_visual_color(bool finished_active,
                                 bool running,
                                 unsigned long elapsed_ms,
                                 bool flash_on) {
    if (finished_active) {
        return flash_on ? PB_SOUND_P3 : PB_SOUND_P1;
    }
    if (running) {
        return PB_SOUND_P2;
    }
    if (elapsed_ms > 0) {
        return PB_TEMP_P1;
    }
    return PB_DS18_P2;
}
