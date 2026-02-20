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
    // 1. Inicializar pines de sensores analógicos (Opcional en ESP32, pero buena práctica)
    pinMode(PIN_SENSOR_SONIDO, INPUT);
    pinMode(PIN_SENSOR_HUMEDAD, INPUT);

    // 3. Inicializar sensor DS18B20
    // El código viejo activaba el pullup interno, lo replicamos:
    pinMode(PIN_TEMP_DS18B20, INPUT_PULLUP); 
    sensors.begin();
    sensors.setResolution(9); // Resolución 9-bit (rápida) como en el código viejo
}

void set_ldr_power(bool state) {
    digitalWrite(PIN_LDR_PWR, state ? HIGH : LOW);
}

void set_devicename() {
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(dev_name, sizeof(dev_name), "PBIT-%02X%02X", mac[4], mac[5]);
}

// --- LECTURA DE SENSORES ---

int read_sound_level() {
    // Leemos el pin 36 (VP)
    int raw = analogRead(PIN_SENSOR_SONIDO);
    
    // Mapeo simple para visualización (0-4095 -> 0-100%)
    // Ajusta el 4095 si tu micrófono no llega al tope o es muy sensible
    int percent = map(raw, 0, 4095, 0, 100);
    return constrain(percent, 0, 100);
}

int read_soil_moisture() {
    int raw = analogRead(PIN_SENSOR_HUMEDAD);
    
    // CALIBRACIÓN DEL CÓDIGO VIEJO:
    // Seco = 895, Mojado = 470
    // Notar que el valor baja cuando se moja (típico de capacitivos)
    int percent = map(raw, 895, 470, 0, 100);
    
    return constrain(percent, 0, 100);
}

float read_ds18b20_temp() {
    // Solicitamos la temperatura al sensor
    sensors.requestTemperatures(); 
    
    // Leemos el valor
    float tempC = sensors.getTempCByIndex(0);
    
    // Verificamos si hay error de desconexión (-127)
    if(tempC == DEVICE_DISCONNECTED_C) {
        return -999.0; // Valor de error personalizado
    }
    
    return tempC;
}

// Función auxiliar (si la necesitas para io.cpp)
int readADC(uint8_t pin) {
    return analogRead(pin);
}