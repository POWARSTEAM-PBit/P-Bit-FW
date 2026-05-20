#include <Arduino.h>
#include <esp_system.h>     // esp_read_mac()
#include <NimBLEDevice.h>
#include <atomic>
#include <math.h>
#include <string.h>
#include <freertos/semphr.h>
#include "io.h"
#include "hw.h"
#include "config.h"


constexpr int DATA_PACKET_LEN = 20;

constexpr char NEW_SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char NEW_CHAR_UUID[]    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

constexpr uint16_t LEGACY_SERVICE_UUID16 = 0x181A; // Environmental Sensing
constexpr uint16_t LEGACY_CHAR_UUID16 = 0x2A6E; // Temperature


NimBLECharacteristic * pNewChar    = nullptr;
NimBLECharacteristic * pLegacyChar = nullptr;

std::atomic<bool> client_connected{false};
std::atomic<bool> g_ble_notify_requested{false};

void ble_request_notify();

namespace {

constexpr uint32_t BLE_NOTIFY_INTERVAL_MS = 250; // 4 Hz max in the hot path.
SemaphoreHandle_t g_ble_tx_mutex = nullptr;
uint32_t g_last_notify_ms = 0;

static bool valid_float(float value) {
    return isfinite(value);
}

static void append_json_field(char* dst, size_t dst_len, size_t& pos,
                              bool& wrote_any, const char* name,
                              float value, uint8_t decimals) {
    if (pos >= dst_len) return;
    int written = snprintf(dst + pos, dst_len - pos,
                           wrote_any ? ",\"%s\":%.*f" : "\"%s\":%.*f",
                           name, decimals, (double)value);
    if (written <= 0) return;
    size_t used = (size_t)written;
    pos += (used < (dst_len - pos)) ? used : (dst_len - pos - 1);
    wrote_any = true;
}

static void append_json_field_int(char* dst, size_t dst_len, size_t& pos,
                                  bool& wrote_any, const char* name,
                                  int value) {
    if (pos >= dst_len) return;
    int written = snprintf(dst + pos, dst_len - pos,
                           wrote_any ? ",\"%s\":%d" : "\"%s\":%d",
                           name, value);
    if (written <= 0) return;
    size_t used = (size_t)written;
    pos += (used < (dst_len - pos)) ? used : (dst_len - pos - 1);
    wrote_any = true;
}

} // namespace


static void assm_pkt(const Reading& rec_pkt, uint8_t (&buf)[DATA_PACKET_LEN]) {
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x02;
    buf[1] = 0x00;

    auto put_u16 = [&](int idx, uint8_t id, uint16_t val) {
        int base = 2 + idx * 3;
        buf[base] = id;
        buf[base + 1] = val & 0xFF;
        buf[base + 2] = (val >> 8) & 0xFF;
    };

    auto put_s16 = [&](int idx, uint8_t id, int16_t val) {
        int base = 2 + idx * 3;
        uint16_t raw = (uint16_t)val; // serializar en complemento a dos little-endian
        buf[base] = id;
        buf[base + 1] = raw & 0xFF;
        buf[base + 2] = (raw >> 8) & 0xFF;
    };

    const bool valid_temp = valid_float(rec_pkt.temperature);
    const bool valid_hum = valid_float(rec_pkt.humidity);
    const bool valid_ldr = valid_float(rec_pkt.ldr);
    const bool valid_mic = valid_float(rec_pkt.mic);
    const bool valid_soil = valid_float(rec_pkt.soil_humidity);
    const bool valid_ds18 = rec_pkt.temp_ds18b20 >= -100.0f && valid_float(rec_pkt.temp_ds18b20);

    if (valid_temp) buf[1] |= (1u << 0);
    if (valid_hum)  buf[1] |= (1u << 1);
    if (valid_ldr)  buf[1] |= (1u << 2);
    if (valid_mic)  buf[1] |= (1u << 3);
    if (valid_soil) buf[1] |= (1u << 4);
    if (valid_ds18) buf[1] |= (1u << 5);

    // IDs 1 (temp) y 6 (ds18) viajan como int16_t x10 para soportar negativos.
    int16_t  t10   = !valid_temp ? 0 : (int16_t)lroundf(rec_pkt.temperature * 10.0f);
    uint16_t h10   = !valid_hum  ? 0 : (uint16_t)lroundf(rec_pkt.humidity * 10.0f);
    uint16_t lraw  = !valid_ldr  ? 0 : (uint16_t)lroundf(rec_pkt.ldr);
    uint16_t mraw  = !valid_mic  ? 0 : (uint16_t)lroundf(rec_pkt.mic);
    uint16_t soil  = !valid_soil ? 0 : (uint16_t)lroundf(rec_pkt.soil_humidity);
    int16_t  ds18  = !valid_ds18 ? 0 : (int16_t)lroundf(rec_pkt.temp_ds18b20 * 10.0f);

    put_s16(0, 1, t10);
    put_u16(1, 2, h10);
    put_u16(2, 3, lraw);
    put_u16(3, 4, mraw);
    put_u16(4, 5, soil);
    put_s16(5, 6, ds18);
}

