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
