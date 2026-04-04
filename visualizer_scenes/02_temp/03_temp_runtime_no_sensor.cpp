// Scene: temp_runtime_no_sensor
// Source: src/ui_temp.cpp
// Variant: runtime snapshot, no sensor

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
tft.setTextColor(TFT_RED, TFT_BLACK);
tft.drawString("Sin sensor", 60, 34);

tft.fillRect(0, 49, 119, 46, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("---", 60, 50);
tft.fillRect(0, 98, 119, 28, TFT_BLACK);

tft.fillRect(120, 36, 28, 88, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_DARKGREY);
tft.fillRect(121, 53, 26, 70, TFT_BLACK);
tft.fillCircle(14, 118, 4, TFT_DARKGREY);
