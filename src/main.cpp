#include <Arduino.h>
#include "tft_display.h"
#include "io.h"
#include "ble.h"
#include "rotary.h"
#include "hw.h"
#include "ldr.h"          // LDR -> lux module

#define SERIAL_BAUD_RATE 115200

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    set_devicename();     // set BLE/device name (your existing code)
    init_tft_display();   // init TFT screen
    set_devicename();     // (kept as in your code)

    ldr_init();           // init ADC for LDR (needed for lux conversion)

    init_ble();           // init BLE server/services
    init_rotary();        // init rotary encoder

    // Create a FreeRTOS task to switch screens
    xTaskCreate(
        switch_screen,   // task function
        "SwitchScreen",  // task name
        4096,            // stack size (bytes)
        NULL,            // parameters
        1,               // priority (Arduino default is 1)
        NULL             // task handle (not used)
    );
}

void loop() {
    // handle rotary encoder events
    rotaryEncoder.loop();

    // notify BLE clients with latest sensor data
    notifyAll();

    // simple delay to avoid spamming BLE and keep loop alive
    delay(2000);
}
