#include "ldr.h"
#include "hw.h"   // only to reuse PIN_LDR if your project defines it

// --------- Pins & basic ADC setup ---------
#ifndef PIN_LDR
  // Fallback pin if your project did not define PIN_LDR
  #define PIN_LDR 39
#endif

static const int   ADC_MAX     = 4095;     // ESP32 12-bit ADC
static const float VREF        = 3.3f;     // approx full-scale voltage
static const float R_FIXED_OHM = 10000.0f; // fixed resistor value (Ohms)
static const int   NUM_SAMPLES = 8;        // average N ADC samples

// Wiring kept as you said you cannot change:
// LDR -> GND, Fixed resistor -> 3.3V, ADC reads the middle.
// With this wiring: darker -> higher ADC, brighter -> lower ADC.
//
// Voltage divider with this wiring:
//   Vout = 3.3 * (R_ldr / (R_ldr + R_fixed))
// => R_ldr = R_fixed * (Vout / (3.3 - Vout))

// --------- Plug-and-play lux model (no calibration) ---------
// We use a generic power-law fit with gamma ≈ 1 for common LDRs:
//   R ≈ A * L^(-γ)  with  γ = 1  =>  L ≈ A / R
// Choose A = 1,000,000 so that typical indoor/bright scenes map well.
// This gives lux that increases with brightness and sits in a sensible range.
static const float A_NO_CAL = 1000000.0f;  // scaling factor
static const float LUX_MIN  = 0.0f;        // clamp low
static const float LUX_MAX  = 20000.0f;    // clamp high (avoid crazy spikes)

// ------------------------------------------------------------

static float _adc_to_volt(int raw) {
  if (raw < 0) raw = 0;
  if (raw > ADC_MAX) raw = ADC_MAX;
  return (raw * VREF) / (float)ADC_MAX;
}

void ldr_init() {
  // Set ADC resolution and attenuation to read ~0..3.3V
  analogReadResolution(12);                   // 0..4095
  analogSetPinAttenuation(PIN_LDR, ADC_11db); // wide input range
  pinMode(PIN_LDR, INPUT);
}

int ldr_read_raw() {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; ++i) {
    sum += analogRead(PIN_LDR);
  }
  return (int)(sum / NUM_SAMPLES);
}

// Estimate LDR resistance (Ohms) from the divider formula
static float _ldr_resistance_ohm() {
  float v = _adc_to_volt(ldr_read_raw());

  // Avoid division by zero at the ends
  const float eps = 0.01f;
  if (v < eps)        v = eps;
  if (v > VREF - eps) v = VREF - eps;

  // R_ldr = R_fixed * (Vout / (3.3 - Vout))
  return R_FIXED_OHM * (v / (VREF - v));
}

// Main API: estimated lux (plug-and-play)
float ldr_read_lux() {
  const float R = _ldr_resistance_ohm();

  // No calibration: lux ≈ A / R
  float lux = A_NO_CAL / R;

  // Clean and clamp
  if (!isfinite(lux)) lux = 0.0f;
  if (lux < LUX_MIN)  lux = LUX_MIN;
  if (lux > LUX_MAX)  lux = LUX_MAX;

  return lux;
}
