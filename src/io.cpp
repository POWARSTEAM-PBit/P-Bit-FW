#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include "io.h"
#include "hw.h"  // Usamos hw para leer DS18B20 y otros
#include "ble.h" 
#include <math.h> 

#define DHT_TYPE DHT11

// Constantes LDR
#define VCC_SUPPLY_VOLTAGE    3300.0 
#define REF_RESISTANCE      10000.0 
#define LUX_CALIBRATION_LOG    1.8 
#define LUX_CALIBRATION_GAMMA   1.4 
#define ADC_SATURATION_THRESHOLD 4050 

Reading global_readings;
volatile bool g_sensor_data_ready = false;

DHT dht(PIN_DHT, DHT_TYPE);

// Forward declarations (funciones internas a este módulo)
static void read_fast_sensors(Reading &r);
static void read_slow_sensors(Reading &r);

void sensor_reading_task(void *param) {
   Serial.println("[IO] Tarea de Lectura iniciada.");
   dht.begin();

   // Inicializar sensores a valores centinela para que la pantalla muestre "---"
   // o "No sensor" mientras no se haya completado la primera lectura lenta (~1 s).
   global_readings.temp_ds18b20 = -999.0f;
   global_readings.temperature  = NAN;
   global_readings.humidity     = NAN;

   uint32_t last_slow_read_ms = 0;

   while (1) {
      uint32_t current_ms = millis();
      if (current_ms - last_slow_read_ms >= SENSOR_READ_INTERVAL_MS) {
         last_slow_read_ms = current_ms;
         read_slow_sensors(global_readings); 
      }
      read_fast_sensors(global_readings);
      g_sensor_data_ready = true;
      notifyAll();
#ifdef FIRMWARE_DEBUG
      static bool _hwm_reported = false;
      if (!_hwm_reported) { _hwm_reported = true; DPRINT("[Stack] SensorTask HWM: %u words\n", uxTaskGetStackHighWaterMark(NULL)); }
#endif
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}

static void read_fast_sensors(Reading &r) {
    // --- LDR (R7=10kΩ pull-up a +3V3, LDR03 pull-down a GND, C4=1µF filtro HW) ---
    // Lógica: luz intensa → ADC bajo | oscuridad → ADC alto
    float ldr_raw = analogRead(PIN_LDR_SIGNAL);
    float ldr_new;
    if (ldr_raw >= ADC_SATURATION_THRESHOLD) {
        ldr_new = 20000.0f;
    } else {
        float v   = (ldr_raw / 4095.0f) * VCC_SUPPLY_VOLTAGE;
        float res = (v > 0 && (VCC_SUPPLY_VOLTAGE - v) > 0) ?
                    (REF_RESISTANCE * (VCC_SUPPLY_VOLTAGE - v)) / v : 999999.0f;
        float log_r = log10(res);
        ldr_new = pow(10.0f, (log_r - LUX_CALIBRATION_LOG) / LUX_CALIBRATION_GAMMA);
    }
    ldr_new = constrain(ldr_new, 0.0f, 20000.0f);

    // EMA software (C4 ya filtra HW): 0.3 nuevo + 0.7 antiguo → suaviza sin retrasar mucho
    static float ldr_ema = -1.0f;
    if (ldr_ema < 0.0f) ldr_ema = ldr_new; // Inicialización en la primera lectura
    ldr_ema = 0.7f * ldr_ema + 0.3f * ldr_new;
    r.ldr = ldr_ema;

    // Delegamos a HW (Limpio y sin duplicados)
    r.mic = read_sound_level();
    r.soil_humidity = read_soil_moisture();
}

static void read_slow_sensors(Reading &r) {
   // DHT Local
   float h = dht.readHumidity();
   float t = dht.readTemperature(); 
   if (!isnan(h) && h >= 0 && h <= 100) r.humidity = h;
   if (!isnan(t) && t >= -20 && t <= 80) r.temperature = t;

   // DS18B20: siempre actualizar — la pantalla maneja -999 como "No sensor"
   r.temp_ds18b20 = read_ds18b20_temp();
}