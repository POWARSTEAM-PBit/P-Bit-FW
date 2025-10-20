#pragma once

#include <Arduino.h>

constexpr size_t MAC_LEN = 6;
constexpr size_t MAX_DEVICE_NAME_LEN = 20;

/**
 * @brief MAC address of the device
 */
extern uint8_t mac[MAC_LEN];

/**
 * @brief Device name based on MAC address
 */
extern char dev_name[MAX_DEVICE_NAME_LEN];

/**
 * @brief Set the device name based on MAC address
 */
void set_devicename();

/**
 * @brief Read the ADC value from the specified pin
 */
int readADC(uint8_t pin);
