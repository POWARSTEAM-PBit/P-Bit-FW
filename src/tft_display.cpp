#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>
#include "io.h"
#include "hw.h"
#include "misc.h"
#include "ble.h"
#include "logo.h"
#include "timer.h"
#include "config.h"

TFT_eSPI tft = TFT_eSPI();

volatile Screen active_screen = BOOT_SCREEN;
unsigned long last_update = 0;

void init_tft_display() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_WHITE);

    // Center the logo
    int x = (tft.width() - LOGO_W) / 2;
    int y = (tft.height() - LOGO_H) / 2;
    tft.pushImage(x, y, LOGO_W, LOGO_H, logoBitmaphorizontal_view);
}

void clear_screen() {
    tft.fillScreen(TFT_BLACK);
}

// Helper function to draw a card/panel
void drawCard(int x, int y, int w, int h, uint16_t borderColor) {
    tft.drawRoundRect(x, y, w, h, 6, borderColor);
    tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 5, borderColor);
}

// Helper function to draw a water drop icon
void drawWaterDrop(int x, int y, uint16_t color) {
    // Draw water drop shape (bigger)
    tft.fillCircle(x, y + 5, 7, color);
    tft.fillTriangle(x - 6, y + 5, x + 6, y + 5, x, y - 7, color);
}

// Helper function to draw header with underline
void drawHeader(const char* title, uint16_t color) {
    int cx = tft.width() / 2;
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(title, cx, 8, 4);
    tft.drawFastHLine(20, 32, tft.width() - 40, color);
}

