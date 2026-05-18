// ui_widgets.cpp
// Reusable drawing helpers shared by every UI screen.

#include "ui_widgets.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <stdio.h>      // Para snprintf()

// Global TFT object definition shared by all modules.
TFT_eSPI tft = TFT_eSPI();

// --- Global widget implementations ---

uint16_t getTempColor(float temp) {
    if (temp <= 15.0f) return TFT_BLUE;
    if (temp > 15.0f && temp <= 22.0f) return TFT_CYAN;
    if (temp > 22.0f && temp <= 27.0f) return TFT_GREEN;
    if (temp > 27.0f && temp <= 32.0f) return TFT_ORANGE;
    return TFT_RED;
}

void drawCard(int x, int y, int w, int h, uint16_t color) {
    tft.drawRoundRect(x, y, w, h, 4, color);
}

void drawHeader(const char* title) {
    const int cx = tft.width() / 2;
    tft.fillRect(0, 0, tft.width(), L_CONTENT_TOP, TFT_BLACK);
    tft.setTextDatum(C_BASELINE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_HEADER);
    tft.drawString(title, cx, L_HEADER_Y);
    tft.setTextFont(0);
    tft.drawFastHLine(LC_MASTER_HEADER_LINE_X,
                      L_HEADER_LINE,
                      LC_MASTER_HEADER_LINE_W,
                      TFT_WHITE);
}

void drawMasterCardHeader(const char* title, uint16_t line_color) {
    const int cx = tft.width() / 2;
    tft.fillRect(0, 0, tft.width(), LC_MASTER_CARD_Y0, TFT_BLACK);
    tft.setTextDatum(C_BASELINE);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_HEADER);
    tft.drawString(title, cx, LC_MASTER_HEADER_BASELINE);
    tft.setTextFont(0);
    tft.drawFastHLine(LC_MASTER_HEADER_LINE_X,
                      L_HEADER_LINE,
                      LC_MASTER_HEADER_LINE_W,
                      line_color);
}

void drawFooterHint(const char* text, int cx, int y, uint16_t color) {
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, TFT_BLACK);
    if (tft.textWidth(text) > tft.width() - 4) {
        tft.setTextFont(1);
    }
    tft.drawString(text, cx, y);
    tft.setTextFont(0);
}

void clearMenuBands() {
    // Clear the shared menu layout bands independently to avoid ghosting.
    tft.fillRect(0, LM_MENU_TITLE_BAND_Y, tft.width(), LM_MENU_TITLE_BAND_H, TFT_BLACK);
    tft.fillRect(0, LM_MENU_BODY_BAND_Y, tft.width(), LM_MENU_BODY_BAND_H, TFT_BLACK);
    tft.fillRect(0, LM_MENU_FOOTER_Y - 10, tft.width(), 16, TFT_BLACK);
}

void drawCenteredMenuFrame(const char* title,
                           uint16_t title_color,
                           const char* footer_text,
                           uint16_t footer_color) {
    clearMenuBands();

    const int cx = tft.width() / 2;
    tft.setTextDatum(MC_DATUM);

    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(title_color, TFT_BLACK);
    tft.drawString(title, cx, 44);
    tft.setTextFont(0);

    drawFooterHint(footer_text, cx, LM_MENU_FOOTER_Y, footer_color);
}

void drawCenteredMenuList(const char* const* items,
                          int item_count,
                          uint8_t selected_index,
                          int start_y,
                          int gap_y,
                          uint16_t selected_color,
                          uint16_t normal_color) {
    const int cx = tft.width() / 2;
    clearMenuBands();
    tft.setTextDatum(MC_DATUM);

    for (int i = 0; i < item_count; ++i) {
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor((i == (int)selected_index) ? selected_color : normal_color, TFT_BLACK);
        tft.drawString(items[i], cx, start_y + (gap_y * i));
    }
    tft.setTextFont(0);
}

static void draw_centered_menu_value(const char* title,
                                     const char* value,
                                     uint16_t value_color,
                                     MenuValueFont value_font,
                                     const char* footer_text,
                                     uint16_t footer_color,
                                     uint16_t title_color) {
    drawCenteredMenuFrame(title, title_color, footer_text, footer_color);

    const int cx = tft.width() / 2;
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(value_font == MENU_VALUE_FONT_TIMER ? FONT_TIMER : FONT_BODY);
    tft.setTextColor(value_color, TFT_BLACK);
    tft.drawString(value, cx, 78);
    tft.setTextFont(0);
}

void drawCenteredMenuValueScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 uint16_t footer_color) {
    draw_centered_menu_value(title, value, value_color, value_font, footer_text, footer_color, TFT_YELLOW);
}

void drawCenteredMenuSavedScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 uint16_t footer_color) {
    draw_centered_menu_value(title, value, value_color, value_font, footer_text, footer_color, TFT_GREEN);
}

void drawCenteredMenuBodyLines(const char* const* lines,
                               const uint16_t* colors,
                               uint8_t line_count,
                               MenuTextFont text_font,
                               int start_y,
                               int gap_y) {
    if (!lines || line_count == 0) return;

    const int cx = tft.width() / 2;
    const GFXfont* font = FONT_BODY;
    if (text_font == MENU_TEXT_FONT_SMALL) {
        font = FONT_SMALL;
    } else if (text_font == MENU_TEXT_FONT_TIMER) {
        font = FONT_TIMER;
    }

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(font);
    for (uint8_t i = 0; i < line_count; ++i) {
        const char* line = lines[i];
        if (!line || line[0] == '\0') continue;
        uint16_t line_color = colors ? colors[i] : TFT_WHITE;
        tft.setTextColor(line_color, TFT_BLACK);
        tft.drawString(line, cx, start_y + (gap_y * i));
    }
    tft.setTextFont(0);
}

