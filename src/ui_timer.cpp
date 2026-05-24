// ui_timer.cpp
// Timer screen renderer optimized for smooth updates.

#include "ui_timer.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard, drawTimerCardContent
#include "timer.h"      // Para estados del timer y getTimeHMS()
#include "languages.h"  // Para L()
#include "fonts.h"      // GFXfont
#include "layout.h"
#include "palette.h"
#include <Arduino.h>
#include <cstring>
#include <climits>
#include <stdio.h>

// External timer state shared with the main loop.
extern bool userTimerRunning;
extern unsigned long userTimerElapsed;
extern volatile bool g_timer_just_reset;
extern volatile bool g_timer_just_finished;
extern volatile bool g_timer_finished_active;

namespace {

constexpr int LT_RUNTIME_SEGMENTS = 5;
constexpr uint16_t kTimerCardBg = 0x0042;   // tft.color565(6, 10, 18)
constexpr uint16_t kTimerPanelBg = 0x0021;  // tft.color565(2, 4, 8)

} // namespace

static uint16_t timer_pair_color(uint16_t state_color);

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
    static const GFXfont* digit_font = FONT_TIMER;
    static char last_fields[3][3] = {{0}, {0}, {0}};
    static uint16_t last_color = 0;

    auto ensure_layout = [&]() {
        if (layout_ready) return;

        auto measure_segments = [&]() {
            const int field_w = tft.textWidth("88") + 2;
            const int colon_w = tft.textWidth(":") + 2;
            return field_w * 3 + colon_w * 2;
        };

        digit_font = FONT_TIMER;
        tft.setFreeFont(digit_font);
        if (measure_segments() > LT_TIME_SPRITE_W - 2) {
            digit_font = FONT_BODY;
            tft.setFreeFont(digit_font);
        }

        const int safe_field_w = tft.textWidth("88") + 2;
        const int safe_colon_w = tft.textWidth(":") + 2;
        const int total_w = safe_field_w * 3 + safe_colon_w * 2;
        int x = (LT_TIME_SPRITE_W - total_w) / 2;

        segment_x[0] = x;        segment_w[0] = safe_field_w; x += safe_field_w;
        segment_x[1] = x;        segment_w[1] = safe_colon_w; x += safe_colon_w;
        segment_x[2] = x;        segment_w[2] = safe_field_w; x += safe_field_w;
        segment_x[3] = x;        segment_w[3] = safe_colon_w; x += safe_colon_w;
        segment_x[4] = x;        segment_w[4] = safe_field_w;
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

    auto render_segment = [&](TFT_eSprite& sprite, const char* text, uint16_t segment_color) {
        sprite.fillSprite(kTimerPanelBg);
        sprite.setTextDatum(MC_DATUM);
        sprite.setTextColor(segment_color, kTimerPanelBg);
        sprite.setFreeFont(digit_font);
        sprite.drawString(text, sprite.width() / 2, sprite.height() / 2);
        sprite.setTextFont(0);
    };

    auto push_segment = [&](int segment_index) {
        const int dst_x = LT_TIME_SPRITE_X + segment_x[segment_index];
        segmentSprites[segment_index].pushSprite(dst_x, LT_TIME_SPRITE_Y);
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
        const uint16_t colon_color = timer_pair_color(color);
        render_segment(segmentSprites[0], fields[0], color);
        render_segment(segmentSprites[1], ":", colon_color);
        render_segment(segmentSprites[2], fields[1], color);
        render_segment(segmentSprites[3], ":", colon_color);
        render_segment(segmentSprites[4], fields[2], color);

        for (int i = 0; i < LT_RUNTIME_SEGMENTS; ++i) {
            segmentSprites[i].pushSprite(LT_TIME_SPRITE_X + segment_x[i], LT_TIME_SPRITE_Y);
        }
    } else {
        if (strcmp(fields[0], last_fields[0]) != 0) {
            render_segment(segmentSprites[0], fields[0], color);
            push_segment(0);
        }
        if (strcmp(fields[1], last_fields[1]) != 0) {
            render_segment(segmentSprites[2], fields[1], color);
            push_segment(2);
        }
        if (strcmp(fields[2], last_fields[2]) != 0) {
            render_segment(segmentSprites[4], fields[2], color);
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

static uint16_t timer_pair_color(uint16_t state_color) {
    if (state_color == PB_DS18_P2)  return PB_HUM_P3;   // Ready: laser blue + aqua.
    if (state_color == PB_SOUND_P2) return PB_HUM_P2;   // Running: acid green + cobalt.
    if (state_color == PB_TEMP_P1)  return PB_DS18_P1;  // Paused: orange + violet.
    if (state_color == PB_SOUND_P3) return PB_SOUND_P1; // Alarm: red + magenta.
    if (state_color == PB_SOUND_P1) return PB_SOUND_P3; // Alarm flash: magenta + red.
    if (state_color == PB_HUM_P1)   return PB_LUZ_P1;   // Config select: cyan + yellow.
    if (state_color == PB_LUZ_P3)   return PB_TEMP_P2;  // Config edit: gold + hot pink.
    return PB_HUM_P1;
}

static void draw_timer_hint(const char* text) {
    tft.fillRect(LT_CARD_X + 8, LT_HINT_CLEAR_Y, LT_CARD_W - 16, LT_HINT_CLEAR_H, kTimerCardBg);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kTimerCardBg);
    if (tft.textWidth(text) > LT_CARD_W - 18) {
        tft.setTextFont(1);
    }
    tft.drawString(text, tft.width() / 2, LT_HINT_Y);
    tft.setTextFont(0);
}

static void draw_timer_menu_hint(bool editing) {
    tft.fillRect(LT_CARD_X + 8, LT_TIMER_MENU_HINT_Y1 - 2, LT_CARD_W - 16, 24, kTimerCardBg);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(TFT_WHITE, kTimerCardBg);
    if (editing) {
        tft.drawString(L(ST_TIMER_CFG_EDIT), tft.width() / 2, LT_TIMER_MENU_HINT_Y1 + 5);
    } else {
        switch (normalizeLanguage(g_language)) {
            case LANG_CAT:
                tft.drawString("Prem editar", tft.width() / 2, LT_TIMER_MENU_HINT_Y1);
                tft.drawString("Mant. desar", tft.width() / 2, LT_TIMER_MENU_HINT_Y2);
                break;
            case LANG_EN:
                tft.drawString("Press edit", tft.width() / 2, LT_TIMER_MENU_HINT_Y1);
                tft.drawString("Hold save", tft.width() / 2, LT_TIMER_MENU_HINT_Y2);
                break;
            case LANG_ES:
            default:
                tft.drawString("Pulsa editar", tft.width() / 2, LT_TIMER_MENU_HINT_Y1);
                tft.drawString("Mant. guardar", tft.width() / 2, LT_TIMER_MENU_HINT_Y2);
                break;
        }
    }
    tft.setTextFont(0);
}

static void draw_timer_menu_shell(bool editing) {
    const uint16_t accent = editing ? PB_LUZ_P3 : PB_HUM_P1;
    const uint16_t secondary = timer_pair_color(accent);
    const uint16_t card_bg = kTimerCardBg;
    const uint16_t panel_bg = kTimerPanelBg;

    tft.fillRoundRect(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, LC_CARD_RADIUS, card_bg);
    drawCard(LT_CARD_X, LT_CARD_Y, LT_CARD_W, LT_CARD_H, accent);

    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(secondary, card_bg);
    tft.drawString(L(ST_TIMER_DURATION), tft.width() / 2, LT_TIMER_MENU_TITLE_Y);
    tft.setTextFont(0);

    tft.fillRoundRect(LT_TIMER_MENU_DIGIT_CARD_X, LT_TIMER_MENU_DIGIT_CARD_Y,
                      LT_TIMER_MENU_DIGIT_CARD_W, LT_TIMER_MENU_DIGIT_CARD_H, 5, panel_bg);
    tft.drawRoundRect(LT_TIMER_MENU_DIGIT_CARD_X, LT_TIMER_MENU_DIGIT_CARD_Y,
                      LT_TIMER_MENU_DIGIT_CARD_W, LT_TIMER_MENU_DIGIT_CARD_H, 5, secondary);

    draw_timer_menu_hint(editing);
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

    const uint16_t selected_color = editing ? PB_LUZ_P3 : PB_HUM_P1;
    const uint16_t normal_color = TFT_WHITE;
    const uint16_t colon_color = timer_pair_color(selected_color);

    tft.fillRect(LT_TIMER_MENU_DIGIT_CARD_X + 2, LT_TIMER_MENU_DIGIT_CARD_Y + 4,
                 LT_TIMER_MENU_DIGIT_CARD_W - 4, 26, kTimerPanelBg);
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont(FONT_TIMER);

    const int w_hh = tft.textWidth(hh);
    const int w_mm = tft.textWidth(mm);
    const int w_ss = tft.textWidth(ss);
    const int w_colon = tft.textWidth(":");
    const int total_w = w_hh + w_colon + w_mm + w_colon + w_ss;
    int x = (tft.width() / 2) - (total_w / 2);
    const int y = LT_TIMER_MENU_VALUE_Y;

    tft.setTextColor(selected_field == TIMER_FIELD_HOURS ? selected_color : normal_color, kTimerPanelBg);
    tft.drawString(hh, x, y);
    x += w_hh;

    tft.setTextColor(colon_color, kTimerPanelBg);
    tft.drawString(":", x, y);
    x += w_colon;

    tft.setTextColor(selected_field == TIMER_FIELD_MINUTES ? selected_color : normal_color, kTimerPanelBg);
    tft.drawString(mm, x, y);
    x += w_mm;

    tft.setTextColor(colon_color, kTimerPanelBg);
    tft.drawString(":", x, y);
    x += w_colon;

    tft.setTextColor(selected_field == TIMER_FIELD_SECONDS ? selected_color : normal_color, kTimerPanelBg);
    tft.drawString(ss, x, y);
    tft.setTextFont(0);
}

static void draw_timer_header(const char* title) {
    drawHeader(title);
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
            draw_timer_header(L(TIT_TIMER));
        }
        const bool menu_content_changed = screen_changed
                                       || menu_hours != last_menu_hours
                                       || menu_minutes != last_menu_minutes
                                       || menu_seconds != last_menu_seconds
                                       || selected_field != last_menu_field
                                       || editing != last_menu_editing;
        const bool menu_shell_changed = screen_changed || editing != last_menu_editing;
        if (menu_content_changed) {
            if (menu_shell_changed) {
                draw_timer_menu_shell(editing);
            }
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
    static uint16_t last_drawn_state = 0; // 0=READY, 1=RUNNING, 2=PAUSED, 3=ALARM
    static bool last_alarm_flash = false;
    static bool last_finished_active = false;

    // Determine the current timer state and the colors that represent it.
    uint16_t current_timer_state = 0; // 0=READY
    uint16_t newColor = PB_DS18_P2;
    const char * stateText = L(ST_TIMER_RDY);
    const char * instructionText = L(ST_PUSH_START);
    const bool timer_finished = g_timer_finished_active;
    const bool alarm_flash = timer_finished && (((millis() / 250UL) & 0x01UL) == 0);

    if (timer_finished) {
        current_timer_state = 3; // ALARM
        newColor = alarm_flash ? PB_SOUND_P3 : PB_SOUND_P1;
        stateText = L(ST_TIMER_DONE);
        instructionText = L(ST_PUSH_RESET);
    } else if (userTimerRunning) {
        current_timer_state = 1; // RUNNING
        newColor = PB_SOUND_P2;
        stateText = L(ST_TIMER_RUN);
        instructionText = L(ST_PUSH_PAUSE);
    } else if (userTimerElapsed > 0) {
        current_timer_state = 2; // PAUSED
        newColor = PB_TEMP_P1;
        stateText = L(ST_TIMER_PAU);
        instructionText = L(ST_PUSH_RESET);
    }

    if (g_timer_just_finished) {
        newColor = PB_SOUND_P3;
    }

    const unsigned long duration_seconds = getTimerDurationSeconds();
    const bool duration_changed = duration_seconds != last_duration_seconds;
    // Detect whether the visible state changed and needs a full redraw.
    bool state_changed_visually = screen_changed
                               || g_timer_just_reset
                               || g_timer_just_finished
                               || (timer_finished && alarm_flash != last_alarm_flash)
                               || timer_finished != last_finished_active
                               || duration_changed
                               || (current_timer_state != last_drawn_state);

    // 3. DIBUJO ESTÁTICO (Título)
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_timer_header(L(TIT_TIMER));
    }
    
    // 4. DIBUJO SEMI-ESTÁTICO (Marco, Estado E INSTRUCCIONES)
    if (state_changed_visually) {

        // Dibujar el card primero: la limpieza interna deja libres el header y el footer.
        const uint16_t borderColor = timer_pair_color(newColor);
        drawTimerCardContent(cx, cy, borderColor, newColor, stateText, getTimeHMS());
        draw_timer_value_sprite(getTimeHMS(), newColor, true);

        // La ayuda vive integrada dentro de la card.
        draw_timer_hint(instructionText);

        // Actualizar el último estado dibujado
        last_drawn_state = current_timer_state;
        last_duration_seconds = duration_seconds;
        last_alarm_flash = alarm_flash;
        last_finished_active = timer_finished;
        g_timer_just_finished = false;
        g_timer_just_reset = false;
    }

    // 5. DIBUJO DINÁMICO (Tiempo) - Se ejecuta frecuentemente.
    if (timer_needs_update && !state_changed_visually) {

        const char * time = getTimeHMS();
        draw_timer_value_sprite(time, newColor, false);
    }
}
