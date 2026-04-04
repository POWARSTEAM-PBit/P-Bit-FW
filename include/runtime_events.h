#pragma once

#include <Arduino.h>
#include "tft_display.h"

// Runtime event helpers centralize the "set / consume" pattern for the
// cross-task flags owned by the sensor task, rotary driver, and UI router.
// This keeps the current firmware behavior while reducing ad-hoc direct writes
// in the core runtime layer.

void runtime_mark_sensor_data_ready();
bool runtime_take_sensor_data_ready();

// Request a regular UI refresh and optionally force a full redraw.
void runtime_request_ui_refresh(bool force_full = false);
void runtime_request_ui_full_redraw();
bool runtime_take_ui_full_redraw();

void runtime_set_ui_overlay(UiOverlayState state);
UiOverlayState runtime_get_ui_overlay();

void runtime_set_last_active_screen_before_sleep(Screen screen);
Screen runtime_get_last_active_screen_before_sleep();
