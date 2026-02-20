// ui_boot.cpp
// Dibuja la secuencia de arranque dinámica (splash screen).

#include "ui_boot.h"
#include "ui_widgets.h" // Para tft
#include "hw.h"         // Para dev_name
#include "led_control.h"// Para set_rgb() y play_tone_blocking()

// 🟢 Inclusión de las dos imágenes de logo
#include "img_logo_cuadrado.h" 
#include "img_POWAR_logo_WEB.h"

// 🟢 Declaración externa de los arrays de imagen
extern const uint16_t PBIT_TFT_160x128_2[]; 
extern const uint16_t POWAR_logo_WEB[]; 

// 🔴 FIX: Eliminamos la implementación local de play_tone_blocking
// (Ahora se llama desde led_control.cpp)

/**
 * @brief Dibuja la secuencia de arranque (animación, luces y sonido).
 * Esta función es intencionalmente bloqueante y se llama DESDE setup().
 */
void run_boot_sequence() {
    
    int cx = tft.width() / 2;
    int cy = tft.height() / 2;

    // --- SECUENCIA DE ARRANQUE "8-BIT" ---

    // 1. (0.0s) PANTALLA 1: Símbolo (PBIT_TFT_160x128_2)
    int x_draw1 = (tft.width() - PBIT_TFT_160X128_2_WIDTH) / 2;
    int y_draw1 = (tft.height() - PBIT_TFT_160X128_2_HEIGHT) / 2;
    tft.pushImage(x_draw1, y_draw1, PBIT_TFT_160X128_2_WIDTH, PBIT_TFT_160X128_2_HEIGHT, (const uint16_t*)PBIT_TFT_160x128_2);

    // 2. (0.0s) SONIDO: Jingle Nota 1 (Do) + LUZ: Rojo
    set_rgb(255, 0, 0); 
    play_tone_blocking(523, 100); // Tono C5
    
    vTaskDelay(pdMS_TO_TICKS(500)); // Pausa 0.5s

    // 3. (0.6s) SONIDO: Jingle Nota 2 (Mi) + LUZ: Verde
    set_rgb(0, 255, 0);
    play_tone_blocking(659, 100); // Tono E5
    
    vTaskDelay(pdMS_TO_TICKS(500)); // Pausa 0.5s

    // 4. (1.2s) PANTALLA 2: Logo Completo (POWAR_logo_WEB)
    int x_draw2 = (tft.width() - POWAR_LOGO_WEB_WIDTH) / 2;
    int y_draw2 = (tft.height() - POWAR_LOGO_WEB_HEIGHT) / 2;
    tft.pushImage(x_draw2, y_draw2, POWAR_LOGO_WEB_WIDTH, POWAR_LOGO_WEB_HEIGHT, (const uint16_t*)POWAR_logo_WEB);
    
    // 5. (1.2s) SONIDO: Jingle Nota 3 (Sol) + LUZ: Azul
    set_rgb(0, 0, 255);
    play_tone_blocking(784, 200); // Tono G5 (más largo)

    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 6. (1.5s) SONIDO: Jingle Nota 4 (Do agudo) + LUZ: Blanca
    set_rgb(255, 255, 255);
    play_tone_blocking(1046, 100); // Tono C6

    // 7. (1.6s) Pausa final
    vTaskDelay(pdMS_TO_TICKS(2000)); // Mostrar logo final 2 segundos

    // 8. (3.6s) Apagado final
    set_rgb(0, 0, 0);
    tft.fillScreen(TFT_BLACK); // Prepara la pantalla para la UI principal
}