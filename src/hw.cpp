#include <Arduino.h>
#include <esp_system.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "hw.h"
#include "config.h"

// --- VARIABLES GLOBALES ---
uint8_t mac[MAC_LEN];
char dev_name[MAX_DEVICE_NAME_LEN];

// Instancias para el sensor de temperatura DS18B20
OneWire oneWire(PIN_TEMP_DS18B20);
DallasTemperature sensors(&oneWire);

// --- INICIALIZACIÓN ---

void init_hw() {
    // 1. Inicializar pines de sensores analógicos
    pinMode(PIN_SENSOR_SONIDO, INPUT);
    pinMode(PIN_SENSOR_HUMEDAD, INPUT);

    // 2. Forzar atenuación 11dB en pines ADC → rango completo 0–3.3V (0–4095).
    //    Sin esto el ESP32 puede quedar en 0dB (~0–0.8V) y saturar/recortar la señal.
    analogSetPinAttenuation(PIN_SENSOR_SONIDO,   ADC_11db);
    analogSetPinAttenuation(PIN_SENSOR_HUMEDAD,  ADC_11db);
    analogSetPinAttenuation(PIN_LDR_SIGNAL,      ADC_11db);

    // 3. Inicializar sensor DS18B20
    // ORDEN CORRECTO:
    //   pinMode(INPUT_PULLUP) PRIMERO → pin en modo digital con pull-up activo,
    //   bus 1-Wire queda HIGH antes de que sensors.begin() haga el escaneo.
    //   delay(10ms) → tiempo para que el bus se estabilice a HIGH antes del escaneo.
    //   sensors.begin() DESPUÉS → la librería escanea con el bus ya estable.
    pinMode(PIN_TEMP_DS18B20, INPUT_PULLUP);
    delay(10);
    sensors.begin();
    sensors.setResolution(9);
    Serial.printf("[DS18B20] init: %d dispositivo(s) en bus\n", sensors.getDeviceCount());
}

void set_devicename() {
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(dev_name, sizeof(dev_name), "PBIT-%02X%02X", mac[4], mac[5]);
}

// --- LECTURA DE SENSORES ---

int read_sound_level() {
    // GM19767P: señal AC centrada en ~1.65V (LM358 inversor, ganancia 0-20×, bias RV2).
    // Mide amplitud PICO A PICO en 50ms para capturar más ciclos de audio completos.
    const unsigned long WINDOW_MS = 50;
    unsigned long t = millis();
    int hi = 0, lo = 4095;
    while (millis() - t < WINDOW_MS) {
        int s = analogRead(PIN_SENSOR_SONIDO);
        if (s > hi) hi = s;
        if (s < lo) lo = s;
    }

    // PEAK_MAX: valor pico a pico que equivale al 100%.
    // Con ganancia 20×, una señal de ~45mV del mic produce ~900 cuentas ADC de swing.
    // Sube este valor si el medidor satura con demasiada facilidad.
    // Bájalo si no llega al 100% ni gritando.
    const int PEAK_MAX = 900;
    int raw = constrain(map(hi - lo, 0, PEAK_MAX, 0, 100), 0, 100);

    // EMA (Exponential Moving Average) para eliminar picos aleatorios.
    // 0.4 nuevo + 0.6 antiguo: responde en ~3-4 lecturas (300-400ms).
    static float ema = 0.0f;
    ema = 0.6f * ema + 0.4f * (float)raw;
    return (int)ema;
}

int read_soil_moisture() {
    // Sensor: Capacitive Soil Moisture Sensor V2.
    // Lógica inversa: más húmedo → voltaje más bajo → ADC más bajo.
    //
    // *** CALIBRACIÓN — ajusta con los valores reales de tu sensor:
    //     1. Deja el sensor en aire seco → anota el raw del Serial → SOIL_DRY
    //     2. Sumerge el sensor en agua  → anota el raw del Serial → SOIL_WET
    //
    // Valores medidos en P-Bit (versión anterior, 3.3V sensor, atenuación 11dB):
    //   Seco  raw ≈ 3408  (~2746mV — correcto para Capacitive V2 a 3.3V)
    //   Mojado raw ≈ 1904 (~1534mV — delta de 1504 cuentas, buena dinámica)
    const int SOIL_DRY = 3408; // Valor medido en seco
    const int SOIL_WET = 1904; // Valor medido sumergido en agua

    int raw = analogRead(PIN_SENSOR_HUMEDAD);

    int percent = map(raw, SOIL_DRY, SOIL_WET, 0, 100);

    // EMA moderada (0.80/0.20): con 1504 cuentas de rango el ruido es <1% por cuenta.
    static float ema = -1.0f;
    int clamped = constrain(percent, 0, 100);
    if (ema < 0.0f) ema = (float)clamped;
    ema = 0.80f * ema + 0.20f * (float)clamped;
    return (int)ema;
}

float read_ds18b20_temp() {
    // Si sensors.begin() en el arranque no detectó el sensor (bus no estable aún),
    // intentar re-escanear. Esto cubre el caso de sensor conectado en caliente o
    // un inicio demasiado rápido antes de que el bus 1-Wire estuviera listo.
    if (sensors.getDeviceCount() == 0) {
        sensors.begin();
        if (sensors.getDeviceCount() == 0) {
            DPRINTLN("[DS18B20] Sin dispositivos en bus (GPIO33/J4)");
            return -999.0f;
        }
        sensors.setResolution(9);
        Serial.printf("[DS18B20] Re-detectado: %d dispositivo(s)\n", sensors.getDeviceCount());
    }

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC == DEVICE_DISCONNECTED_C || tempC < -55.0f || tempC > 125.0f) {
        return -999.0f;
    }
    return tempC;
}