void switch_screen(void *param) {
    Screen last_drawn = BOOT_SCREEN;
    Reading current_reading;
    uint32_t last_sensor_update = 0;
    constexpr uint32_t SENSOR_UPDATE_INTERVAL_MS = pdTICKS_TO_MS(SENSOR_READ_INTERVAL);
    bool screen_changed = false;
    bool sensor_ready = false;
    const char * q = nullptr;
    uint32_t last_timer_second = 0;  // Track last displayed second

    while (1) {
        screen_changed = (active_screen != last_drawn);
        sensor_ready = (millis() - last_sensor_update > SENSOR_UPDATE_INTERVAL_MS);

        if (sensor_ready) {
            read_sensors(current_reading);
            last_sensor_update = millis();
        }

        // Check if timer screen needs update
        bool timer_needs_update = false;
        if (active_screen == TIMER_SCREEN && (userTimerRunning || userTimerElapsed > 0)) {
            uint32_t current_timer_second = userTimerRunning ? 
                (millis() - userTimerStart) / 1000 : userTimerElapsed / 1000;
            timer_needs_update = (current_timer_second != last_timer_second);
        }

        if (screen_changed || sensor_ready || timer_needs_update) {
            int cx = tft.width() / 2;
            int cy = tft.height() / 2;

            if (screen_changed) {
                clear_screen();
                last_drawn = active_screen;
            }

            switch (active_screen) {
                case BOOT_SCREEN: {
                    tft.fillScreen(TFT_WHITE);
                    int x = (tft.width() - LOGO_W) / 2;
                    int y = (tft.height() - LOGO_H) / 2;
                    tft.pushImage(x, y, LOGO_W, LOGO_H, logoBitmaphorizontal_view);
                    break;
                }

                case SCREEN_1: {
                    if (screen_changed) {
                        drawHeader("Environment", TFT_CYAN);
                        
                        // Temperature card
                        drawCard(10, 45, tft.width() - 20, 35, TFT_DARKGREY);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("TEMPERATURE", 20, 55, 2);
                        
                        // Humidity card
                        drawCard(10, 90, tft.width() - 20, 35, TFT_DARKGREY);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("HUMIDITY", 20, 100, 2);
                    }

                    if (sensor_ready) {
                        // Temperature value
                        tft.fillRect(12, 63, tft.width() - 24, 15, TFT_BLACK);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString(String(current_reading.temperature, 1) + " °C", 20, 70, 4);
                        
                        // Humidity value with water drop icon
                        tft.fillRect(12, 108, tft.width() - 24, 15, TFT_BLACK);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        String humidityStr = String(current_reading.humidity, 1) + " %";
                        tft.drawString(humidityStr, 20, 115, 4);
                        
                        // Draw water drop icon next to humidity value
                        int textWidth = tft.textWidth(humidityStr, 4);
                        drawWaterDrop(30 + textWidth, 115, TFT_CYAN);
                    }
                    break;
                }

                case SCREEN_2: {
                    if (screen_changed) {
                        drawHeader("Light & Sound", TFT_YELLOW);
                        
                        // LDR card
                        drawCard(10, 45, tft.width() - 20, 35, TFT_DARKGREY);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("LIGHT SENSOR", 20, 55, 2);
                        
                        // Mic card
                        drawCard(10, 90, tft.width() - 20, 35, TFT_DARKGREY);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("MICROPHONE", 20, 100, 2);
                    }

                    if (sensor_ready) {
                        // LDR value
                        tft.fillRect(12, 63, tft.width() - 24, 15, TFT_BLACK);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString(String((int)current_reading.ldr), 20, 70, 4);
                        
                        // Mic value
                        tft.fillRect(12, 108, tft.width() - 24, 15, TFT_BLACK);
                        tft.setTextDatum(ML_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString(String((int)current_reading.mic), 20, 115, 4);
                    }
                    break;
                }

                case SCREEN_3: {
                    if (screen_changed) {
                        drawHeader("System Info", TFT_GREEN);
                    }

                    if (sensor_ready) {
                        // Main info card
                        tft.fillRect(0, 45, tft.width(), tft.height() - 45, TFT_BLACK);
                        drawCard(10, 50, tft.width() - 20, 75, TFT_DARKGREY);
                        
                        int x = 20;
                        int y = 60;
                        int line_h = 22;

                        // Device name with icon
                        tft.setTextDatum(TL_DATUM);
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("DEVICE", x, y, 2);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString(String(dev_name), x, y + 14, 2);

                        // BLE status with colored indicator
                        y += line_h + 18;
                        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
                        tft.drawString("BLE", x, y, 2);
                        
                        // Status indicator dot and status text on same line
                        uint16_t statusColor = client_connected ? TFT_GREEN : TFT_RED;
                        int statusX = x + 70; // bring closer to label
                        tft.fillCircle(statusX, y + 7, 4, statusColor);

                        // Adjust spacing dynamically depending on text length
                        tft.setTextColor(statusColor, TFT_BLACK);
                        String statusText = client_connected ? "CONNECTED" : "DISCONNECTED";

                        // Calculate text width and align so it stays centered in card area
                        int textWidth = tft.textWidth(statusText, 2);
                        int textX = statusX + 10;  // small offset after the dot
                        if (textX + textWidth > tft.width() - 15) {
                            textX = tft.width() - textWidth - 15; // keep inside card margin
                        }

                        tft.drawString(statusText, textX, y, 2);

                    }
                    break;
                }
                
                case TIMER_SCREEN: {
                    if (screen_changed) {
                        drawHeader("Timer", TFT_ORANGE);
                        
                        if (!userTimerRunning && userTimerElapsed == 0) {
                            tft.setTextDatum(MC_DATUM);
                            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
                            tft.drawString("Press knob to start", cx, cy + 20, 2);
                        }
                    }

                    if (userTimerRunning || userTimerElapsed > 0) {
                        const char * time = getTimeHMS();
                        
                        // Timer display card
                        tft.fillRect(0, 50, tft.width(), 75, TFT_BLACK);
                        drawCard(15, 55, tft.width() - 30, 65, userTimerRunning ? TFT_CYAN : TFT_GREEN);
                        
                        tft.setTextDatum(MC_DATUM);
                        
                        if (userTimerRunning) {
                            // Running state
                            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
                            tft.drawString("ELAPSED", cx, 65, 2);
                            tft.setTextColor(TFT_CYAN, TFT_BLACK);
                            tft.drawString(time, cx, 95, 4);
                        } else {
                            // Stopped state
                            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
                            tft.drawString("FINAL TIME", cx, 65, 2);
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.drawString(time, cx, 95, 4);
                        }
                    }
                    break;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}