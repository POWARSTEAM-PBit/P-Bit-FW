#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <esp_system.h>     // esp_read_mac()
#include <NimBLEDevice.h>

// ---------------- Pins (change only if your board is different) ----------------
constexpr uint8_t PIN_DHT      = 4;   // DHT11 data
constexpr uint8_t PIN_LDR      = 39;  // ADC1 (SENSOR_VN)
constexpr uint8_t PIN_MIC      = 36;  // ADC1 (SENSOR_VP)
constexpr uint8_t PIN_ONEWIRE  = 32;  // DS18B20 data (optional)
constexpr uint8_t I2C_SDA      = 21;
constexpr uint8_t I2C_SCL      = 22;

#define DHT_TYPE DHT11        // change to DHT22 if needed

// ---------------- Web Bluetooth UUIDs (must match your web) ----------------
static const char* NEW_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
static const char* NEW_CHAR_UUID    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

// Legacy (send compact JSON over Temperature characteristic)
static const uint16_t LEGACY_SERVICE_UUID16 = 0x181A; // Environmental Sensing
static const uint16_t LEGACY_CHAR_UUID16    = 0x2A6E; // Temperature

// ---------------- Globals ----------------
DHT dht(PIN_DHT, DHT_TYPE);
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

// Use pointer so we can construct LCD ONCE with the probed I2C address.
LiquidCrystal_PCF8574* lcd = nullptr;

NimBLECharacteristic* pNewChar    = nullptr;
NimBLECharacteristic* pLegacyChar = nullptr;

bool     clientConnected = false;
uint32_t lastPushMs = 0;
uint32_t lastLcdMs  = 0;

// cached sensor values
float v_temp = NAN, v_hum = NAN, v_ldr = NAN, v_mic = NAN, v_batt = 100.0f;

// ---------------- Helpers ----------------

// Probe LCD address (0x27/0x3F), then construct LCD once
void initLCD() {
  auto probe = [](uint8_t addr)->bool {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
  };
  uint8_t addr = probe(0x27) ? 0x27 : (probe(0x3F) ? 0x3F : 0x27);

  lcd = new LiquidCrystal_PCF8574(addr);
  lcd->begin(16, 2);
  lcd->setBacklight(255);

  lcd->clear();
  lcd->setCursor(0,0); lcd->print(" P-BIT  ");
  lcd->setCursor(8,0); lcd->print("BLE:... ");
  lcd->setCursor(0,1); lcd->print("LCD @0x"); lcd->print(addr, HEX);
}

// Average ADC to reduce noise
int readADC(uint8_t pin, int samples = 16) {
  if (pin >= 34) pinMode(pin, INPUT);
  uint32_t sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  return sum / samples; // 0..4095
}

// Build 17-byte packet: [0x02,0x00,(id,u16LE)*5]
// ids: 1=temp(0.1C), 2=hum(0.1%), 3=ldr(raw), 4=mic(raw), 5=batt(%)
std::string pack17() {
  uint8_t buf[17]; memset(buf, 0, sizeof(buf));
  buf[0] = 0x02; buf[1] = 0x00;

  auto put = [&](int idx, uint8_t id, uint16_t val) {
    int base = 2 + idx * 3;
    buf[base]   = id;
    buf[base+1] = val & 0xFF;
    buf[base+2] = (val >> 8) & 0xFF;
  };

  uint16_t t10  = isnan(v_temp) ? 0 : (uint16_t)lroundf(v_temp * 10.0f);
  uint16_t h10  = isnan(v_hum)  ? 0 : (uint16_t)lroundf(v_hum  * 10.0f);
  uint16_t lraw = isnan(v_ldr)  ? 0 : (uint16_t)lroundf(v_ldr);
  uint16_t mraw = isnan(v_mic)  ? 0 : (uint16_t)lroundf(v_mic);
  uint16_t bat  = isnan(v_batt) ? 0 : (uint16_t)lroundf(v_batt);

  put(0, 1, t10);
  put(1, 2, h10);
  put(2, 3, lraw);
  put(3, 4, mraw);
  put(4, 5, bat);

  return std::string((char*)buf, sizeof(buf));
}

// Compact JSON for legacy characteristic
String makeJson() {
  String s = "{";
  if (!isnan(v_temp)) s += "\"temp\":" + String(v_temp,1) + ",";
  if (!isnan(v_hum))  s += "\"hum\":"  + String(v_hum,1)  + ",";
  if (!isnan(v_ldr))  s += "\"ldr\":"  + String((int)v_ldr) + ",";
  if (!isnan(v_mic))  s += "\"mic\":"  + String((int)v_mic) + ",";
  if (!isnan(v_batt)) s += "\"batt\":" + String((int)v_batt) + ",";
  if (s[s.length()-1] == ',') s.remove(s.length()-1);
  s += "}";
  return s;
}

