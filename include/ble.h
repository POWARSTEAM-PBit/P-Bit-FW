#pragma once
#include <atomic>

extern std::atomic<bool> client_connected;

/**
 * @brief Initialize BLE services and characteristics
 */
void init_ble();

/**
 * @brief Notify all connected clients with updated data
 */
void notifyAll();

/**
 * @brief Request an immediate BLE notification from the safe service path.
 */
void ble_request_notify();

/**
 * @brief Service rate-limited BLE notifications from the sensor/runtime task.
 */
void ble_service();
