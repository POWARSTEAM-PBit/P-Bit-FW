// ui_widgets.cpp
// Reusable drawing helpers shared by every UI screen.

#include "ui_widgets.h"
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include "palette.h"
#include <math.h>
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

void clearMenuBands(uint8_t bands) {
    // Clear only the requested bands to avoid unnecessary ghosting and footer flicker.
    if (bands & kMenuBand_Title)
        tft.fillRect(0, LM_MENU_TITLE_BAND_Y, tft.width(), LM_MENU_TITLE_BAND_H, TFT_BLACK);
    if (bands & kMenuBand_Body)
        tft.fillRect(0, LM_MENU_BODY_BAND_Y, tft.width(), LM_MENU_BODY_BAND_H, TFT_BLACK);
    if (bands & kMenuBand_Footer)
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
    tft.drawString(title, cx, 41);
    tft.setTextFont(0);

    tft.drawFastHLine(34, LM_MENU_FOOTER_Y - 12, 92, tft.color565(18, 36, 58));
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
    // Items span title+body bands (y=36..~100 depending on count).
    // Skip kMenuBand_Footer — the hint "Turn to navigate / press to select"
    // is drawn once by drawCenteredMenuFrame and never changes during navigation.
    // This eliminates footer flicker on every encoder tick.
    clearMenuBands(kMenuBand_Title | kMenuBand_Body);
    tft.setTextDatum(MC_DATUM);

    for (int i = 0; i < item_count; ++i) {
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor((i == (int)selected_index) ? selected_color : normal_color, TFT_BLACK);
        tft.drawString(items[i], cx, start_y + (gap_y * i));
    }
    tft.setTextFont(0);
}

static void draw_settings_grid_tile(int x, int y, int w, int h,
                                    const char* label,
                                    bool selected,
                                    bool reset_action,
                                    bool exit_action) {
    const uint16_t panel_bg = selected
        ? (exit_action ? tft.color565(34, 4, 14) : tft.color565(18, 12, 34))
        : tft.color565(4, 8, 18);
    const uint16_t idle_border = reset_action ? PB_LUZ_P2
                               : exit_action ? PB_SOUND_P3
                               : tft.color565(28, 52, 70);
    const uint16_t border = selected ? (exit_action ? PB_SOUND_P3
                                      : reset_action ? PB_LUZ_P1
                                      : PB_LUZ_P1)
                                    : idle_border;
    const uint16_t text = selected ? TFT_WHITE
                        : reset_action ? PB_LUZ_P1
                        : exit_action ? PB_SOUND_P3
                        : PB_HUM_P3;

    // Clear the full tile box first: rounded fills do not cover the corner
    // pixels, so a fast selection change can otherwise leave selected-border
    // residue in hardware.
    tft.fillRect(x, y, w, h, TFT_BLACK);
    tft.fillRoundRect(x, y, w, h, 4, panel_bg);
    tft.drawRoundRect(x, y, w, h, 4, border);
    if (selected) {
        tft.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 3,
                          exit_action ? PB_LUZ_P1 : PB_TEMP_P2);
    }

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(text, panel_bg);
    tft.setFreeFont(FONT_SMALL);
    if (tft.textWidth(label) > w - 8) {
        tft.setTextFont(1);
    }
    tft.drawString(label, x + w / 2, y + h / 2 - 3);
    tft.setTextFont(0);
}

