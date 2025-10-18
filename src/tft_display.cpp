#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>
#include "io.h"
#include "hw.h"

TFT_eSPI tft = TFT_eSPI();

Screen active_screen = SCREEN_1;
unsigned long last_update = 0;

const char *quotes[] = {
    "Keep it simple.",
    "Make it work, make it right, make it fast.",
    "Stay curious.",
    "Code. Debug. Repeat.",
    "Dream big. Code bigger.",
    "Be the change you debug.",
    "Every bug is a lesson.",
    "Perfection is iteration.",
    "Think twice, code once.",
    "Simplicity is power."
};

constexpr size_t QUOTE_COUNT = sizeof(quotes) / sizeof(quotes[0]);

void init_tft_display() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("TFT LCD Ready", tft.width() / 2, tft.height() / 2, 2);
}

void clear_screen() {
    tft.fillScreen(TFT_BLACK);
}

void switch_screen(void *param) {
    Screen last_drawn = SCREEN_1;
    Reading current_reading;

    while (1) {
        if (active_screen != last_drawn) {
            clear_screen();
            read_sensors(current_reading);
            last_drawn = active_screen;

            // Center positions
            int cx = tft.width() / 2;
            int cy = tft.height() / 2;
            int line = 18; // line spacing

            switch (active_screen) {
                // 🌡️ Screen 1 - Temperature & Humidity
                case SCREEN_1:
                    tft.setTextDatum(TC_DATUM);
                    tft.setTextColor(TFT_CYAN, TFT_BLACK);
                    tft.drawString("Environment Data", cx, 10, 2);

                    tft.setTextDatum(MC_DATUM);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString("Temperature:", cx, cy - line, 1);
                    tft.drawString(String(current_reading.temperature, 1) + " °C", cx, cy, 1);
                    tft.drawString("Humidity: " + String(current_reading.humidity, 1) + " %", cx, cy + line, 1);
                    break;

                // 🔦 Screen 2 - Light & Sound
                case SCREEN_2:
                    tft.setTextDatum(TC_DATUM);
                    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                    tft.drawString("Light & Sound", cx, 10, 2);

                    tft.setTextDatum(MC_DATUM);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString("LDR: " + String((int)current_reading.ldr), cx, cy - line, 1);
                    tft.drawString("Mic: " + String((int)current_reading.mic), cx, cy + line, 1);
                    break;

                // 🔋 Screen 3 - Battery + Device Info
                case SCREEN_3:
                    tft.setTextDatum(TC_DATUM);
                    tft.setTextColor(TFT_GREEN, TFT_BLACK);
                    tft.drawString("System Info", cx, 10, 2);

                    tft.setTextDatum(MC_DATUM);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString("Battery: " + String(current_reading.batt, 1) + " %", cx, cy - line, 1);
                    tft.drawString("Device: " + String(dev_name), cx, cy + line, 1);
                    break;

                // 💬 Screen 4 - Quote of the Day
                case SCREEN_4: {
                    tft.setTextDatum(TC_DATUM);
                    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                    tft.drawString("Quote of the Day", cx, 10, 2);

                    const char *q = quotes[random(QUOTE_COUNT)];
                    tft.setTextDatum(MC_DATUM);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);

                    String quoteStr(q);
                    if (quoteStr.length() > 25) {
                        tft.drawString(quoteStr.substring(0, 25), cx, cy - 10, 1);
                        tft.drawString(quoteStr.substring(25), cx, cy + 10, 1);
                    } else {
                        tft.drawString(quoteStr, cx, cy, 1);
                    }
                    break;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
