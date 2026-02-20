// ui_system.cpp
// Dibuja la pantalla de Información del Sistema.

#include "ui_system.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard
#include "hw.h"         // Para dev_name
#include "ble.h"        // Para client_connected

// DECLARACIONES EXTERNAS
extern bool client_connected;
extern bool g_sound_enabled; 
extern char dev_name[];
extern TFT_eSPI tft; // Aseguramos que tft esté disponible

void draw_system_screen(bool screen_changed, bool data_changed) {
    
    // 1. Dibujo Estático (Solo si la pantalla cambió)
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader("System Info", TFT_GREEN);
        
        // El marco (Tarjeta) va de Y=40 a Y=110
        drawCard(10, 40, tft.width() - 20, 70, TFT_DARKGREY);
    }

    // 2. Dibujo Dinámico (Si la pantalla cambió O los datos cambiaron)
    if (screen_changed || data_changed) {
        
        // 🟢 FIX: Ajustamos la limpieza para que quepa DENTRO del marco.
        // (Y=41 a Y=109). Deja 1px de margen interno.
        tft.fillRect(11, 41, tft.width() - 22, 68, TFT_BLACK);

        // (Mantenemos tu diseño y coordenadas de texto)
        int x_draw = 20; 
        int y_draw = 45; 
        int line_h = 22;
        int textX_Offset = 50; 
        
        tft.setTextDatum(TL_DATUM);
        
        // --- DEVICE NAME ---
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        tft.drawString("DEVICE", x_draw, y_draw, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(dev_name), x_draw, y_draw + 15, 2);
        
        y_draw += line_h + 18;
        
        // --- BLE STATUS ---
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        tft.drawString("BLE", x_draw, y_draw, 2);
        
        uint16_t statusColor = client_connected ? TFT_GREEN : TFT_RED;
        String statusText = client_connected ? "CONNECTED" : "DISCONNECTED";
        tft.setTextColor(statusColor, TFT_BLACK);
        tft.drawString(statusText, x_draw + 30, y_draw, 2);


        // --- PIE DE PÁGINA (Mute) ---
        int cx = tft.width() / 2;
        int footer_y = tft.height() - 15;
        
        // Limpiar el área del pie de página
        tft.fillRect(0, footer_y, tft.width(), 20, TFT_BLACK); 
        
        tft.setTextDatum(TC_DATUM);
        uint16_t soundColor = g_sound_enabled ? TFT_GREEN : TFT_RED;
        String soundText = g_sound_enabled ? "Sound: ON (Push)" : "Sound: OFF (Push)";
        
        tft.setTextColor(soundColor, TFT_BLACK);
        tft.drawString(soundText, cx, footer_y, 1);
    }
}