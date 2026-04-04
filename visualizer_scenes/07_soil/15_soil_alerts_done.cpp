// Scene: soil_alerts_done
// Source: src/ui_soil.cpp
// Variant: visualizer-safe / summary
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SUELO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_GREEN);
tft.fillRect(0, 38, 160, 22, TFT_BLACK);
tft.fillRect(0, 60, 160, 48, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.drawString("Guardado", 80, 44);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Alertas", 80, 78);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("ON", 80, 94);
tft.drawString("Pulsa para menu", 80, 118);
