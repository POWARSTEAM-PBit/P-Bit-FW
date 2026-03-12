// main.cpp
// Núcleo principal del P-Bit.
// Gestiona la inicialización de módulos, las variables globales y la lógica de energía.

#include <Arduino.h>
#include "tft_display.h" // Cabecera principal de la UI
#include "io.h"         
#include "ble.h"
#include "rotary.h"
#include "hw.h"
#include "led_control.h" 
#include <esp_sleep.h> // Para Light/Deep Sleep
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/rtc_io.h>
#include "ui_widgets.h" // Para tft
#include "ui_boot.h"    // Para la secuencia de arranque
#include "lang_select.h" // Para selección de idioma
#include "config.h"

#define SERIAL_BAUD_RATE 115200

// --- DEFINICIONES DE VARIABLES GLOBALES ---
bool g_is_fahrenheit = false; // Estado C/F
bool g_sound_enabled = true; // El sonido está activado por defecto

// Variables de gestión de energía
volatile unsigned long g_last_activity_ms = 0;
volatile PowerMode g_power_mode = POWER_ACTIVE;

// --- DECLARACIONES EXTERNAS ---
extern bool client_connected; // Desde ble.cpp
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

static void saveCurrentScreenForSleep() {
    if (isRestorableScreen(active_screen)) {
        g_last_active_screen_before_sleep = active_screen;
        g_rtc_last_active_screen = static_cast<uint8_t>(active_screen);
    }
}

static void persistPowerState(PowerMode mode, SleepIntent intent) {
    g_rtc_last_power_mode = static_cast<uint8_t>(mode);
    g_rtc_last_sleep_intent = static_cast<uint8_t>(intent);
}

static void restorePersistedScreen() {
    active_screen = getPersistedSleepScreen();
    g_last_active_screen_before_sleep = active_screen;
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
    g_ui_overlay_state = UI_OVERLAY_SLEEP_WARNING;
}

static void enterDeepSleepMode() {
    Serial.println("[Power] Entering Deep Sleep.");
    saveCurrentScreenForSleep();
    persistPowerState(g_power_mode, SLEEP_INTENT_DEEP_SLEEP);
    playSleepSignal(3, 255, 0, 0, DEEP_SLEEP_BEEP_HZ);
    g_ui_overlay_state = UI_OVERLAY_BLACKOUT;
    vTaskDelay(pdMS_TO_TICKS(40));
    set_rgb(0, 0, 0);

    // El ST7735 no tiene control de backlight por pin en esta placa.
    // Para evitar que el panel quede blanco al perder contexto SPI,
    // lo forzamos a display-off y sleep-in antes del deep sleep.
    tft.writecommand(0x28); // DISPOFF
    tft.writecommand(0x10); // SLPIN
    vTaskDelay(pdMS_TO_TICKS(120));

    // GPIO13 es RTC IO. Lo dejamos alto con pull-up RTC para evitar
    // wakes espurios o reboots aparentes al entrar en deep sleep.
    pinMode((uint8_t)DI_ENCODER_SW, INPUT_PULLUP);
    rtc_gpio_init((gpio_num_t)DI_ENCODER_SW);
    rtc_gpio_set_direction((gpio_num_t)DI_ENCODER_SW, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en((gpio_num_t)DI_ENCODER_SW);
    rtc_gpio_pulldown_dis((gpio_num_t)DI_ENCODER_SW);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)DI_ENCODER_SW, LOW);
    esp_deep_sleep_start();
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
    
    // Inicialización de módulos
    set_devicename();
    init_tft_display();
    init_ble();
    init_leds_and_buzzer();
    init_hw();

    // ---------------------------------------------------------
    // LÓGICA DE ARRANQUE SILENCIOSO (Evita animación al despertar)
    // ---------------------------------------------------------

    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    esp_reset_reason_t reset_reason = esp_reset_reason();
    logBootDiagnostics(wakeup_reason, reset_reason);

    switch(wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0 : // Despertar por Botón (GPIO 13)
            Serial.println("[Power] Waking up from Deep Sleep (Button).");
            loadLanguage();           // Cargar idioma desde NVS sin mostrar menú
            restorePersistedScreen();
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            break;

        default : // Encendido normal (Power On)
            Serial.println("[Power] Cold Boot detected. Running boot sequence.");
            run_boot_sequence();
            active_screen = TEMP_SCREEN;
            g_last_active_screen_before_sleep = active_screen;
            g_is_fahrenheit = false;
            g_power_mode = POWER_ACTIVE;
            persistPowerState(POWER_ACTIVE, SLEEP_INTENT_NONE);
            showLanguageMenu();       // Siempre en cold boot — usa rotaryEncoder internamente
            g_is_fahrenheit = false;
            break;
    }
    // ---------------------------------------------------------

    // init_rotary() reconfigura el encoder con límites y callbacks para la app principal
    init_rotary();
    g_is_fahrenheit = false;

    // El contador de inactividad empieza cuando la app ya está lista.
    // Así evitamos que boot + menú de idioma dejen un sleep "pendiente".
    g_last_activity_ms = now_ms();

    // --- FreeRTOS Tasks ---

    // Tarea de Pantalla (UI): Núcleo 1
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
    
    // Tarea de Sensores (Núcleo 0)
    BaseType_t sensor_task_ok = xTaskCreatePinnedToCore(
        sensor_reading_task, 
        "SensorTask",        
        4096,                
        NULL,                
        1,                   // Prioridad 1
        NULL,                
        0                    // Asignar al Núcleo 0
    );
    if (sensor_task_ok != pdPASS) failFastOnTaskCreateError("SensorTask");
}

void loop() {
    // El loop principal (Núcleo 1) solo gestiona tareas rápidas, debug y energía
    rotaryEncoder.loop();
    loop_buzzer(); 
    
    unsigned long inactivity_time = now_ms() - g_last_activity_ms;

    if (g_power_mode == POWER_ACTIVE && inactivity_time >= IDLE_TIMEOUT_MS) {
        enterIdleMode();
    }

    if (inactivity_time >= DEEP_SLEEP_TIMEOUT_MS && !client_connected) {
        enterDeepSleepMode();
    }
    
    vTaskDelay(pdMS_TO_TICKS(10)); 
}
