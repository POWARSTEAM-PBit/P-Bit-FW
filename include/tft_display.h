#pragma once

typedef enum {
    BOOT_SCREEN,
    SCREEN_1,
    SCREEN_2,
    SCREEN_3,
    TIMER_SCREEN
} Screen;

extern volatile Screen active_screen;

/**
 * @brief Initialize the TFT display
 */
void init_tft_display();

/**
 * @brief The following thread automatically switches screen
 * when the rotary nob is turned.
 */
void switch_screen(void *param);
