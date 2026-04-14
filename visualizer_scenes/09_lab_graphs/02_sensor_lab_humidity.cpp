// Scene: sensor_lab_humidity
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
tft.drawRoundRect(8, 33, 144, 40, 4, tft.color565(120, 92, 255));
tft.fillTriangle(20, 45, 16, 52, 24, 52, TFT_CYAN);
tft.fillCircle(20, 55, 4, TFT_CYAN);
tft.drawCircle(20, 55, 4, TFT_CYAN);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_CYAN, tft.color565(8, 12, 18));
tft.drawString("HUM", 33, 44);
tft.setTextDatum(TR_DATUM);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextColor(tft.color565(170, 96, 255), tft.color565(8, 12, 18));
tft.drawString("74", 136, 42);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, tft.color565(8, 12, 18));
tft.drawString("%", 139, 45);

tft.fillRoundRect(8, 78, 144, 34, 4, tft.color565(4, 8, 20));
tft.drawRoundRect(8, 78, 144, 34, 4, tft.color565(120, 92, 255));
tft.fillRect(10, 80, 140, 30, tft.color565(4, 8, 20));
tft.drawFastHLine(10, 88, 140, tft.color565(12, 18, 34));
tft.drawFastHLine(10, 98, 140, tft.color565(12, 18, 34));
tft.drawFastVLine(10, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(20, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(30, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(40, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(50, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(60, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(70, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(80, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(90, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(100, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(110, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(120, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(130, 80, 30, tft.color565(110, 96, 190));
tft.drawFastVLine(140, 80, 30, tft.color565(110, 96, 190));
tft.drawLine(12, 106, 88, 106, tft.color565(16,16,22));
tft.drawLine(12, 105, 88, 105, TFT_CYAN);
tft.drawLine(88, 106, 112, 92, tft.color565(16,16,22));
tft.drawLine(88, 105, 112, 91, TFT_CYAN);
tft.drawLine(112, 92, 132, 84, tft.color565(16,16,22));
tft.drawLine(112, 91, 132, 83, TFT_CYAN);
tft.drawLine(132, 84, 146, 90, tft.color565(16,16,22));
tft.drawLine(132, 83, 146, 89, TFT_CYAN);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
