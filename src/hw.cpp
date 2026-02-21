#include <Arduino.h>
#include <esp_system.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "hw.h"

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

    // 2. Atenuación ADC — solo para micrófono y LDR (no para sensor de suelo,
    //    cuya calibración usa los valores del firmware original sin atenuación explícita).
    analogSetPinAttenuation(PIN_SENSOR_SONIDO, ADC_11db);
    analogSetPinAttenuation(PIN_LDR_SIGNAL,    ADC_11db);

    // 3. Inicializar sensor DS18B20
    // ORDEN CRÍTICO (igual que el firmware original que funcionaba):
    //   sensors.begin() PRIMERO → la librería OneWire configura el pin internamente.
    //   pinMode(INPUT_PULLUP) DESPUÉS → activa pull-up para mantener bus HIGH en reposo.
    sensors.begin();
    sensors.setResolution(9);
    pinMode(PIN_TEMP_DS18B20, INPUT_PULLUP);
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
    // Misma estructura que el firmware original (analogRead sin atenuación explícita).
    // Calibración original a 10-bit: seco=895, mojado=470 → escalado a 12-bit (*4):
    //   SOIL_DRY = 3580  (~2.89V en seco)
    //   SOIL_WET = 1880  (~1.52V en agua)
    // *** Si los valores del Serial siguen siendo ~600, el sensor tiene problema de VCC.
    //     Ajusta SOIL_DRY y SOIL_WET con los raw reales que leas en el Serial.
    const int SOIL_DRY = 3580;
    const int SOIL_WET = 1880;

    int raw = analogRead(PIN_SENSOR_HUMEDAD);
    Serial.printf("[Soil] raw=%d  V=%.0fmV\n", raw, (raw / 4095.0f) * 3300.0f);

    int percent = constrain(map(raw, SOIL_DRY, SOIL_WET, 0, 100), 0, 100);
    return percent;
}

float read_ds18b20_temp() {
    // Si sensors.begin() en el arranque no detectó el sensor (bus no estable aún),
    // intentar re-escanear. Esto cubre el caso de sensor conectado en caliente o
    // un inicio demasiado rápido antes de que el bus 1-Wire estuviera listo.
    if (sensors.getDeviceCount() == 0) {
        sensors.begin();
        if (sensors.getDeviceCount() == 0) {
            Serial.println("[DS18B20] Sin dispositivos en bus (GPIO33/J4)");
            return -999.0f;
        }
        sensors.setResolution(9);
        Serial.printf("[DS18B20] Re-detectado: %d dispositivo(s)\n", sensors.getDeviceCount());
    }

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    if (tempC == DEVICE_DISCONNECTED_C) {
        return -999.0f;
    }
    return tempC;
}

// Función auxiliar (si la necesitas para io.cpp)
int readADC(uint8_t pin) {
    return analogRead(pin);
}