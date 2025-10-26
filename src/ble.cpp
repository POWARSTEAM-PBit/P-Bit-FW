#include <Arduino.h>
#include <esp_system.h>     // esp_read_mac()
#include <NimBLEDevice.h>
#include "io.h"
#include "hw.h"

// 17-byte custom packet length
constexpr int DATA_PACKET_LEN = 17;

// Custom service/characteristic UUIDs (must match frontend)
constexpr char NEW_SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char NEW_CHAR_UUID[]    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

// Legacy (Environmental Sensing / Temperature) - for compatibility
constexpr uint16_t LEGACY_SERVICE_UUID16 = 0x181A;
constexpr uint16_t LEGACY_CHAR_UUID16    = 0x2A6E;

NimBLECharacteristic *pNewChar    = nullptr;
NimBLECharacteristic *pLegacyChar = nullptr;

bool clientConnected = false;

void notifyAll();  // forward declare

// Assemble 17-byte custom packet from sensor reading
std::string assm_pkt(Reading rec_pkt) {
    uint8_t buf[DATA_PACKET_LEN];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x02;  // packet version/type
    buf[1] = 0x00;  // reserved

    auto put = [&](int idx, uint8_t id, uint16_t val) {
        // 5 blocks * 3 bytes each = 15 bytes after header (2)
        int base = 2 + idx * 3;
        buf[base]     = id;                 // field id
        buf[base + 1] = val & 0xFF;         // low byte
        buf[base + 2] = (val >> 8) & 0xFF;  // high byte
    };

    // Convert to scaled integers (or 0 if NaN)
    uint16_t t10  = isnan(rec_pkt.temperature) ? 0 : (uint16_t)lroundf(rec_pkt.temperature * 10.0f);
    uint16_t h10  = isnan(rec_pkt.humidity)    ? 0 : (uint16_t)lroundf(rec_pkt.humidity * 10.0f);
    uint16_t lraw = isnan(rec_pkt.ldr)         ? 0 : (uint16_t)lroundf(rec_pkt.ldr);
    uint16_t mraw = isnan(rec_pkt.mic)         ? 0 : (uint16_t)lroundf(rec_pkt.mic);
    uint16_t bat  = isnan(rec_pkt.batt)        ? 0 : (uint16_t)lroundf(rec_pkt.batt);

    // Put data blocks (id, value)
    put(0, 1, t10);  // 1 = temperature * 10
    put(1, 2, h10);  // 2 = humidity * 10
    put(2, 3, lraw); // 3 = ldr raw
    put(3, 4, mraw); // 4 = mic raw
    put(4, 5, bat);  // 5 = battery raw

    return std::string((char*)buf, sizeof(buf));
}

// Build a small JSON string (for legacy/compat path)
String makeJson(Reading rec_pkt) {
    String s = "{";
    if (!isnan(rec_pkt.temperature)) s += "\"temp\":" + String(rec_pkt.temperature, 1) + ",";
    if (!isnan(rec_pkt.humidity))    s += "\"hum\":"  + String(rec_pkt.humidity, 1)  + ",";
    if (!isnan(rec_pkt.ldr))         s += "\"ldr\":"  + String((int)rec_pkt.ldr)     + ",";
    if (!isnan(rec_pkt.mic))         s += "\"mic\":"  + String((int)rec_pkt.mic)     + ",";
    if (!isnan(rec_pkt.batt))        s += "\"batt\":" + String((int)rec_pkt.batt)    + ",";
    if (s[s.length() - 1] == ',') s.remove(s.length() - 1); // remove last comma
    s += "}";
    return s;
}

// ========================= BLE CALLBACKS =========================
class ServerCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*) override {
    clientConnected = true;
  }
  void onDisconnect(NimBLEServer*) override {
    clientConnected = false;
    // Restart advertising so the web page can reconnect
    NimBLEDevice::startAdvertising();
  }
};

class NewCharCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    // If web writes 0x01, push an instant notification packet
    std::string v = c->getValue();
    if (!v.empty() && (uint8_t)v[0] == 0x01) {
      notifyAll();
    }
  }
};

// Notify both characteristics (custom + legacy)
void notifyAll() {
    Reading received;
    read_sensors(received);               // read all sensors

    std::string pkt = assm_pkt(received); // 17-byte custom binary
    String js = makeJson(received);       // small JSON for legacy

    if (pNewChar) {
        pNewChar->setValue(pkt);
        pNewChar->notify();
    }

    if (pLegacyChar) {
        pLegacyChar->setValue((uint8_t*)js.c_str(), js.length());
        pLegacyChar->notify();
    }
}

// ========================= BLE INIT =========================
void init_ble() {
    // dev_name is prepared in hw.cpp (e.g., "PBIT-XXYY")
    NimBLEDevice::init(dev_name);

    // Set TX power (P9 = high). You can tune this later.
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    // Create server and set callbacks
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCB());

    // Create custom service + characteristic (notify + write)
    NimBLEService *newSvc = pServer->createService(NEW_SERVICE_UUID);
    pNewChar = newSvc->createCharacteristic(
        NEW_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
    );
    pNewChar->setCallbacks(new NewCharCB());
    newSvc->start();

    // Create legacy service + characteristic (notify only)
    NimBLEService *legSvc = pServer->createService(NimBLEUUID(LEGACY_SERVICE_UUID16));
    pLegacyChar = legSvc->createCharacteristic(
        NimBLEUUID(LEGACY_CHAR_UUID16),
        NIMBLE_PROPERTY::NOTIFY
    );
    legSvc->start();

    // Prepare advertising
    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();

    // Advertise both service UUIDs so the web app can discover either path
    adv->addServiceUUID(NEW_SERVICE_UUID);
    adv->addServiceUUID(NimBLEUUID(LEGACY_SERVICE_UUID16));

    // ***** IMPORTANT FIX *****
    // Put the device name into the scan response (for namePrefix "PBIT-").
    adv->setScanResponse(true);

    // Build scan response data with the device name
    NimBLEAdvertisementData scanData;
    scanData.setName(dev_name);            // include "PBIT-xxxx" name
    adv->setScanResponseData(scanData);    // apply scan response payload

    // (optional) suggest connection params for better stability
    // adv->setMinPreferred(0x06); // 7.5 ms
    // adv->setMaxPreferred(0x12); // 25 ms

    // Start advertising
    adv->start();

    Serial.println("[BLE] Advertising started (name in scan response)");
}
