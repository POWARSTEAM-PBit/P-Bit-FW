// Scene: soil_runtime_optimal
// Source: src/ui_soil.cpp
// Variant: runtime / optimal
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SUELO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_GREEN);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("Humedad", 60, 34);
tft.fillRect(0, 49, 118, 50, TFT_BLACK);
const char* soilStr = "46";
const char* unitStr = "%";
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
const int intW = tft.textWidth(soilStr);
tft.setFreeFont(&Roboto_Regular7pt8b);
const int unitW = tft.textWidth(unitStr);
const int startX = 60 - (intW + unitW) / 2;
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString(soilStr, startX, 50);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString(unitStr, startX + intW, 50);
tft.setTextFont(0);
tft.fillRect(0, 98, 118, 28, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Optimo", 60, 108);
tft.setTextFont(0);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_DARKGREY);
tft.fillRect(121, 78, 26, 45, TFT_GREEN);
tft.fillCircle(14, 118, 4, TFT_GREEN);
