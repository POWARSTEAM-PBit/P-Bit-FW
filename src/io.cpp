#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include "io.h"
#include "hw.h"  // Reuse the hardware layer for DS18B20 and shared sensor helpers.
#include "ble.h"
#include "alert_engine.h"
#include "runtime_events.h"
#include "graph_buffer.h"
#include <math.h>

#define DHT_TYPE DHT11

// LDR calibration constants.
#define VCC_SUPPLY_VOLTAGE    3300.0 
#define REF_RESISTANCE      10000.0 
#define LDR_R10_OHMS        15000.0
#define LDR_GAMMA              0.60
#define LDR_LUX_MAX         20000.0
#define ADC_LOW_RAIL_THRESHOLD 10
#define ADC_HIGH_RAIL_THRESHOLD 4050

Reading global_readings;
volatile bool g_sensor_data_ready = false;
portMUX_TYPE readings_mux = portMUX_INITIALIZER_UNLOCKED;
extern bool g_alarm_sound_enabled;

DHT dht(PIN_DHT, DHT_TYPE);

// Internal helpers for the sensor task.
static void read_fast_sensors(Reading &r);
static void read_slow_sensors(Reading &r);
static uint8_t dht_temp_fail_count = 0;
static uint8_t dht_hum_fail_count = 0;

void sensor_reading_task(void *param) {
    DPRINTLN("[IO] Sensor task started.");
   dht.begin();

   Reading local_r;
    // Start with sentinel values so the UI can show "---" or "No sensor"
    // until the first slow pass completes.
   local_r.temp_ds18b20 = -999.0f;
   local_r.temperature  = NAN;
   local_r.humidity     = NAN;
   local_r.soil_humidity = NAN;
   local_r.ldr = NAN;
   local_r.ldr_raw = NAN;
   local_r.mic = NAN;

   portENTER_CRITICAL(&readings_mux);
   global_readings = local_r;
   portEXIT_CRITICAL(&readings_mux);

   uint32_t last_slow_read_ms = 0;
   float mic_peak_accum = 0.0f;
   TickType_t last_wake_tick = xTaskGetTickCount();

   while (1) {
      uint32_t current_ms = millis();
      if (!isnan(local_r.mic) && local_r.mic > mic_peak_accum) {
         mic_peak_accum = local_r.mic;
      }
      if (current_ms - last_slow_read_ms >= SENSOR_READ_INTERVAL_MS) {
         last_slow_read_ms = current_ms;
         read_slow_sensors(local_r);

         // Push valid readings to the graph history buffers.
         portENTER_CRITICAL(&g_graph_mux);
         if (!isnan(local_r.temperature)) graph_buffer_push(g_graph_temp,     local_r.temperature);
         if (!isnan(local_r.humidity))    graph_buffer_push(g_graph_humidity,  local_r.humidity);
         if (local_r.temp_ds18b20 >= -100.0f) graph_buffer_push(g_graph_ds18, local_r.temp_ds18b20);
         if (!isnan(local_r.ldr))         graph_buffer_push(g_graph_light,     local_r.ldr);
         if (!isnan(local_r.soil_humidity)) graph_buffer_push(g_graph_soil,    local_r.soil_humidity);
         graph_buffer_push(g_graph_sound, mic_peak_accum);
         portEXIT_CRITICAL(&g_graph_mux);
         mic_peak_accum = 0.0f;
      }
      read_fast_sensors(local_r);

       // Copy the local snapshot to the shared struct inside a critical section.
       portENTER_CRITICAL(&readings_mux);
       global_readings = local_r;
       portEXIT_CRITICAL(&readings_mux);

       // Refresh the shared alert state as soon as the new snapshot is ready.
       alert_engine_refresh_from_reading(local_r, g_alarm_sound_enabled);

       runtime_mark_sensor_data_ready();

      ble_service();

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
      vTaskDelayUntil(&last_wake_tick, pdMS_TO_TICKS(30)); // 20 ms sound window + stable 30 Hz cadence
   }
}

