// Scene: sound_saved_calibration
// Source: src/ui_sound.cpp
// Variant: visualizer-safe / saved
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SONIDO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_MAGENTA);
tft.fillRect(0, 38, 160, 22, TFT_BLACK);
tft.fillRect(0, 60, 160, 48, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.drawString("Guardado", 80, 44);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Sil < 20", 80, 68);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Nor < 60", 80, 84);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("Alt < 85", 80, 100);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Pulsa para menu", 80, 118);
