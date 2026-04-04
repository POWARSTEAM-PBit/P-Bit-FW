#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include "io.h"
#include "hw.h"  // Reuse the hardware layer for DS18B20 and shared sensor helpers.
#include "ble.h" 
#include "alert_engine.h"
#include "runtime_events.h"
#include <math.h> 

#define DHT_TYPE DHT11

// LDR calibration constants.
#define VCC_SUPPLY_VOLTAGE    3300.0 
#define REF_RESISTANCE      10000.0 
#define LUX_CALIBRATION_LOG    1.8 
#define LUX_CALIBRATION_GAMMA   1.4 
#define ADC_SATURATION_THRESHOLD 4050 

Reading global_readings;
volatile bool g_sensor_data_ready = false;
portMUX_TYPE readings_mux = portMUX_INITIALIZER_UNLOCKED;
extern bool g_sound_enabled;

DHT dht(PIN_DHT, DHT_TYPE);

// Internal helpers for the sensor task.
static void read_fast_sensors(Reading &r);
static void read_slow_sensors(Reading &r);
static uint8_t dht_temp_fail_count = 0;
static uint8_t dht_hum_fail_count = 0;

void sensor_reading_task(void *param) {
    Serial.println("[IO] Sensor task started.");
   dht.begin();

   Reading local_r;
    // Start with sentinel values so the UI can show "---" or "No sensor"
    // until the first slow pass completes.
   local_r.temp_ds18b20 = -999.0f;
   local_r.temperature  = NAN;
   local_r.humidity     = NAN;
   local_r.soil_humidity = NAN;
   local_r.ldr = 0.0f;
   local_r.mic = 0.0f;

   portENTER_CRITICAL(&readings_mux);
   global_readings = local_r;
   portEXIT_CRITICAL(&readings_mux);

   uint32_t last_slow_read_ms = 0;

   while (1) {
      uint32_t current_ms = millis();
      if (current_ms - last_slow_read_ms >= SENSOR_READ_INTERVAL_MS) {
         last_slow_read_ms = current_ms;
         read_slow_sensors(local_r); 
      }
      read_fast_sensors(local_r);

       // Copy the local snapshot to the shared struct inside a critical section.
       portENTER_CRITICAL(&readings_mux);
       global_readings = local_r;
       portEXIT_CRITICAL(&readings_mux);

       // Refresh the shared alert state as soon as the new snapshot is ready.
       alert_engine_refresh_from_reading(local_r, g_sound_enabled);

       runtime_mark_sensor_data_ready();

      // Skip BLE packet assembly entirely when nobody is connected.
      if (client_connected.load()) {
          notifyAll();
      }

#if PBIT_ENABLE_SERIAL_PLOTTER
       // --- STEAM / Serial Plotter mode ---
       // Replace invalid readings with 0.0 so the IDE plotter stays stable.
      float p_temp = isnan(local_r.temperature) ? 0.0f : local_r.temperature;
      float p_hum = isnan(local_r.humidity) ? 0.0f : local_r.humidity;
      float p_ldr = isnan(local_r.ldr) ? 0.0f : local_r.ldr;
      float p_mic = isnan(local_r.mic) ? 0.0f : local_r.mic;
      float p_soil = isnan(local_r.soil_humidity) ? 0.0f : local_r.soil_humidity;
      float p_ds18 = (local_r.temp_ds18b20 < -100.0f) ? 0.0f : local_r.temp_ds18b20;

      Serial.printf("Temp:%.1f, Hum:%.1f, Luz:%.0f, Sonido:%.0f, Suelo:%.0f, DS18:%.1f\n",
                    p_temp, p_hum, p_ldr, p_mic, p_soil, p_ds18);
#endif

#ifdef FIRMWARE_DEBUG
      static bool _hwm_reported = false;
      if (!_hwm_reported) { _hwm_reported = true; DPRINT("[Stack] SensorTask HWM: %u words\n", uxTaskGetStackHighWaterMark(NULL)); }
#endif
      vTaskDelay(pdMS_TO_TICKS(100));
   }
}

static void read_fast_sensors(Reading &r) {
    // LDR front-end: 10k pull-up to 3.3V, LDR to GND, with a hardware RC filter.
    // Logic is inverted: bright light -> lower ADC, darkness -> higher ADC.
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
    r.ldr_raw = ldr_raw;

    // Software EMA on top of the hardware filter: smooths the reading without lagging too much.
    static float ldr_ema = -1.0f;
    if (ldr_ema < 0.0f) ldr_ema = ldr_new; // Initialize on the first sample.
    ldr_ema = 0.7f * ldr_ema + 0.3f * ldr_new;
    r.ldr = ldr_ema;

    // Delegate the remaining sensor reads to the hardware layer.
    r.mic = read_sound_level();
    r.soil_humidity = read_soil_moisture();
}

static void read_slow_sensors(Reading &r) {
    // Local DHT11 read.
   float h = dht.readHumidity();
   float t = dht.readTemperature(); 
   if (!isnan(h) && h >= 0 && h <= 100) {
      r.humidity = h;
      dht_hum_fail_count = 0;
   } else {
      if (dht_hum_fail_count < 2) dht_hum_fail_count++;
      if (dht_hum_fail_count >= 2) r.humidity = NAN;
   }
   if (!isnan(t) && t >= -20 && t <= 80) {
      r.temperature = t;
      dht_temp_fail_count = 0;
   } else {
      if (dht_temp_fail_count < 2) dht_temp_fail_count++;
      if (dht_temp_fail_count >= 2) r.temperature = NAN;
   }

    // Always refresh DS18B20; the UI maps -999 to "No sensor".
   r.temp_ds18b20 = read_ds18b20_temp();
}
