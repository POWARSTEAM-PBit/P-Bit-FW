#include "tft_display.h"

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

typedef enum {
    SCREEN_1,
    SCREEN_2,
    SCREEN_3
} Screen;

Screen active_screen = SCREEN_1;
unsigned long last_update = 0;

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
    while (1) {
        unsigned long now = millis();

        if ((now - last_update) > 2000) {
            last_update = now;
            clear_screen();

            switch (active_screen) {
                case SCREEN_1:
                    // TODO: Implement SCREEN_1 display logic
                    // Cool gradient background
    for (int y = 0; y < tft.height(); y++) {
        uint8_t r = map(y, 0, tft.height(), 0, 255);
        uint8_t g = map(y, 0, tft.height(), 50, 200);
        uint8_t b = map(y, 0, tft.height(), 100, 255);
        uint16_t color = tft.color565(r, g, b);
        tft.drawFastHLine(0, y, tft.width(), color);
    }

    // Stylized logo / text overlay
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("P-BIT", tft.width() / 2, tft.height() / 2 - 20, 6);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Firmware", tft.width() / 2, tft.height() / 2 + 20, 2);

    // Decorative circles
    tft.fillCircle(20, 20, 10, TFT_RED);
    tft.fillCircle(tft.width() - 20, tft.height() - 20, 10, TFT_BLUE);
    break;

                case SCREEN_2:
                    // TODO: Implement SCREEN_2 display logic
                    tft.setTextColor(TFT_CYAN, TFT_BLACK);
                    tft.fillRect(20, 30, 100, 60, TFT_CYAN);
                    tft.drawString("Rectangle", 64, 110, 2);
                    break;

                case SCREEN_3:
                    // TODO: Implement SCREEN_3 display logic
                    tft.fillCircle(80, 60, 30, TFT_RED);
                    tft.setTextColor(TFT_WHITE, TFT_BLACK);
                    tft.drawString("Circle", 64, 110, 2);
                    break;

                default:
                    // Optionally handle unexpected values
                    break;
            }

            // Cycle to next screen
            active_screen = (Screen)((active_screen + 1) % 3);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Yield to other tasks, avoid busy wait
    }
}
