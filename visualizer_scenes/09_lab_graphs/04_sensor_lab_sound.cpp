// Scene: sensor_lab_sound
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
tft.drawRoundRect(8, 33, 144, 40, 4, tft.color565(120, 255, 96));
tft.fillRoundRect(18, 45, 5, 8, 2, TFT_MAGENTA);
tft.drawFastVLine(20, 53, 3, TFT_MAGENTA);
tft.drawFastHLine(17, 55, 7, TFT_MAGENTA);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_MAGENTA, tft.color565(8, 12, 18));
tft.drawString("MIC", 33, 44);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(TFT_GREEN, tft.color565(8, 12, 18));
tft.drawString("0", 136, 42);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_MAGENTA, tft.color565(8, 12, 18));
tft.drawString("%", 139, 45);

tft.fillRoundRect(8, 78, 144, 34, 4, tft.color565(4, 8, 20));
tft.drawRoundRect(8, 78, 144, 34, 4, tft.color565(120, 255, 96));
tft.fillRect(10, 80, 140, 30, tft.color565(4, 8, 20));
tft.drawFastHLine(10, 88, 140, tft.color565(18, 22, 28));
tft.drawFastHLine(10, 98, 140, tft.color565(18, 22, 28));
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
tft.drawFastHLine(10, 107, 140, tft.color565(120,255,96));

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
