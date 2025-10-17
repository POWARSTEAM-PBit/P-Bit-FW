#include <Arduino.h>
#include "tft_display.h"
#include "io.h"
#include "queue.h"
#include "ble.h"

#define SERIAL_BAUD_RATE 115200


void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    init_tft_display();
    init_queue();
    init_ble();

    // Create FreeRTOS task for screen switching
    xTaskCreate(
        switch_screen,   // Task function
        "SwitchScreen",  // Name
        4096,            // Stack size (bytes)
        NULL,            // Parameters
        1,               // Priority (Arduino usually runs at priority 1)
        NULL             // Task handle (not used)
    );
    xTaskCreate(
        io_rec,   // Task function
        "IOReading",  // Name
        4096,            // Stack size (bytes)
        NULL,            // Parameters
        1,               // Priority (Arduino usually runs at priority 1)
        NULL             // Task handle (not used)
    );
}

void loop() {
    // Nothing needed here, all handled by FreeRTOS task
    Reading received;
    if (xQueueReceive(reading_queue, &received, pdMS_TO_TICKS(1000))) {
        Serial.printf("Humidity: %.1f %% | Temp: %.1f °C | LDR: %.0f | Mic: %.0f | Batt: %.1f %%\n",
                      received.humidity, received.temperature,
                      received.ldr, received.mic, received.batt);
    }
    delay(1000); // Just to keep loop alive
}