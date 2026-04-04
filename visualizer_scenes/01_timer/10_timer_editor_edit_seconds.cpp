// Scene: timer_editor_edit_seconds
// Source: src/ui_timer.cpp
// Variant: exact-layout / representative-values

tft.fillScreen(TFT_BLACK);

tft.fillRect(0, 0, tft.width(), 32, TFT_BLACK);
tft.setTextDatum(TC_DATUM);
tft.setTextColor(TFT_BLUE, TFT_BLACK);
tft.setFreeFont(&Roboto_Medium10pt8b);
tft.drawString("TEMPORIZADOR", 80, 2);
tft.setTextFont(0);
tft.drawFastHLine(10, 28, 140, TFT_BLUE);

tft.fillRect(0, 38, tft.width(), 22, TFT_BLACK);
tft.fillRect(0, 60, tft.width(), 48, TFT_BLACK);
tft.fillRect(0, 108, tft.width(), 16, TFT_BLACK);

tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_YELLOW, TFT_BLACK);
tft.setFreeFont(&Roboto_Regular7pt8b);
tft.drawString("DURACIÓN", 80, 44);
tft.setTextFont(0);

tft.setTextDatum(MC_DATUM);
tft.setTextColor(TFT_CYAN, TFT_BLACK);
tft.setFreeFont(&Roboto_Light6pt8b);
tft.drawString("Gira ajusta", 80, 118);
tft.setTextFont(0);

tft.fillRect(14, 54, 132, 46, TFT_BLACK);
tft.fillRect(19, 59, 122, 36, TFT_BLACK);
tft.drawRoundRect(16, 56, 128, 42, 4, TFT_GREEN);

tft.fillRect(22, 60, 116, 34, TFT_BLACK);
tft.setTextDatum(TL_DATUM);
tft.setFreeFont(&IBMPlexMono_Regular12pt8b);

const char* hh = "00";
const char* mm = "05";
const char* ss = "30";
const uint16_t selected_color = TFT_GREEN;
const uint16_t normal_color = TFT_WHITE;
const uint16_t colon_color = TFT_DARKGREY;
const int w_hh = tft.textWidth(hh);
const int w_mm = tft.textWidth(mm);
const int w_ss = tft.textWidth(ss);
const int w_colon = tft.textWidth(":");
const int total_w = w_hh + w_colon + w_mm + w_colon + w_ss;
int x = (tft.width() / 2) - (total_w / 2);
const int y = 68;

tft.setTextColor(normal_color, TFT_BLACK);
tft.drawString(hh, x, y);
x += w_hh;
tft.setTextColor(colon_color, TFT_BLACK);
tft.drawString(":", x, y);
x += w_colon;
tft.setTextColor(normal_color, TFT_BLACK);
tft.drawString(mm, x, y);
x += w_mm;
tft.setTextColor(colon_color, TFT_BLACK);
tft.drawString(":", x, y);
x += w_colon;
tft.setTextColor(selected_color, TFT_BLACK);
tft.drawString(ss, x, y);
tft.setTextFont(0);
