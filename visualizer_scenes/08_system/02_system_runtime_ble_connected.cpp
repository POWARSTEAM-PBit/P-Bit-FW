// Scene: system_runtime_ble_connected
// Source: src/ui_system.cpp
// Variant: visualizer-safe / runtime
tft.fillScreen(TFT_BLACK);
tft.fillRect(0, 0, 160, 32, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.drawString("INFO SISTEMA", 80, 2);
tft.drawFastHLine(10, 28, 140, TFT_GREEN);
tft.drawRoundRect(10, 36, 140, 68, 4, TFT_DARKGREY);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
tft.setTextDatum(TL_DATUM);
tft.drawString("DEV", 20, 40);
tft.drawString("UP", 20, 54);
tft.drawString("BLE", 20, 68);
tft.drawString("IDI", 20, 82);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.drawString("P-BIT", 52, 40);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.drawString("00:00:00", 52, 54);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("CONECTADO", 52, 68);
tft.setTextColor(TFT_WHITE, TFT_BLACK);
tft.drawString("ESP", 52, 82);
tft.fillRect(0, 106, 160, 20, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_GREEN, TFT_BLACK);
tft.drawString("Sonido: ON (Pulsa)", 80, 116);
