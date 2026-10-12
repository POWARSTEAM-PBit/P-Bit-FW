#pragma once
#include <cstdint>
#include <cstring>
struct GFXfont {};
constexpr uint16_t TFT_BLACK=0, TFT_WHITE=0xffff, TFT_DARKGREY=0x7bef, TFT_CYAN=0x07ff, TFT_YELLOW=0xffe0;
constexpr int MC_DATUM=4;
extern unsigned native_full_clears;
class TFT_eSPI {
public:
    int width() const { return 160; }
    void fillScreen(uint16_t) { ++native_full_clears; }
    void fillRect(int,int,int,int,uint16_t) {}
    void fillRoundRect(int,int,int,int,int,uint16_t) {}
    void drawRoundRect(int,int,int,int,int,uint16_t) {}
    void setTextDatum(int) {}
    void setFreeFont(const GFXfont*) {}
    int textWidth(const char* text) const { return static_cast<int>(std::strlen(text) * 6); }
    void setTextFont(int) {}
    void setTextColor(uint16_t,uint16_t) {}
    void drawString(const char*,int,int) {}
};
