// Scene: soil_runtime_no_sensor
// Source: src/ui_soil.cpp
// Variant: runtime / no sensor
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SUELO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_GREEN);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("Humedad", 60, 34);
tft.fillRect(0, 49, 118, 50, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.drawString("Sin sensor", 60, 58);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Check J6 (GPIO35)", 60, 74);
tft.setTextFont(0);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.fillRect(0, 98, 118, 28, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_DARKGREY);
tft.fillRect(121, 37, 26, 86, TFT_BLACK);
tft.fillCircle(14, 118, 4, TFT_DARKGREY);
