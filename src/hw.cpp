#include <Arduino.h>
#include <esp_system.h>
#include "hw.h"

uint8_t mac[MAC_LEN];
char dev_name[MAX_DEVICE_NAME_LEN];

void set_devicename() {
    esp_read_mac(mac, ESP_MAC_BT);
    // Make sure MAC has been read first
    snprintf(dev_name, sizeof(dev_name), "PBIT-%02X%02X", mac[4], mac[5]);
    Serial.printf("[BOOT] DevName: %s\n", dev_name);
}

int readADC(uint8_t pin) {
    uint32_t sum = 0;
    int samples = 16;
    
    if (pin >= 34) {
        pinMode(pin, INPUT);
    }
    
    for (int i = 0; i < samples; i++) {
        sum += analogRead(pin);
        delayMicroseconds(200);
    }
    
    return sum / samples; // 0..4095
}