// ui_system.cpp
// Dibuja la pantalla de Información del Sistema.

#include "ui_system.h"
#include "ui_widgets.h" // Para tft, drawHeader, drawCard
#include "hw.h"         // Para dev_name
#include "ble.h"        // Para client_connected
#include "languages.h"  // Para L() y g_language
#include "fonts.h"      // GFXfont Inter (Latin-1: á é í ó ú ñ à è ç...)
#include <stdio.h>      // Para snprintf

// DECLARACIONES EXTERNAS
extern bool client_connected;
extern bool g_sound_enabled;
extern char dev_name[];
extern TFT_eSPI tft;

// Códigos de idioma para mostrar en la fila LAN
static const char* const LANG_CODES[] = { "ESP", "CAT", "ENG" };

void draw_system_screen(bool screen_changed, bool data_changed) {

    const int x_draw  = 20;
    const int x_val   = x_draw + 30; // x de los valores (alineado en las cuatro filas)
    const int y_dev   = 43;  // fila DEVICE
    const int y_up    = 58;  // fila UPTIME
    const int y_ble   = 73;  // fila BLE
    const int y_lang  = 88;  // fila LANG (nueva)

    // 1. Estático — solo cuando cambia la pantalla
    if (screen_changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_SYS), TFT_GREEN);
        drawCard(10, 38, tft.width() - 20, 67, TFT_DARKGREY); // y=38..105

        tft.setTextDatum(TL_DATUM);

        // Etiquetas estáticas de las cuatro filas
        // OLD (sin Latin-1): tft.drawString("DEV", x_draw, y_dev, 2); ...
        tft.setFreeFont(FONT_BODY);
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
    if (!screen_changed
        && client_connected == last_ble_connected
        && g_sound_enabled  == last_sound_enabled
        && uptime_s         == last_uptime_s) return;
    last_ble_connected = client_connected;
    last_sound_enabled = g_sound_enabled;
    last_uptime_s      = uptime_s;

    // 2. Dinámico — zona de valores (clear mínimo por fila)
    tft.setTextDatum(TL_DATUM);
    const int clear_w = tft.width() - x_val - 11; // hasta el borde interior del card

    // --- UPTIME ---
    char uptimeStr[12];
    uint32_t h = uptime_s / 3600;
    uint32_t m = (uptime_s % 3600) / 60;
    uint32_t s = uptime_s % 60;
    snprintf(uptimeStr, sizeof(uptimeStr), "%02u:%02u:%02u", h, m, s);
    // OLD (sin Latin-1): tft.drawString(uptimeStr, x_val, y_up, 2);
    tft.fillRect(x_val, y_up, clear_w, 16, TFT_BLACK);
    tft.setFreeFont(FONT_BODY);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(uptimeStr, x_val, y_up);

    // --- BLE STATUS ---
    // OLD (sin Latin-1): tft.drawString(statusText, x_val, y_ble, 2);
    tft.fillRect(x_val, y_ble, clear_w, 16, TFT_BLACK);
    uint16_t statusColor = client_connected ? TFT_GREEN : TFT_RED;
    const char* statusText = client_connected ? L(ST_CONNECTED) : L(ST_DISCONN);
    tft.setTextColor(statusColor, TFT_BLACK);
    tft.drawString(statusText, x_val, y_ble);
    tft.setTextFont(0); // liberar GFXfont

    // --- PIE DE PÁGINA (Mute) ---
    int cx = tft.width() / 2;
    int footer_y = tft.height() - 10;
    tft.fillRect(0, footer_y, tft.width(), 16, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    uint16_t soundColor = g_sound_enabled ? TFT_GREEN : TFT_RED;
    const char* soundText = g_sound_enabled ? L(ST_SND_ON) : L(ST_SND_OFF);
    // OLD (sin Latin-1): tft.drawString(soundText, cx, footer_y, 1);
    tft.setFreeFont(FONT_SMALL);
    tft.setTextColor(soundColor, TFT_BLACK);
    tft.drawString(soundText, cx, footer_y);
    tft.setTextFont(0); // liberar GFXfont
}
