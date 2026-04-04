// Scene: overlay_restarting
// Source: src/tft_display.cpp
// Variant: exact-layout

tft.fillScreen(TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("Reiniciando...", tft.width() / 2, tft.height() / 2);
tft.setTextFont(0);
