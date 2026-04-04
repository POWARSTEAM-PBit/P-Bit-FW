// Scene: overlay_sleep_warning
// Source: src/tft_display.cpp
// Variant: exact-layout

tft.fillScreen(TFT_BLACK);
tft.setTextDatum(MC_DATUM);

tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("ZZZ", tft.width() / 2, 50);

tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("Reposo", tft.width() / 2, 88);

tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa para volver", tft.width() / 2, 108);

tft.setTextFont(0);
