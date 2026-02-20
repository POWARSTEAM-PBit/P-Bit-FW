#include "ui_sensors.h"
#include "ui_widgets.h" 
#include "io.h"         

extern Reading global_readings;

void draw_sound_screen(bool sc, bool dc) {
    if (sc) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.setTextSize(2); tft.setCursor(10, 20); tft.print("RUIDO");
        tft.drawCircle(80, 75, 30, TFT_DARKGREY);
    }
    if (dc || sc) {
        int val = global_readings.mic; 
        
        // Indicador visual
        uint16_t color = (val > 50) ? TFT_ORANGE : TFT_GREEN;
        if (val > 80) color = TFT_RED;
        int r = map(val, 0, 100, 5, 28);

        tft.fillCircle(80, 75, 29, TFT_BLACK); // Borrar anterior
        tft.fillCircle(80, 75, r, color);      // Pintar nuevo

        tft.fillRect(0, 110, 160, 18, TFT_BLACK); // Limpiar texto
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2); tft.setCursor(60, 110); 
        tft.print(val); tft.print("%");
    }
}

void draw_soil_screen(bool sc, bool dc) {
    if (sc) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setTextSize(2); tft.setCursor(10, 20); tft.print("HUMEDAD SUELO");
        tft.drawRect(30, 60, 100, 20, TFT_WHITE); // Marco
    }
    if (dc || sc) {
        int val = global_readings.soil_humidity; 
        int w = map(val, 0, 100, 0, 96);
        
        tft.fillRect(32, 62, 96, 16, TFT_BLACK); // Borrar
        tft.fillRect(32, 62, w, 16, TFT_BLUE);   // Llenar
        
        tft.fillRect(0, 90, 160, 30, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(3); tft.setCursor(60, 90); 
        tft.print(val); tft.print("%");
    }
}

void draw_ds18_screen(bool sc, bool dc) {
    if (sc) {
        tft.fillScreen(TFT_NAVY); 
        tft.setTextColor(TFT_WHITE, TFT_NAVY);
        tft.setTextSize(2); tft.setCursor(10, 10); tft.print("TEMP. SONDA");
    }
    if (dc || sc) {
        float val = global_readings.temp_ds18b20;
        tft.fillRect(0, 50, 160, 60, TFT_NAVY);
        tft.setTextColor(TFT_WHITE, TFT_NAVY);
        tft.setTextSize(4); tft.setCursor(20, 60); 
        if(val < -100) {
            tft.setTextSize(2); tft.print("ERROR");
        } else {
            tft.print(val, 1); 
        }
    }
}