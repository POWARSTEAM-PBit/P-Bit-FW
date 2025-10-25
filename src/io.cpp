#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "io.h"
#include "hw.h"
#include "ldr.h"   // LDR -> lux helper

#define DHT_TYPE DHT11          // change to DHT22 if needed
constexpr uint8_t PIN_DHT     = 4;   // DHT11 data
constexpr uint8_t PIN_ONEWIRE = 32;  // DS18B20 data (optional)
constexpr uint8_t PIN_LDR     = 39;  // ADC1 (SENSOR_VN)
constexpr uint8_t PIN_MIC     = 36;  // ADC1 (SENSOR_VP)

DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

// Read all sensors and fill the Reading struct
void read_sensors(Reading &r) {
    // Start DHT & DS18B20 (simple way; you can move to setup() for speed)
    dht.begin();
    ds18b20.begin();

    // --- DHT: humidity & temperature ---
    r.humidity    = dht.readHumidity();
    r.temperature = dht.readTemperature();

    // --- DS18B20: override temperature if valid ---
    ds18b20.requestTemperatures();
    float t2 = ds18b20.getTempCByIndex(0);
    if (t2 != DEVICE_DISCONNECTED_C && t2 > -100 && t2 < 120) {
        r.temperature = t2;
    }

    // --- LDR: convert to lux (float) and store as integer for packet/UI ---
    // Wiring kept: LDR -> GND, fixed resistor -> 3V3, ADC reads the middle.
    // ldr_init() must be called once in setup() before using this.
    float lux = ldr_read_lux();
    r.ldr = isnan(lux) ? 0 : (uint16_t)lroundf(lux);

    // --- Microphone: keep raw ADC value ---
    r.mic = readADC(PIN_MIC);

    // --- Battery: placeholder (100%) ---
    r.batt = 100.0f;
}