void drawSettingsGridMenu(const char* const* primary_items,
                          uint8_t primary_count,
                          uint8_t selected_index,
                          const char* reset_text,
                          const char* exit_text,
                          bool force_redraw,
                          int previous_selected_index) {
    constexpr int kTileW = 70;
    constexpr int kTileH = 22;
    constexpr int kX0 = 8;
    constexpr int kX1 = 82;
    constexpr int kY0 = 32;
    constexpr int kRowGap = 26;

    if (primary_count > 4) primary_count = 4;
    const uint8_t reset_index = primary_count;
    const uint8_t exit_index = primary_count + 1;
    const uint8_t item_count = primary_count + 2;

    auto draw_tile_at_index = [&](uint8_t index) {
        if (index >= item_count) return;
        if (index < primary_count) {
            const int x = (index & 1) ? kX1 : kX0;
            const int y = kY0 + (index / 2) * kRowGap;
            draw_settings_grid_tile(x, y, kTileW, kTileH,
                                    primary_items[index],
                                    selected_index == index,
                                    false,
                                    false);
            return;
        }
        if (index == reset_index) {
            draw_settings_grid_tile(kX0, kY0 + 2 * kRowGap, kTileW, kTileH,
                                    reset_text,
                                    selected_index == reset_index,
                                    true,
                                    false);
            return;
        }
        draw_settings_grid_tile(kX1, kY0 + 2 * kRowGap, kTileW, kTileH,
                                exit_text,
                                selected_index == exit_index,
                                false,
                                true);
    };

    if (force_redraw || previous_selected_index < 0 || previous_selected_index >= item_count) {
        clearMenuBands(kMenuBand_Title | kMenuBand_Body);
        for (uint8_t i = 0; i < item_count; ++i) {
            draw_tile_at_index(i);
        }
        return;
    }

    const int prev = previous_selected_index;
    const int curr = (int)selected_index;
    const int delta = (prev > curr) ? (prev - curr) : (curr - prev);
    if (delta > 1) {
        for (uint8_t i = 0; i < item_count; ++i) {
            draw_tile_at_index(i);
        }
        return;
    }

    if ((uint8_t)previous_selected_index != selected_index) {
        draw_tile_at_index((uint8_t)previous_selected_index);
    }
    draw_tile_at_index(selected_index);
}

constexpr int kMenuValueCardX = 20;
constexpr int kMenuValueCardY = 58;
constexpr int kMenuValueCardW = 120;
constexpr int kMenuValueCardH = 38;
constexpr int kMenuValueTextY = 75;

static uint16_t menu_value_card_bg() { return tft.color565(4, 8, 18); }
static uint16_t menu_value_card_shadow() { return tft.color565(18, 12, 34); }

static void draw_centered_menu_value_shell(const char* title,
                                           uint16_t value_color,
                                           const char* footer_text,
                                           uint16_t footer_color,
                                           uint16_t title_color) {
    const uint16_t card_bg = menu_value_card_bg();
    const uint16_t card_shadow = menu_value_card_shadow();

    drawCenteredMenuFrame(title, title_color, footer_text, footer_color);
    tft.fillRoundRect(kMenuValueCardX, kMenuValueCardY, kMenuValueCardW, kMenuValueCardH, 5, card_bg);
    tft.drawRoundRect(kMenuValueCardX, kMenuValueCardY, kMenuValueCardW, kMenuValueCardH, 5, value_color);
    tft.drawRoundRect(kMenuValueCardX + 1, kMenuValueCardY + 1, kMenuValueCardW - 2, kMenuValueCardH - 2, 4, card_shadow);
}

static void update_centered_menu_value(const char* value,
                                       uint16_t value_color,
                                       MenuValueFont value_font) {
    const int cx = tft.width() / 2;
    const uint16_t card_bg = menu_value_card_bg();
    const uint16_t card_shadow = menu_value_card_shadow();

    tft.drawRoundRect(kMenuValueCardX, kMenuValueCardY, kMenuValueCardW, kMenuValueCardH, 5, value_color);
    tft.drawRoundRect(kMenuValueCardX + 1, kMenuValueCardY + 1, kMenuValueCardW - 2, kMenuValueCardH - 2, 4, card_shadow);
    tft.fillRect(kMenuValueCardX + 3, kMenuValueCardY + 3,
                 kMenuValueCardW - 6, kMenuValueCardH - 6, card_bg);

    tft.setTextDatum(MC_DATUM);
    const bool prefer_timer = (value_font == MENU_VALUE_FONT_TIMER);
    tft.setFreeFont(prefer_timer ? FONT_TIMER : FONT_BODY);
    if (tft.textWidth(value) > kMenuValueCardW - 12) {
        tft.setFreeFont(FONT_SMALL);
    }
    if (tft.textWidth(value) > kMenuValueCardW - 12) {
        tft.setTextFont(1);
    }
    tft.setTextColor(value_color, card_bg);
    tft.drawString(value, cx, kMenuValueTextY);
    tft.setTextFont(0);
}

