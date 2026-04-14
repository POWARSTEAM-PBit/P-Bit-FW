// Scene: sensor_lab_soil_no_sensor
// Source: src/ui_lab_focus.cpp
// Variant: visualizer-safe / no-sensor state

tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("SENSOR LAB", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_WHITE);

tft.fillRoundRect(8, 33, 144, 40, 4, tft.color565(8, 12, 18));
tft.drawRoundRect(8, 33, 144, 40, 4, tft.color565(96, 255, 196));
tft.drawFastHLine(16, 58, 9, TFT_GREEN);
tft.drawFastVLine(20, 48, 10, TFT_GREEN);
tft.drawLine(20, 52, 16, 54, TFT_GREEN);
tft.drawLine(16, 54, 18, 50, TFT_GREEN);
tft.drawLine(20, 51, 24, 53, TFT_GREEN);
tft.drawLine(24, 53, 22, 49, TFT_GREEN);
tft.drawLine(20, 48, 18, 45, TFT_GREEN);
tft.drawLine(20, 48, 22, 45, TFT_GREEN);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_GREEN, tft.color565(8, 12, 18));
tft.drawString("SUELO", 33, 44);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, tft.color565(8, 12, 18));
tft.drawString("Sin sensor", 146, 48);

tft.fillRoundRect(8, 78, 144, 34, 4, tft.color565(4, 8, 20));
tft.drawRoundRect(8, 78, 144, 34, 4, TFT_DARKGREY);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, tft.color565(4, 8, 20));
tft.drawString("Sin sensor", 80, 94);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
