// Scene: estado_lab_overview
// Source: src/ui_lab_dash.cpp
// Variant: visualizer-safe / current compact layout

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("ESTADO LAB", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_WHITE);

tft.drawRoundRect(8, 33, 144, 84, 4, 0x39CC);

// TEMP
tft.fillRect(9, 34, 142, 18, TFT_BLACK);
tft.fillRect(12, 34, 3, 17, TFT_MAGENTA);
tft.drawFastHLine(9, 51, 142, 0x2104);
tft.drawRoundRect(18, 36, 5, 10, 2, TFT_MAGENTA);
tft.fillCircle(20, 48, 4, TFT_MAGENTA);
tft.drawCircle(20, 48, 4, TFT_MAGENTA);
tft.drawFastVLine(20, 37, 8, TFT_MAGENTA);
tft.setTextDatum(ML_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("TEMP", 35, 42);
tft.setTextDatum(MR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
tft.drawString("27.5C", 147, 41);

// HUM
tft.fillRect(9, 54, 142, 18, TFT_BLACK);
tft.fillRect(12, 54, 3, 17, TFT_CYAN);
tft.drawFastHLine(9, 71, 142, 0x2104);
tft.fillTriangle(20, 54, 16, 61, 24, 61, tft.color565(84, 255, 255));
tft.fillCircle(20, 64, 4, tft.color565(84, 255, 255));
tft.drawCircle(20, 64, 4, tft.color565(84, 255, 255));
tft.setTextDatum(ML_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("HUM", 35, 62);
tft.setTextDatum(MR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("66%", 147, 61);

// LUZ
tft.fillRect(9, 74, 142, 18, TFT_BLACK);
tft.fillRect(12, 74, 3, 17, tft.color565(255, 255, 72));
tft.drawFastHLine(9, 91, 142, 0x2104);
tft.fillCircle(20, 80, 3, tft.color565(255, 255, 72));
tft.drawCircle(20, 80, 3, tft.color565(255, 255, 72));
tft.drawFastHLine(14, 80, 3, tft.color565(255, 255, 72));
tft.drawFastHLine(24, 80, 3, tft.color565(255, 255, 72));
tft.drawFastVLine(20, 74, 3, tft.color565(255, 255, 72));
tft.drawFastVLine(20, 85, 3, tft.color565(255, 255, 72));
tft.drawLine(16, 77, 14, 75, tft.color565(255, 255, 72));
tft.drawLine(24, 77, 26, 75, tft.color565(255, 255, 72));
tft.drawLine(16, 83, 14, 85, tft.color565(255, 255, 72));
tft.drawLine(24, 83, 26, 85, tft.color565(255, 255, 72));
tft.setTextDatum(ML_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("LUZ", 35, 82);
tft.setTextDatum(MR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(tft.color565(255, 255, 72), TFT_BLACK);
tft.drawString("96 lux", 147, 81);

// MIC
tft.fillRect(9, 94, 142, 18, TFT_BLACK);
tft.fillRect(12, 94, 3, 17, tft.color565(180, 100, 255));
tft.fillRoundRect(18, 95, 5, 8, 2, tft.color565(180, 100, 255));
tft.drawFastVLine(20, 103, 3, tft.color565(180, 100, 255));
tft.drawFastHLine(17, 105, 7, tft.color565(180, 100, 255));
tft.setTextDatum(ML_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("MIC", 35, 102);
tft.setTextDatum(MR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(tft.color565(180, 100, 255), TFT_BLACK);
tft.drawString("0%", 147, 101);