static void draw_centered_menu_value(const char* title,
                                     const char* value,
                                     uint16_t value_color,
                                     MenuValueFont value_font,
                                     const char* footer_text,
                                     bool force_redraw,
                                     uint16_t footer_color,
                                     uint16_t title_color) {
    if (force_redraw) {
        draw_centered_menu_value_shell(title, value_color, footer_text, footer_color, title_color);
    }
    update_centered_menu_value(value, value_color, value_font);
}

void drawCenteredMenuValueScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 bool force_redraw,
                                 uint16_t footer_color) {
    draw_centered_menu_value(title, value, value_color, value_font, footer_text, force_redraw, footer_color, TFT_YELLOW);
}

void drawCenteredMenuSavedScreen(const char* title,
                                 const char* value,
                                 uint16_t value_color,
                                 MenuValueFont value_font,
                                 const char* footer_text,
                                 uint16_t footer_color) {
    draw_centered_menu_value(title, value, value_color, value_font, footer_text, true, footer_color, PB_SOUND_P1);
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
        tft.setFreeFont(font);
        if (tft.textWidth(line) > tft.width() - 14) {
            tft.setFreeFont(FONT_SMALL);
        }
        if (tft.textWidth(line) > tft.width() - 14) {
            tft.setTextFont(1);
        }
        tft.setTextColor(line_color, TFT_BLACK);
        tft.drawString(line, cx, start_y + (gap_y * i));
    }
    tft.setTextFont(0);
}

void drawBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal) {
    const int inner_w = w - 4;
    const int inner_h = h - 4;
    if (inner_w <= 0 || inner_h <= 0 || maxVal <= minVal) return;

    tft.drawRoundRect(x, y, w, h, 3, TFT_DARKGREY);
    if (isnan(value)) value = minVal;
    value = constrain(value, minVal, maxVal);
    int fill_w = (int)roundf(((value - minVal) / (maxVal - minVal)) * inner_w);
    fill_w = constrain(fill_w, 0, inner_w);

    tft.fillRect(x + 2, y + 2, inner_w, inner_h, TFT_BLACK);
    if (fill_w <= 0) return;

    int radius = 2;
    if (radius > fill_w / 2) radius = fill_w / 2;
    if (radius > inner_h / 2) radius = inner_h / 2;
    if (radius > 0) {
        tft.fillRoundRect(x + 2, y + 2, fill_w, inner_h, radius, color);
    } else {
        tft.fillRect(x + 2, y + 2, fill_w, inner_h, color);
    }
}

namespace {

struct BarGraphCache {
    bool used = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int last_fill_w = -1;
    uint16_t last_color = 0;
};

BarGraphCache g_bar_graph_cache[4];

int calculate_bar_fill_width(float value, float minVal, float maxVal, int inner_w) {
    if (isnan(value)) value = minVal;
    value = constrain(value, minVal, maxVal);
    int fill_w = (int)roundf(((value - minVal) / (maxVal - minVal)) * inner_w);
    return constrain(fill_w, 0, inner_w);
}

void draw_bar_fill(int x, int y, int inner_w, int inner_h, int fill_w, uint16_t color) {
    (void)inner_w;
    if (fill_w <= 0) return;

    int radius = 2;
    if (radius > fill_w / 2) radius = fill_w / 2;
    if (radius > inner_h / 2) radius = inner_h / 2;
    if (radius > 0) {
        tft.fillRoundRect(x + 2, y + 2, fill_w, inner_h, radius, color);
    } else {
        tft.fillRect(x + 2, y + 2, fill_w, inner_h, color);
    }
}

BarGraphCache& get_bar_graph_cache(int x, int y, int w, int h) {
    for (BarGraphCache& cache : g_bar_graph_cache) {
        if (cache.used && cache.x == x && cache.y == y && cache.w == w && cache.h == h) {
            return cache;
        }
    }

    for (BarGraphCache& cache : g_bar_graph_cache) {
        if (!cache.used) {
            cache.used = true;
            cache.x = x;
            cache.y = y;
            cache.w = w;
            cache.h = h;
            return cache;
        }
    }

    BarGraphCache& cache = g_bar_graph_cache[0];
    cache.used = true;
    cache.x = x;
    cache.y = y;
    cache.w = w;
    cache.h = h;
    cache.last_fill_w = -1;
    cache.last_color = 0;
    return cache;
}

} // namespace

