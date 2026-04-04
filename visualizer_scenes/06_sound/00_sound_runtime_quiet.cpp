// Scene: sound_runtime_quiet
// Source: src/ui_sound.cpp
// Variant: visualizer-safe / runtime
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("SONIDO", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_MAGENTA);

const int cx = tft.width() / 2;
const char* value_str = "12";
const char* unit_str = "%";

tft.fillRect(0, 34, 160, 52, TFT_BLACK);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
int numW = tft.textWidth(value_str);
tft.setFreeFont(&Roboto_Regular7pt8b);
int unitW = tft.textWidth(unit_str);
int startX = cx - (numW + unitW) / 2;
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString(value_str, startX, 38);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString(unit_str, startX + numW, 38);

tft.drawRoundRect(20, 91, 120, 14, 3, TFT_DARKGREY);
tft.fillRoundRect(22, 93, 14, 10, 2, TFT_GREEN);
tft.fillRect(36, 93, 102, 10, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Tranquilo", 80, 109);
tft.fillCircle(14, 118, 4, TFT_BLACK);
tft.drawCircle(14, 118, 4, TFT_DARKGREY);
tft.drawCircle(14, 118, 3, TFT_DARKGREY);
