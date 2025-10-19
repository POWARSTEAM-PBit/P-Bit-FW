#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>
#include "io.h"
#include "hw.h"
#include "misc.h"
#include "ble.h"
#include "logo.h"
#include "timer.h"

TFT_eSPI tft = TFT_eSPI();

Screen active_screen = BOOT_SCREEN;
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

void switch_screen(void *param) {
    Screen last_drawn = BOOT_SCREEN;
    Reading current_reading;
    uint32_t last_sensor_update = 0;
    constexpr uint32_t SENSOR_UPDATE_INTERVAL_MS = 1000;
    bool screen_changed = false;
    bool sensor_ready = false;
    const char * q = nullptr;

    while (1) {
        screen_changed = (active_screen != last_drawn);
        sensor_ready = (millis() - last_sensor_update > SENSOR_UPDATE_INTERVAL_MS);

        if (sensor_ready) {
            read_sensors(current_reading);
            last_sensor_update = millis();
        }

        if (screen_changed || sensor_ready) {
            int cx = tft.width() / 2;
            int cy = tft.height() / 2;
            int line = 18;

            if (screen_changed) {
                clear_screen();
                last_drawn = active_screen;
            }

            tft.setTextDatum(TC_DATUM);

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
                        tft.setTextColor(TFT_CYAN, TFT_BLACK);
                        tft.drawString("Environment Data", cx, 10, 2);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.setTextDatum(MC_DATUM);
                        tft.drawString("Temperature:", cx, cy - line - 8, 1);
                        tft.drawString("Humidity:", cx, cy + line - 8, 1);
                    }

                    if (sensor_ready) {
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.setTextPadding(80);
                        tft.drawString(String(current_reading.temperature, 1) + " °C", cx, cy - line + 8, 1);
                        tft.drawString(String(current_reading.humidity, 1) + " %", cx, cy + line + 8, 1);
                        tft.setTextPadding(0);
                    }
                    break;
                }

                case SCREEN_2: {
                    if (screen_changed) {
                        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                        tft.drawString("Light & Sound", cx, 10, 2);
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString("LDR:", cx, cy - line - 8, 1);
                        tft.drawString("Mic:", cx, cy + line - 8, 1);
                    }

                    if (sensor_ready) {
                        tft.setTextPadding(80);
                        tft.drawString(String((int)current_reading.ldr), cx, cy - line + 8, 1);
                        tft.drawString(String((int)current_reading.mic), cx, cy + line + 8, 1);
                        tft.setTextPadding(0);
                    }
                    break;
                }

                case SCREEN_3: {
                    if (screen_changed) {
                        tft.setTextColor(TFT_GREEN, TFT_BLACK);
                        tft.drawString("System Info", cx, 10, 2);
                        tft.setTextDatum(TL_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    }

                    if (sensor_ready) {
                        int x = 60;     // shifted right for better centering
                        int y = 50;     // starting Y position below title
                        int line_h = 20;

                        // Clear only text area (keep title)
                        tft.fillRect(0, y, tft.width(), tft.height() - y, TFT_BLACK);

                        // Device
                        y += line_h;
                        tft.drawString("Device: " + String(dev_name), x, y, 1);

                        // BLE status
                        y += line_h;
                        tft.setTextColor(clientConnected ? TFT_GREEN : TFT_RED, TFT_BLACK);
                        tft.drawString(String("BLE: ") + (clientConnected ? "CONNECTED" : "DISCONNECTED"), x, y, 1);

                        // Reset color
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    }
                    break;
                }

                case SCREEN_4: {
                    if (screen_changed) {
                        tft.setTextDatum(TC_DATUM);
                        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                        tft.drawString("Quote of the Day", cx, 10, 2);
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);

                        q = get_qotd();
                        String quoteStr(q);
                        if (quoteStr.length() > 25) {
                            tft.drawString(quoteStr.substring(0, 25), cx, cy - 10, 1);
                            tft.drawString(quoteStr.substring(25), cx, cy + 10, 1);
                        } else {
                            tft.drawString(quoteStr, cx, cy, 1);
                        }
                    }
                    break;
                }
                case TIMER_SCREEN: {
                    if (screen_changed) {
                        tft.fillScreen(TFT_BLACK);
                        tft.setTextDatum(TC_DATUM);
                        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                        tft.drawString("Timer", cx, 10, 2);
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString("Press knob to start", cx, cy, 2);
                    }

                    if (userTimerRunning || userTimerElapsed > 0) {
                        TimeHMS t;
                        getTimeHMS(t);  // Fill 't' by reference

                        // Format time as HH:MM:SS
                        char timeStr[9]; // 8 chars + null
                        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", t.hours, t.minutes, t.seconds);

                        // Clear previous time area
                        tft.fillRect(0, cy + 20, tft.width(), 40, TFT_BLACK);
                        tft.setTextDatum(MC_DATUM);

                        if (userTimerRunning) {
                            tft.setTextColor(TFT_CYAN, TFT_BLACK);
                            tft.drawString(timeStr, cx, cy + 30, 4);  // bigger font size, adjust as needed
                        } else {
                            tft.setTextColor(TFT_GREEN, TFT_BLACK);
                            tft.drawString("Final: " + String(timeStr), cx, cy + 30, 4);
                        }
                    }
                    break;
}

            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
