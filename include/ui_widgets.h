#pragma once
#include <TFT_eSPI.h>
// FreeSans9pt7b is already available through TFT_eSPI.h -> gfxfont.h (LOAD_GFXFF).

// Global TFT object shared by all UI modules.
extern TFT_eSPI tft;

typedef void (*SensorIconDrawFn)(int cx, int cy, uint16_t color);

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
void drawHeader(const char* title);
void drawMasterCardHeader(const char* title, uint16_t line_color = TFT_WHITE);
void drawFooterHint(const char* text, int cx, int y, uint16_t color = TFT_CYAN);

// Bitmask flags for clearMenuBands — combine with | to clear only what changed.
constexpr uint8_t kMenuBand_Title  = 0x01;   // y 26..59  — titles and subtitles
constexpr uint8_t kMenuBand_Body   = 0x02;   // y 60..107 — editable content / list items
constexpr uint8_t kMenuBand_Footer = 0x04;   // y 108..123 — footer hints
constexpr uint8_t kMenuBand_All    = 0x07;   // shorthand: all three bands

// Clear selected menu layout bands before drawing a new state.
// Default kMenuBand_All is backward-compatible with all existing callers.
void clearMenuBands(uint8_t bands = kMenuBand_All);
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
// Draw a 2x3 settings grid. Primary items occupy the first slots; Reset and Exit
// are always pinned to the bottom row for consistent muscle memory.
void drawSettingsGridMenu(const char* const* primary_items,
                          uint8_t primary_count,
                          uint8_t selected_index,
                          const char* reset_text,
                          const char* exit_text);
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
void drawFillTank(int x, int y, int w, int h, uint16_t fixedColor, float value, float minVal, float maxVal, int radius = 0);
void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal);
void drawCachedBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal, bool force_redraw = false);
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
