// Scene: boot_language_selector
// Source: src/lang_select.cpp
// Variant: exact-layout

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, tft.width(), 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.drawString("Idioma", 80, 8);
tft.setTextFont(0);
tft.drawFastHLine(20, 32, tft.width() - 40, TFT_GREEN);

tft.fillRect(0, 38, tft.width(), 20, TFT_BLACK);
tft.setTextFont(2);
tft.setTextDatum(TL_DATUM);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString(">", 12, 42, 2);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("Español", 80, 40);
tft.setTextFont(0);

tft.fillRect(0, 62, tft.width(), 20, TFT_BLACK);
tft.setTextFont(2);
tft.setTextDatum(TL_DATUM);
tft.setTextColor(TFT_BLACK, TFT_BLACK);
tft.drawString(">", 12, 66, 2);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Catalán", 80, 64);
tft.setTextFont(0);

tft.fillRect(0, 86, tft.width(), 20, TFT_BLACK);
tft.setTextFont(2);
tft.setTextDatum(TL_DATUM);
tft.setTextColor(TFT_BLACK, TFT_BLACK);
tft.drawString(">", 12, 90, 2);
tft.setFreeFont(&IBMPlexSans_Regular9pt8b);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("English", 80, 88);
tft.setTextFont(0);

tft.fillRect(0, 108, tft.width(), 16, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa para elegir", 80, 116);
tft.setTextFont(0);
