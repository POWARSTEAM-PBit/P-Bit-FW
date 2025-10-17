#pragma once

#include <Arduino.h>   // for uint8_t

#define MAC_LEN 6
#define MAX_DEVICE_NAME_LEN 20

extern uint8_t mac[MAC_LEN];
extern char dev_name[MAX_DEVICE_NAME_LEN];

void set_devicename();
int readADC(uint8_t pin);
