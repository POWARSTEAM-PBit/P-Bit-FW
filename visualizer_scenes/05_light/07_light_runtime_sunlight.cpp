// Scene: light_runtime_sunlight
// Source: src/ui_light.cpp
// Variant: exact-layout
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("LUZ", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_YELLOW);

const int cx = tft.width() / 2;
const char* value_str = "12k";
const char* unit_str = "";

tft.fillRect(0, 34, 160, 52, TFT_BLACK);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
int numW = tft.textWidth(value_str);
tft.setFreeFont(&Roboto_Regular7pt8b);
int unitW = tft.textWidth(unit_str);
int startX = cx - (numW + unitW) / 2;
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString(value_str, startX, 38);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString(unit_str, startX + numW, 38);

tft.drawRoundRect(20, 91, 120, 14, 3, TFT_DARKGREY);
tft.fillRoundRect(22, 93, 116, 10, 2, TFT_ORANGE);
tft.fillRect(138, 93, 4, 10, TFT_BLACK);
tft.fillRect(0, 106, 160, 16, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_ORANGE, TFT_BLACK);
tft.drawString("Luz solar", 80, 114);
tft.fillCircle(14, 118, 4, TFT_ORANGE);
tft.drawCircle(14, 118, 4, TFT_ORANGE);
