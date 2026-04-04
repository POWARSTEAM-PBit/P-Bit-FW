// Scene: temp_edit_high
// Source: src/ui_temp.cpp
// Variant: visualizer-safe / menu edit

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.drawString("TEMPERATURA", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_RED);

tft.fillRect(0, 38, 160, 22, TFT_BLACK);
tft.fillRect(0, 60, 160, 48, TFT_BLACK);
tft.fillRect(0, 108, 160, 16, TFT_BLACK);

tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Límite alto", 80, 44);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("28 C", 80, 78);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("Gira y pulsa", 80, 118);
