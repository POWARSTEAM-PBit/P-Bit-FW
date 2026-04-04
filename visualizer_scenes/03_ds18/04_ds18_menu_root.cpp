// Scene: ds18_menu_root
// Source: src/ui_ds18.cpp
// Variant: visualizer-safe / menu root

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("TERMÓMETRO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_ORANGE);

tft.fillRect(0, 38, 160, 22, TFT_BLACK);
tft.fillRect(0, 60, 160, 48, TFT_BLACK);
tft.fillRect(0, 108, 160, 16, TFT_BLACK);

tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Calibración", 80, 46);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("Unidad", 80, 60);
tft.drawString("Alertas", 80, 74);
tft.drawString("Reset", 80, 88);
tft.drawString("Salir", 80, 102);

tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Pulsa para elegir", 80, 118);
