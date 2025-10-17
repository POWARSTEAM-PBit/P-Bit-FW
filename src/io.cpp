#include <Arduino.h>
#include "queue.h"
#include "config.h"
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "io.h"

#define DHT_TYPE DHT11        // change to DHT22 if needed
constexpr uint8_t PIN_DHT = 4;   // DHT11 data
constexpr uint8_t PIN_ONEWIRE  = 32;  // DS18B20 data (optional)
constexpr uint8_t PIN_LDR      = 39;  // ADC1 (SENSOR_VN)
constexpr uint8_t PIN_MIC      = 36;  // ADC1 (SENSOR_VP)


DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

int readADC(uint8_t pin, int samples = 16) {
  if (pin >= 34) pinMode(pin, INPUT);
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  return sum / samples; // 0..4095
}

void read_sensors(Reading &r) {

    dht.begin();
    ds18b20.begin();
    
    // DHT
    r.humidity = dht.readHumidity();
    r.temperature = dht.readTemperature();

    // DS18B20 (if present) overrides temperature
    ds18b20.requestTemperatures();
    float t2 = ds18b20.getTempCByIndex(0);
    if (t2 != DEVICE_DISCONNECTED_C && t2 > -100 && t2 < 120) {
        r.temperature = t2;
    }

    // Analog sensors
    r.ldr = readADC(PIN_LDR);
    r.mic = readADC(PIN_MIC);

    // Battery placeholder (if no ADC pin for battery voltage)
    r.batt = 100.0f;
}

void io_rec(void *param) {
    Reading new_reading;

    while (1) {
        read_sensors(new_reading);

        if (!isnan(new_reading.temperature) && !isnan(new_reading.humidity)) {
            xQueueSend(reading_queue, &new_reading, portMAX_DELAY);
        } else {
            Serial.println("Sensor read failed (NaN values)");
        }

        vTaskDelay(SENSOR_READ_INTERVAL);
    }
}
