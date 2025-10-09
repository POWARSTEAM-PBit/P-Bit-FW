#include <TFT_eSPI.h>
#include <dhtESP32-rmt.h>
#include <SPI.h>
#include <ESP32RotaryEncoder.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define MAX_ADC_READING           1023
#define VCC_REF_VOLTAGE           3300
#define REF_RESISTANCE            10000     // measure this for best results
#define LUX_CALC_SCALAR           12518931 // from experiment
#define LUX_CALC_EXPONENT         -1.405   // from experiment

#define RGB_R   05
#define RGB_G   17
#define RGB_B   16

#define BUZZER_TIME_ON  1

// Encoder pins
const uint8_t DI_ENCODER_A   = 14;
const uint8_t DI_ENCODER_B   = 12;
const int8_t  DI_ENCODER_SW  = 13;
const int8_t  DO_ENCODER_VCC = -1;

// DS18B20 configuration
#define ONE_WIRE_BUS 32  // Pin where the DS18B20 is connected

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

TFT_eSPI tft = TFT_eSPI();
float temp;
float hum;
float ds18b20_temp = 0.0;  // Variable to store DS18B20 temperature

unsigned int timeCount;
RotaryEncoder rotaryEncoder( DI_ENCODER_A, DI_ENCODER_B, DI_ENCODER_SW, DO_ENCODER_VCC );
unsigned int screen;
int audio[20];
int audio_p = 0;
int refresh = 100;
int rgb_state = 0;
int buzzer = BUZZER_TIME_ON;

void knobCallback( long value )
{
  buzzer = BUZZER_TIME_ON;
  analogWrite(18, 63);
  screen = value;
  timeCount = 100;
}

// Function to initialize sensors
void setup()
{
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);

  // Initialize DS18B20 sensor with debugging information
  sensors.begin();
  Serial.println("DS18B20 sensor initialized on GPIO32.");
}

// Function to read temperature from DS18B20
void readDS18B20()
{
  sensors.requestTemperatures();
  ds18b20_temp = sensors.getTempCByIndex(0);
  Serial.print("DS18B20 Temperature: ");
  Serial.println(ds18b20_temp);
}

// Update display based on encoder screen value
void loop()
{
  // Read DS18B20 temperature periodically
  if (screen == 1) {
    readDS18B20();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.print("DS18B20 Temp:");
    tft.setCursor(0, 30);
    tft.printf("%.2f C", ds18b20_temp);
    delay(500);
  }
  // Other screen handling
}
