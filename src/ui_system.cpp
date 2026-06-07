#include "ui_system.h"
#include "ui_widgets.h"
#include "hw.h"
#include "ble.h"
#include "settings_store.h"
#include "languages.h"
#include "fonts.h"
#include "layout.h"
#include "lang_select.h"
#include "runtime_events.h"
#include "palette.h"
#include <stdio.h>
#include <esp_system.h>

static SysMenuState g_sys_menu_state = SYS_MODE_NORMAL;
static uint8_t g_sys_menu_index = 0;
static uint32_t g_sys_sleep_ms = 0;
static uint8_t g_sys_lang_index = 0;
static bool g_sys_sound_enabled = false;
static bool g_sys_alarm_sound_enabled = false;
static uint8_t g_sys_reset_choice = 0;
static uint8_t g_sys_saved_kind = 0;

extern bool g_sound_enabled;
extern bool g_alarm_sound_enabled;
extern bool g_is_fahrenheit;
extern char dev_name[];
extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

// Sleep presets are stored as raw milliseconds so the menu can stay simple.
const uint32_t SLEEP_OPTIONS[] = { 30000, 60000, 120000, 300000, 600000, 0 };
const int NUM_SLEEP_OPTIONS = sizeof(SLEEP_OPTIONS) / sizeof(SLEEP_OPTIONS[0]);

static void draw_system_header(const char* title) {
    drawHeader(title);
}

static void draw_system_reset_restart_overlay() {
    tft.fillScreen(TFT_BLACK);
    draw_system_header(L(MENU_FULL_RESET));
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(L(MENU_RESET_DONE), tft.width() / 2, 55);
    tft.drawString(L(ST_RESTARTING), tft.width() / 2, 78);
    tft.setTextFont(0);
}

static const char* get_sleep_option_name(int index) {
    switch (index) {
        case 0: return L(MENU_SLEEP_30S);
        case 1: return L(MENU_SLEEP_1M);
        case 2: return L(MENU_SLEEP_2M);
        case 3: return L(MENU_SLEEP_5M);
        case 4: return L(MENU_SLEEP_10M);
        case 5:
        default:
            return L(MENU_NEVER);
    }
}

static const char* get_language_name(uint8_t index) {
    switch (index) {
        case LANG_CAT: return L(LANG_CAT_NAME);
        case LANG_EN: return L(LANG_EN_NAME);
        case LANG_ES:
        default:
            return L(LANG_ES_NAME);
    }
}

static void draw_system_id_panel(int x, int y, int w, int h,
                                 const char* label, const char* value,
                                 bool show_ble_badge, bool ble_connected) {
    const uint16_t panel_bg = tft.color565(2, 4, 10);
    tft.fillRoundRect(x, y, w, h, 4, panel_bg);
    tft.drawRoundRect(x, y, w, h, 4, PB_SOIL_P1);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(PB_HUM_P3, panel_bg);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(label, x + 10, y + h / 2 - 2);
    tft.setTextColor(PB_LUZ_P1, panel_bg);
    if (show_ble_badge) {
        tft.setTextDatum(ML_DATUM);
        tft.drawString(value, x + 40, y + h / 2 - 2);

        const uint16_t badge_color = ble_connected ? PB_SOUND_P2 : PB_SOUND_P3;
        const int badge_x = x + w - 40;
        const int badge_y = y + 2;
        tft.fillRoundRect(badge_x, badge_y, 31, h - 4, 3, panel_bg);
        tft.drawRoundRect(badge_x, badge_y, 31, h - 4, 3, badge_color);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(badge_color, panel_bg);
        tft.drawString(L(SYS_BLE_LABEL), badge_x + 15, y + h / 2 - 2);
    } else {
        tft.setTextDatum(MR_DATUM);
        tft.drawString(value, x + w - 10, y + h / 2 - 2);
    }
    tft.setTextFont(0);
}

static void draw_system_text_panel(int x, int y, int w, int h,
                                   const char* label, const char* value,
                                   uint16_t label_color, uint16_t value_color,
                                   uint16_t border_color,
                                   int value_y_adjust = 0,
                                   int label_y_adjust = 0) {
    const uint16_t panel_bg = tft.color565(2, 4, 10);
    tft.fillRoundRect(x, y, w, h, 4, panel_bg);
    tft.drawRoundRect(x, y, w, h, 4, border_color);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(label_color, panel_bg);
    tft.drawString(label, x + w / 2, y + label_y_adjust);
    tft.setTextColor(value_color, panel_bg);
    tft.drawString(value, x + w / 2, y + h - 14 + value_y_adjust);
    tft.setTextFont(0);
}

