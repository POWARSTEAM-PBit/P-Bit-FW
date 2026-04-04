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

} // namespace

static void draw_timer_value_sprite(const char* time, uint16_t color, bool force_redraw = false) {
    static TFT_eSprite timerSprite = TFT_eSprite(&tft);
    static bool sprite_ready = false;
    static char last_time_str[16] = "";

    if (!sprite_ready) {
        timerSprite.setColorDepth(16);
        timerSprite.createSprite(LT_TIME_SPRITE_W, LT_TIME_SPRITE_H);
        sprite_ready = true;
    }

    if (!force_redraw && strcmp(time, last_time_str) == 0) {
        return;
    }

    strncpy(last_time_str, time, sizeof(last_time_str) - 1);
    last_time_str[sizeof(last_time_str) - 1] = '\0';

    timerSprite.fillSprite(TFT_BLACK);
    timerSprite.setTextDatum(MC_DATUM);
    timerSprite.setTextColor(color, TFT_BLACK);
    timerSprite.setFreeFont(FONT_TIMER);
    timerSprite.drawString(time, timerSprite.width() / 2, timerSprite.height() / 2 + 1);
    timerSprite.setTextFont(0);

    // Clear only the digit band and push the sprite inside the card. The runtime
    // card is now tall enough that this band no longer touches the border, so we
    // can avoid redrawing the full frame on every timer tick and reduce ghosting.
    tft.fillRect(LT_TIME_SPRITE_X, LT_TIME_SPRITE_Y, LT_TIME_SPRITE_W, LT_TIME_SPRITE_H, TFT_BLACK);
    timerSprite.pushSprite(LT_TIME_SPRITE_X, LT_TIME_SPRITE_Y, TFT_BLACK);
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
