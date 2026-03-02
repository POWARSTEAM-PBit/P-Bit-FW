// ui_system.cpp
// Dibuja la pantalla de Información del Sistema.

#include "ui_system.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard
#include "hw.h"         // Para dev_name
#include "ble.h"        // Para client_connected
#include "languages.h"  // Para L() y g_language
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include "layout.h"
#include <stdio.h>      // Para snprintf

// DECLARACIONES EXTERNAS
extern bool client_connected;
extern bool g_sound_enabled;
extern char dev_name[];
extern TFT_eSPI tft;
extern Reading g_ui_readings_snapshot;

// Códigos de idioma para mostrar en la fila LAN
static const char* const LANG_CODES[] = { "ESP", "CAT", "ENG" };

void draw_system_screen(bool screen_changed, bool data_changed) {

    const int x_draw  = LS_LABEL_X;
    const int x_val   = LS_VALUE_X;
    const int y_dev   = LS_ROW_DEV;
    const int y_up    = LS_ROW_UP;
    const int y_ble   = LS_ROW_BLE;
    const int y_lang  = LS_ROW_LAN;

    // 1. Estático — solo cuando cambia la pantalla
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_SYS), TFT_GREEN);
        drawCard(LS_CARD_X, LS_CARD_Y, LS_CARD_W, LS_CARD_H, TFT_DARKGREY);

        tft.setTextDatum(TL_DATUM);

        // Etiquetas estáticas de las cuatro filas
        // OLD (sin Latin-1): tft.drawString("DEV", x_draw, y_dev, 2); ...
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
        tft.drawString("DEV",  x_draw, y_dev);
        tft.drawString("UP",   x_draw, y_up);
        tft.drawString("BLE",  x_draw, y_ble);
        tft.drawString("LAN",  x_draw, y_lang);

        // DEVICE nombre (siempre estático)
        // OLD (sin Latin-1): tft.drawString(dev_name, x_val, y_dev, 2);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString(dev_name, x_val, y_dev);

        // LANG — estático (no cambia sin reinicio)
        // OLD (sin Latin-1): tft.drawString(LANG_CODES[...], x_val, y_lang, 2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(LANG_CODES[(uint8_t)g_language], x_val, y_lang);
        tft.setTextFont(0); // liberar GFXfont
    }

    // Salida temprana si ningún dato dinámico cambió
    uint32_t uptime_s = millis() / 1000;
    static uint32_t last_uptime_s      = UINT32_MAX;
    static bool     last_ble_connected = false;
    static bool     last_sound_enabled = true;
    bool uptime_changed = screen_changed || (uptime_s != last_uptime_s);
    bool ble_changed = screen_changed || (client_connected != last_ble_connected);
    bool sound_changed = screen_changed || (g_sound_enabled != last_sound_enabled);
    if (!screen_changed
        && client_connected == last_ble_connected
        && g_sound_enabled  == last_sound_enabled
        && uptime_s         == last_uptime_s) return;

    // 2. Dinámico — zona de valores (clear mínimo por fila)
    tft.setTextDatum(TL_DATUM);
    const int clear_w = tft.width() - x_val - 11; // hasta el borde interior del card

    // --- UPTIME ---
    if (uptime_changed) {
        char uptimeStr[12];
        uint32_t h = uptime_s / 3600;
        uint32_t m = (uptime_s % 3600) / 60;
        uint32_t s = uptime_s % 60;
        snprintf(uptimeStr, sizeof(uptimeStr), "%02u:%02u:%02u", h, m, s);
        tft.fillRect(x_val, y_up, clear_w, 16, TFT_BLACK);
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.drawString(uptimeStr, x_val, y_up);
    }

    // --- BLE STATUS ---
    if (ble_changed) {
        tft.fillRect(x_val, y_ble, clear_w, 16, TFT_BLACK);
        uint16_t statusColor = client_connected ? TFT_GREEN : TFT_RED;
        const char* statusText = client_connected ? L(ST_CONNECTED) : L(ST_DISCONN);
        tft.setFreeFont(FONT_INFO);
        tft.setTextColor(statusColor, TFT_BLACK);
        tft.drawString(statusText, x_val, y_ble);
        tft.setTextFont(0); // liberar GFXfont
    }

    // --- PIE DE PÁGINA (Mute) ---
    if (sound_changed) {
        int cx = tft.width() / 2;
        int footer_y = LS_FOOTER_Y;
        tft.fillRect(0, footer_y, tft.width(), 16, TFT_BLACK);
        tft.setTextDatum(TC_DATUM);
        uint16_t soundColor = g_sound_enabled ? TFT_GREEN : TFT_RED;
        const char* soundText = g_sound_enabled ? L(ST_SND_ON) : L(ST_SND_OFF);
        tft.setFreeFont(FONT_SMALL);
        tft.setTextColor(soundColor, TFT_BLACK);
        tft.drawString(soundText, cx, footer_y);
        tft.setTextFont(0); // liberar GFXfont
    }

    last_ble_connected = client_connected;
    last_sound_enabled = g_sound_enabled;
    last_uptime_s      = uptime_s;
}
