// Scene: graph_humidity
// Source: src/ui_graph.cpp
// Variant: visualizer-safe / current compact top band

tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("GRÁFICA", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_WHITE);

tft.fillRect(0, 32, 160, 16, TFT_BLACK);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
tft.drawString("HUM", 12, 31);
tft.setTextDatum(TR_DATUM);
tft.setTextColor(tft.color565(80, 255, 255), TFT_BLACK);
tft.drawString("71%", 146, 31);

tft.drawRect(8, 49, 144, 62, tft.color565(0, 84, 130));
TFT_eSprite g2 = TFT_eSprite(&tft);
g2.setColorDepth(16);
g2.createSprite(142, 60);
g2.fillSprite(tft.color565(4, 8, 18));
g2.drawFastHLine(0, 0, 142, tft.color565(14, 38, 92));
g2.drawFastHLine(0, 12, 142, tft.color565(18, 62, 118));
g2.drawFastHLine(0, 24, 142, tft.color565(28, 88, 150));
g2.drawFastHLine(0, 36, 142, tft.color565(108, 176, 220));
g2.drawFastHLine(0, 48, 142, TFT_WHITE);
g2.drawFastVLine(0, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(12, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(24, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(36, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(48, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(60, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(72, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(84, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(96, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(108, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(120, 0, 60, tft.color565(92, 82, 156));
g2.drawFastVLine(132, 0, 60, tft.color565(92, 82, 156));
g2.setTextFont(1);
g2.setTextColor(TFT_WHITE, TFT_BLACK);
g2.setTextDatum(TL_DATUM);
g2.drawString("71%", 2, 2, 1);
g2.setTextDatum(BL_DATUM);
g2.setTextColor(TFT_WHITE, TFT_BLACK);
g2.drawString("40%", 2, 59, 1);
g2.drawLine(8, 50, 60, 50, tft.color565(10,12,18));
g2.drawLine(8, 49, 60, 49, tft.color565(80, 255, 255));
g2.drawLine(60, 50, 88, 51, tft.color565(10,12,18));
g2.drawLine(60, 49, 88, 50, tft.color565(80, 255, 255));
g2.drawLine(88, 51, 104, 52, tft.color565(10,12,18));
g2.drawLine(88, 50, 104, 51, tft.color565(80, 255, 255));
g2.drawLine(104, 52, 114, 44, tft.color565(10,12,18));
g2.drawLine(104, 51, 114, 43, tft.color565(80, 255, 255));
g2.drawLine(114, 44, 124, 24, tft.color565(10,12,18));
g2.drawLine(114, 43, 124, 23, tft.color565(80, 255, 255));
g2.drawLine(124, 24, 134, 9, tft.color565(10,12,18));
g2.drawLine(124, 23, 134, 8, tft.color565(80, 255, 255));
g2.drawLine(134, 9, 141, 18, tft.color565(10,12,18));
g2.drawLine(134, 8, 141, 17, tft.color565(80, 255, 255));
g2.pushSprite(9, 50);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
