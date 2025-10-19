#pragma once

extern bool clientConnected;

/**
 * @brief Initialize BLE services and characteristics
 */
void init_ble();

/**
 * @brief Notify all connected clients with updated data
 */
void notifyAll();