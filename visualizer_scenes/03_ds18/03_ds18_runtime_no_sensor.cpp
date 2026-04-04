// Scene: ds18_runtime_no_sensor
// Source: src/ui_ds18.cpp
// Variant: runtime snapshot, no sensor

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
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.drawString("Sin sensor", 60, 34);

tft.fillRect(0, 49, 118, 46, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("---", 60, 50);
tft.fillRect(0, 98, 118, 28, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Check J4", 60, 108);

tft.fillRect(120, 36, 28, 88, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_DARKGREY);
tft.fillRect(121, 53, 26, 70, TFT_BLACK);
tft.drawFastHLine(121, 97, 26, TFT_DARKGREY);
tft.drawFastHLine(147, 97, 2, TFT_DARKGREY);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("0", 151, 93);
tft.fillCircle(14, 118, 4, TFT_DARKGREY);
