#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>
#include "io.h"
#include "hw.h"
#include "misc.h"

TFT_eSPI tft = TFT_eSPI();

Screen active_screen = SCREEN_1;
unsigned long last_update = 0;


void init_tft_display() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
}

void clear_screen() {
    tft.fillScreen(TFT_BLACK);
}

void switch_screen(void * param) {
    Screen last_drawn = SCREEN_1;
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
                case SCREEN_1:
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
                case SCREEN_2:
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
                case SCREEN_3:
                    if (screen_changed) {
                        tft.setTextColor(TFT_GREEN, TFT_BLACK);
                        tft.drawString("System Info", cx, 10, 2);
                        tft.setTextDatum(MC_DATUM);
                        tft.setTextColor(TFT_WHITE, TFT_BLACK);
                        tft.drawString("Battery:", cx, cy - line - 8, 1);
                        tft.drawString("Device:", cx, cy + line - 8, 1);
                    }

                    if (sensor_ready) {
                        tft.setTextPadding(80);
                        tft.drawString(String(current_reading.batt, 1) + " %", cx, cy - line + 8, 1);
                        tft.drawString(String(dev_name), cx, cy + line + 8, 1);
                        tft.setTextPadding(0);
                    }
                    break;
                case SCREEN_4:
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
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
