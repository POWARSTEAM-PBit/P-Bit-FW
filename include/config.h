#include <Arduino.h>

/**
 * @brief Interval for reading sensor data
 */
constexpr TickType_t SENSOR_READ_INTERVAL = pdMS_TO_TICKS(20000); // 2 seconds
