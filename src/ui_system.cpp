#include "ui_system.h"
#include "ui_widgets.h"
#include "hw.h"
#include "ble.h"
#include "languages.h"
#include "fonts.h"
#include "layout.h"
#include "lang_select.h"
#include "runtime_events.h"
#include <stdio.h>

static SysMenuState g_sys_menu_state = SYS_MODE_NORMAL;
static uint8_t g_sys_menu_index = 0;
static uint32_t g_sys_sleep_ms = 0;
static uint8_t g_sys_lang_index = 0;
static bool g_sys_sound_enabled = true;
static uint8_t g_sys_reset_choice = 0;
static uint8_t g_sys_saved_kind = 0;

extern bool g_sound_enabled;
extern bool g_is_fahrenheit;
extern char dev_name[];
extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

static const char* const LANG_CODES[] = { "ESP", "CAT", "ENG" };
constexpr size_t LANG_CODES_COUNT = sizeof(LANG_CODES) / sizeof(LANG_CODES[0]);

// Sleep presets are stored as raw milliseconds so the menu can stay simple.
const uint32_t SLEEP_OPTIONS[] = { 30000, 60000, 120000, 300000, 600000, 0 };
const int NUM_SLEEP_OPTIONS = sizeof(SLEEP_OPTIONS) / sizeof(SLEEP_OPTIONS[0]);

static void draw_system_header(const char* title) {
    drawHeader(title);
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
    g_sys_lang_index = (uint8_t)g_language;
    g_sys_sound_enabled = g_sound_enabled;
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
        case SYS_MODE_MENU: return 4;
        case SYS_MODE_EDIT_SOUND: return 1;
        case SYS_MODE_EDIT_SLEEP: return NUM_SLEEP_OPTIONS - 1;
        case SYS_MODE_EDIT_LANG: return (int)LANG_CODES_COUNT - 1;
        case SYS_MODE_CONFIRM_RESET: return 1;
        case SYS_MODE_SAVED: return 0;
        default: return 0;
    }
}

