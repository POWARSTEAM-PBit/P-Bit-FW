// ui_timer.cpp
// Timer screen renderer optimized for smooth updates.

#include "ui_timer.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard, drawTimerCardContent
#include "timer.h"      // Para estados del timer y getTimeHMS()
#include "languages.h"  // Para L()
#include "fonts.h"      // GFXfont
#include "layout.h"
#include <cstring>
#include <climits>
#include <stdio.h>

// External timer state shared with the main loop.
extern bool userTimerRunning;
extern unsigned long userTimerStart;
extern unsigned long userTimerElapsed;
extern volatile bool g_timer_just_reset;
extern volatile bool g_timer_just_finished;

namespace {

constexpr int LT_EDITOR_CARD_Y = 64;
constexpr int LT_EDITOR_CARD_H = 42;
constexpr int LT_EDITOR_TIME_Y = 82;
constexpr int LT_RUNTIME_SEGMENTS = 5;

} // namespace

static void draw_timer_value_sprite(const char* time, uint16_t color, bool force_redraw = false) {
    static TFT_eSprite segmentSprites[LT_RUNTIME_SEGMENTS] = {
        TFT_eSprite(&tft),
        TFT_eSprite(&tft),
        TFT_eSprite(&tft),
        TFT_eSprite(&tft),
        TFT_eSprite(&tft)
    };
    static bool sprites_ready = false;
    static bool layout_ready = false;
    static int segment_x[LT_RUNTIME_SEGMENTS] = {0, 0, 0, 0, 0};
    static int segment_w[LT_RUNTIME_SEGMENTS] = {0, 0, 0, 0, 0};
    static char last_fields[3][3] = {{0}, {0}, {0}};
    static uint16_t last_color = 0;

    auto ensure_layout = [&]() {
        if (layout_ready) return;

        tft.setFreeFont(FONT_TIMER);
        const int field_w = tft.textWidth("88") + 2;
        const int colon_w = tft.textWidth(":") + 2;
        const int total_w = field_w * 3 + colon_w * 2;
        int x = (LT_TIME_SPRITE_W - total_w) / 2;

        segment_x[0] = x;        segment_w[0] = field_w; x += field_w;
        segment_x[1] = x;        segment_w[1] = colon_w; x += colon_w;
        segment_x[2] = x;        segment_w[2] = field_w; x += field_w;
        segment_x[3] = x;        segment_w[3] = colon_w; x += colon_w;
        segment_x[4] = x;        segment_w[4] = field_w;
        tft.setTextFont(0);

        layout_ready = true;
    };

    auto ensure_sprites = [&]() {
        if (sprites_ready) return;
        ensure_layout();
        for (int i = 0; i < LT_RUNTIME_SEGMENTS; ++i) {
            segmentSprites[i].setColorDepth(16);
            segmentSprites[i].createSprite(segment_w[i], LT_TIME_SPRITE_H);
        }
        sprites_ready = true;
    };

    auto render_segment = [&](TFT_eSprite& sprite, const char* text) {
        sprite.fillSprite(TFT_BLACK);
        sprite.setTextDatum(MC_DATUM);
        sprite.setTextColor(color, TFT_BLACK);
        sprite.setFreeFont(FONT_TIMER);
        sprite.drawString(text, sprite.width() / 2, sprite.height() / 2 + 1);
        sprite.setTextFont(0);
    };

    auto push_segment = [&](int segment_index) {
        const int dst_x = LT_TIME_SPRITE_X + segment_x[segment_index];
        tft.fillRect(dst_x, LT_TIME_SPRITE_Y, segment_w[segment_index], LT_TIME_SPRITE_H, TFT_BLACK);
        segmentSprites[segment_index].pushSprite(dst_x, LT_TIME_SPRITE_Y, TFT_BLACK);
    };

    ensure_sprites();

    char fields[3][3] = {
        { time[0], time[1], '\0' },
        { time[3], time[4], '\0' },
        { time[6], time[7], '\0' }
    };

    const bool color_changed = (last_color != color);
    if (!force_redraw && !color_changed
        && strcmp(fields[0], last_fields[0]) == 0
        && strcmp(fields[1], last_fields[1]) == 0
        && strcmp(fields[2], last_fields[2]) == 0) {
        return;
    }

    if (force_redraw || color_changed || last_fields[0][0] == '\0') {
        tft.fillRect(LT_TIME_SPRITE_X, LT_TIME_SPRITE_Y, LT_TIME_SPRITE_W, LT_TIME_SPRITE_H, TFT_BLACK);

        render_segment(segmentSprites[0], fields[0]);
        render_segment(segmentSprites[1], ":");
        render_segment(segmentSprites[2], fields[1]);
        render_segment(segmentSprites[3], ":");
        render_segment(segmentSprites[4], fields[2]);

        for (int i = 0; i < LT_RUNTIME_SEGMENTS; ++i) {
            segmentSprites[i].pushSprite(LT_TIME_SPRITE_X + segment_x[i], LT_TIME_SPRITE_Y, TFT_BLACK);
        }
    } else {
        if (strcmp(fields[0], last_fields[0]) != 0) {
            render_segment(segmentSprites[0], fields[0]);
            push_segment(0);
        }
        if (strcmp(fields[1], last_fields[1]) != 0) {
            render_segment(segmentSprites[2], fields[1]);
            push_segment(2);
        }
        if (strcmp(fields[2], last_fields[2]) != 0) {
            render_segment(segmentSprites[4], fields[2]);
            push_segment(4);
        }
    }

    strncpy(last_fields[0], fields[0], sizeof(last_fields[0]) - 1);
    last_fields[0][sizeof(last_fields[0]) - 1] = '\0';
    strncpy(last_fields[1], fields[1], sizeof(last_fields[1]) - 1);
    last_fields[1][sizeof(last_fields[1]) - 1] = '\0';
    strncpy(last_fields[2], fields[2], sizeof(last_fields[2]) - 1);
    last_fields[2][sizeof(last_fields[2]) - 1] = '\0';
    last_color = color;
}

