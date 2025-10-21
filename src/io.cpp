#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include "io.h"
#include "hw.h"

#define DHT_TYPE DHT11
constexpr uint8_t PIN_DHT = 4;
constexpr uint8_t PIN_LDR = 39;
constexpr uint8_t PIN_MIC = 36;
constexpr uint8_t PIN_MOISTURE = 35;  // Connected to IO35 red port

DHT dht(PIN_DHT, DHT_TYPE);

// Calibration values (measure these with your sensor!)
const int AIR_VALUE = 2800;    // Sensor value in air (dry) - adjust after testing
const int WATER_VALUE = 1200;  // Sensor value in water (wet) - adjust after testing

void setup_sensors() {
    dht.begin();
    analogSetAttenuation(ADC_11db);  // For 0-3.3V range
    pinMode(PIN_MOISTURE, INPUT);
}

void read_sensors(Reading &r) {
    r.humidity = dht.readHumidity();
    r.temperature = dht.readTemperature();
    
    // Check if DHT reading failed
    if (isnan(r.humidity) || isnan(r.temperature)) {
        printf("DHT11: Failed to read!\n");
        r.humidity = 0;
        r.temperature = 0;
    } else {
        printf("DHT11: %.1f°C, %.1f%% RH\n", r.temperature, r.humidity);
    }
    
    // Read moisture sensor (analog)
    int moistureRaw = analogRead(PIN_MOISTURE);
    
    // Convert to percentage (0% = dry/air, 100% = wet/water)
    int moisturePercent = map(moistureRaw, AIR_VALUE, WATER_VALUE, 0, 100);
    moisturePercent = constrain(moisturePercent, 0, 100);
    
    // Determine moisture level
    const char* moistureLevel;
    int intervals = (AIR_VALUE - WATER_VALUE) / 3;
    
    if (moistureRaw > WATER_VALUE && moistureRaw < (WATER_VALUE + intervals)) {
        moistureLevel = "Very Wet";
    } else if (moistureRaw > (WATER_VALUE + intervals) && moistureRaw < (AIR_VALUE - intervals)) {
        moistureLevel = "Wet";
    } else if (moistureRaw < AIR_VALUE && moistureRaw > (AIR_VALUE - intervals)) {
        moistureLevel = "Dry";
    } else {
        moistureLevel = "Very Dry";
    }
    
    printf("Moisture: %d (raw) | %d%% | %s\n", moistureRaw, moisturePercent, moistureLevel);
    
    // Store in your Reading struct (uncomment and add field to struct)
    // r.moisture = moisturePercent;
    
    // Analog sensors
    r.ldr = readADC(PIN_LDR);
    r.mic = readADC(PIN_MIC);
    
    printf("LDR: %d | MIC: %d\n", r.ldr, r.mic);
    
    // Battery placeholder
    r.batt = 100.0f;
    
    printf("---\n");
}