#pragma once

typedef enum {
    SCREEN_1,
    SCREEN_2,
    SCREEN_3,
    SCREEN_4
} Screen;

extern Screen active_screen;

void init_tft_display();

/**
 * @brief The following thread automatically switches screen
 * when the rotary nob is turned.
 */
void switch_screen(void *param);
