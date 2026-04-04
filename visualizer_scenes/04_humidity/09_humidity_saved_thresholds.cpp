// Scene: humidity_saved_thresholds
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
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Guardado", 80, 44);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("Seco 30%", 80, 68);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Muy húmedo 70%", 80, 84);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Pulsa para menú", 80, 118);