static void draw_system_audio_panel(int x, int y, int w, int h) {
    const uint16_t panel_bg = tft.color565(2, 4, 10);
    const uint16_t beep_col = g_sound_enabled ? PB_SOUND_P2 : PB_SOUND_P3;
    const uint16_t alarm_col = g_alarm_sound_enabled ? PB_SOUND_P2 : PB_SOUND_P3;
    const int left_cx = x + w / 4;
    const int right_cx = x + (w * 3) / 4;
    tft.fillRoundRect(x, y, w, h, 4, panel_bg);
    tft.drawRoundRect(x, y, w, h, 4, PB_SOUND_P1);
    tft.setTextDatum(TC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(PB_HUM_P3, panel_bg);
    tft.drawString(L(MENU_SOUND), left_cx, y + 2);
    tft.drawString(L(MENU_ALARM_SOUND), right_cx, y + 2);
    tft.setTextColor(beep_col, panel_bg);
    tft.drawString(g_sound_enabled ? L(ST_ON) : L(ST_OFF), left_cx, y + h - 18);
    tft.setTextColor(alarm_col, panel_bg);
    tft.drawString(g_alarm_sound_enabled ? L(ST_ON) : L(ST_OFF), right_cx, y + h - 18);
    tft.setTextFont(0);
}

static void request_system_redraw(bool force_full = false) {
    runtime_request_ui_refresh(force_full);
}

static int get_sleep_option_index(uint32_t sleep_ms) {
    for (int i = 0; i < NUM_SLEEP_OPTIONS; ++i) {
        if (sleep_ms == SLEEP_OPTIONS[i]) return i;
    }
    return 0;
}

void start_system_menu() {
    // Snapshot the active system settings before editing them in the menu.
    g_sys_menu_state = SYS_MODE_MENU;
    g_sys_menu_index = 0;
    g_sys_sleep_ms = get_sleep_timeout();
    g_sys_lang_index = (uint8_t)normalizeLanguage(g_language);
    g_sys_sound_enabled = g_sound_enabled;
    g_sys_alarm_sound_enabled = g_alarm_sound_enabled;
    g_sys_reset_choice = 0;
    g_sys_saved_kind = 0;
    request_system_redraw(true);
}

bool system_menu_is_active() {
    return g_sys_menu_state != SYS_MODE_NORMAL;
}

SysMenuState get_system_menu_state() {
    return g_sys_menu_state;
}

int get_system_encoder_min() { return 0; }

int get_system_encoder_max() {
    switch (g_sys_menu_state) {
        case SYS_MODE_MENU: return 5;
        case SYS_MODE_EDIT_SOUND: return 1;
        case SYS_MODE_EDIT_ALARM_SOUND: return 1;
        case SYS_MODE_EDIT_SLEEP: return NUM_SLEEP_OPTIONS - 1;
        case SYS_MODE_EDIT_LANG: return (int)LANG_COUNT - 1;
        case SYS_MODE_CONFIRM_RESET: return 1;
        case SYS_MODE_SAVED: return 0;
        default: return 0;
    }
}

int get_system_encoder_value() {
    switch (g_sys_menu_state) {
        case SYS_MODE_MENU: return g_sys_menu_index;
        case SYS_MODE_EDIT_SOUND: return g_sys_sound_enabled ? 1 : 0;
        case SYS_MODE_EDIT_ALARM_SOUND: return g_sys_alarm_sound_enabled ? 1 : 0;
        case SYS_MODE_EDIT_SLEEP: return get_sleep_option_index(g_sys_sleep_ms);
        case SYS_MODE_EDIT_LANG: return g_sys_lang_index;
        case SYS_MODE_CONFIRM_RESET: return g_sys_reset_choice;
        case SYS_MODE_SAVED: return 0;
        default: return 0;
    }
}

void set_system_input_value(int value) {
    int next = constrain(value, get_system_encoder_min(), get_system_encoder_max());

    switch (g_sys_menu_state) {
        case SYS_MODE_MENU:
            if ((uint8_t)next != g_sys_menu_index) {
                g_sys_menu_index = (uint8_t)next;
                request_system_redraw(false);
            }
            break;
        case SYS_MODE_EDIT_SOUND: {
            bool enabled = (next == 1);
            if (enabled != g_sys_sound_enabled) {
                g_sys_sound_enabled = enabled;
                request_system_redraw(false);
            }
            break;
        }
        case SYS_MODE_EDIT_ALARM_SOUND: {
            bool enabled = (next == 1);
            if (enabled != g_sys_alarm_sound_enabled) {
                g_sys_alarm_sound_enabled = enabled;
                request_system_redraw(false);
            }
            break;
        }
        case SYS_MODE_EDIT_SLEEP: {
            uint32_t next_sleep_ms = SLEEP_OPTIONS[next];
            if (next_sleep_ms != g_sys_sleep_ms) {
                g_sys_sleep_ms = next_sleep_ms;
                request_system_redraw(false);
            }
            break;
        }
        case SYS_MODE_EDIT_LANG:
            if ((uint8_t)next != g_sys_lang_index) {
                g_sys_lang_index = (uint8_t)next;
                request_system_redraw(false);
            }
            break;
        case SYS_MODE_CONFIRM_RESET:
            if ((uint8_t)next != g_sys_reset_choice) {
                g_sys_reset_choice = (uint8_t)next;
                request_system_redraw(false);
            }
            break;
        default:
            break;
    }
}

uint8_t handle_system_button() {
    bool force_full = false;

    switch (g_sys_menu_state) {
        case SYS_MODE_MENU:
            if (g_sys_menu_index == 0) {
                g_sys_menu_state = SYS_MODE_EDIT_SOUND;
            } else if (g_sys_menu_index == 1) {
                g_sys_menu_state = SYS_MODE_EDIT_ALARM_SOUND;
            } else if (g_sys_menu_index == 2) {
                g_sys_menu_state = SYS_MODE_EDIT_SLEEP;
            } else if (g_sys_menu_index == 3) {
                g_sys_menu_state = SYS_MODE_EDIT_LANG;
            } else if (g_sys_menu_index == 4) {
                g_sys_reset_choice = 0;
                g_sys_menu_state = SYS_MODE_CONFIRM_RESET;
            } else {
                g_sys_menu_state = SYS_MODE_NORMAL;
                force_full = true;
            }
            break;
        case SYS_MODE_EDIT_SOUND:
            save_sound_enabled(g_sys_sound_enabled);
            g_sys_saved_kind = 0;
            g_sys_menu_state = SYS_MODE_SAVED;
            break;
        case SYS_MODE_EDIT_ALARM_SOUND:
            save_alarm_sound_enabled(g_sys_alarm_sound_enabled);
            g_sys_saved_kind = 4;
            g_sys_menu_state = SYS_MODE_SAVED;
            break;
        case SYS_MODE_EDIT_SLEEP:
            save_sleep_timeout(g_sys_sleep_ms);
            g_sys_saved_kind = 1;
            g_sys_menu_state = SYS_MODE_SAVED;
            break;
        case SYS_MODE_EDIT_LANG:
            saveLanguage((Language)g_sys_lang_index);
            g_sys_saved_kind = 2;
            g_sys_menu_state = SYS_MODE_SAVED;
            force_full = true;
            break;
        case SYS_MODE_CONFIRM_RESET:
            if (g_sys_reset_choice == 1) {
                reset_all_settings();
                draw_system_reset_restart_overlay();
                delay(700);
                esp_restart();
                return 0; // unreachable
            } else {
                g_sys_menu_state = SYS_MODE_MENU;
            }
            break;
        case SYS_MODE_SAVED:
            g_sys_menu_state = SYS_MODE_MENU;
            break;
        default:
            g_sys_menu_state = SYS_MODE_NORMAL;
            force_full = true;
            break;
    }

    request_system_redraw(force_full || g_sys_menu_state == SYS_MODE_NORMAL);
    return (uint8_t)g_sys_menu_state;
}

static void draw_system_menu_screen(bool screen_changed) {
    const int cx = tft.width() / 2;
    static SysMenuState last_drawn_state = SYS_MODE_NORMAL;
    static int last_menu_index = -1;
    static int last_sound_value = -1;
    static int last_alarm_sound_value = -1;
    static int last_sleep_index = -1;
    static int last_lang_index = -1;
    static int last_reset_choice = -1;
    static int last_saved_kind = -1;

    bool state_changed = screen_changed || (g_sys_menu_state != last_drawn_state);
    bool needs_redraw = state_changed;

    if (g_sys_menu_state == SYS_MODE_MENU) {
        needs_redraw = needs_redraw || (last_menu_index != (int)g_sys_menu_index);
    } else if (g_sys_menu_state == SYS_MODE_EDIT_SOUND) {
        needs_redraw = needs_redraw || (last_sound_value != (g_sys_sound_enabled ? 1 : 0));
    } else if (g_sys_menu_state == SYS_MODE_EDIT_ALARM_SOUND) {
        needs_redraw = needs_redraw || (last_alarm_sound_value != (g_sys_alarm_sound_enabled ? 1 : 0));
    } else if (g_sys_menu_state == SYS_MODE_EDIT_SLEEP) {
        needs_redraw = needs_redraw || (last_sleep_index != get_sleep_option_index(g_sys_sleep_ms));
    } else if (g_sys_menu_state == SYS_MODE_EDIT_LANG) {
        needs_redraw = needs_redraw || (last_lang_index != (int)g_sys_lang_index);
    } else if (g_sys_menu_state == SYS_MODE_CONFIRM_RESET) {
        needs_redraw = needs_redraw || (last_reset_choice != (int)g_sys_reset_choice);
    } else if (g_sys_menu_state == SYS_MODE_SAVED) {
        needs_redraw = needs_redraw || (last_saved_kind != (int)g_sys_saved_kind);
    }

    if (!needs_redraw) return;

    if (state_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_system_header(L(MENU_SETTINGS));
        last_menu_index = -1;
        last_sound_value = -1;
        last_alarm_sound_value = -1;
        last_sleep_index = -1;
        last_lang_index = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
    }

    tft.setTextDatum(MC_DATUM);

    if (g_sys_menu_state == SYS_MODE_MENU) {
        const char* items[] = {
            L(MENU_SOUND),
            L(MENU_ALARM_SOUND),
            L(MENU_SLEEP),
            L(MENU_TITLE)
        };
        drawSettingsGridMenu(items, 4, g_sys_menu_index, L(MENU_RESET), L(MENU_EXIT));
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_sys_menu_index;
    } else if (g_sys_menu_state == SYS_MODE_EDIT_SOUND) {
        drawCenteredMenuValueScreen(L(MENU_SOUND),
                                    g_sys_sound_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_sys_sound_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_sound_value = g_sys_sound_enabled ? 1 : 0;
    } else if (g_sys_menu_state == SYS_MODE_EDIT_ALARM_SOUND) {
        drawCenteredMenuValueScreen(L(MENU_ALARM_SOUND),
                                    g_sys_alarm_sound_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_sys_alarm_sound_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_alarm_sound_value = g_sys_alarm_sound_enabled ? 1 : 0;
    } else if (g_sys_menu_state == SYS_MODE_EDIT_SLEEP) {
        drawCenteredMenuValueScreen(L(MENU_SLEEP),
                                    get_sleep_option_name(get_sleep_option_index(g_sys_sleep_ms)),
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_sleep_index = get_sleep_option_index(g_sys_sleep_ms);
    } else if (g_sys_menu_state == SYS_MODE_EDIT_LANG) {
        drawCenteredMenuValueScreen(L(MENU_TITLE),
                                    get_language_name(g_sys_lang_index),
                                    TFT_WHITE,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_lang_index = (int)g_sys_lang_index;
    } else if (g_sys_menu_state == SYS_MODE_CONFIRM_RESET) {
        // Reset confirmation uses the shared binary prompt helper.
        drawResetChoicePrompt(L(MENU_FULL_RESET),
                              L(MENU_RESTORE_ALL),
                              L(MENU_TO_DEFAULTS),
                              L(MENU_NO),
                              L(MENU_YES),
                              g_sys_reset_choice,
                              L(ST_TURN_PUSH));
        last_reset_choice = (int)g_sys_reset_choice;
    } else if (g_sys_menu_state == SYS_MODE_SAVED) {
        if (g_sys_saved_kind == 0) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        g_sys_sound_enabled ? L(ST_ON) : L(ST_OFF),
                                        g_sys_sound_enabled ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_sys_saved_kind == 4) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        g_sys_alarm_sound_enabled ? L(ST_ON) : L(ST_OFF),
                                        g_sys_alarm_sound_enabled ? TFT_GREEN : TFT_RED,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_sys_saved_kind == 1) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        get_sleep_option_name(get_sleep_option_index(g_sys_sleep_ms)),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else if (g_sys_saved_kind == 2) {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        get_language_name(g_sys_lang_index),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        } else {
            drawCenteredMenuSavedScreen(L(MENU_SAVED),
                                        L(MENU_RESET_DONE),
                                        TFT_WHITE,
                                        MENU_VALUE_FONT_BODY,
                                        L(ST_PUSH_MENU));
        }
        last_saved_kind = (int)g_sys_saved_kind;
    }

    last_drawn_state = g_sys_menu_state;
}

void draw_system_screen(bool screen_changed, bool data_changed) {
    (void)data_changed;

    if (system_menu_is_active()) {
        draw_system_menu_screen(screen_changed);
        return;
    }

    constexpr int card_x = LC_SCREEN_X;
    constexpr int card_y = LC_CARD_TOP;
    constexpr int card_w = LC_SCREEN_W;
    constexpr int card_h = LC_SCREEN_BOTTOM - LC_CARD_TOP + 1;
    constexpr int panel_x0 = 8;
    constexpr int panel_x1 = 82;
    constexpr int panel_w = 70;
    constexpr int device_y = 31;
    constexpr int mid_y = 52;
    constexpr int bottom_y = 89;
    constexpr int bottom_h = 34;
    const uint16_t card_bg = tft.color565(4, 8, 18);

    static bool s_ble_enabled = false; // cached per screen_changed — doesn't change at runtime
    if (screen_changed) {
        s_ble_enabled = load_ble_enabled_store();
        tft.fillScreen(TFT_BLACK);
        draw_system_header(L(TIT_SYS));
        tft.fillRoundRect(card_x, card_y, card_w, card_h, LC_CARD_RADIUS, card_bg);
        drawCard(card_x, card_y, card_w, card_h, PB_HUM_P1);
    }

    uint32_t uptime_s = millis() / 1000;
    bool ble_connected = s_ble_enabled ? client_connected.load() : false;
    static uint32_t last_uptime_s = UINT32_MAX;
    static bool last_ble_connected = false;
    static bool last_sound_enabled = false;
    static bool last_alarm_sound_enabled = false;

    if (!screen_changed
        && ble_connected == last_ble_connected
        && uptime_s == last_uptime_s
        && g_sound_enabled == last_sound_enabled
        && g_alarm_sound_enabled == last_alarm_sound_enabled) {
        return;
    }

    if (screen_changed || ble_connected != last_ble_connected) {
        draw_system_id_panel(panel_x0, device_y, 144, 19,
                             L(SYS_DEV_LABEL), dev_name,
                             s_ble_enabled, ble_connected);
    }

    if (screen_changed || uptime_s != last_uptime_s) {
        char uptimeStr[12];
        snprintf(uptimeStr, sizeof(uptimeStr), "%02u:%02u:%02u", uptime_s / 3600, (uptime_s % 3600) / 60, uptime_s % 60);
        draw_system_text_panel(panel_x0, mid_y, panel_w, 35,
                               L(SYS_UP_LABEL), uptimeStr,
                               PB_HUM_P3, PB_DS18_P2, PB_DS18_P2, -5, 1);
        draw_system_text_panel(panel_x1, mid_y, panel_w, 35,
                               L(SYS_LANG_LABEL), get_language_name((uint8_t)normalizeLanguage(g_language)),
                               PB_HUM_P3, TFT_WHITE, PB_LUZ_P2, -5, 1);
    }

    if (screen_changed || g_sound_enabled != last_sound_enabled || g_alarm_sound_enabled != last_alarm_sound_enabled) {
        draw_system_audio_panel(panel_x0, bottom_y, 144, bottom_h);
    }

    last_ble_connected = ble_connected;
    last_uptime_s = uptime_s;
    last_sound_enabled = g_sound_enabled;
    last_alarm_sound_enabled = g_alarm_sound_enabled;
}
