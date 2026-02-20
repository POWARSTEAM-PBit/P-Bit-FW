// ui_placeholder.cpp
// Dibuja una pantalla genérica para funciones no implementadas.

#include "ui_placeholder.h"
#include "ui_widgets.h" // Para tft y drawHeader
#include "io.h"         // Para Screen
#include "config.h"     // Para Screen

// Variable externa para saber qué pantalla estamos mostrando
extern Screen active_screen;
extern TFT_eSPI tft; // Aseguramos que tft esté disponible

void draw_placeholder_screen(bool screen_changed, bool data_changed) {
    
    // Solo dibujamos si la pantalla ha cambiado (es estático)
    if (!screen_changed) {
        return;
    }

    tft.fillScreen(TFT_BLACK);
    int cx = tft.width() / 2;
    int cy = tft.height() / 2;

    const char* title = "Error";

    // Asignar un título basado en la pantalla activa
    switch (active_screen) {
        case SOUND_SCREEN:
            title = "Sound Sensor";
            break;
        case SOIL_SCREEN:
            title = "Soil Sensor";
            break;
        case DS18B20_SCREEN:
            title = "Ext. Temp Sensor";
            break;
        default:
            title = "Test Screen"; // Fallback (aunque TEST_SCREEN ya no debería llegar aquí)
            break;
    }

    drawHeader(title, TFT_RED);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Screen not", cx, cy - 10, 4);
    tft.drawString("implemented", cx, cy + 20, 4);
}