#pragma once

#include <stdint.h>

// pbit_dht.h
// Driver scheduler-friendly para DHT11 vía RMT peripheral del ESP32.
// "Scheduler-friendly" = no usa noInterrupts() ni portENTER_CRITICAL.
// No es estrictamente non-blocking: la lectura puede esperar hasta ~100 ms
// en xRingbufferReceive, pero cediendo CPU al scheduler en todo momento.
//
// Adaptado de htmltiger/dhtESP32-rmt v1.0 (MIT) con parches del P-Bit:
//   - Inicialización explícita del buffer de bits (`data[5] = {0}`) para
//     eliminar UB del shift-left sobre bytes sin inicializar.
//   - Eliminada la tabla dinámica de pines con `new`: singleton estático
//     porque el P-Bit usa un único DHT soldado en PIN_DHT.
//   - Validación `tot_items >= 1` antes de leer `rx_items[0]`.
//   - Pulsos calculados en uint16_t para evitar truncamiento de la suma
//     duration0 + duration1.
//   - Cleanup explícito (rmt_rx_stop + rmt_driver_uninstall) en todos los
//     error paths intermedios, no solo en el éxito.
//   - PIN, tipo (DHT11) y canal RMT hardcoded a la configuración del P-Bit.
//   - API simplificada: una sola función sin parámetros.
//   - Estados prefijados PBIT_DHT_* para evitar colisiones futuras.
//
// Atribución original:
//   Copyright (c) 2023 htmltiger — https://github.com/htmltiger/dhtESP32-rmt
//   Licencia MIT (texto íntegro preservado en src/pbit_dht.cpp).
//
// Justificación de la vendorización: ver `docs/TECHNICAL.md` y
// `CHANGELOG.md` entrada "Estabilidad — Vendorización driver DHT RMT".

enum PbitDhtStatus : uint8_t {
    PBIT_DHT_OK         = 0,
    PBIT_DHT_TOO_SOON   = 1,
    PBIT_DHT_DRIVER     = 2,
    PBIT_DHT_TIMEOUT    = 3,
    PBIT_DHT_NACK       = 4,
    PBIT_DHT_BAD_DATA   = 5,
    PBIT_DHT_CHECKSUM   = 6,
    PBIT_DHT_UNDERFLOW  = 7,
    PBIT_DHT_OVERFLOW   = 8,
};

// Lee temperatura y humedad del DHT11 soldado en PIN_DHT (include/hw.h).
// Rate-limit interno: 1 s entre lecturas; llamadas tempranas devuelven
// PBIT_DHT_TOO_SOON sin modificar t/h. Solo modifica t y h si retorna
// PBIT_DHT_OK (checksum válido).
// Scheduler-friendly: vTaskDelay + xRingbufferReceive ceden CPU; no se
// bloquean interrupciones globales en ningún momento.
uint8_t pbit_dht11_read(float &temperature, float &humidity);
