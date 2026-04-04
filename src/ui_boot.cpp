// ui_boot.cpp
// Dynamic boot sequence renderer used during startup.

#include "ui_boot.h"
#include "ui_widgets.h" // Para tft
#include "hw.h"         // Para dev_name
#include "led_control.h"// Para set_rgb() y play_tone_blocking()

// Include the two logo assets used by the startup animation.
#include "img_logo_cuadrado.h" 
#include "img_POWAR_logo_WEB.h"

// External declarations for the logo image arrays.
extern const uint16_t PBIT_TFT_160x128_2[]; 
extern const uint16_t POWAR_logo_WEB[]; 

// The local play_tone_blocking implementation was removed; the shared helper
// now lives in led_control.cpp.

namespace {
struct BootStep {
    int freq_hz;
    int tone_ms;
    int gap_ms;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct BootProfile {
    const BootStep* steps;
    size_t step_count;
    size_t logo_switch_after_step;
    int final_hold_ms;
};

enum BootProfileId : uint8_t {
    BOOT_PROFILE_LEGACY = 0,
    BOOT_PROFILE_NEW,
    BOOT_PROFILE_RETRO_ARCADE,
    BOOT_PROFILE_TECH_CLEAN
};

// Active boot profile for this build:
// - LEGACY: original slow sequence
// - NEW: short rising arpeggio
// - RETRO_ARCADE: more playful chiptune signature
// - TECH_CLEAN: cleaner, more restrained signature
constexpr BootProfileId ACTIVE_BOOT_PROFILE = BOOT_PROFILE_TECH_CLEAN;

static const BootStep BOOT_STEPS_LEGACY[] = {
    {523, 100, 500, 255,   0,   0},  // C5
    {659, 100, 500,   0, 255,   0},  // E5
    {784, 200, 100,   0,   0, 255},  // G5
    {1046, 100,   0, 255, 255, 255}, // C6
};

static const BootStep BOOT_STEPS_NEW[] = {
    {392,  65, 25, 255,  40,   0},   // G4
    {523,  70, 25, 255, 140,   0},   // C5
    {659,  75, 25,   0, 220,  80},   // E5
    {784,  90, 20,   0, 120, 255},   // G5
    {1046, 120,  0, 255, 255, 255},  // C6
};

static const BootStep BOOT_STEPS_RETRO_ARCADE[] = {
    {659,   55, 18, 255,  30,   0},  // E5
    {784,   55, 18, 255, 170,   0},  // G5
    {988,   60, 18, 255, 255,   0},  // B5
    {1318,  70, 16,   0, 220, 255},  // E6
    {1568, 100,  0, 255, 255, 255},  // G6
};

static const BootStep BOOT_STEPS_TECH_CLEAN[] = {
    {440,  60, 22,   0, 120, 255},   // A4
    {660,  70, 22,   0, 180, 255},   // E5
    {880,  80, 18,  80, 220, 255},   // A5
    {1175, 95,  0, 255, 255, 255},   // D6
};

static const BootProfile BOOT_PROFILES[] = {
    { BOOT_STEPS_LEGACY,       sizeof(BOOT_STEPS_LEGACY) / sizeof(BOOT_STEPS_LEGACY[0]),       2, 2000 },
    { BOOT_STEPS_NEW,          sizeof(BOOT_STEPS_NEW) / sizeof(BOOT_STEPS_NEW[0]),              2,  850 },
    { BOOT_STEPS_RETRO_ARCADE, sizeof(BOOT_STEPS_RETRO_ARCADE) / sizeof(BOOT_STEPS_RETRO_ARCADE[0]), 2,  720 },
    { BOOT_STEPS_TECH_CLEAN,   sizeof(BOOT_STEPS_TECH_CLEAN) / sizeof(BOOT_STEPS_TECH_CLEAN[0]), 2,  700 },
};
} // namespace

/**
 * @brief Dibuja la secuencia de arranque (animación, luces y sonido).
 * Esta función es intencionalmente bloqueante y se llama DESDE setup().
 */
void run_boot_sequence() {
    const BootProfile& profile = BOOT_PROFILES[(uint8_t)ACTIVE_BOOT_PROFILE];

    // --- SECUENCIA DE ARRANQUE "8-BIT" ---

    // 1. (0.0s) PANTALLA 1: Símbolo (PBIT_TFT_160x128_2)
    int x_draw1 = (tft.width() - PBIT_TFT_160X128_2_WIDTH) / 2;
    int y_draw1 = (tft.height() - PBIT_TFT_160X128_2_HEIGHT) / 2;
    tft.pushImage(x_draw1, y_draw1, PBIT_TFT_160X128_2_WIDTH, PBIT_TFT_160X128_2_HEIGHT, (const uint16_t*)PBIT_TFT_160x128_2);

    for (size_t i = 0; i < profile.step_count; ++i) {
        if (i == profile.logo_switch_after_step) {
            int x_draw2 = (tft.width() - POWAR_LOGO_WEB_WIDTH) / 2;
            int y_draw2 = (tft.height() - POWAR_LOGO_WEB_HEIGHT) / 2;
            tft.pushImage(x_draw2, y_draw2, POWAR_LOGO_WEB_WIDTH, POWAR_LOGO_WEB_HEIGHT, (const uint16_t*)POWAR_logo_WEB);
        }

        const BootStep& step = profile.steps[i];
        set_rgb(step.r, step.g, step.b);
        play_tone_blocking(step.freq_hz, step.tone_ms);
        if (step.gap_ms > 0) {
            vTaskDelay(pdMS_TO_TICKS(step.gap_ms));
        }
    }

    // Retención final según perfil.
    vTaskDelay(pdMS_TO_TICKS(profile.final_hold_ms));

    // Apagado final
    set_rgb(0, 0, 0);
    tft.fillScreen(TFT_BLACK); // Prepara la pantalla para la UI principal
}
