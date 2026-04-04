// Scene: temp_runtime_ok
// Source: src/ui_temp.cpp
// Variant: runtime snapshot, split decimal value

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.drawString("TEMPERATURA", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_RED);

tft.fillRect(0, 30, 119, 18, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa > C", 60, 34);

tft.fillRect(0, 49, 119, 46, TFT_BLACK);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
int tempIntW = tft.textWidth("23");
tft.setFreeFont(&Roboto_Regular7pt8b);
int tempDecW = tft.textWidth(".4");
int tempStartX = 60 - (tempIntW + tempDecW) / 2;
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.drawString("23", tempStartX, 50);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.drawString(".4", tempStartX + tempIntW, 50);
tft.fillRect(0, 98, 119, 28, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("Celsius", 60, 108);

tft.fillRect(120, 36, 28, 88, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_GREEN);
tft.fillRect(121, 53, 26, 70, TFT_BLACK);
tft.fillRect(121, 70, 26, 53, TFT_GREEN);
tft.drawFastHLine(121, 70, 26, TFT_WHITE);
tft.fillCircle(14, 118, 4, TFT_GREEN);
