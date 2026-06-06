#include <Arduino.h>
#include "config.h"
#include "pbit_dht.h"  // Driver DHT11 RMT local — sustituye Adafruit DHT por IWDT timeout.
#include "io.h"
#include "hw.h"  // Reuse the hardware layer for DS18B20 and shared sensor helpers.
#include "ble.h"
#include "alert_engine.h"
#include "runtime_events.h"
#include "graph_buffer.h"
#include "sensor_connection_notice.h"
#include <math.h>

// pbit_dht expone enum PbitDhtStatus; no usamos macro local DHT_TYPE.

// LDR empirical calibration v1 (2026-05-29).
// Manual fit: lux = 10 * ((4095 - raw) / (raw + 150))^2.
// The current divider maps high ADC counts to darkness.
#define LDR_ADC_MAX_COUNTS      4095.0f
#define LDR_RAW_OFFSET           150.0f
#define LDR_EMPIRICAL_SCALE       10.0f
#define LDR_LUX_MAX             8000.0f
#define ADC_HIGH_RAIL_THRESHOLD 4050

Reading global_readings;
volatile bool g_sensor_data_ready = false;
portMUX_TYPE readings_mux = portMUX_INITIALIZER_UNLOCKED;
extern bool g_alarm_sound_enabled;

// pbit_dht no usa instancia: la lectura es función libre pbit_dht11_read(...).

// Internal helpers for the sensor task.
static void read_fast_sensors(Reading &r);
static void read_slow_sensors(Reading &r);
static uint8_t dht_temp_fail_count = 0;
static uint8_t dht_hum_fail_count = 0;

static float ldr_raw_to_lux(float raw) {
    const float safe_raw = constrain(raw, 0.0f, LDR_ADC_MAX_COUNTS);
    if (safe_raw >= ADC_HIGH_RAIL_THRESHOLD) return 0.0f;

    const float x = (LDR_ADC_MAX_COUNTS - safe_raw) / (safe_raw + LDR_RAW_OFFSET);
    const float lux = LDR_EMPIRICAL_SCALE * x * x;
    return isfinite(lux) ? constrain(lux, 0.0f, LDR_LUX_MAX) : LDR_LUX_MAX;
}

void sensor_reading_task(void *param) {
    DPRINTLN("[IO] Sensor task started.");
    // pbit_dht no requiere init explícita: el driver RMT se instala y desinstala
    // dentro de cada lectura.

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
         if (!isnan(local_r.ldr_raw))     graph_buffer_push(g_graph_light_raw, local_r.ldr_raw);
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
      // Reporte periódico de Stack HWM — muestreo cada 1 s, log al
      // empeorar el peor caso o cada 60 s. En este stack (ESP32 +
      // framework-arduinoespressif32) uxTaskGetStackHighWaterMark()
      // devuelve directamente bytes, NO words (a diferencia del vanilla
      // FreeRTOS). Ver header local task.h: "in bytes not words".
      {
          static uint32_t hwm_last_sample_ms = 0;
          static uint32_t hwm_last_report_ms = 0;
          static UBaseType_t hwm_worst = (UBaseType_t)-1;
          const uint32_t hwm_now = millis();
          if (hwm_now - hwm_last_sample_ms >= 1000) {
              hwm_last_sample_ms = hwm_now;
              const UBaseType_t hwm = uxTaskGetStackHighWaterMark(NULL);
              const bool worsened = (hwm < hwm_worst);
              if (worsened) hwm_worst = hwm;
              if (worsened || (hwm_now - hwm_last_report_ms >= 60000)) {
                  hwm_last_report_ms = hwm_now;
                  DPRINT("[Stack] SensorTask HWM free: %u bytes (worst: %u)\n",
                         (unsigned)hwm, (unsigned)hwm_worst);
              }
          }
      }
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
        float ldr_raw_sample = (float)ldr_sum / (float)LDR_ADC_SAMPLES;

        constexpr uint8_t LDR_AVG_WINDOW = 10;
        static float ldr_raw_window[LDR_AVG_WINDOW] = {0.0f};
        static uint8_t ldr_raw_index = 0;
        static uint8_t ldr_raw_count = 0;
        static float ldr_raw_sum = 0.0f;

        if (ldr_raw_count < LDR_AVG_WINDOW) {
            ldr_raw_count++;
        } else {
            ldr_raw_sum -= ldr_raw_window[ldr_raw_index];
        }
        ldr_raw_window[ldr_raw_index] = ldr_raw_sample;
        ldr_raw_sum += ldr_raw_sample;
        ldr_raw_index = (uint8_t)((ldr_raw_index + 1) % LDR_AVG_WINDOW);

        float ldr_raw = ldr_raw_sum / (float)ldr_raw_count;
        float ldr_new = ldr_raw_to_lux(ldr_raw);
        r.ldr_raw = ldr_raw;
        r.ldr = ldr_new;
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
        sensor_connection_notice_note_sample(SZ_SOIL, !isnan(r.soil_humidity));
    }
    // else: r.soil_humidity retains its previous value.
}

