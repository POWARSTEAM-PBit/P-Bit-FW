#include "ui_ble_toggle.h"
#include "settings_store.h"
#include "ui_icons.h"
#include "fonts.h"
#include "languages.h"
#include "runtime_events.h"
#include <TFT_eSPI.h>
#include <esp_system.h>
#include <stdio.h>

extern TFT_eSPI tft;

// Vivid cobalt blue — full-bleed background for the BLE secret screen.
static constexpr uint16_t BLE_BG = 0x021F; // color565(0, 64, 255)

static int g_ble_selection = 0; // 0 = OFF, 1 = ON

void init_ble_toggle_screen() {
    g_ble_selection = load_ble_enabled_store() ? 1 : 0;
}

int  get_ble_toggle_encoder_min()   { return 0; }
int  get_ble_toggle_encoder_max()   { return 1; }
int  get_ble_toggle_encoder_value() { return g_ble_selection; }

void set_ble_toggle_input_value(int value) {
    const int next = constrain(value, 0, 1);
    if (next != g_ble_selection) {
        g_ble_selection = next;
        runtime_request_ui_refresh(false);
    }
}

static void draw_selector(bool full) {
    if (full) {
        tft.fillRect(0, 88, 160, 38, BLE_BG);
    }

    tft.setFreeFont(FONT_BODY);
    tft.setTextDatum(MC_DATUM);

    // OFF button (left half)
    const uint16_t off_bg = (g_ble_selection == 0) ? TFT_WHITE : BLE_BG;
    const uint16_t off_fg = (g_ble_selection == 0) ? BLE_BG    : TFT_DARKGREY;
    tft.fillRoundRect(10, 94, 62, 24, 4, off_bg);
    if (g_ble_selection != 0) tft.drawRoundRect(10, 94, 62, 24, 4, TFT_DARKGREY);
    tft.setTextColor(off_fg, off_bg);
    tft.drawString(L(ST_OFF), 41, 104);

    // ON button (right half)
    const uint16_t on_bg = (g_ble_selection == 1) ? TFT_WHITE : BLE_BG;
    const uint16_t on_fg = (g_ble_selection == 1) ? BLE_BG    : TFT_DARKGREY;
    tft.fillRoundRect(88, 94, 62, 24, 4, on_bg);
    if (g_ble_selection != 1) tft.drawRoundRect(88, 94, 62, 24, 4, TFT_DARKGREY);
    tft.setTextColor(on_fg, on_bg);
    tft.drawString(L(ST_ON), 119, 104);

    tft.setTextFont(0);
}

void draw_ble_toggle_screen(bool screen_changed, bool data_changed) {
    static int last_selection = -1;

    if (!screen_changed && !data_changed && g_ble_selection == last_selection) return;

    if (screen_changed) {
        tft.fillScreen(BLE_BG);
        // Large Bluetooth icon, white, centered in upper portion
        pbit_draw_bluetooth_icon_xl(80, 42, TFT_WHITE);
        // Label
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont(FONT_BODY);
        tft.setTextColor(TFT_WHITE, BLE_BG);
        tft.drawString(L(SYS_BLE_LABEL), 80, 74);
        tft.setTextFont(0);
        draw_selector(true);
    } else if (g_ble_selection != last_selection) {
        draw_selector(false);
    }

    last_selection = g_ble_selection;
}

uint8_t handle_ble_toggle_button() {
    save_ble_enabled_store(g_ble_selection == 1);
    // Full-screen restart overlay before rebooting
    tft.fillScreen(BLE_BG);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_WHITE, BLE_BG);
    char status[16];
    snprintf(status, sizeof(status), "%s %s", L(SYS_BLE_LABEL), g_ble_selection == 1 ? L(ST_ON) : L(ST_OFF));
    tft.drawString(status, 80, 55);
    tft.drawString(L(ST_RESTARTING), 80, 78);
    tft.setTextFont(0);
    delay(700);
    esp_restart();
    return 0; // unreachable
}
