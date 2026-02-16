#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "io.h"
#include "hw.h"

#define DHT_TYPE DHT11        // change to DHT22 if needed
constexpr uint8_t PIN_DHT = 4;   // DHT11 data
constexpr uint8_t PIN_ONEWIRE  = 32;  // DS18B20 data (optional)
constexpr uint8_t PIN_LDR      = 39;  // ADC1 (SENSOR_VN)
constexpr uint8_t PIN_MIC      = 36;  // ADC1 (SENSOR_VP)


DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

static bool io_initialized = false;
static Reading cached_reading{};
static unsigned long last_sensor_read_ms = 0;
static constexpr unsigned long SENSOR_READ_INTERVAL_MS = pdTICKS_TO_MS(SENSOR_READ_INTERVAL);

void init_io() {
    if (io_initialized) return;

    dht.begin();
    ds18b20.begin();
    io_initialized = true;
}

void read_sensors(Reading &r) {
    
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

void get_sensor_reading(Reading &r, bool forceRefresh) {
    if (!io_initialized) {
        init_io();
    }

    const unsigned long now = millis();
    const bool stale = !last_sensor_read_ms || (now - last_sensor_read_ms >= SENSOR_READ_INTERVAL_MS);

    if (forceRefresh || stale) {
        read_sensors(cached_reading);
        last_sensor_read_ms = now;
    }

    r = cached_reading;
}