int get_system_encoder_value() {
    switch (g_sys_menu_state) {
        case SYS_MODE_MENU: return g_sys_menu_index;
        case SYS_MODE_EDIT_SOUND: return g_sys_sound_enabled ? 1 : 0;
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
                g_sys_menu_state = SYS_MODE_EDIT_SLEEP;
            } else if (g_sys_menu_index == 2) {
                g_sys_menu_state = SYS_MODE_EDIT_LANG;
            } else if (g_sys_menu_index == 3) {
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
        case SYS_MODE_EDIT_SLEEP:
            save_sleep_timeout(g_sys_sleep_ms);
            g_sys_saved_kind = 1;
            g_sys_menu_state = SYS_MODE_SAVED;
            break;
        case SYS_MODE_EDIT_LANG:
            saveLanguage((Language)g_sys_lang_index);
            g_sys_saved_kind = 2;
            g_sys_menu_state = SYS_MODE_SAVED;
            break;
        case SYS_MODE_CONFIRM_RESET:
            if (g_sys_reset_choice == 1) {
                reset_all_settings();
                g_language = LANG_ES;
                g_is_fahrenheit = false;
                g_sys_sound_enabled = g_sound_enabled;
                g_sys_sleep_ms = get_sleep_timeout();
                g_sys_lang_index = (uint8_t)g_language;
                g_sys_reset_choice = 0;
                g_sys_saved_kind = 3;
                g_sys_menu_state = SYS_MODE_SAVED;
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
        last_sleep_index = -1;
        last_lang_index = -1;
        last_reset_choice = -1;
        last_saved_kind = -1;
    }

    tft.setTextDatum(MC_DATUM);

    if (g_sys_menu_state == SYS_MODE_MENU) {
        // Root menu now uses the shared centered-list helper.
        const char* items[] = {
            L(MENU_SOUND),
            L(MENU_SLEEP),
            L(MENU_TITLE),
            L(MENU_RESET),
            L(MENU_EXIT)
        };
        drawCenteredMenuList(items, 5, g_sys_menu_index, LM_MENU5_Y0, LM_MENU5_GAP);
        drawFooterHint(L(INSTR_SEL), cx, LM_MENU_FOOTER_Y);
        last_menu_index = (int)g_sys_menu_index;
    } else if (g_sys_menu_state == SYS_MODE_EDIT_SOUND) {
        drawCenteredMenuValueScreen(L(MENU_SOUND),
                                    g_sys_sound_enabled ? L(ST_ON) : L(ST_OFF),
                                    g_sys_sound_enabled ? TFT_GREEN : TFT_RED,
                                    MENU_VALUE_FONT_BODY,
                                    L(ST_TURN_PUSH));
        last_sound_value = g_sys_sound_enabled ? 1 : 0;
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

    const int x_draw  = LS_LABEL_X;
    const int x_val   = LS_VALUE_X;
    const int y_dev   = LS_ROW_DEV;
    const int y_up    = LS_ROW_UP;
    const int y_ble   = LS_ROW_BLE;
    const int y_lang  = LS_ROW_LAN;
    const int cx      = tft.width() / 2;

    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        draw_system_header(L(TIT_SYS));
        drawCard(LS_CARD_X, LS_CARD_Y, LS_CARD_W, LS_CARD_H, TFT_DARKGREY);
        tft.setTextDatum(TL_DATUM);
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        tft.drawString(L(SYS_DEV_LABEL), x_draw, y_dev);
        tft.drawString(L(SYS_UP_LABEL), x_draw, y_up);
        tft.drawString(L(SYS_BLE_LABEL), x_draw, y_ble);
        tft.drawString(L(SYS_LANG_LABEL), x_draw, y_lang);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(dev_name, x_val, y_dev);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(LANG_CODES[(uint8_t)g_language], x_val, y_lang);
        tft.setTextFont(0);
    }

    uint32_t uptime_s = millis() / 1000;
    bool ble_connected = client_connected.load();
    static uint32_t last_uptime_s = UINT32_MAX;
    static bool last_ble_connected = false;
    static bool last_sound_enabled = true;

    if (!screen_changed
        && ble_connected == last_ble_connected
        && uptime_s == last_uptime_s
        && g_sound_enabled == last_sound_enabled) {
        return;
    }

    if (screen_changed || uptime_s != last_uptime_s) {
        char uptimeStr[12];
        snprintf(uptimeStr, sizeof(uptimeStr), "%02u:%02u:%02u", uptime_s / 3600, (uptime_s % 3600) / 60, uptime_s % 60);
        const int uptime_clear_w = (LS_CARD_X + LS_CARD_W - 2) - x_val + 1;
        tft.fillRect(x_val, y_up - 1, uptime_clear_w, 12, TFT_BLACK);
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.drawString(uptimeStr, x_val, y_up);
        tft.setTextDatum(TL_DATUM);
    }

    if (screen_changed || ble_connected != last_ble_connected) {
        tft.fillRect(x_val, y_ble - 1, tft.width() - x_val - 11, 12, TFT_BLACK);
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(ble_connected ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString(ble_connected ? L(ST_CONNECTED) : L(ST_DISCONN), x_val, y_ble);
    }

    if (screen_changed || g_sound_enabled != last_sound_enabled) {
        // Footer status is always kept in sync with the persisted sound flag.
        tft.fillRect(0, LS_FOOTER_Y - 10, tft.width(), 16, TFT_BLACK);
        drawFooterHint(g_sound_enabled ? L(ST_SND_ON) : L(ST_SND_OFF),
                       cx,
                       LS_FOOTER_Y,
                       g_sound_enabled ? TFT_GREEN : TFT_RED);
    }

    last_ble_connected = ble_connected;
    last_uptime_s = uptime_s;
    last_sound_enabled = g_sound_enabled;
}