static void read_fast_sensors(Reading &r) {
    // LDR: sample at ~5 Hz (every 6 fast cycles ≈ 180 ms) instead of 30 Hz.
    // Light conditions change slowly — 5 Hz is more than enough for a smooth display.
    // Sound is the only sensor that benefits from the full 30 Hz rate (VU accuracy).
    static uint8_t ldr_cycle = 0;
    if (++ldr_cycle >= 6) {
        ldr_cycle = 0;
        // LDR front-end used by the current board maps higher ADC counts to darkness:
        // light lowers the LDR resistance, which lowers the ADC voltage in this divider.
        constexpr uint8_t LDR_ADC_SAMPLES = 4;
        uint32_t ldr_sum = 0;
        for (uint8_t i = 0; i < LDR_ADC_SAMPLES; ++i) {
            ldr_sum += (uint32_t)read_adc_raw(PIN_LDR_SIGNAL);
            delayMicroseconds(150);
        }
        float ldr_raw = (float)ldr_sum / (float)LDR_ADC_SAMPLES;
        float ldr_new;
        if (ldr_raw <= ADC_LOW_RAIL_THRESHOLD) {
            ldr_new = LDR_LUX_MAX;
        } else if (ldr_raw >= ADC_HIGH_RAIL_THRESHOLD) {
            ldr_new = 0.0f;
        } else {
            float v   = (ldr_raw / 4095.0f) * VCC_SUPPLY_VOLTAGE;
            float res = (v > 0 && (VCC_SUPPLY_VOLTAGE - v) > 0) ?
                        (REF_RESISTANCE * v) / (VCC_SUPPLY_VOLTAGE - v) : 999999.0f;
            // GL55/GL5528-style CdS model:
            // R = R10 * (10 lux / lux)^gamma  =>  lux = 10 * (R10 / R)^(1/gamma).
            // R10 is set to the GL5528 midpoint (10-20 kOhm at 10 lux).
            ldr_new = 10.0f * powf(LDR_R10_OHMS / res, 1.0f / LDR_GAMMA);
        }
        if (!isfinite(ldr_new)) ldr_new = LDR_LUX_MAX;
        ldr_new = constrain(ldr_new, 0.0f, LDR_LUX_MAX);
        r.ldr_raw = ldr_raw;

        // Software EMA on top of the hardware filter: smooths the reading without lagging too much.
        static float ldr_ema = -1.0f;
        if (ldr_ema < 0.0f) ldr_ema = ldr_new; // Initialize on the first sample.
        ldr_ema = 0.7f * ldr_ema + 0.3f * ldr_new;
        r.ldr = ldr_ema;
    }
    // else: r.ldr and r.ldr_raw retain their previous values — no display update needed.

    // Sound: always sample at full rate (20 ms window is required for VU accuracy).
    r.mic = read_sound_level();

    // Soil: sample at ~5 Hz (every 6 fast cycles ≈ 180 ms).
    // The capacitive sensor is cheap to read (~2.4 ms), and the faster cadence makes
    // insertion/removal and calibration feel immediate without disturbing the 30 Hz task.
    static uint8_t soil_cycle = 0;
    if (++soil_cycle >= 6) {
        soil_cycle = 0;
        r.soil_humidity = read_soil_moisture();
    }
    // else: r.soil_humidity retains its previous value.
}

static void read_slow_sensors(Reading &r) {
    // DHT11 hardware caps internal sampling at ~1 Hz — reading more often just returns
    // cached values. To halve the per-cycle blocking, alternate humidity and temperature
    // reads so each individual call costs ~25 ms instead of ~50 ms. Each channel ends
    // up refreshed every 2 s, which is fine for room conditions that change slowly.
    static uint8_t dht_slot = 0;  // 0 = humidity, 1 = temperature
    if (dht_slot == 0) {
        float h = dht.readHumidity();
        if (!isnan(h) && h >= 0 && h <= 100) {
            r.humidity = h;
            dht_hum_fail_count = 0;
        } else {
            if (dht_hum_fail_count < 2) dht_hum_fail_count++;
            if (dht_hum_fail_count >= 2) r.humidity = NAN;
        }
        dht_slot = 1;
    } else {
        float t = dht.readTemperature();
        if (!isnan(t) && t >= -20 && t <= 80) {
            r.temperature = t;
            dht_temp_fail_count = 0;
        } else {
            if (dht_temp_fail_count < 2) dht_temp_fail_count++;
            if (dht_temp_fail_count >= 2) r.temperature = NAN;
        }
        dht_slot = 0;
    }

    // DS18B20 in async mode (set up by init_hw): this read returns the conversion issued
    // ~1 s ago and kicks off a new one in the background. Total cost ~7 ms (vs ~94 ms sync).
    r.temp_ds18b20 = read_ds18b20_temp();
}
