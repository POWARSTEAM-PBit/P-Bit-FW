// main.cpp
// Main firmware entry point.
// This file wires startup, global state, task creation, and the power policy.

#include <Arduino.h>
#include "tft_display.h" // Main UI router
#include "io.h"         
#include "ble.h"
#include "rotary.h"
#include "hw.h"
#include "led_control.h" 
#include <esp_sleep.h> // Sleep modes and wake-up helpers
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/rtc_io.h>
#include "ui_widgets.h" // Owns the global TFT instance
#include "ui_boot.h"    // Boot animation and splash flow
#include "timer.h"
#include "ui_soil.h"
#include "ui_temp.h"
#include "ui_humidity.h"
#include "ui_ds18.h"
#include "ui_sound.h"
#include "ui_light.h"
#include "ui_system.h"
#include "lang_select.h" // Cold-boot language selector
#include "config.h"
#include "runtime_events.h"
#include "alert_engine.h"

#define SERIAL_BAUD_RATE 115200

// --- GLOBAL STATE ---
bool g_is_fahrenheit = false; // Shared temperature unit flag
bool g_sound_enabled; // Loaded from NVS during hardware init


// Power-management state used by the UI router and the rotary driver.
volatile unsigned long g_last_activity_ms = 0;
volatile PowerMode g_power_mode = POWER_ACTIVE;

RTC_DATA_ATTR uint8_t g_rtc_last_active_screen = TEMP_SCREEN;
RTC_DATA_ATTR uint8_t g_rtc_last_power_mode = POWER_ACTIVE;
RTC_DATA_ATTR uint8_t g_rtc_last_sleep_intent = 0;
RTC_DATA_ATTR uint32_t g_rtc_boot_counter = 0;

enum SleepIntent : uint8_t {
    SLEEP_INTENT_NONE = 0,
    SLEEP_INTENT_IDLE = 1,
    SLEEP_INTENT_DEEP_SLEEP = 2
};

static bool isRestorableScreen(Screen screen) {
    return screen >= TEMP_SCREEN && screen <= TIMER_SCREEN;
}

static Screen getPersistedSleepScreen() {
    Screen persisted = static_cast<Screen>(g_rtc_last_active_screen);
    return isRestorableScreen(persisted) ? persisted : TEMP_SCREEN;
}

static unsigned long now_ms() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

static bool settings_ui_active() {
    return soilCalibrationIsActive()
        || temp_menu_is_active()
        || humidity_menu_is_active()
        || ds18_menu_is_active()
        || sound_menu_is_active()
        || light_menu_is_active()
        || system_menu_is_active()
        || timer_menu_is_active();
}

static void saveCurrentScreenForSleep() {
    if (isRestorableScreen(active_screen)) {
        runtime_set_last_active_screen_before_sleep(active_screen);
        g_rtc_last_active_screen = static_cast<uint8_t>(active_screen);
    }
}

static void persistPowerState(PowerMode mode, SleepIntent intent) {
    g_rtc_last_power_mode = static_cast<uint8_t>(mode);
    g_rtc_last_sleep_intent = static_cast<uint8_t>(intent);
}

static void restorePersistedScreen() {
    active_screen = getPersistedSleepScreen();
    runtime_set_last_active_screen_before_sleep(active_screen);
}

static void playSleepSignal(uint8_t flashes, uint8_t r, uint8_t g, uint8_t b, int tone_hz) {
    for (uint8_t i = 0; i < flashes; i++) {
        set_rgb(r, g, b);
        if (g_sound_enabled) play_tone_blocking(tone_hz, 80);
        else                 vTaskDelay(pdMS_TO_TICKS(80));
        set_rgb(0, 0, 0);
        if (i + 1 < flashes) vTaskDelay(pdMS_TO_TICKS(120));
    }
}

static void enterIdleMode() {
    if (g_power_mode == POWER_IDLE) return;

    Serial.println("[Power] Entering IDLE mode.");
    saveCurrentScreenForSleep();
    playSleepSignal(2, 255, 80, 0, IDLE_BEEP_HZ);
    set_rgb(0, 0, 0);

    g_power_mode = POWER_IDLE;
    persistPowerState(POWER_IDLE, SLEEP_INTENT_IDLE);
    runtime_set_ui_overlay(UI_OVERLAY_SLEEP_WARNING);
}

