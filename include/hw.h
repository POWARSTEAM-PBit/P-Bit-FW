#pragma once

#include <Arduino.h>

constexpr size_t MAC_LEN = 6;
constexpr size_t MAX_DEVICE_NAME_LEN = 20;

// --- CONFIGURACIÓN DE PINES (Extraídos de tu código viejo) ---

#define PIN_LDR_PWR        21   // Control de encendido LDR (Del código actual)
#define PIN_LDR_SIGNAL     39   // Del código viejo: analogReadMilliVolts(39)

#define PIN_SENSOR_SONIDO  36   // Del código viejo: analogReadMilliVolts(36)
#define PIN_SENSOR_HUMEDAD 35   // Del código viejo: SOIL_SENSOR_PIN 35
#define PIN_TEMP_DS18B20   33   // Del código viejo: DS18B20_PIN 33

/**
 * @brief MAC address of the device
 */
extern uint8_t mac[MAC_LEN];

/**
 * @brief Device name based on MAC address
 */
extern char dev_name[MAX_DEVICE_NAME_LEN];

// --- DECLARACIONES DE FUNCIONES ---

void set_devicename();
int readADC(uint8_t pin); // Mantengo esta por compatibilidad si la usas en io.cpp

/**
 * @brief Inicializa pines de hardware y sensores (OneWire, LDR, etc).
 */
void init_hw(); 

/**
 * @brief Controla el encendido/apagado de la luz que afecta al sensor LDR.
 */
void set_ldr_power(bool state);

// --- NUEVAS FUNCIONES DE SENSORES ---

/**
 * @brief Lee el nivel de sonido y devuelve un porcentaje (0-100)
 */
int read_sound_level();

/**
 * @brief Lee la humedad del suelo y devuelve porcentaje (0-100) calibrado
 */
int read_soil_moisture();

/**
 * @brief Lee temperatura en °C del sensor DS18B20
 */
float read_ds18b20_temp();