#include <Arduino.h>
#include "io.h"
#include "queue.h"

#define MAX_QUEUE_SIZE 10

QueueHandle_t reading_queue;

void init_queue() {
    reading_queue = xQueueCreate(MAX_QUEUE_SIZE, sizeof(Reading));

    if (reading_queue == nullptr) {
        Serial.println("Failed to create queue!");
        while (true);
    }
}
