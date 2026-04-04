// Scene: soil_menu_root
// Source: src/ui_soil.cpp
// Variant: visualizer-safe / menu
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SUELO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_GREEN);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Calibrar sensor", 80, 46);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Editar umbrales", 80, 60);
tft.drawString("Alertas", 80, 74);
tft.drawString("Reset", 80, 88);
tft.drawString("Salir", 80, 102);
tft.fillRect(0, 108, 160, 20, TFT_BLACK);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Pulsa para elegir", 80, 118);