void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal) {
    tft.drawRoundRect(x, y, w, h, 3, TFT_DARKGREY);
    value = constrain(value, minVal, maxVal); 
    int fill_w = map(value, minVal, maxVal, 0, w - 4);
    tft.fillRoundRect(x + 2, y + 2, fill_w, h - 4, 2, color);
    tft.fillRect(x + 2 + fill_w, y + 2, (w - 4) - fill_w, h - 4, TFT_BLACK);
}

void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius) {
    float normalized = constrain((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    int fill_height  = (int)(normalized * (h - 2));
    int empty_height = (h - 2) - fill_height;

    // Top empty region stays black.
    if (empty_height > 0)
        tft.fillRect(x + 1, y + 1, w - 2, empty_height, TFT_BLACK);
    // Bottom filled region uses the requested color.
    if (fill_height > 0) {
        int r = radius;
        if (r > fill_height / 2) r = fill_height / 2;
        if (r > (w - 2) / 2)    r = (w - 2) / 2;
        if (r > 0)
            tft.fillRoundRect(x + 1, y + 1 + empty_height, w - 2, fill_height, r, fixedColor);
        else
            tft.fillRect(x + 1, y + 1 + empty_height, w - 2, fill_height, fixedColor);
    }
}

/**
 * Draw and refresh the dynamic content inside the timer card.
 * The caller controls the card border color and the rendered state text.
 */
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time) {
    (void)cy;
    (void)newColor;
    (void)time;

    // Clear only the timer card region so the header and footer remain untouched.
    tft.fillRect(LT_CARD_X - 2, LT_CARD_Y - 2, LT_CARD_W + 4, LT_CARD_H + 4, TFT_BLACK);
    tft.fillRect(LT_CARD_X + 3, LT_CARD_Y + 3, LT_CARD_W - 6, LT_CARD_H - 6, TFT_BLACK);
    drawCard(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, borderColor);
    
    tft.fillRect(LT_CARD_X + 8, LT_STATE_Y - 10, LT_CARD_W - 16, 18, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString(stateText, cx, LT_STATE_Y);
    tft.setTextFont(0); // liberar GFXfont
}

void drawAlertJewel(int cx, int cy, AlertJewelState state, uint16_t color) {
    constexpr int R = 4;
    const int box_x = cx - R - 2;
    const int box_y = cy - R - 2;

    tft.fillRect(box_x, box_y, (R * 2) + 4, (R * 2) + 4, TFT_BLACK);

    switch (state) {
        case ALERT_JEWEL_OFF:
            tft.fillCircle(cx, cy, R, TFT_BLACK);
            tft.drawCircle(cx, cy, R, TFT_DARKGREY);
            tft.drawCircle(cx, cy, R - 1, TFT_DARKGREY);
            tft.drawLine(cx - 3, cy - 3, cx + 3, cy + 3, TFT_DARKGREY);
            break;
        case ALERT_JEWEL_OK:
        case ALERT_JEWEL_WARN:
        case ALERT_JEWEL_CRIT:
        default:
            tft.fillCircle(cx, cy, R, color);
            tft.drawCircle(cx, cy, R, TFT_BLACK);
            tft.drawCircle(cx, cy, R - 1, TFT_BLACK);
            break;
    }
}

void drawResetChoicePrompt(const char* title,
                           const char* line1,
                           const char* line2,
                           const char* no_text,
                           const char* yes_text,
                           uint8_t selected_choice,
                           const char* footer_text,
                           uint16_t footer_color) {
    clearMenuBands();

    const int cx = tft.width() / 2;
    const int no_x = cx - 28;
    const int yes_x = cx + 28;

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString(title, cx, 44);

    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    if (line1 && line1[0] != '\0') {
        tft.drawString(line1, cx, 68);
    }
    if (line2 && line2[0] != '\0') {
        tft.drawString(line2, cx, 84);
    }

    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(selected_choice == 0 ? TFT_YELLOW : TFT_DARKGREY, TFT_BLACK);
    tft.drawString(no_text, no_x, 102);
    tft.setTextColor(selected_choice == 1 ? TFT_RED : TFT_DARKGREY, TFT_BLACK);
    tft.drawString(yes_text, yes_x, 102);
    tft.setTextFont(0);

    if (footer_text && footer_text[0] != '\0') {
        drawFooterHint(footer_text, cx, LM_MENU_FOOTER_Y, footer_color);
    }
}

/**
 * Split the integer and decimal parts so the value can be centered cleanly.
 */
void drawSplitDecimalValue(float value, int cx, int topY, uint16_t color, uint16_t bg_color) {
    char valStr[8];
    snprintf(valStr, sizeof(valStr), "%.1f", value);

    int dot = 0;
    while (valStr[dot] && valStr[dot] != '.') dot++;
    char intStr[6];
    for (int i = 0; i < dot; i++) intStr[i] = valStr[i];
    intStr[dot] = '\0';
    const char* decStr = valStr + dot;

    tft.setFreeFont(FONT_VALUE);
    int intW = tft.textWidth(intStr);
    tft.setFreeFont(FONT_BODY);
    int decW = tft.textWidth(decStr);
    int startX = cx - (intW + decW) / 2;

    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(color, bg_color);
    tft.setFreeFont(FONT_VALUE);
    tft.drawString(intStr, startX, topY);
    tft.setFreeFont(FONT_BODY);
    tft.drawString(decStr, startX + intW, topY);
    tft.setTextFont(0); // liberar GFXfont
}
