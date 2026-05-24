#pragma once
// P-Bit layout system for a 160x128 landscape display (rotation = 1).
// The constants below define the shared safe areas used by every screen.

// ── Global ───────────────────────────────────────────────
constexpr int L_HEADER_Y    = 18;   // Main title baseline for every screen header.
constexpr int L_HEADER_LINE = 23;   // Divider line below the title.
constexpr int L_CONTENT_TOP = 27;   // First usable Y coordinate below the header.
constexpr int L_MARGIN_SIDE = 10;   // Shared side margin for centered horizontal elements.
constexpr int L_ALERT_JEWEL_X = 14;  // Safe anchor for the runtime alert indicator.
constexpr int L_ALERT_JEWEL_Y = 118; // Y position for the runtime alert indicator.

// Shared spacing baseline used by the main UI family.
// These are the reference limits we try to keep consistent across cards:
// - title top offset: `L_HEADER_Y`
// - divider Y: `L_HEADER_LINE`
// - divider side margins: `L_MARGIN_SIDE`
// - content start: `L_CONTENT_TOP`
// - divider-to-card gap: 3 px
// - standard card gap: 4 px
// - compact cards may consume the lower band when no footer hint is present

// ── Menus / shared layout bands ───────────────────────────
constexpr int LM_MENU_TITLE_BAND_Y = 26;  // Shared top band for titles and subtitles (starts 3 px below header line).
constexpr int LM_MENU_TITLE_BAND_H = 34;  // Height of the shared title band (covers 26..59).
constexpr int LM_MENU_BODY_BAND_Y = 60;   // Shared body band for editable content or centered lists.
constexpr int LM_MENU_BODY_BAND_H = 48;   // Height of the shared body band.
constexpr int LM_MENU_FOOTER_Y = 118;     // Shared footer line for hints such as "Turn and push".
constexpr int LM_MENU4_Y0 = 36;           // First row used by 4-item menus (center of first item).
constexpr int LM_MENU4_GAP = 16;          // Vertical spacing between rows in 4-item menus.
constexpr int LM_MENU3_Y0 = 36;           // First row used by 3-item menus.
constexpr int LM_MENU3_GAP = 22;           // Vertical spacing between rows in 3-item menus.
constexpr int LM_MENU5_Y0 = 36;           // First row used by 5-item menus.
constexpr int LM_MENU5_GAP = 14;           // Vertical spacing between rows in 5-item menus.
constexpr int LM_SUMMARY2_Y0 = 70;        // First row used by 2-line summaries.
constexpr int LM_SUMMARY2_GAP = 18;       // Vertical spacing between rows in 2-line summaries.
constexpr int LM_SUMMARY3_Y0 = 68;        // First row used by 3-line summaries.
constexpr int LM_SUMMARY3_GAP = 16;       // Vertical spacing between rows in 3-line summaries.

// ── Family A: vertical tank screens (Temp, Humidity, Soil, DS18) ─
constexpr int LA_LEFT_CX    = 60;   // Horizontal center of the left text panel.
constexpr int LA_HINT_Y     = 34;   // Small helper text baseline.
constexpr int LA_VALUE_TOP  = 50;   // Top Y coordinate of the large value.
constexpr int LA_UNIT_Y     = 92;   // Legacy reference for secondary text or units.
constexpr int LA_CATEGORY_Y = 108;  // Lower label baseline for category or status text.
constexpr int LA_TANK_X     = 120;  // Left X coordinate of the vertical tank graphic.
constexpr int LA_TANK_Y     = 36;   // Top Y coordinate of the vertical tank graphic.
constexpr int LA_TANK_W     = 28;   // Width of the vertical tank graphic.
constexpr int LA_TANK_H     = 88;   // Height of the vertical tank graphic.