static void read_slow_sensors(Reading &r) {
    // pbit_dht11_read: una sola llamada devuelve T+H simultáneamente vía RMT
    // peripheral. Scheduler-friendly: no usa noInterrupts() — evita el IWDT
    // timeout que disparaba Adafruit DHT. Rate-limit interno: 1 s entre
    // lecturas (devuelve PBIT_DHT_TOO_SOON si se llama antes). Solo modifica
    // las salidas si checksum OK.
    float dht_t = NAN;
    float dht_h = NAN;
    const uint8_t dht_status = pbit_dht11_read(dht_t, dht_h);

    if (dht_status == PBIT_DHT_OK) {
        // Humedad
        if (!isnan(dht_h) && dht_h >= 0 && dht_h <= 100) {
            r.humidity = dht_h;
            dht_hum_fail_count = 0;
        } else {
            if (dht_hum_fail_count < 2) dht_hum_fail_count++;
            if (dht_hum_fail_count >= 2) r.humidity = NAN;
        }
        // Temperatura
        if (!isnan(dht_t) && dht_t >= -20 && dht_t <= 80) {
            r.temperature = dht_t;
            dht_temp_fail_count = 0;
        } else {
            if (dht_temp_fail_count < 2) dht_temp_fail_count++;
            if (dht_temp_fail_count >= 2) r.temperature = NAN;
        }
    } else if (dht_status == PBIT_DHT_TOO_SOON) {
        // Rate limit interno: mantener valores anteriores. No cuenta como fallo.
    } else {
        // Error transitorio (TIMEOUT, NACK, BAD_DATA, CHECKSUM, etc.). El DHT11
        // está soldado al PCB, así que no esperamos "sin sensor"; estos errores
        // son glitches que el counter tolera hasta NAN.
        if (dht_hum_fail_count < 2) dht_hum_fail_count++;
        if (dht_hum_fail_count >= 2) r.humidity = NAN;
        if (dht_temp_fail_count < 2) dht_temp_fail_count++;
        if (dht_temp_fail_count >= 2) r.temperature = NAN;
    }

    // Instrumentación bajo FIRMWARE_DEBUG: contadores DHT cada 60 s.
#ifdef FIRMWARE_DEBUG
    {
        static uint32_t cnt_ok = 0;
        static uint32_t cnt_too_soon = 0;
        static uint32_t cnt_err = 0;
        static uint8_t  last_err = PBIT_DHT_OK;
        static uint32_t last_log_ms = 0;

        if (dht_status == PBIT_DHT_OK)            cnt_ok++;
        else if (dht_status == PBIT_DHT_TOO_SOON) cnt_too_soon++;
        else { cnt_err++; last_err = dht_status; }

        const uint32_t now_ms = millis();
        if (now_ms - last_log_ms >= 60000) {
            last_log_ms = now_ms;
            DPRINT("[DHT] OK:%u TOO_SOON:%u ERR:%u (last_err:%u)\n",
                   (unsigned)cnt_ok, (unsigned)cnt_too_soon, (unsigned)cnt_err, (unsigned)last_err);
        }
    }
#endif

    // DS18B20 in async mode (set up by init_hw): this read returns the conversion issued
    // ~1 s ago and kicks off a new one in the background. Total cost ~7 ms (vs ~94 ms sync).
    r.temp_ds18b20 = read_ds18b20_temp();
    sensor_connection_notice_note_sample(SZ_DS18, r.temp_ds18b20 >= -100.0f);
}
