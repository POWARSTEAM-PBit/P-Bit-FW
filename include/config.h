#pragma once // <-- ¡ESTA ES LA LÍNEA QUE ARREGLA EL ERROR!

#include <Arduino.h>

/**
 * @brief Interval for reading sensor data
 */
constexpr TickType_t SENSOR_READ_INTERVAL = pdMS_TO_TICKS(1000); // 1 segundono v