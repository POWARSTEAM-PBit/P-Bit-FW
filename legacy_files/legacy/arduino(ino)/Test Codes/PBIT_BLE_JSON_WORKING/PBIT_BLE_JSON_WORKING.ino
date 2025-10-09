// P-Bit Firmware with BLE JSON and Modular Sensor Readings

#include <TFT_eSPI.h>
#include <dhtESP32-rmt.h>
#include <SPI.h>
#include <ESP32RotaryEncoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define MAX_ADC_READING 1023
#define VCC_REF_VOLTAGE 3300
#define REF_RESISTANCE 10000
#define LUX_CALC_SCALAR 12518931
#define LUX_CALC_EXPONENT -1.405

#define RGB_R 5
#define RGB_G 17
#define RGB_B 16
#define BUZZER_TIME_ON 5
#define DHT_PIN 4
#define DHT_TYPE DHT11
#define DS18B20_PIN 33
#define SOIL_SENSOR_PIN 35

const uint8_t DI_ENCODER_A = 14;
const uint8_t DI_ENCODER_B = 12;
const int8_t DI_ENCODER_SW = 13;
const int8_t DO_ENCODER_VCC = -1;

TFT_eSPI tft = TFT_eSPI();
RotaryEncoder rotaryEncoder(DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC);
dht dht;  // Correct class from dhtESP32-rmt
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);

BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;
bool deviceConnected = false;

float air_temp = 0.0, air_hum = 0.0, soil_temp = 0.0;
int soil_hum = 0, light = 0, noise = 0;
int audio[20], audio_p = 0;
unsigned int timeCount = 0;
int screen = 0, refresh = 100, rgb_state = 0, buzzer = BUZZER_TIME_ON;

unsigned long lastJSONSent = 0;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override { deviceConnected = true; }
  void onDisconnect(BLEServer* pServer) override { deviceConnected = false; }
};

void readDHT() {
  dht.read(&air_temp, &air_hum, NULL);
}

void readSoil() {
  int soilRaw = analogRead(SOIL_SENSOR_PIN);
  soil_hum = map(soilRaw, 895, 470, 0, 100);
  soil_hum = constrain(soil_hum, 0, 100);
}

void readLight() {
  int Vo = analogReadMilliVolts(39);
  float ldrResistance = REF_RESISTANCE * Vo / (VCC_REF_VOLTAGE - Vo);
  light = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
}

void readNoise() {
  int maxVal = 0;
  for (int i = 0; i < 20; i++) if (audio[i] > maxVal) maxVal = audio[i];
  noise = maxVal - 1404;
}

void readSoilTemp() {
  sensors.requestTemperatures();
  soil_temp = sensors.getTempCByIndex(0);
}

void sendJSON() {
  if (!deviceConnected || millis() - lastJSONSent < 2000) return;
  lastJSONSent = millis();

  StaticJsonDocument<256> doc;
  doc["device"] = "PBIT";
  doc["air_temp"] = air_temp;
  doc["air_hum"] = air_hum;
  doc["light"] = light;
  doc["noise"] = noise;
  doc["soil_temp"] = soil_temp;
  doc["soil_hum"] = soil_hum;

  char jsonStr[256];
  serializeJson(doc, jsonStr);
  pCharacteristic->setValue((uint8_t*)jsonStr, strlen(jsonStr));
  pCharacteristic->notify();
}

void setupBLE() {
  BLEDevice::init("PBIT");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService* service = pServer->createService("181A");
  pCharacteristic = service->createCharacteristic("2A6E", BLECharacteristic::PROPERTY_NOTIFY);
  service->start();

  BLEAdvertising* advertising = pServer->getAdvertising();
  advertising->start();
}

void knobCallback(long value) {
  buzzer = BUZZER_TIME_ON;
  analogWrite(18, 63);
  screen = value;
  timeCount = 100;
}

void buttonCallback(unsigned long duration) {
  Serial.printf("button: %ums\n", duration);
}

void setup() {
  Serial.begin(115200);
  tft.init(); tft.setRotation(1); tft.fillScreen(TFT_BLACK);
  rotaryEncoder.setEncoderType(EncoderType::FLOATING);
  rotaryEncoder.setBoundaries(0, 6, true);
  rotaryEncoder.onTurned(&knobCallback);
  rotaryEncoder.onPressed(&buttonCallback);
  rotaryEncoder.begin();

  analogWrite(RGB_R, 255); analogWrite(RGB_G, 255); analogWrite(RGB_B, 255);
  analogReadResolution(10);

  dht.begin(DHT_TYPE, DHT_PIN);
  sensors.begin(); sensors.setResolution(9);

  pinMode(DS18B20_PIN, INPUT_PULLUP);
  setupBLE();
}

void loop() {
  timeCount++;
  if (timeCount >= refresh) {
    timeCount = 0;
    tft.fillScreen(TFT_BLACK);
    readDHT();
    readLight();
    readNoise();
    readSoil();
    readSoilTemp();
  }

  audio[audio_p] = analogReadMilliVolts(36);
  audio_p = (audio_p + 1) % 20;

  if (buzzer > 0) {
    if (buzzer == 1) analogWrite(18, 0);
    buzzer--;
  }

  sendJSON();
  delay(10);
}