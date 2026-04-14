// Scene: sensor_lab_temp
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
tft.drawRoundRect(8, 33, 144, 40, 4, tft.color565(184, 96, 184));
tft.drawRoundRect(18, 45, 5, 10, 2, TFT_ORANGE);
tft.fillCircle(20, 57, 4, TFT_ORANGE);
tft.drawFastVLine(20, 46, 8, TFT_ORANGE);

tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_ORANGE, tft.color565(8, 12, 18));
tft.drawString("TEMP", 33, 44);

tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(TFT_MAGENTA, tft.color565(8, 12, 18));
tft.drawString("27.8", 136, 42);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_ORANGE, tft.color565(8, 12, 18));
tft.drawString("C", 139, 45);

tft.fillRoundRect(8, 78, 144, 34, 4, tft.color565(4, 8, 20));
tft.drawRoundRect(8, 78, 144, 34, 4, tft.color565(184, 96, 184));
tft.fillRect(10, 80, 140, 30, tft.color565(4, 8, 20));
tft.drawFastHLine(10, 88, 140, tft.color565(12, 18, 34));
tft.drawFastHLine(10, 98, 140, tft.color565(12, 18, 34));
tft.drawFastVLine(10, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(20, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(30, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(40, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(50, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(60, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(70, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(80, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(90, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(100, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(110, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(120, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(130, 80, 30, tft.color565(110, 70, 150));
tft.drawFastVLine(140, 80, 30, tft.color565(110, 70, 150));
tft.drawLine(12, 103, 44, 103, tft.color565(16,16,22));
tft.drawLine(12, 102, 44, 102, TFT_ORANGE);
tft.drawLine(44, 103, 78, 102, tft.color565(16,16,22));
tft.drawLine(44, 102, 78, 101, TFT_ORANGE);
tft.drawLine(78, 102, 108, 100, tft.color565(16,16,22));
tft.drawLine(78, 101, 108, 99, TFT_ORANGE);
tft.drawLine(108, 100, 140, 96, tft.color565(16,16,22));
tft.drawLine(108, 99, 140, 95, TFT_ORANGE);
tft.drawRoundRect(8, 78, 144, 34, 4, tft.color565(184, 96, 184));

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
