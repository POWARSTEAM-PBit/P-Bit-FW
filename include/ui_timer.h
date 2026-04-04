// ui_timer.h
#pragma once
#include "tft_display.h"

// Public API. The timer renderer needs the update flag to redraw only the
// time sprite while the timer is running.
void draw_timer_screen(bool screen_changed, bool data_changed, bool timer_needs_update);
