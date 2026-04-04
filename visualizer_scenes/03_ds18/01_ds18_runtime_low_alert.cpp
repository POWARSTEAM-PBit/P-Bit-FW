// Scene: ds18_runtime_low_alert
// Source: src/ui_ds18.cpp
// Variant: runtime snapshot, split decimal value

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("TERMÓMETRO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_ORANGE);

tft.fillRect(0, 30, 118, 18, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa > C", 60, 34);

tft.fillRect(0, 49, 118, 46, TFT_BLACK);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
int ds18IntW = tft.textWidth("14");
tft.setFreeFont(&Roboto_Regular7pt8b);
int ds18DecW = tft.textWidth(".5");
int ds18StartX = 60 - (ds18IntW + ds18DecW) / 2;
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.drawString("14", ds18StartX, 50);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.drawString(".5", ds18StartX + ds18IntW, 50);
tft.fillRect(0, 98, 118, 28, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("Celsius", 60, 108);

tft.fillRect(120, 36, 28, 88, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_BLUE);
tft.fillRect(121, 53, 26, 70, TFT_BLACK);
tft.fillRect(121, 85, 26, 12, TFT_BLUE);
tft.fillRect(121, 97, 26, 26, TFT_BLUE);
tft.drawFastHLine(121, 97, 26, TFT_WHITE);
tft.drawFastHLine(147, 97, 2, TFT_WHITE);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("0", 151, 93);
tft.fillCircle(14, 118, 4, TFT_BLUE);
