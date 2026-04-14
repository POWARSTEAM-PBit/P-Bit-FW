// Scene: graph_temp
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
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("TEMP", 12, 31);
tft.setTextDatum(TR_DATUM);
tft.setTextColor(tft.color565(88, 255, 96), TFT_BLACK);
tft.drawString("27.8C", 146, 31);

tft.drawRect(8, 49, 144, 62, tft.color565(132, 74, 0));
TFT_eSprite g = TFT_eSprite(&tft);
g.setColorDepth(16);
g.createSprite(142, 60);
g.fillSprite(tft.color565(4, 8, 18));
g.drawFastHLine(0, 0, 142, tft.color565(120, 28, 34));
g.drawFastHLine(0, 12, 142, tft.color565(136, 62, 18));
g.drawFastHLine(0, 24, 142, tft.color565(132, 102, 26));
g.drawFastHLine(0, 36, 142, tft.color565(20, 86, 110));
g.drawFastHLine(0, 48, 142, tft.color565(18, 44, 98));
g.drawFastVLine(0, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(12, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(24, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(36, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(48, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(60, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(72, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(84, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(96, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(108, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(120, 0, 60, tft.color565(110, 52, 148));
g.drawFastVLine(132, 0, 60, tft.color565(110, 52, 148));
g.setTextFont(1);
g.setTextColor(tft.color565(255, 214, 56), TFT_BLACK);
g.setTextDatum(TL_DATUM);
g.drawString("29C", 2, 2, 1);
g.setTextDatum(BL_DATUM);
g.setTextColor(tft.color565(54, 202, 255), TFT_BLACK);
g.drawString("25C", 2, 59, 1);
g.drawLine(0, 43, 40, 43, tft.color565(10,12,18));
g.drawLine(0, 42, 40, 42, tft.color565(88, 255, 96));
g.drawLine(40, 43, 84, 42, tft.color565(10,12,18));
g.drawLine(40, 42, 84, 41, tft.color565(88, 255, 96));
g.drawLine(84, 42, 106, 38, tft.color565(10,12,18));
g.drawLine(84, 41, 106, 37, tft.color565(88, 255, 96));
g.drawLine(106, 38, 141, 30, tft.color565(10,12,18));
g.drawLine(106, 37, 141, 29, tft.color565(88, 255, 96));
g.pushSprite(9, 50);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa: cambiar sensor", 80, 120);
