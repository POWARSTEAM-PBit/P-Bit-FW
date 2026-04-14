// Scene: sensor_lab_light
// Source: src/ui_lab_focus.cpp
// Variant: visualizer-safe / current compact layout

tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("SENSOR LAB", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_WHITE);

tft.fillRoundRect(8, 33, 144, 40, 4, tft.color565(8, 12, 18));
tft.drawRoundRect(8, 33, 144, 40, 4, tft.color565(92, 176, 255));
tft.fillCircle(20, 49, 3, TFT_YELLOW);
tft.drawCircle(20, 49, 3, TFT_YELLOW);
tft.drawFastHLine(14, 49, 3, TFT_YELLOW);
tft.drawFastHLine(23, 49, 3, TFT_YELLOW);
tft.drawFastVLine(20, 43, 3, TFT_YELLOW);
tft.drawFastVLine(20, 52, 3, TFT_YELLOW);
tft.drawLine(16, 46, 14, 44, TFT_YELLOW);
tft.drawLine(24, 46, 26, 44, TFT_YELLOW);
tft.drawLine(16, 52, 14, 54, TFT_YELLOW);
tft.drawLine(24, 52, 26, 54, TFT_YELLOW);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_YELLOW, tft.color565(8, 12, 18));
tft.drawString("LUZ", 33, 44);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(TFT_CYAN, tft.color565(8, 12, 18));
tft.drawString("88", 136, 42);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_YELLOW, tft.color565(8, 12, 18));
tft.drawString("lux", 139, 45);

tft.fillRoundRect(8, 78, 144, 34, 4, tft.color565(4, 8, 20));
tft.drawRoundRect(8, 78, 144, 34, 4, tft.color565(92, 176, 255));
tft.fillRect(10, 80, 140, 30, tft.color565(4, 8, 20));
tft.drawFastHLine(10, 88, 140, tft.color565(12, 18, 34));
tft.drawFastHLine(10, 98, 140, tft.color565(12, 18, 34));
tft.drawFastVLine(10, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(20, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(30, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(40, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(50, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(60, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(70, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(80, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(90, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(100, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(110, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(120, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(130, 80, 30, tft.color565(68, 90, 120));
tft.drawFastVLine(140, 80, 30, tft.color565(68, 90, 120));
tft.drawLine(12, 92, 68, 92, tft.color565(16,16,22));
tft.drawLine(12, 91, 68, 91, TFT_YELLOW);
tft.drawLine(68, 92, 92, 104, tft.color565(16,16,22));
tft.drawLine(68, 91, 92, 103, TFT_YELLOW);
tft.drawLine(92, 104, 116, 104, tft.color565(16,16,22));
tft.drawLine(92, 103, 116, 103, TFT_YELLOW);
tft.drawLine(116, 104, 124, 86, tft.color565(16,16,22));
tft.drawLine(116, 103, 124, 85, TFT_YELLOW);
tft.drawLine(124, 86, 136, 104, tft.color565(16,16,22));
tft.drawLine(124, 85, 136, 103, TFT_YELLOW);
tft.drawLine(136, 104, 146, 106, tft.color565(16,16,22));
tft.drawLine(136, 103, 146, 105, TFT_YELLOW);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
