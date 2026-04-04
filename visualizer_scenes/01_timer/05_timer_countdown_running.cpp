// Scene: timer_countdown_running
// Source: src/ui_timer.cpp
// Variant: exact-layout / visualizer-safe

TFT_eSprite timerSprite = TFT_eSprite(&tft);

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, tft.width(), 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.drawString("TEMPORIZADOR", 80, 2);
tft.setTextFont(0);
tft.drawFastHLine(10, 28, 140, TFT_BLUE);

tft.fillRect(14, 51, 132, 60, TFT_BLACK);
tft.fillRect(19, 56, 122, 50, TFT_BLACK);
tft.drawRoundRect(16, 53, 128, 56, 4, TFT_GREEN);

tft.fillRect(24, 54, 112, 18, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.drawString("CORRIENDO", 80, 64);
tft.setTextFont(0);

timerSprite.setColorDepth(16);
timerSprite.createSprite(122, 28);
timerSprite.fillSprite(TFT_BLACK);
timerSprite.setTextDatum(MC_DATUM);
timerSprite.setTextColor(TFT_GREEN, TFT_BLACK);
timerSprite.setFreeFont(&IBMPlexMono_Regular12pt8b);
timerSprite.drawString("00:04:33", 61, 15);
timerSprite.setTextFont(0);

tft.fillRect(19, 73, 122, 28, TFT_BLACK);
timerSprite.pushSprite(19, 73, TFT_BLACK);
tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("00:05:00", 80, 120);
tft.setTextFont(0);

tft.setTextDatum(MC_DATUM);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
tft.drawString("Pulsa pausar", 80, 34);
tft.setTextFont(0);