// ── Family B: horizontal bar screens (Sound, Light) ───────────
constexpr int LB_VALUE_TOP  = 38;   // Top Y coordinate for the large numeric value.
constexpr int LB_BAR_X      = 20;   // Left X coordinate of the horizontal bar.
constexpr int LB_BAR_Y      = 91;   // Top Y coordinate of the horizontal bar.
constexpr int LB_BAR_W      = 120;  // Width of the horizontal bar.
constexpr int LB_BAR_H      = 14;   // Height of the horizontal bar.
constexpr int LB_CATEGORY_Y = 114;  // Category label baseline below the bar.

// ── Timer ────────────────────────────────────────────────
constexpr int LT_HINT_Y     = 101; // Instruction top inside the timer card.
constexpr int LT_HINT_CLEAR_Y = 97;
constexpr int LT_HINT_CLEAR_H = 22;
constexpr int LT_CARD_X     = 2;
constexpr int LT_CARD_Y     = 27;
constexpr int LT_CARD_W     = 156;
constexpr int LT_CARD_H     = 100;
constexpr int LT_TIMER_MENU_TITLE_Y = LT_CARD_Y + 7;
constexpr int LT_TIMER_MENU_VALUE_Y = LT_CARD_Y + 33;
constexpr int LT_TIMER_MENU_HINT_Y1 = 95;
constexpr int LT_TIMER_MENU_HINT_Y2 = 108;
constexpr int LT_STATUS_TEXT_Y = LT_CARD_Y + 8;
constexpr int LT_DIGIT_CARD_X = LT_CARD_X + 13;
constexpr int LT_DIGIT_CARD_Y = LT_CARD_Y + 29;
constexpr int LT_DIGIT_CARD_W = LT_CARD_W - 26;
constexpr int LT_DIGIT_CARD_H = 38;
constexpr int LT_TIMER_MENU_DIGIT_CARD_X = LT_DIGIT_CARD_X + 2;
constexpr int LT_TIMER_MENU_DIGIT_CARD_Y = LT_DIGIT_CARD_Y - 1;
constexpr int LT_TIMER_MENU_DIGIT_CARD_W = LT_DIGIT_CARD_W - 4;
constexpr int LT_TIMER_MENU_DIGIT_CARD_H = LT_DIGIT_CARD_H;
constexpr int LT_TIME_SPRITE_X = LT_DIGIT_CARD_X + 2;
constexpr int LT_TIME_SPRITE_Y = LT_DIGIT_CARD_Y + 2;
constexpr int LT_TIME_SPRITE_W = LT_DIGIT_CARD_W - 4;
constexpr int LT_TIME_SPRITE_H = 28;

// ── Graph screen ─────────────────────────────────────────
// The graph area has a 1-px border; LG_GRAPH_W/H refer to the interior (sprite size).
// Outer border: drawRect(LG_GRAPH_X, LG_GRAPH_Y, LG_GRAPH_W+2, LG_GRAPH_H+2, color)
// Sprite push : (LG_GRAPH_X+1, LG_GRAPH_Y+1)
constexpr int LG_SENSOR_Y  = 27;   // Sensor name/value band top Y (between header and graph).
constexpr int LG_GRAPH_X   = 2;    // Left edge of the outer border rect.
constexpr int LG_GRAPH_Y   = 46;   // Top edge of the outer border rect.
constexpr int LG_GRAPH_W   = 154;  // Interior width  (== sprite width,  samples visible).
constexpr int LG_GRAPH_H   = 78;   // Interior height (== sprite height).
constexpr int LG_HINT_Y    = 120;  // Footer hint baseline.