static void draw_timer_target_label(const char* label, uint16_t color) {
    tft.fillRect(0, LT_TARGET_CLEAR_Y, tft.width(), LT_TARGET_CLEAR_H, TFT_BLACK);
    if (!label || label[0] == '\0') {
        return;
    }
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(color, TFT_BLACK);
    tft.drawString(label, tft.width() / 2, LT_TARGET_Y);
    tft.setTextFont(0);
}

static void draw_timer_editor_value(int hours,
                                    int minutes,
                                    int seconds,
                                    TimerMenuField selected_field,
                                    bool editing) {
    char hh[4];
    char mm[4];
    char ss[4];
    snprintf(hh, sizeof(hh), "%02d", hours);
    snprintf(mm, sizeof(mm), "%02d", minutes);
    snprintf(ss, sizeof(ss), "%02d", seconds);

    const uint16_t selected_color = editing ? TFT_GREEN : TFT_YELLOW;
    const uint16_t normal_color = TFT_WHITE;
    const uint16_t colon_color = TFT_DARKGREY;

    tft.fillRect(LT_CARD_X + 6, LT_EDITOR_CARD_Y + 4, LT_CARD_W - 12, LT_EDITOR_CARD_H - 8, TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_TIMER);

    const int w_hh = tft.textWidth(hh);
    const int w_mm = tft.textWidth(mm);
    const int w_ss = tft.textWidth(ss);
    const int w_colon = tft.textWidth(":");
    const int total_w = w_hh + w_colon + w_mm + w_colon + w_ss;
    int x = (tft.width() / 2) - (total_w / 2);
    const int y = LT_EDITOR_TIME_Y - 14;

    tft.setTextColor(selected_field == TIMER_FIELD_HOURS ? selected_color : normal_color, TFT_BLACK);
    tft.drawString(hh, x, y);
    x += w_hh;

    tft.setTextColor(colon_color, TFT_BLACK);
    tft.drawString(":", x, y);
    x += w_colon;

    tft.setTextColor(selected_field == TIMER_FIELD_MINUTES ? selected_color : normal_color, TFT_BLACK);
    tft.drawString(mm, x, y);
    x += w_mm;

    tft.setTextColor(colon_color, TFT_BLACK);
    tft.drawString(":", x, y);
    x += w_colon;

    tft.setTextColor(selected_field == TIMER_FIELD_SECONDS ? selected_color : normal_color, TFT_BLACK);
    tft.drawString(ss, x, y);
    tft.setTextFont(0);
}

static void draw_timer_header(const char* title, uint16_t color) {
    drawHeader(title, color);
}

