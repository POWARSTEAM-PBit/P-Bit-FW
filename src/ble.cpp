#include <Arduino.h>
#include <esp_system.h>     // esp_read_mac()
#include <NimBLEDevice.h>
#include "io.h"
#include "hw.h"

// ================= CONSTANTS =================
constexpr int DATA_PACKET_LEN = 17;

constexpr char NEW_SERVICE_UUID[] = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
constexpr char NEW_CHAR_UUID[]    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

constexpr uint16_t LEGACY_SERVICE_UUID16 = 0x181A; // Environmental Sensing
constexpr uint16_t LEGACY_CHAR_UUID16    = 0x2A6E; // Temperature

// ================= GLOBALS =================
NimBLECharacteristic *pNewChar    = nullptr;
NimBLECharacteristic *pLegacyChar = nullptr;

bool clientConnected = false;

void notifyAll();

extern QueueHandle_t reading_queue;

// ======================================================
// Packet and JSON builders
// ======================================================
std::string assm_pkt(Reading rec_pkt) {
    uint8_t buf[DATA_PACKET_LEN];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x02;
    buf[1] = 0x00;

    auto put = [&](int idx, uint8_t id, uint16_t val) {
        int base = 2 + idx * 3;
        buf[base] = id;
        buf[base + 1] = val & 0xFF;
        buf[base + 2] = (val >> 8) & 0xFF;
    };

    uint16_t t10  = isnan(rec_pkt.temperature) ? 0 : (uint16_t)lroundf(rec_pkt.temperature * 10.0f);
    uint16_t h10  = isnan(rec_pkt.humidity)    ? 0 : (uint16_t)lroundf(rec_pkt.humidity * 10.0f);
    uint16_t lraw = isnan(rec_pkt.ldr)         ? 0 : (uint16_t)lroundf(rec_pkt.ldr);
    uint16_t mraw = isnan(rec_pkt.mic)         ? 0 : (uint16_t)lroundf(rec_pkt.mic);
    uint16_t bat  = isnan(rec_pkt.batt)        ? 0 : (uint16_t)lroundf(rec_pkt.batt);

    put(0, 1, t10);
    put(1, 2, h10);
    put(2, 3, lraw);
    put(3, 4, mraw);
    put(4, 5, bat);

    return std::string((char*)buf, sizeof(buf));
}

String makeJson(Reading rec_pkt) {
    String s = "{";
    if (!isnan(rec_pkt.temperature)) s += "\"temp\":" + String(rec_pkt.temperature, 1) + ",";
    if (!isnan(rec_pkt.humidity))    s += "\"hum\":"  + String(rec_pkt.humidity, 1)  + ",";
    if (!isnan(rec_pkt.ldr))         s += "\"ldr\":"  + String((int)rec_pkt.ldr) + ",";
    if (!isnan(rec_pkt.mic))         s += "\"mic\":"  + String((int)rec_pkt.mic) + ",";
    if (!isnan(rec_pkt.batt))        s += "\"batt\":" + String((int)rec_pkt.batt) + ",";
    if (s[s.length() - 1] == ',') s.remove(s.length() - 1);
    s += "}";
    return s;
}

// ======================================================
// BLE CALLBACKS
// ======================================================
class ServerCB : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
        clientConnected = true;
        Serial.println("[BLE] Client connected");
    }

    void onDisconnect(NimBLEServer* pServer) {
        clientConnected = false;
        Serial.println("[BLE] Client disconnected");
        NimBLEDevice::startAdvertising();
    }
};

class NewCharCB : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c) {
        std::string v = c->getValue();
        if (!v.empty() && (uint8_t)v[0] == 0x01) {
            Serial.println("[BLE] Web requested instant packet");
            notifyAll();
        }
    }
};


void notifyAll() {
    Reading received;

    read_sensors(received);
    std::string pkt = assm_pkt(received);
    String js = makeJson(received);

    if (pNewChar) {
        pNewChar->setValue(pkt);
        pNewChar->notify();
    }

    if (pLegacyChar) {
        pLegacyChar->setValue((uint8_t*)js.c_str(), js.length());
        pLegacyChar->notify();
    }
}


void init_ble() {

    NimBLEDevice::init(dev_name);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

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

    Serial.println("[BLE] Advertising started");
}
