#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

Screen active_screen = SCREEN_1;
unsigned long last_update = 0;


const char* quotes[] = {
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
    tft.drawString("TFT LCD Test", tft.width() / 2, tft.height() / 2, 4);
}

void clear_screen() {
    tft.fillScreen(TFT_BLACK);
}

void switch_screen(void *param) {
    Screen last_drawn = SCREEN_1;

    while (1) {
        if (active_screen != last_drawn) {
            clear_screen();
            last_drawn = active_screen;

            switch (active_screen) {
                case SCREEN_1:
                    tft.fillScreen(TFT_BLACK);
                    tft.setTextColor(TFT_GREEN, TFT_BLACK);
                    tft.drawString("Screen 1", 64, 40, 4);
                    tft.drawString("Gradient Mode", 64, 80, 2);
                    break;

                case SCREEN_2:
                    tft.setTextColor(TFT_CYAN, TFT_BLACK);
                    tft.fillRect(20, 30, 100, 60, TFT_CYAN);
                    tft.drawString("Screen 2", 64, 110, 2);
                    break;

                case SCREEN_3:
                    tft.fillCircle(80, 60, 30, TFT_RED);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString("Screen 3", 64, 110, 2);
                    break;
                case SCREEN_4:
                    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
                    tft.drawString("QOTD", 64, 20, 4);

                    // Pick random quote
                    const char* q = quotes[random(QUOTE_COUNT)];
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.setTextDatum(TC_DATUM);

                    // Display centered multi-line text
                    int y = 60;
                    int lineHeight = 20;
                    String quoteStr(q);

                    // Split long quotes manually if desired
                    if (quoteStr.length() > 20) {
                        tft.drawString(quoteStr.substring(0, 20), tft.width() / 2, y, 2);
                        tft.drawString(quoteStr.substring(20), tft.width() / 2, y + lineHeight, 2);
                    } else {
                        tft.drawString(quoteStr, tft.width() / 2, y + 10, 2);
                    }
                    break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // check every 100ms
    }
}