// The timer renderer needs all three arguments so it can refresh only the
// parts of the UI that actually changed.
void draw_timer_screen(bool screen_changed, bool data_changed, bool timer_needs_update) {
    (void)data_changed;

    int cx = tft.width() / 2;
    int cy = tft.height() / 2 + 10;
    static int last_menu_hours = -1;
    static int last_menu_minutes = -1;
    static int last_menu_seconds = -1;
    static int last_menu_field = -1;
    static bool last_menu_editing = false;
    static unsigned long last_duration_seconds = ULONG_MAX;

    if (timer_menu_is_active()) {
        const int menu_hours = getTimerMenuHours();
        const int menu_minutes = getTimerMenuMinutes();
        const int menu_seconds = getTimerMenuSeconds();
        const int selected_field = (int)getTimerMenuSelectedField();
        const bool editing = timer_menu_is_editing();
        if (screen_changed) {
            tft.fillScreen(TFT_BLACK);
            draw_timer_header(L(TIT_TIMER), TFT_BLUE);
        }
        if (screen_changed
            || menu_hours != last_menu_hours
            || menu_minutes != last_menu_minutes
            || menu_seconds != last_menu_seconds
            || selected_field != last_menu_field
            || editing != last_menu_editing) {
            drawCenteredMenuFrame(L(ST_TIMER_DURATION),
                                  TFT_YELLOW,
                                  editing ? L(ST_TIMER_CFG_EDIT) : L(ST_TIMER_CFG_SELECT),
                                  TFT_CYAN);
            tft.fillRect(LT_CARD_X - 2, LT_EDITOR_CARD_Y - 2, LT_CARD_W + 4, LT_EDITOR_CARD_H + 4, TFT_BLACK);
            tft.fillRect(LT_CARD_X + 3, LT_EDITOR_CARD_Y + 3, LT_CARD_W - 6, LT_EDITOR_CARD_H - 6, TFT_BLACK);
            drawCard(LT_CARD_X, LT_EDITOR_CARD_Y, LT_CARD_W, LT_EDITOR_CARD_H, editing ? TFT_GREEN : TFT_BLUE);
            draw_timer_editor_value(menu_hours, menu_minutes, menu_seconds, (TimerMenuField)selected_field, editing);
            last_menu_hours = menu_hours;
            last_menu_minutes = menu_minutes;
            last_menu_seconds = menu_seconds;
            last_menu_field = selected_field;
            last_menu_editing = editing;
        }
        return;
    }

    // Keep track of the last drawn state so we only repaint on changes.
    static uint16_t last_drawn_state = 0; // 0=READY, 1=RUNNING, 2=PAUSED

    // Determine the current timer state and the colors that represent it.
    uint16_t current_timer_state = 0; // 0=READY
    uint16_t borderColor = TFT_BLUE;
    uint16_t newColor = TFT_BLUE;
    const char * stateText = L(ST_TIMER_RDY);
    const char * instructionText = L(ST_PUSH_START);

    if (userTimerRunning) {
        current_timer_state = 1; // RUNNING
        borderColor = TFT_GREEN;
        newColor = TFT_GREEN;
        stateText = L(ST_TIMER_RUN);
        instructionText = L(ST_PUSH_PAUSE);
    } else if (userTimerElapsed > 0) {
        current_timer_state = 2; // PAUSED
        borderColor = TFT_YELLOW;
        newColor = TFT_YELLOW;
        stateText = L(ST_TIMER_PAU);
        instructionText = L(ST_PUSH_RESET);
    }

    if (g_timer_just_finished) {
        borderColor = TFT_RED;
        newColor = TFT_RED;
    }

    const unsigned long duration_seconds = getTimerDurationSeconds();
    const bool duration_changed = duration_seconds != last_duration_seconds;
    const bool countdown_mode = (getTimerPresetMs() > 0);

    // Detect whether the visible state changed and needs a full redraw.
    bool state_changed_visually = screen_changed
                               || g_timer_just_reset
                               || g_timer_just_finished
                               || duration_changed
                               || (current_timer_state != last_drawn_state);

    // 3. DIBUJO ESTÁTICO (Título)
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_timer_header(L(TIT_TIMER), TFT_BLUE);
    }
    
    // 4. DIBUJO SEMI-ESTÁTICO (Marco, Estado E INSTRUCCIONES)
    if (state_changed_visually) {

        // Dibujar el card primero: la limpieza interna deja libres el header y el footer.
        drawTimerCardContent(cx, cy, borderColor, newColor, stateText, getTimeHMS());
        draw_timer_value_sprite(getTimeHMS(), newColor, true);
        if (countdown_mode) {
            draw_timer_target_label(getTimerDurationLabel(), TFT_DARKGREY);
        } else {
            draw_timer_target_label(nullptr, TFT_DARKGREY);
        }

        // La ayuda vive en su propia banda para no pelear con el card.
        tft.fillRect(0, LT_HINT_CLEAR_Y, tft.width(), LT_HINT_CLEAR_H, TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.setFreeFont(FONT_SMALL);
        tft.drawString(instructionText, cx, LT_HINT_Y);
        tft.setTextFont(0);

        // Actualizar el último estado dibujado
        last_drawn_state = current_timer_state;
        last_duration_seconds = duration_seconds;
        g_timer_just_finished = false;
        g_timer_just_reset = false;
    }

    // 5. DIBUJO DINÁMICO (Tiempo) - Se ejecuta frecuentemente.
    if (timer_needs_update && !state_changed_visually) {

        const char * time = getTimeHMS();
        draw_timer_value_sprite(time, newColor, false);
    }
}
