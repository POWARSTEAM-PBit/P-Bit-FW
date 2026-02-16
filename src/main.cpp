#include <Arduino.h>
#include "tft_display.h"
#include "io.h"
#include "config.h"
#include "ble.h"
#include "rotary.h"
#include "hw.h"

#define SERIAL_BAUD_RATE 115200


void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    set_devicename();
    init_tft_display();
    init_io();
    init_ble();
    init_rotary();
    
    xTaskCreate(
        switch_screen,   // Task function
        "SwitchScreen",  // Name
        4096,            // Stack size (bytes)
        NULL,            // Parameters
        1,               // Priority (Arduino usually runs at priority 1)
        NULL             // Task handle (not used)
    );
}

void loop() {

    rotaryEncoder.loop();
    notifyAll();

    delay(pdTICKS_TO_MS(SENSOR_READ_INTERVAL)); // Keep loop alive and aligned with sensor interval
}