static void logBootDiagnostics(esp_sleep_wakeup_cause_t wakeup_reason, esp_reset_reason_t reset_reason) {
    Serial.printf("[Boot] Reset reason: %d\n", (int)reset_reason);
    Serial.printf("[Boot] Wakeup cause: %d\n", (int)wakeup_reason);
    Serial.printf("[Boot] RTC last screen: %u\n", g_rtc_last_active_screen);
    Serial.printf("[Boot] RTC last power mode: %u\n", g_rtc_last_power_mode);
    Serial.printf("[Boot] RTC last sleep intent: %u\n", g_rtc_last_sleep_intent);
    Serial.printf("[Boot] RTC boot counter: %lu\n", (unsigned long)g_rtc_boot_counter);
}

static void failFastOnTaskCreateError(const char* task_name) {
    Serial.printf("[RTOS] ERROR: No se pudo crear la tarea %s\n", task_name);
    delay(200);
    esp_restart();
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    g_rtc_boot_counter++;
    
    // Module initialization.
    set_devicename();
    init_tft_display();
    init_ble();
    init_leds_and_buzzer();
    alert_engine_reset();
    init_hw();

    // -----------------------------------------------------------------
    // Wake source handling.
    // We keep wake-from-sleep and cold boot separated so the UI can
    // restore the right state without replaying the full startup flow.
    // -----------------------------------------------------------------

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    esp_reset_reason_t reset_reason = esp_reset_reason();
    logBootDiagnostics(wakeup_reason, reset_reason);

    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : // Woke up from the encoder button (GPIO 13)
            Serial.println("[Power] Waking up from Deep Sleep (Button).");
            loadLanguage();           // Restore the last language without showing the selector
            restorePersistedScreen();
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            break;

        default : // Cold boot / power-on
            Serial.println("[Power] Cold Boot detected. Running boot sequence.");
            run_boot_sequence();
            active_screen = TEMP_SCREEN;
            runtime_set_last_active_screen_before_sleep(active_screen);
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            showLanguageMenu();       // Always show the language menu on first boot.
            g_is_fahrenheit = false;
            break;
    }
    // -----------------------------------------------------------------

    // Reconfigure the encoder for the main app after boot/restore flow.
    init_rotary();
    g_is_fahrenheit = false;

    // Start the inactivity timer only once the app is fully ready.
    // This avoids carrying boot/menu time into the sleep scheduler.
    g_last_activity_ms = now_ms();

    // --- FreeRTOS tasks ---

    // UI task on core 1.
    BaseType_t ui_task_ok = xTaskCreatePinnedToCore(
        switch_screen,   
        "SwitchScreen",  
        4096,            
        NULL,            
        1,               
        NULL,            
        1                
    );
    if (ui_task_ok != pdPASS) failFastOnTaskCreateError("SwitchScreen");
    
    // Sensor acquisition task on core 0.
    BaseType_t sensor_task_ok = xTaskCreatePinnedToCore(
        sensor_reading_task, 
        "SensorTask",        
        4096,                
        NULL,                
        1,                   // Priority 1.
        NULL,                
        0                    // Pin to core 0.
    );
    if (sensor_task_ok != pdPASS) failFastOnTaskCreateError("SensorTask");
}

void loop() {
    // The main loop only services fast control tasks, debug, and power policy.
    rotaryEncoder.loop();
    poll_rotary_aux();
    loop_buzzer(); 
    
    unsigned long inactivity_time = now_ms() - g_last_activity_ms;

    uint32_t sleep_timeout_ms = get_sleep_timeout();
    bool auto_sleep_enabled = (sleep_timeout_ms > 0);
    uint32_t idle_timeout_ms = UINT32_MAX;
    if (auto_sleep_enabled) {
        idle_timeout_ms = (sleep_timeout_ms > SLEEP_WARNING_MS)
            ? (sleep_timeout_ms - SLEEP_WARNING_MS)
            : sleep_timeout_ms;
    }
    bool block_sleep = settings_ui_active() || userTimerRunning || !auto_sleep_enabled;

    if (g_power_mode == POWER_ACTIVE && inactivity_time >= idle_timeout_ms && !block_sleep) {
        enterIdleMode();
    }

    if (auto_sleep_enabled &&
        inactivity_time >= sleep_timeout_ms &&
        !client_connected.load() &&
        !block_sleep &&
        g_power_mode != POWER_IDLE) {
        // This hardware revision blanks the TFT on deep sleep.
        // Product-wise we keep a visible idle state with ZZZ until the user wakes it.
        enterIdleMode();
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); 
}
