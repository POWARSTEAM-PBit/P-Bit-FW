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
#include <nvs_flash.h>
#include <driver/rtc_io.h>
#include <stdio.h>
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
#include "settings_store.h"
#include "layout.h"
#include "runtime_events.h"
#include "alert_engine.h"
#if PBIT_ENABLE_GRAPH_LAB
#include "sensor_zone.h"
#endif

#define SERIAL_BAUD_RATE 115200

// --- GLOBAL STATE ---
bool g_is_fahrenheit = false; // Shared temperature unit flag
bool g_sound_enabled; // Loaded from NVS during hardware init


// Power-management state used by the UI router and the rotary driver.
volatile unsigned long g_last_activity_ms = 0;
volatile PowerMode g_power_mode = POWER_ACTIVE;

RTC_DATA_ATTR uint8_t g_rtc_last_active_screen = FIRST_APP_SCREEN;
RTC_DATA_ATTR uint8_t g_rtc_last_power_mode = POWER_ACTIVE;
RTC_DATA_ATTR uint8_t g_rtc_last_sleep_intent = 0;
RTC_DATA_ATTR uint32_t g_rtc_boot_counter = 0;

enum SleepIntent : uint8_t {
    SLEEP_INTENT_NONE = 0,
    SLEEP_INTENT_IDLE = 1,
    SLEEP_INTENT_DEEP_SLEEP = 2
};

#if PBIT_ENABLE_GRAPH_LAB
static bool isHiddenRestoreScreen(Screen screen) {
    switch (screen) {
        case LAB_DASH_OVERVIEW_SCREEN:
        case LAB_ICON_SET_A_SCREEN:
        case LAB_ICON_SET_B_SCREEN:
        case LAB_ICON_SET_C_SCREEN:
        case LAB_ICON_SIZES_ENV_SCREEN:
        case LAB_ICON_SIZES_EXT_SCREEN:
        case LAB_ICON_TEST_SCREEN:
            return true;
        default:
            return false;
    }
}
#endif

static bool isRestorableScreen(Screen screen) {
    if (screen < TEMP_SCREEN || screen > LAST_APP_SCREEN) {
        return false;
    }
#if PBIT_ENABLE_GRAPH_LAB
    if (isHiddenRestoreScreen(screen)) {
        return false;
    }
#endif
    return true;
}

static Screen getPersistedSleepScreen() {
    Screen persisted = static_cast<Screen>(g_rtc_last_active_screen);
    return isRestorableScreen(persisted) ? persisted : FIRST_APP_SCREEN;
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

    DPRINTLN("[Power] Entering IDLE mode.");
    saveCurrentScreenForSleep();
    playSleepSignal(2, 255, 80, 0, IDLE_BEEP_HZ);
    set_rgb(0, 0, 0);

    g_power_mode = POWER_IDLE;
    persistPowerState(POWER_IDLE, SLEEP_INTENT_IDLE);
    runtime_set_ui_overlay(UI_OVERLAY_SLEEP_WARNING);
}

static void logBootDiagnostics(esp_sleep_wakeup_cause_t wakeup_reason, esp_reset_reason_t reset_reason) {
    DPRINT("[Boot] Reset reason: %d\n", (int)reset_reason);
    DPRINT("[Boot] Wakeup cause: %d\n", (int)wakeup_reason);
    DPRINT("[Boot] RTC last screen: %u\n", g_rtc_last_active_screen);
    DPRINT("[Boot] RTC last power mode: %u\n", g_rtc_last_power_mode);
    DPRINT("[Boot] RTC last sleep intent: %u\n", g_rtc_last_sleep_intent);
    DPRINT("[Boot] RTC boot counter: %lu\n", (unsigned long)g_rtc_boot_counter);
}

static void failFastOnTaskCreateError(const char* task_name) {
    DPRINT("[RTOS] ERROR: No se pudo crear la tarea %s\n", task_name);
    delay(200);
    esp_restart();
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    {
        esp_err_t nvs_ret = nvs_flash_init();
        if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            nvs_flash_erase();
            nvs_flash_init();
        }
    }
    g_rtc_boot_counter++;
    
    // Module initialization.
    set_devicename();
    init_tft_display();

    // BLE is factory-disabled. The build-hash reset above clears ble_en on every
    // new flash, so the device always ships with BLE off until unlocked via the
    // secret 60 s hold gesture on SYSTEM_SCREEN.
    if (load_ble_enabled_store()) {
        init_ble();
    }
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
    if (reset_reason == ESP_RST_PANIC ||
        reset_reason == ESP_RST_TASK_WDT ||
        reset_reason == ESP_RST_INT_WDT) {
        DPRINTLN("[Boot] WARNING: Previous boot ended abnormally (panic/WDT).");
    }

    // Factory reset guard: wipe NVS every time a new binary is flashed.
    // Uses FNV-1a hash of the compile timestamp — unique per build, no manual versioning needed.
    auto fw_build_hash = []() -> uint32_t {
        const char* s = __DATE__ " " __TIME__;
        uint32_t h = 2166136261u;
        for (; *s; ++s) h = (h ^ (uint8_t)*s) * 16777619u;
        return h;
    };
    const uint32_t kBuildHash = fw_build_hash();
    const uint32_t stored_stamp = load_fw_build_stamp_store();
    const bool settings_wiped = (stored_stamp != kBuildHash);
    if (settings_wiped) {
        DPRINT("[Boot] New firmware detected (stamp %08X->%08X) -- wiping NVS.\n",
               stored_stamp, kBuildHash);
        clear_all_settings_store();
        save_fw_build_stamp_store(kBuildHash);
    }

    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : // Woke up from the encoder button (GPIO 13)
            DPRINTLN("[Power] Waking up from Deep Sleep (Button).");
            loadLanguage();           // Restore the last language without showing the selector
            restorePersistedScreen();
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            break;

        default : // Cold boot / power-on
            DPRINTLN("[Power] Cold Boot detected. Running boot sequence.");
            run_boot_sequence();
            active_screen = FIRST_APP_SCREEN;
            runtime_set_last_active_screen_before_sleep(active_screen);
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            if (settings_wiped) {
                // First boot with this firmware version — force language selection.
                showLanguageMenu();
            } else {
                // Returning cold boot — language already confirmed, load silently.
                loadLanguage();
            }
            tft.fillScreen(TFT_BLACK); // Force a clean slate before the first app screen redraw.
            delay(25);                // Give the display driver time to settle.
            tft.fillScreen(TFT_BLACK);
            g_is_fahrenheit = false;
            break;
    }
    // -----------------------------------------------------------------

    // Load sensor-zone NVS state (sensor selection + per-sensor viz modes).
#if PBIT_ENABLE_GRAPH_LAB
    sz_init();
#endif

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
