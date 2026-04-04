// Scene: humidity_menu_root
// Source: src/ui_humidity.cpp
// Variant: exact-layout
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("HUMEDAD", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_CYAN);
tft.fillRect(0, 38, 160, 22, TFT_BLACK);
tft.fillRect(0, 60, 160, 48, TFT_BLACK);
tft.fillRect(0, 108, 160, 16, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Límites", 80, 46);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("Alertas", 80, 62);
tft.drawString("Reset", 80, 78);
tft.drawString("Salir", 80, 94);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Gira y pulsa", 80, 118);