// ── System Info ──────────────────────────────────────────
constexpr int LS_CARD_X     = 10;   // Left X coordinate of the system info card.
constexpr int LS_CARD_Y     = 36;   // Top Y coordinate of the system info card.
constexpr int LS_CARD_W     = 140;  // Width of the system info card.
constexpr int LS_CARD_H     = 68;   // Height of the system info card.
constexpr int LS_LABEL_X    = 20;   // X position for the fixed left-side labels.
constexpr int LS_VALUE_X    = 52;   // X position for the dynamic right-side values.
constexpr int LS_ROW_DEV    = 40;   // DEV row baseline.
constexpr int LS_ROW_UP     = 54;   // UP row baseline.
constexpr int LS_ROW_BLE    = 82;   // BLE row baseline (last — hidden when disabled, no gap).
constexpr int LS_ROW_LAN    = 68;   // Language row baseline.
constexpr int LS_FOOTER_Y   = 116;  // Footer baseline below the card.

// ── Card layout system ─────────────────────────────────────
// Agreed baseline for all card-based screens.
constexpr int LC_CARD_TOP        = 27;  // first outer border Y for lab cards (3 px below header line)
constexpr int LC_CARD_TOP_AIRY   = 31;  // content start for non-card interior blocks
constexpr int LC_MARGIN_DENSE    = 2;   // lateral margin for full-width lab cards
constexpr int LC_MARGIN_SPACIOUS = 2;   // lateral margin for full-width lab cards
constexpr int LC_GAP             = 4;   // standard card/block gap (horizontal and vertical)
constexpr int LC_FOOTER_Y        = 120; // footer hint anchor, universal across card screens
constexpr int LC_SCREEN_X        = 2;   // common external left border for lab cards.
constexpr int LC_SCREEN_W        = 156; // common external width for lab cards.
constexpr int LC_SCREEN_BOTTOM   = 126; // common lower visual limit for cards without footer.
constexpr int LC_CARD_RADIUS     = 4;   // common corner radius for card shells.

// ── Validated card master rule ──────────────────────────────
// Confirmed on real 160x128 TFT hardware on 2026-05-15.
// Use this geometry as the reference for future card-based screens.
constexpr int LC_MASTER_HEADER_BASELINE = L_HEADER_Y;
constexpr int LC_MASTER_HEADER_TEXT_TOP = 0;
constexpr int LC_MASTER_HEADER_LINE_Y   = L_HEADER_LINE;
constexpr int LC_MASTER_HEADER_LINE_X   = 4;
constexpr int LC_MASTER_HEADER_LINE_W   = 152;
constexpr int LC_MASTER_CARD_X0         = LC_SCREEN_X;
constexpr int LC_MASTER_CARD_Y0         = LC_CARD_TOP;
constexpr int LC_MASTER_CARD_W          = 76;
constexpr int LC_MASTER_CARD_H          = 48;
constexpr int LC_MASTER_CARD_GAP        = 4;
constexpr int LC_MASTER_CARD_RADIUS     = LC_CARD_RADIUS;
constexpr int LC_MASTER_CARD_X1         = LC_MASTER_CARD_X0 + LC_MASTER_CARD_W + LC_MASTER_CARD_GAP;
constexpr int LC_MASTER_CARD_Y1         = LC_MASTER_CARD_Y0 + LC_MASTER_CARD_H + LC_MASTER_CARD_GAP;
constexpr int LC_MASTER_CARD_CX0        = LC_MASTER_CARD_X0 + (LC_MASTER_CARD_W / 2);
constexpr int LC_MASTER_CARD_CX1        = LC_MASTER_CARD_X1 + (LC_MASTER_CARD_W / 2);
constexpr int LC_MASTER_CARD_CY0        = LC_MASTER_CARD_Y0 + (LC_MASTER_CARD_H / 2);
constexpr int LC_MASTER_CARD_CY1        = LC_MASTER_CARD_Y1 + (LC_MASTER_CARD_H / 2);
constexpr int LC_MASTER_CARD_BOTTOM     = LC_MASTER_CARD_Y1 + LC_MASTER_CARD_H - 1;
constexpr int LC_MASTER_FOOTER_BASELINE = 126;
constexpr int LC_MASTER_FOOTER_TEXT_TOP = 118;
