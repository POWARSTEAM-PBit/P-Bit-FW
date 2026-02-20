// ui_timer.cpp
// Dibuja la pantalla del Cronómetro, optimizada para fluidez.

#include "ui_timer.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard, drawTimerCardContent
#include "timer.h"      // Para estados del timer y getTimeHMS()

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
    const char * stateText = "READY";
    // 🟢 NUEVO (Problema 4): Texto de instrucción
    const char * instructionText = "Push-Start";
    
    if (userTimerRunning) {
        current_timer_state = 1; // RUNNING
        borderColor = TFT_GREEN;
        newColor = TFT_GREEN;
        stateText = "RUNNING";
        instructionText = "Push-Pause"; // Instrucción cuando está corriendo
    } else if (userTimerElapsed > 0) {
        current_timer_state = 2; // PAUSED
        borderColor = TFT_YELLOW;
        newColor = TFT_YELLOW;
        stateText = "PAUSED";
        instructionText = "Hold-Reset / Push-Run"; // Instrucción cuando está en pausa
    }

    // 2. DETECTAR CAMBIO VISUAL DE ESTADO
    bool state_changed_visually = screen_changed || g_timer_just_reset || (current_timer_state != last_drawn_state);

    // 3. DIBUJO ESTÁTICO (Título)
    if (screen_changed) {
        drawHeader("Timer", TFT_ORANGE);
    }
    
    // 4. DIBUJO SEMI-ESTÁTICO (Marco, Estado E INSTRUCCIONES)
    if (state_changed_visually) {
        
        // 🟢 NUEVO: Dibujar las instrucciones (Problema 4)
        // Limpiar el área de instrucciones (entre el Header (Y=32) y la Tarjeta (Y=55))
        tft.fillRect(0, 35, tft.width(), 18, TFT_BLACK); 
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString(instructionText, cx, 40, 1); // Y=40
        
        // Dibuja el marco, el texto de estado y el tiempo (resuelve recuadro cortado)
        drawTimerCardContent(cx, cy, borderColor, newColor, stateText, getTimeHMS());

        // Actualizar el último estado dibujado
        last_drawn_state = current_timer_state; 
    }
    
    // 5. DIBUJO DINÁMICO (Tiempo) - Se ejecuta 100 veces por segundo.
    if (timer_needs_update && !state_changed_visually) { 

        const char * time = getTimeHMS();
        
        // 🟢 FIX (Problema 3): Eliminamos el fillRect que causaba el flickeo.
        // tft.fillRect(cx - 35, 88, 70, 20, TFT_BLACK); // 🔴 ELIMINADO
        
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(newColor, TFT_BLACK); // El fondo negro sobrescribe
        tft.drawString(time, cx, 95, 4); 
    }
}