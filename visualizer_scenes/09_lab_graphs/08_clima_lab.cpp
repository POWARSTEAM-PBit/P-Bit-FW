// Scene: clima_lab
// Source: src/ui_lab_dual.cpp
// Variant: visualizer-safe / current compact dual cards

tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("CLIMA LAB", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_WHITE);

tft.drawFastVLine(80, 36, 76, 0x39CC);

// Left temp card
tft.drawRoundRect(6, 36, 72, 76, 4, tft.color565(255, 144, 0));
tft.fillRect(7, 37, 70, 74, TFT_BLACK);
tft.drawRoundRect(14, 41, 5, 10, 2, tft.color565(255, 216, 72));
tft.fillCircle(16, 53, 4, tft.color565(255, 216, 72));
tft.drawCircle(16, 53, 4, tft.color565(255, 216, 72));
tft.drawFastVLine(16, 42, 8, tft.color565(255, 216, 72));
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(tft.color565(255, 144, 0), TFT_BLACK);
tft.drawString("TEMP", 42, 40);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(tft.color565(255, 216, 72), TFT_BLACK);
tft.drawString("C", 71, 39);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);
tft.setTextColor(tft.color565(255, 184, 64), TFT_BLACK);
tft.drawString("27.7", 42, 62);
tft.drawRoundRect(12, 101, 60, 7, 3, TFT_DARKGREY);
tft.fillRoundRect(14, 103, 33, 3, 2, tft.color565(255, 144, 0));

// Right humidity card
tft.drawRoundRect(82, 36, 72, 76, 4, tft.color565(0, 230, 255));
tft.fillRect(83, 37, 70, 74, TFT_BLACK);
tft.fillTriangle(92, 40, 88, 47, 96, 47, tft.color565(176, 255, 255));
tft.fillCircle(92, 50, 4, tft.color565(176, 255, 255));
tft.drawCircle(92, 50, 4, tft.color565(176, 255, 255));
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(tft.color565(0, 230, 255), TFT_BLACK);
tft.drawString("HUM", 118, 40);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(tft.color565(176, 255, 255), TFT_BLACK);
tft.drawString("%", 147, 39);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);
tft.setTextColor(tft.color565(176, 255, 255), TFT_BLACK);
tft.drawString("74", 118, 62);
tft.drawRoundRect(88, 101, 60, 7, 3, TFT_DARKGREY);
tft.fillRoundRect(90, 103, 44, 3, 2, tft.color565(0, 230, 255));
