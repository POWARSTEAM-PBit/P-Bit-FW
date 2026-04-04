#pragma once
#include <TFT_eSPI.h>
// FreeSans9pt7b is already available through TFT_eSPI.h -> gfxfont.h (LOAD_GFXFF).

// Global TFT object shared by all UI modules.
extern TFT_eSPI tft;

// --- Widget prototypes ---
typedef enum {
    ALERT_JEWEL_OFF = 0,
    ALERT_JEWEL_OK,
    ALERT_JEWEL_WARN,
    ALERT_JEWEL_CRIT
} AlertJewelState;

typedef enum {
    MENU_VALUE_FONT_BODY = 0,
    MENU_VALUE_FONT_TIMER
} MenuValueFont;

typedef enum {
    MENU_TEXT_FONT_SMALL = 0,
    MENU_TEXT_FONT_BODY,
    MENU_TEXT_FONT_TIMER
} MenuTextFont;

uint16_t getTempColor(float temp);
void drawCard(int x, int y, int w, int h, uint16_t color);
void drawHeader(const char* title, uint16_t color);
void drawFooterHint(const char* text, int cx, int y, uint16_t color = TFT_CYAN);
// Clear the shared menu title/body/footer bands before drawing a new state.
void clearMenuBands();
// Draw the common centered menu frame: title at the top band and footer hint at the bottom.
void drawCenteredMenuFrame(const char* title,
                           uint16_t title_color,
                           const char* footer_text,
                           uint16_t footer_color = TFT_CYAN);
// Draw a centered vertical menu list using the shared typography and spacing.
void drawCenteredMenuList(const char* const* items,
                           int item_count,
                          uint8_t selected_index,
                          int start_y,
                          int gap_y,
                          uint16_t selected_color = TFT_YELLOW,
                          uint16_t normal_color = TFT_WHITE);
// Draw the common "title + value + footer hint" menu layout.
void drawCenteredMenuValueScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 uint16_t footer_color = TFT_CYAN);
// Draw the common saved/confirmation screen variant using the same centered layout.
void drawCenteredMenuSavedScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 uint16_t footer_color = TFT_CYAN);
// Draw centered summary lines inside the shared body band using a consistent font and spacing.
void drawCenteredMenuBodyLines(const char* const* lines,
                               const uint16_t* colors,
                               uint8_t line_count,
                               MenuTextFont text_font,
                               int start_y,
                               int gap_y);
void drawStringFit(const char* str, int x, int y, uint8_t bigFont, uint8_t smallFont, int maxW);
void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
void drawSplitDecimalValue(float value, int cx, int topY, uint16_t color, uint16_t bg_color);
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time);
void drawAlertJewel(int cx, int cy, AlertJewelState state, uint16_t color);
void drawResetChoicePrompt(const char* title,
                           const char* line1,
                           const char* line2,
                           const char* no_text,
                           const char* yes_text,
                           uint8_t selected_choice,
                           const char* footer_text = nullptr,
                           uint16_t footer_color = TFT_CYAN);
