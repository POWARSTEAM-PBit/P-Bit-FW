// pbit_dht.cpp
// Driver DHT11 scheduler-friendly vía RMT peripheral del ESP32.
// Adaptado de htmltiger/dhtESP32-rmt v1.0 (MIT) con parches del P-Bit.
// Ver include/pbit_dht.h para detalles de la adaptación.
//
// ---------------------------------------------------------------------------
// Licencia MIT original (htmltiger/dhtESP32-rmt v1.0):
//
//   Copyright (c) 2023 htmltiger — https://github.com/htmltiger/dhtESP32-rmt
//
//   Permission is hereby granted, free of charge, to any person obtaining a
//   copy of this software and associated documentation files (the "Software"),
//   to deal in the Software without restriction, including without limitation
//   the rights to use, copy, modify, merge, publish, distribute, sublicense,
//   and/or sell copies of the Software, and to permit persons to whom the
//   Software is furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included
//   in all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//   DEALINGS IN THE SOFTWARE.
// ---------------------------------------------------------------------------

#include "pbit_dht.h"

#include <Arduino.h>
#include "driver/rmt.h"
#include "hw.h"  // PIN_DHT

namespace {

// Canal RMT reservado para este driver. ESP32 tiene 8 canales (0..7); el
// P-Bit no usa RMT en otra parte (verificado en auditoría 2026-06-04). Si
// en el futuro otra librería pide RMT, revisar conflicto antes de cambiar
// este valor.
constexpr rmt_channel_t kDhtRmtChannel = RMT_CHANNEL_1;

// Rate-limit del DHT11: el sensor solo entrega muestras nuevas ~1 Hz.
// Llamadas más frecuentes devuelven PBIT_DHT_TOO_SOON sin gastar ciclos
// configurando el periférico RMT en vano.
constexpr uint32_t kDht11MinIntervalMs = 1000;

// Helper de cleanup uniforme: deja el pin HIGH y libera el driver RMT.
// Se llama solo en paths donde rmt_driver_install retornó ESP_OK.
inline void cleanup_rmt_and_pin(gpio_num_t pin) {
    gpio_set_level(pin, 1);
    rmt_rx_stop(kDhtRmtChannel);
    rmt_driver_uninstall(kDhtRmtChannel);
}

}  // namespace

uint8_t pbit_dht11_read(float &temperature, float &humidity) {
    // Rate-limit (singleton: el P-Bit usa un único DHT soldado en PIN_DHT).
    static uint32_t last_read_ms = 0;
    const uint32_t now_ms = millis();
    if (last_read_ms != 0 && (now_ms - last_read_ms) < kDht11MinIntervalMs) {
        return PBIT_DHT_TOO_SOON;
    }
    last_read_ms = now_ms;

    const gpio_num_t dht_pin = static_cast<gpio_num_t>(PIN_DHT);

    rmt_config_t rmt_rx = {};
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.channel = kDhtRmtChannel;
    rmt_rx.gpio_num = dht_pin;
    rmt_rx.clk_div = 80;                       // 80 MHz / 80 = 1 us por tick
    rmt_rx.mem_block_num = 1;
    rmt_rx.flags = 0;
    rmt_rx.rx_config.idle_threshold = 250;     // us de silencio = fin de trama
    rmt_rx.rx_config.filter_ticks_thresh = 30; // filtra glitches <30 us
    rmt_rx.rx_config.filter_en = true;

    if (rmt_config(&rmt_rx) != ESP_OK) {
        return PBIT_DHT_DRIVER;
    }

    if (rmt_driver_install(rmt_rx.channel, 512,
                           ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED) != ESP_OK) {
        return PBIT_DHT_DRIVER;
    }

    // A partir de aquí, cualquier return debe pasar por cleanup_rmt_and_pin.
    RingbufHandle_t dhtbuf = nullptr;
    if (rmt_get_ringbuf_handle(kDhtRmtChannel, &dhtbuf) != ESP_OK || dhtbuf == nullptr) {
        cleanup_rmt_and_pin(dht_pin);
        return PBIT_DHT_DRIVER;
    }

    // Start signal: GPIO LOW durante 22 ms (DHT11), luego HIGH y captura RMT.
    // vTaskDelay cede CPU al scheduler — no bloquea interrupts.
    gpio_set_level(dht_pin, 1);
    gpio_pullup_dis(dht_pin);
    gpio_pulldown_dis(dht_pin);
    gpio_set_direction(dht_pin, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_intr_type(dht_pin, GPIO_INTR_DISABLE);
    gpio_set_level(dht_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(22));
    gpio_set_level(dht_pin, 1);

    if (rmt_rx_start(kDhtRmtChannel, true) != ESP_OK) {
        cleanup_rmt_and_pin(dht_pin);
        return PBIT_DHT_DRIVER;
    }

    uint8_t error = PBIT_DHT_OK;
    size_t rx_size = 0;
    rmt_item32_t *rx_items =
        (rmt_item32_t *)xRingbufferReceive(dhtbuf, &rx_size, pdMS_TO_TICKS(100));

    if (rx_items == nullptr) {
        cleanup_rmt_and_pin(dht_pin);
        return PBIT_DHT_TIMEOUT;
    }

    const int tot_items = (int)(rx_size / sizeof(rmt_item32_t));

    // Parche P-Bit: validar tamaño ANTES de leer rx_items[0]. La librería
    // original calcula ack_pulse antes de verificar tot_items, lo que es UB
    // si el ringbuffer devolvió 0 items.
    if (tot_items < 41) {
        error = PBIT_DHT_UNDERFLOW;
    } else if (tot_items > 42) {
        error = PBIT_DHT_OVERFLOW;
    } else {
        // Parche P-Bit: pulsos en uint16_t para evitar truncamiento.
        // duration0 y duration1 son uint16_t cada uno; su suma no cabe en
        // uint8_t.
        const uint16_t ack_pulse =
            (uint16_t)rx_items[0].duration0 + (uint16_t)rx_items[0].duration1;

        if (ack_pulse < 130 || ack_pulse > 180) {
            error = PBIT_DHT_NACK;
        } else {
            // Parche P-Bit: inicializar buffer a 0. La librería original usa
            // `uint8_t data[6];` sin inicializar antes de hacer `data[i/8] <<= 1`,
            // lo cual es UB. Solo se accede a [0..4], así que dimensionamos exacto.
            uint8_t data[5] = {0};

            for (uint8_t i = 0; i < 40; i++) {
                const uint16_t pulse =
                    (uint16_t)rx_items[i + 1].duration0 + (uint16_t)rx_items[i + 1].duration1;
                if (pulse > 55 && pulse < 145) {
                    data[i / 8] <<= 1;
                    if (pulse > 110) {
                        data[i / 8] |= 1;
                    }
                } else {
                    error = PBIT_DHT_BAD_DATA;
                }
            }

            if (error == PBIT_DHT_OK) {
                const uint8_t total = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
                if (data[4] == total) {
                    // DHT11 entrega enteros: data[1] y data[3] son siempre 0.
                    humidity = (float)data[0];
                    temperature = (float)data[2];
                } else {
                    error = PBIT_DHT_CHECKSUM;
                }
            }
        }
    }

    vRingbufferReturnItem(dhtbuf, (void *)rx_items);
    cleanup_rmt_and_pin(dht_pin);
    return error;
}
