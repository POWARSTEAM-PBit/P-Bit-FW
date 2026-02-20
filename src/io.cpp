#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include "io.h"
#include "hw.h"  // Usamos hw para leer DS18B20 y otros
#include "ble.h" 
#include <math.h> 

#define DHT_TYPE DHT11 
constexpr uint8_t PIN_DHT = 4; // Pin DHT (Seguro ahora con TFT_RST=-1)

// Constantes LDR
#define VCC_SUPPLY_VOLTAGE    3300.0 
#define REF_RESISTANCE      10000.0 
#define LUX_CALIBRATION_LOG    1.8 
#define LUX_CALIBRATION_GAMMA   1.4 
#define ADC_SATURATION_THRESHOLD 4050 

Reading global_readings;
volatile bool g_sensor_data_ready = false;

DHT dht(PIN_DHT, DHT_TYPE);
const uint32_t SLOW_SENSOR_INTERVAL_MS = pdTICKS_TO_MS(SENSOR_READ_INTERVAL);

void sensor_reading_task(void *param) {
   Serial.println("[IO] Tarea de Lectura iniciada.");
   dht.begin(); 
   
   uint32_t last_slow_read_ms = 0;
   
   while (1) {
      uint32_t current_ms = millis();
      if (current_ms - last_slow_read_ms >= SLOW_SENSOR_INTERVAL_MS) {
         last_slow_read_ms = current_ms;
         read_slow_sensors(global_readings); 
      }
      read_fast_sensors(global_readings);
      g_sensor_data_ready = true;
      notifyAll();
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}

void read_fast_sensors(Reading &r) {
    // LDR (Calculo local complejo mantenido)
    float ldr_raw = analogRead(PIN_LDR_SIGNAL);
    if (ldr_raw >= ADC_SATURATION_THRESHOLD) {
        r.ldr = 20000.0; 
    } else {
        float v = (ldr_raw / 4095.0) * VCC_SUPPLY_VOLTAGE;
        float res = (v > 0 && (VCC_SUPPLY_VOLTAGE - v) > 0) ? 
                    (REF_RESISTANCE * (VCC_SUPPLY_VOLTAGE - v)) / v : 999999.0;
        float log_r = log10(res);
        r.ldr = pow(10, (log_r - LUX_CALIBRATION_LOG) / LUX_CALIBRATION_GAMMA);
    }
    r.ldr = constrain(r.ldr, 0.0, 20000.0); 
    
    // Delegamos a HW (Limpio y sin duplicados)
    r.mic = read_sound_level();
    r.soil_humidity = read_soil_moisture();
    r.batt = 100.0f; 
}

void read_slow_sensors(Reading &r) {
   // DHT Local
   float h = dht.readHumidity();
   float t = dht.readTemperature(); 
   if (!isnan(h) && h >= 0 && h <= 100) r.humidity = h;
   if (!isnan(t) && t >= -20 && t <= 80) r.temperature = t;

   // DS18B20 desde HW (Ya no bloquea aquí directamente, lo gestiona HW)
   float t2 = read_ds18b20_temp();
   if (t2 > -900.0) r.temp_ds18b20 = t2;
}