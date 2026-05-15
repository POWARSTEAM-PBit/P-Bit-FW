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

tft.drawFastVLine(80, 36, 68, 0x39CC);

// Left temp card
tft.drawRoundRect(7, 36, 72, 68, 4, tft.color565(255, 144, 0));
tft.fillRect(8, 37, 70, 66, TFT_BLACK);
tft.drawRoundRect(16, 41, 5, 10, 2, tft.color565(255, 216, 72));
tft.fillCircle(18, 53, 4, tft.color565(255, 216, 72));
tft.drawCircle(18, 53, 4, tft.color565(255, 216, 72));
tft.drawFastVLine(18, 42, 8, tft.color565(255, 216, 72));
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(tft.color565(255, 144, 0), TFT_BLACK);
tft.drawString("TEMP", 43, 40);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(tft.color565(255, 216, 72), TFT_BLACK);
tft.drawString("C", 75, 39);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);
tft.setTextColor(tft.color565(255, 184, 64), TFT_BLACK);
tft.drawString("27.7", 43, 58);
tft.drawRoundRect(13, 90, 60, 7, 3, TFT_DARKGREY);
tft.fillRoundRect(15, 92, 33, 3, 2, tft.color565(255, 144, 0));

// Right humidity card
tft.drawRoundRect(81, 36, 72, 68, 4, tft.color565(0, 230, 255));
tft.fillRect(82, 37, 70, 66, TFT_BLACK);
tft.fillTriangle(91, 40, 87, 47, 95, 47, tft.color565(176, 255, 255));
tft.fillCircle(91, 50, 4, tft.color565(176, 255, 255));
tft.drawCircle(91, 50, 4, tft.color565(176, 255, 255));
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(tft.color565(0, 230, 255), TFT_BLACK);
tft.drawString("HUM", 117, 40);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(tft.color565(176, 255, 255), TFT_BLACK);
tft.drawString("%", 145, 39);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);
tft.setTextColor(tft.color565(176, 255, 255), TFT_BLACK);
tft.drawString("74", 117, 58);
tft.drawRoundRect(86, 90, 60, 7, 3, TFT_DARKGREY);
tft.fillRoundRect(88, 92, 44, 3, 2, tft.color565(0, 230, 255));