// Read all sensors
void readSensors() {
  // DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) v_hum = h;
  if (!isnan(t)) v_temp = t;

  // DS18B20 (if present) overrides temperature
  ds18b20.requestTemperatures();
  float t2 = ds18b20.getTempCByIndex(0);
  if (t2 != DEVICE_DISCONNECTED_C && t2 > -100 && t2 < 120) v_temp = t2;

  // ADC raw
  v_ldr = readADC(PIN_LDR);
  v_mic = readADC(PIN_MIC);

  // Battery placeholder
  v_batt = 100.0f;
}

// Update LCD (top: BLE state; bottom: values toggle every 2s)
void updateLCD() {
  if (!lcd) return;

  lcd->setCursor(0,0);
  lcd->print(" P-BIT  ");
  lcd->setCursor(8,0);
  lcd->print(clientConnected ? "BLE:Conn" : "BLE:Adv ");

  static bool flip = false;
  if (millis() - lastLcdMs > 2000) { flip = !flip; lastLcdMs = millis(); }

  char line[17];
  if (!flip) {
    snprintf(line, sizeof(line), "T:%4.1f H:%4.1f",
             isnan(v_temp)?0:v_temp, isnan(v_hum)?0:v_hum);
  } else {
    snprintf(line, sizeof(line), "L:%4d M:%4d",
             (int)(isnan(v_ldr)?0:v_ldr), (int)(isnan(v_mic)?0:v_mic));
  }
  lcd->setCursor(0,1);
  lcd->print(line);
  for (int i = strlen(line); i < 16; i++) lcd->print(' ');
}

// Notify both characteristics
void notifyAll() {
  if (pNewChar) {
    std::string pkt = pack17();
    pNewChar->setValue(pkt);
    pNewChar->notify();
  }
  if (pLegacyChar) {
    String js = makeJson();
    pLegacyChar->setValue((uint8_t*)js.c_str(), js.length());
    pLegacyChar->notify();
  }
}

// ---------------- BLE callbacks ----------------
class ServerCB : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*) override { clientConnected = true; }
  void onDisconnect(NimBLEServer*) override {
    clientConnected = false;
    NimBLEDevice::startAdvertising();
  }
};

class NewCharCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    std::string v = c->getValue();
    if (!v.empty() && (uint8_t)v[0] == 0x01) {
      // Web requests an instant packet
      readSensors();
      notifyAll();
    }
  }
};

// ---------------- setup / loop ----------------
void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[BOOT] setup start");

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("[BOOT] I2C OK");

  initLCD();
  Serial.println("[BOOT] LCD OK");

  dht.begin();
  ds18b20.begin();
  Serial.println("[BOOT] Sensors OK");

  uint8_t mac[6]; esp_read_mac(mac, ESP_MAC_BT);
  char devName[20];
  snprintf(devName, sizeof(devName), "PBIT-%02X%02X", mac[4], mac[5]);
  Serial.printf("[BOOT] DevName: %s\n", devName);

  NimBLEDevice::init(devName);
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  Serial.println("[BOOT] NimBLE init OK");

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCB());
  Serial.println("[BOOT] Server OK");

  // New protocol (binary 17B)
  NimBLEService* newSvc = pServer->createService(NEW_SERVICE_UUID);
  pNewChar = newSvc->createCharacteristic(
      NEW_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);
  pNewChar->setCallbacks(new NewCharCB());
  newSvc->start();
  Serial.println("[BOOT] New service OK");

  // Legacy protocol (JSON over 0x2A6E)
  NimBLEService* legSvc = pServer->createService(NimBLEUUID(LEGACY_SERVICE_UUID16));
  pLegacyChar = legSvc->createCharacteristic(
      NimBLEUUID(LEGACY_CHAR_UUID16), NIMBLE_PROPERTY::NOTIFY);
  legSvc->start();
  Serial.println("[BOOT] Legacy service OK");

  // Advertise both services
  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(NEW_SERVICE_UUID);
  adv->addServiceUUID(NimBLEUUID(LEGACY_SERVICE_UUID16));
  adv->setScanResponse(true);
  adv->start();
  Serial.println("[BOOT] Advertising started");

  if (lcd) {
    lcd->setCursor(0,1);
    lcd->print("Ready: "); lcd->print(devName);
  }
}

void loop() {
  uint32_t now = millis();
  if (now - lastPushMs > 1000) {
    lastPushMs = now;
    readSensors();
    notifyAll();
  }
  updateLCD();
  delay(20);
}
