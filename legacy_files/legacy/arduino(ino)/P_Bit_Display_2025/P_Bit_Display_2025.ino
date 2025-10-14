/******************************************************
 * Project: P-Bit Display 2025 (Soft Edition)
 * Author: Anssar Almashama
 * Description:
 *   Full startup sequence and sensor dashboard for the
 *   POWAR STEAM P-Bit device.
 *
 *   Sequence:
 *   1️⃣ Logo Screen  → beige background (can be white)
 *   2️⃣ Neon Screen  → black background, green "P-Bit"
 *   3️⃣ Sensor Screens → Temperature, Humidity, Light,
 *                         Soil Moisture, Soil Temp, Sound
 ******************************************************/

#include <TFT_eSPI.h>              // TFT display library
#include <OneWire.h>               // For DS18B20 temperature sensor
#include <DallasTemperature.h>     // DS18B20 management
#include <ESP32RotaryEncoder.h>    // Rotary encoder input

// ==== Pin Configuration ====
#define BUZZER_PIN     25
#define ONE_WIRE_BUS   32
#define ENCODER_A      14
#define ENCODER_B      12
#define ENCODER_SW     13

// ==== Display Setup ====
TFT_eSPI tft = TFT_eSPI();
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RotaryEncoder rotaryEncoder(ENCODER_A, ENCODER_B, ENCODER_SW, -1);

// ==== Color Palette ====
// NOTE: This beige color can be changed to white if preferred.
#define COLOR_BEIGE      tft.color565(252, 249, 234)
#define COLOR_HUMIDITY   tft.color565(186, 223, 219)
#define COLOR_LIGHT      tft.color565(248, 247, 186)
#define COLOR_TEMP       tft.color565(245, 210, 210)
#define COLOR_SOIL       tft.color565(213, 231, 181)
#define COLOR_SOIL_HEAT  tft.color565(217, 160, 104)
#define COLOR_SOUND      tft.color565(51, 150, 211)
#define COLOR_TEXT_DARK  tft.color565(19, 36, 64)
#define COLOR_TEXT_LIGHT tft.color565(252, 249, 234)
#define COLOR_NEON       tft.color565(51, 237, 17)

// ==== Variables ====
unsigned int screen = 0;
unsigned long lastUpdate = 0;
float ds18b20_temp = 0.0;

// ==== Function Declarations ====
void showLogoScreen();
void showSplashScreen();
void drawTemperature();
void drawHumidity();
void drawLight();
void drawSoilMoisture();
void drawSoilTemp();
void drawSound();
void beepSwitch();
void startupTone();

// ==== SETUP ====
void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(1);
  sensors.begin();
  pinMode(BUZZER_PIN, OUTPUT);

  // Rotary encoder initialization
  rotaryEncoder.setAcceleration(0);
  rotaryEncoder.setBoundaries(0, 5, false);
  rotaryEncoder.reset();

  // Step 1: Logo
  showLogoScreen();

  // Step 2: Splash (P-Bit)
  showSplashScreen();

  // Step 3: Initial screen
  drawTemperature();
}

// ==== LOOP ====
void loop() {
  rotaryEncoder.loop();

  // Check encoder rotation
  if (rotaryEncoder.isEncoderButtonClicked()) {
    // Optional: handle button press if needed
  }

  if (rotaryEncoder.isEncoderValueChanged()) {
    beepSwitch(); // Short beep for feedback
    screen = rotaryEncoder.readEncoder();

    switch (screen) {
      case 0: drawTemperature(); break;
      case 1: drawHumidity(); break;
      case 2: drawLight(); break;
      case 3: drawSoilMoisture(); break;
      case 4: drawSoilTemp(); break;
      case 5: drawSound(); break;
    }
  }
}

/* ======================================================
 *                 DISPLAY FUNCTIONS
 * ====================================================*/

// ---- Logo Screen ----
void showLogoScreen() {
  tft.fillScreen(COLOR_BEIGE);
  // Draw logo at center (assuming 128x128 logo) -NEEDS TO BE UPDATED
  // 🔹 Insert logo drawing here after image converted it to .h
  // e.g. tft.pushImage(x, y, width, height, logoBitmap);

  delay(1000); // Display logo for 1 second

  // Startup ascending tone (chime)
  startupTone();

  // Fast fade-out effect (simple screen clear for performance)
  for (int i = 0; i < 3; i++) {
    tft.fillScreen(COLOR_BEIGE);
    delay(100);
    tft.fillScreen(TFT_BLACK);
    delay(100);
  }
  tft.fillScreen(TFT_BLACK);
}

// ---- Splash Screen ("P-Bit") ----
void showSplashScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(COLOR_NEON);
  tft.setTextFont(7);  // Digital-clock style font
  tft.setTextSize(1);
  tft.setCursor(40, 60);
  tft.print("P-Bit");
  delay(1000);
}

// ---- Temperature ----
void drawTemperature() {
  sensors.requestTemperatures();
  ds18b20_temp = sensors.getTempCByIndex(0);

  tft.fillScreen(COLOR_TEMP);
  tft.setTextColor(COLOR_TEXT_DARK);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("🌡 Temperature");

  tft.setTextSize(4);
  tft.setCursor(20, 60);
  if (ds18b20_temp == DEVICE_DISCONNECTED_C) {
    tft.print("Error");
  } else {
    tft.printf("%.1f C", ds18b20_temp);
  }
}

// ---- Humidity ----
void drawHumidity() {
  tft.fillScreen(COLOR_HUMIDITY);
  tft.setTextColor(COLOR_TEXT_DARK);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("💧 Humidity");

  tft.setTextSize(4);
  tft.setCursor(40, 60);
  tft.print("45.2 %"); // Placeholder
}

// ---- Light ----
void drawLight() {
  tft.fillScreen(COLOR_LIGHT);
  tft.setTextColor(COLOR_TEXT_DARK);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("🔆 Light");

  tft.setTextSize(4);
  tft.setCursor(50, 60);
  tft.print("820 lx"); // Placeholder
}

// ---- Soil Moisture ----
void drawSoilMoisture() {
  tft.fillScreen(COLOR_SOIL);
  tft.setTextColor(COLOR_TEXT_DARK);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("🌱 Soil Moisture");

  tft.setTextSize(4);
  tft.setCursor(40, 60);
  tft.print("67 %"); // Placeholder
}

// ---- Soil Temperature ----
void drawSoilTemp() {
  tft.fillScreen(COLOR_SOIL_HEAT);
  tft.setTextColor(COLOR_TEXT_LIGHT);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("🌿🌡 Soil Temp");

  tft.setTextSize(4);
  tft.setCursor(30, 60);
  tft.print("26.8 C"); // Placeholder
}

// ---- Sound ----
void drawSound() {
  tft.fillScreen(COLOR_SOUND);
  tft.setTextColor(COLOR_TEXT_LIGHT);
  tft.setTextFont(2);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("🔊 Sound");

  tft.setTextSize(4);
  tft.setCursor(40, 60);
  tft.print("54 %"); // Placeholder
}

/* ======================================================
 *                 SOUND FUNCTIONS
 * ====================================================*/

// Short beep on screen switch
void beepSwitch() {
  tone(BUZZER_PIN, 1200, 80);
  delay(100);
  noTone(BUZZER_PIN);
}

// Startup ascending chime
void startupTone() {
  for (int freq = 800; freq < 1500; freq += 50) {
    tone(BUZZER_PIN, freq, 30);
    delay(30);
  }
  noTone(BUZZER_PIN);
}
