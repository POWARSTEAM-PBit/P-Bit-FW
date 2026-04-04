// Scene: humidity_runtime_no_sensor
// Source: src/ui_humidity.cpp
// Variant: runtime / no sensor
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("HUMEDAD", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_CYAN);
tft.fillRect(0, 30, 118, 18, TFT_BLACK);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Relativa del aire", 60, 34);
tft.fillRect(0, 49, 118, 46, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular24pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("---", 60, 50);
tft.setTextFont(0);
tft.fillRect(0, 98, 118, 28, TFT_BLACK);
tft.drawRoundRect(120, 36, 28, 88, 3, TFT_DARKGREY);
tft.fillRect(122, 38, 24, 84, TFT_BLACK);
tft.fillCircle(14, 118, 4, TFT_DARKGREY);
tft.drawCircle(14, 118, 4, TFT_DARKGREY);
