// ui_timer.cpp
// Dibuja la pantalla del Cronómetro, optimizada para fluidez.

#include "ui_timer.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard, drawTimerCardContent
#include "timer.h"      // Para estados del timer y getTimeHMS()
#include "languages.h"  // Para L()
#include "fonts.h"      // GFXfont
#include "layout.h"
#include <cstring>

// Declaraciones externas de estado del timer
extern bool userTimerRunning;
extern unsigned long userTimerStart;
extern unsigned long userTimerElapsed;
extern volatile bool g_timer_just_reset;

// 🟢 FIX: La implementación debe aceptar los 3 argumentos
void draw_timer_screen(bool screen_changed, bool data_changed, bool timer_needs_update) {
    
    int cx = tft.width() / 2;
    int cy = tft.height() / 2 + 10;

    // Variable estática para rastrear el último estado dibujado
    static uint16_t last_drawn_state = 0; // 0=READY, 1=RUNNING, 2=PAUSED

    // 1. DETERMINAR EL ESTADO ACTUAL Y COLORES
    uint16_t current_timer_state = 0; // 0=READY
    uint16_t borderColor = TFT_BLUE;
    uint16_t newColor = TFT_BLUE;
    const char * stateText = L(ST_TIMER_RDY);
    const char * instructionText = L(ST_PUSH_START);

    if (userTimerRunning) {
        current_timer_state = 1; // RUNNING
        borderColor = TFT_GREEN;
        newColor = TFT_GREEN;
        stateText = L(ST_TIMER_RUN);
        instructionText = L(ST_PUSH_PAUSE);
    } else if (userTimerElapsed > 0) {
        current_timer_state = 2; // PAUSED
        borderColor = TFT_YELLOW;
        newColor = TFT_YELLOW;
        stateText = L(ST_TIMER_PAU);
        instructionText = L(ST_PUSH_RESET);
    }

    // 2. DETECTAR CAMBIO VISUAL DE ESTADO
    bool state_changed_visually = screen_changed || g_timer_just_reset || (current_timer_state != last_drawn_state);

    // 3. DIBUJO ESTÁTICO (Título)
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_TIMER), TFT_BLUE);
    }
    
    // 4. DIBUJO SEMI-ESTÁTICO (Marco, Estado E INSTRUCCIONES)
    if (state_changed_visually) {

        // Dibujar el card primero — su fillRect interno limpia y=42..117
        drawTimerCardContent(cx, cy, borderColor, newColor, stateText, getTimeHMS());

        // Dibujar la instrucción DESPUÉS del card para que no sea borrada por su fillRect
        // Limpiar zona de instrucción (y=33..47, hasta el fillRect del card que empieza en LT_CARD_Y-2=48)
        tft.fillRect(0, 33, tft.width(), 15, TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.setFreeFont(FONT_SMALL);
        tft.drawString(instructionText, cx, LT_HINT_Y);
        tft.setTextFont(0);

        // Actualizar el último estado dibujado
        last_drawn_state = current_timer_state;
    }

    // 5. DIBUJO DINÁMICO (Tiempo) - Se ejecuta frecuentemente.
    if (timer_needs_update && !state_changed_visually) {

        const char * time = getTimeHMS();

        // Cache: solo redibujar si el string cambió — evita fillRect+drawString innecesarios
        // (GFXfont no limpia el fondo, así que el fillRect previo causaba flickering constante)
        static char last_time_str[16] = "";
        if (strcmp(time, last_time_str) == 0) return;
        strncpy(last_time_str, time, sizeof(last_time_str) - 1);

        // Limpiar zona del tiempo (GFXfont no limpia el fondo automáticamente)
        tft.fillRect(LT_CARD_X + 3, LT_TIME_Y - 12, LT_CARD_W - 6, 24, TFT_BLACK);

        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(newColor, TFT_BLACK);
        tft.setFreeFont(FONT_TIMER);
        tft.drawString(time, cx, LT_TIME_Y);
        tft.setTextFont(0);
    }
}