static size_t makeJson(const Reading& rec_pkt, char* dst, size_t dst_len) {
    if (!dst || dst_len == 0) return 0;

    size_t pos = 0;
    bool wrote_any = false;
    dst[pos++] = '{';
    dst[pos] = '\0';

    if (valid_float(rec_pkt.temperature)) append_json_field(dst, dst_len, pos, wrote_any, "temp", rec_pkt.temperature, 1);
    if (valid_float(rec_pkt.humidity))    append_json_field(dst, dst_len, pos, wrote_any, "hum",  rec_pkt.humidity, 1);
    if (valid_float(rec_pkt.ldr))         append_json_field_int(dst, dst_len, pos, wrote_any, "ldr",  (int)lroundf(rec_pkt.ldr));
    if (valid_float(rec_pkt.mic))         append_json_field_int(dst, dst_len, pos, wrote_any, "mic",  (int)lroundf(rec_pkt.mic));
    if (valid_float(rec_pkt.soil_humidity)) append_json_field_int(dst, dst_len, pos, wrote_any, "soil", (int)lroundf(rec_pkt.soil_humidity));
    if (rec_pkt.temp_ds18b20 >= -100.0f && valid_float(rec_pkt.temp_ds18b20)) {
        append_json_field(dst, dst_len, pos, wrote_any, "ds18", rec_pkt.temp_ds18b20, 1);
    }

    if (pos < dst_len - 1) {
        dst[pos++] = '}';
        dst[pos] = '\0';
    } else {
        dst[dst_len - 2] = '}';
        dst[dst_len - 1] = '\0';
        pos = dst_len - 1;
    }
    return pos;
}

// ======================================================
// BLE CALLBACKS
// ======================================================
// ---------------- BLE callbacks ----------------
class ServerCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*) override { client_connected = true; }
  void onDisconnect(NimBLEServer*) override {
    client_connected = false;
    NimBLEDevice::startAdvertising();
  }
};

class NewCharCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    std::string v = c->getValue();
    if (!v.empty() && (uint8_t)v[0] == 0x01) {
      // Web requests an instant packet. The actual notify is serviced from the
      // sensor/runtime task to avoid re-entering NimBLE from this callback.
      ble_request_notify();
    }
  }
};

void ble_request_notify() {
    g_ble_notify_requested.store(true);
}

void notifyAll() {
    if (!client_connected.load()) return;
    if (g_ble_tx_mutex && xSemaphoreTake(g_ble_tx_mutex, 0) != pdTRUE) {
        return;
    }

    Reading snapshot;
    
    // Snapshot seguro usando el spinlock
    portENTER_CRITICAL(&readings_mux);
    snapshot = global_readings;
    portEXIT_CRITICAL(&readings_mux);

    uint8_t pkt[DATA_PACKET_LEN];
    char js[160];
    assm_pkt(snapshot, pkt);
    const size_t js_len = makeJson(snapshot, js, sizeof(js));

    if (pNewChar) {
        pNewChar->setValue(pkt, sizeof(pkt));
        pNewChar->notify();
    }

    if (pLegacyChar && js_len > 0) {
        pLegacyChar->setValue((uint8_t*)js, js_len);
        pLegacyChar->notify();
    }

    if (g_ble_tx_mutex) {
        xSemaphoreGive(g_ble_tx_mutex);
    }
}

void ble_service() {
    if (!client_connected.load()) {
        g_ble_notify_requested.store(false);
        return;
    }

    const uint32_t now = millis();
    const bool immediate = g_ble_notify_requested.exchange(false);
    if (!immediate && (uint32_t)(now - g_last_notify_ms) < BLE_NOTIFY_INTERVAL_MS) {
        return;
    }

    g_last_notify_ms = now;
    notifyAll();
}


void init_ble() {

    NimBLEDevice::init(dev_name);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    if (!g_ble_tx_mutex) {
        g_ble_tx_mutex = xSemaphoreCreateMutex();
    }

    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCB());

    NimBLEService *newSvc = pServer->createService(NEW_SERVICE_UUID);
    pNewChar = newSvc->createCharacteristic(
        NEW_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
    );
    pNewChar->setCallbacks(new NewCharCB());
    newSvc->start();

    NimBLEService *legSvc = pServer->createService(NimBLEUUID(LEGACY_SERVICE_UUID16));
    pLegacyChar = legSvc->createCharacteristic(
        NimBLEUUID(LEGACY_CHAR_UUID16),
        NIMBLE_PROPERTY::NOTIFY
    );
    legSvc->start();

    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->addServiceUUID(NEW_SERVICE_UUID);
    adv->addServiceUUID(NimBLEUUID(LEGACY_SERVICE_UUID16));
    adv->start();

    DPRINTLN("[BLE] Advertising started");
}