void drawCachedBarGraph(int x, int y, int w, int h, uint16_t color, float value, float minVal, float maxVal, bool force_redraw) {
    const int inner_w = w - 4;
    const int inner_h = h - 4;
    if (inner_w <= 0 || inner_h <= 0 || maxVal <= minVal) return;

    BarGraphCache& cache = get_bar_graph_cache(x, y, w, h);
    const bool shell_needed = force_redraw || cache.last_fill_w < 0;
    if (shell_needed) {
        tft.drawRoundRect(x, y, w, h, 3, TFT_DARKGREY);
        tft.fillRect(x + 2, y + 2, inner_w, inner_h, TFT_BLACK);
        cache.last_fill_w = 0;
        cache.last_color = color;
    }

    const int fill_w = calculate_bar_fill_width(value, minVal, maxVal, inner_w);
    const bool color_changed = (cache.last_color != color);
    if (!shell_needed && !color_changed && fill_w == cache.last_fill_w) {
        return;
    }

    if (fill_w < cache.last_fill_w) {
        tft.fillRect(x + 2 + fill_w, y + 2, cache.last_fill_w - fill_w, inner_h, TFT_BLACK);
    }

    draw_bar_fill(x, y, inner_w, inner_h, fill_w, color);
    cache.last_fill_w = fill_w;
    cache.last_color = color;
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
 * newColor is used by the outer card and digits; borderColor is the inner card accent.
 */
void drawTimerCardContent(int cx, int cy, uint16_t borderColor, uint16_t newColor, const char* stateText, const char* time) {
    (void)cx;
    (void)cy;
    (void)time;

    const uint16_t card_bg = tft.color565(6, 10, 18);
    const uint16_t panel_bg = tft.color565(2, 4, 8);
    // Stable full-card shell. The outer border matches the timer digits.
    tft.fillRoundRect(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, LC_CARD_RADIUS, card_bg);
    drawCard(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, newColor);

    // Status centered above the numeric card.
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, card_bg);
    tft.drawString(stateText, tft.width() / 2, LT_STATUS_TEXT_Y);
    tft.setTextFont(0);

    // Inner card only for the timer digits.
    tft.fillRoundRect(LT_DIGIT_CARD_X, LT_DIGIT_CARD_Y, LT_DIGIT_CARD_W, LT_DIGIT_CARD_H, 5, panel_bg);
    tft.fillRect(LT_TIME_SPRITE_X, LT_TIME_SPRITE_Y, LT_TIME_SPRITE_W, LT_TIME_SPRITE_H, panel_bg);
    tft.drawRoundRect(LT_DIGIT_CARD_X, LT_DIGIT_CARD_Y, LT_DIGIT_CARD_W, LT_DIGIT_CARD_H, 5, borderColor);
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

namespace {

constexpr int kResetPanelX = 8;
constexpr int kResetPanelY = 30;
constexpr int kResetPanelW = 144;
constexpr int kResetPanelH = 76;
constexpr int kResetButtonW = 54;
constexpr int kResetButtonH = 22;
constexpr int kResetButtonY = 76;
constexpr int kResetNoX = 20;
constexpr int kResetYesX = 86;

uint16_t reset_danger_bg() { return tft.color565(132, 0, 12); }
uint16_t reset_panel_bg() { return tft.color565(54, 0, 8); }
uint16_t reset_panel_border() { return tft.color565(255, 118, 64); }
uint16_t reset_danger_text() { return tft.color565(255, 232, 206); }
uint16_t reset_danger_dim() { return tft.color565(220, 126, 126); }

void draw_reset_choice_button(int x,
                              const char* label,
                              bool selected,
                              uint16_t selected_bg,
                              uint16_t selected_text,
                              uint16_t panel_bg,
                              uint16_t danger_dim) {
    tft.fillRoundRect(x, kResetButtonY, kResetButtonW, kResetButtonH, 4,
                      selected ? selected_bg : panel_bg);
    tft.drawRoundRect(x, kResetButtonY, kResetButtonW, kResetButtonH, 4,
                      selected ? TFT_WHITE : danger_dim);

    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(selected ? selected_text : danger_dim,
                     selected ? selected_bg : panel_bg);
    tft.drawString(label, x + kResetButtonW / 2, kResetButtonY + kResetButtonH / 2 - 2);
    tft.setTextFont(0);
}

} // namespace

void drawResetChoicePromptShell(const char* title,
                                const char* line1,
                                const char* line2,
                                const char* footer_text,
                                uint16_t footer_color) {
    const uint16_t danger_bg = reset_danger_bg();
    const uint16_t panel_bg = reset_panel_bg();
    const uint16_t panel_border = reset_panel_border();
    const uint16_t danger_text = reset_danger_text();
    tft.fillScreen(danger_bg);

    const int cx = tft.width() / 2;

    tft.setTextDatum(C_BASELINE);
    tft.setFreeFont(FONT_HEADER);
    tft.setTextColor(TFT_WHITE, danger_bg);
    tft.drawString(title, cx, L_HEADER_Y);
    tft.setTextFont(0);
    tft.drawFastHLine(LC_MASTER_HEADER_LINE_X,
                      L_HEADER_LINE,
                      LC_MASTER_HEADER_LINE_W,
                      TFT_WHITE);

    tft.fillRoundRect(kResetPanelX, kResetPanelY, kResetPanelW, kResetPanelH, 5, panel_bg);
    tft.drawRoundRect(kResetPanelX, kResetPanelY, kResetPanelW, kResetPanelH, 5, panel_border);

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(danger_text, panel_bg);
    if (line1 && line1[0] != '\0') {
        tft.drawString(line1, cx, 41);
    }
    if (line2 && line2[0] != '\0') {
        tft.drawString(line2, cx, 54);
    }

    tft.drawFastHLine(kResetPanelX + 12, 69, kResetPanelW - 24, panel_border);

    if (footer_text && footer_text[0] != '\0') {
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(footer_color, danger_bg);
        if (tft.textWidth(footer_text) > tft.width() - 4) {
            tft.setTextFont(1);
        }
        tft.drawString(footer_text, cx, LM_MENU_FOOTER_Y - 4);
        tft.setTextFont(0);
    }
}

void updateResetChoiceButtons(const char* no_text,
                              const char* yes_text,
                              uint8_t selected_choice) {
    const uint16_t danger_bg = reset_danger_bg();
    const uint16_t panel_bg = reset_panel_bg();
    const uint16_t danger_dim = reset_danger_dim();
    const bool no_selected = (selected_choice == 0);
    const bool yes_selected = (selected_choice == 1);

    tft.setTextDatum(MC_DATUM);
    draw_reset_choice_button(kResetNoX, no_text, no_selected, TFT_YELLOW, TFT_BLACK, panel_bg, danger_dim);
    draw_reset_choice_button(kResetYesX, yes_text, yes_selected, TFT_WHITE, danger_bg, panel_bg, danger_dim);
}

void drawResetChoicePrompt(const char* title,
                           const char* line1,
                           const char* line2,
                           const char* no_text,
                           const char* yes_text,
                           uint8_t selected_choice,
                           const char* footer_text,
                           uint16_t footer_color) {
    drawResetChoicePromptShell(title, line1, line2, footer_text, footer_color);
    updateResetChoiceButtons(no_text, yes_text, selected_choice);
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
