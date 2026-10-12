#include "ui_plant_settings.h"

#include "fonts.h"
#include "hw.h"
#include "languages.h"
#include "layout.h"
#include "palette.h"
#include "runtime_events.h"
#include "ui_widgets.h"
#include <Arduino.h>
#include <math.h>
#include <stdio.h>

extern bool g_is_fahrenheit;

namespace {

constexpr uint8_t PLANT_MENU_PRIMARY_COUNT = 2;
constexpr uint8_t PLANT_MENU_RESET_INDEX = PLANT_MENU_PRIMARY_COUNT;
constexpr uint8_t PLANT_MENU_EXIT_INDEX = PLANT_MENU_PRIMARY_COUNT + 1;
constexpr int CHOICE_Y = 94;
constexpr int CHOICE_W = 70;
constexpr int CHOICE_H = 19;
constexpr uint16_t kPanelBg = 0x0841;

// Encoder writes, display reads. Never hold this lock across NVS or TFT I/O.
portMUX_TYPE g_settings_mux = portMUX_INITIALIZER_UNLOCKED;
PlantSettingsData g_settings{};
uint32_t g_revision = 0;
bool g_loaded = false;
bool g_last_soil_review = false;

PlantSettingsMenuState g_state = PLANT_SETTINGS_NORMAL;
PlantSettingsData g_draft{};
uint8_t g_menu_index = 0;
uint8_t g_edit_field = 0;
uint8_t g_choice = 0;
bool g_draft_personalized = false;
bool g_edit_fahrenheit = false;

bool calibration_matches(const PlantSettingsData& settings, const SoilCalibrationData& calibration) {
    return settings.soil_dry_raw == calibration.dry_raw
        && settings.soil_wet_raw == calibration.wet_raw;
}

void publish_settings(const PlantSettingsData& settings) {
    const SoilCalibrationData calibration = get_current_soil_calibration();
    portENTER_CRITICAL(&g_settings_mux);
    g_settings = settings;
    g_last_soil_review = settings.mode == PlantProfileMode::Custom
        && !calibration_matches(settings, calibration);
    ++g_revision;
    g_loaded = true;
    portEXIT_CRITICAL(&g_settings_mux);
}

void change_state(PlantSettingsMenuState next) {
    g_state = next;
    runtime_request_ui_refresh(true);
}

void reset_draft() {
    g_draft = get_plant_settings_snapshot().settings;
    g_edit_field = 0;
    g_choice = 0;
    g_draft_personalized = false;
    g_edit_fahrenheit = g_is_fahrenheit;
}

uint8_t edit_channel() { return g_edit_field < 6 ? g_edit_field / 2 : 2; }
bool edit_high() { return (g_edit_field & 1u) != 0; }

float display_temperature(float value) {
    return g_edit_fahrenheit ? value * 1.8f + 32.0f : value;
}

float celsius_temperature(int value) {
    return g_edit_fahrenheit ? (static_cast<float>(value) - 32.0f) / 1.8f
                             : static_cast<float>(value);
}

float current_reference() {
    const PlantPetReference& reference = g_draft.custom_profile.references[edit_channel()];
    return edit_high() ? reference.high : reference.low;
}

LangKey channel_key(uint8_t channel) {
    return channel == 0 ? LAB_SOIL_SHORT : channel == 1 ? LAB_TEMP_SHORT : LAB_AIR_SHORT;
}

uint16_t channel_color(uint8_t channel) {
    return channel == 0 ? PB_SOIL_P1 : channel == 1 ? PB_TEMP_P1 : PB_HUM_P1;
}

const char* profile_label(PlantProfileMode mode) {
    return mode == PlantProfileMode::General ? L(PLANT_PROFILE_GENERAL_KEY) : L(PLANT_PROFILE_CUSTOM_KEY);
}

void draw_choice(uint8_t index, bool selected, const char* label) {
    const int x = index == 0 ? 8 : 82;
    const uint16_t border = selected ? PB_LUZ_P1 : TFT_DARKGREY;
    tft.fillRect(x, CHOICE_Y, CHOICE_W, CHOICE_H, TFT_BLACK);
    tft.fillRoundRect(x, CHOICE_Y, CHOICE_W, CHOICE_H, 3, kPanelBg);
    tft.drawRoundRect(x, CHOICE_Y, CHOICE_W, CHOICE_H, 3, border);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(FONT_SMALL);
    if (tft.textWidth(label) > CHOICE_W - 8) tft.setTextFont(1);
    tft.setTextColor(selected ? TFT_WHITE : TFT_CYAN, kPanelBg);
    tft.drawString(label, x + CHOICE_W / 2, CHOICE_Y + CHOICE_H / 2 - 2);
    tft.setTextFont(0);
}

void draw_choices(uint8_t choice) {
    draw_choice(0, choice == 0, L(PLANT_MENU_CANCEL_KEY));
    draw_choice(1, choice == 1, L(MENU_SAVE));
}

void draw_save_summary() {
    const PlantPetProfile& profile = g_draft.mode == PlantProfileMode::General
        ? plant_pet_general_profile() : g_draft.custom_profile;
    char lines[3][40];
    const char* pointers[3];
    uint16_t colors[3];
    for (uint8_t i = 0; i < 3; ++i) {
        const PlantPetReference& reference = profile.references[i];
        if (i == 1) {
            const float low = display_temperature(reference.low);
            const float high = display_temperature(reference.high);
            snprintf(lines[i], sizeof(lines[i]), "%s %.1f - %.1f %s", L(channel_key(i)),
                     low, high, g_edit_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT));
        } else {
            snprintf(lines[i], sizeof(lines[i]), "%s %.0f - %.0f%%", L(channel_key(i)),
                     reference.low, reference.high);
        }
        pointers[i] = lines[i];
        colors[i] = channel_color(i);
    }
    drawCenteredMenuFrame(profile_label(g_draft.mode), PB_LUZ_P1, L(ST_TURN_PUSH));
    drawCenteredMenuBodyLines(pointers, colors, 3, MENU_TEXT_FONT_SMALL, 57, 12);
}

bool commit_draft() {
    // Reconfirming all six references binds the customized soil percentage to
    // the current calibration. Selecting General preserves the customized set.
    if (g_draft_personalized || (g_draft.mode == PlantProfileMode::Custom
        && g_draft.soil_dry_raw == 0 && g_draft.soil_wet_raw == 0)) {
        const SoilCalibrationData calibration = get_current_soil_calibration();
        g_draft.soil_dry_raw = calibration.dry_raw;
        g_draft.soil_wet_raw = calibration.wet_raw;
    }
    if (!save_plant_settings_store(g_draft)) return false;
    publish_settings(g_draft);
    return true;
}

} // namespace

void load_plant_settings() {
    publish_settings(load_plant_settings_store());
}

PlantSettingsSnapshot get_plant_settings_snapshot() {
    if (!g_loaded) load_plant_settings();
    const SoilCalibrationData calibration = get_current_soil_calibration();
    PlantSettingsSnapshot snapshot{};
    portENTER_CRITICAL(&g_settings_mux);
    const bool review = g_settings.mode == PlantProfileMode::Custom
        && !calibration_matches(g_settings, calibration);
    if (review != g_last_soil_review) {
        g_last_soil_review = review;
        ++g_revision;
    }
    snapshot.settings = g_settings;
    snapshot.revision = g_revision;
    snapshot.soil_needs_review = review;
    portEXIT_CRITICAL(&g_settings_mux);
    return snapshot;
}

PlantPetProfile get_active_plant_profile() {
    const PlantSettingsSnapshot snapshot = get_plant_settings_snapshot();
    return snapshot.settings.mode == PlantProfileMode::General
        ? plant_pet_general_profile() : snapshot.settings.custom_profile;
}

uint32_t plant_settings_revision() { return get_plant_settings_snapshot().revision; }
bool plant_settings_soil_needs_review() { return get_plant_settings_snapshot().soil_needs_review; }

void start_plant_settings_menu() {
    reset_draft();
    g_menu_index = 0;
    change_state(PLANT_SETTINGS_MENU);
}

bool plant_settings_menu_is_active() { return g_state != PLANT_SETTINGS_NORMAL; }
PlantSettingsMenuState get_plant_settings_menu_state() { return g_state; }

bool cancel_plant_settings_menu() {
    if (!plant_settings_menu_is_active()) return false;
    reset_draft();
    change_state(PLANT_SETTINGS_NORMAL);
    return true;
}

bool plant_settings_encoder_is_circular() {
    return g_state == PLANT_SETTINGS_MENU || g_state == PLANT_SETTINGS_PROFILE
        || g_state == PLANT_SETTINGS_CONFIRM_SAVE || g_state == PLANT_SETTINGS_CONFIRM_RESET;
}

int get_plant_settings_encoder_min() {
    if (g_state != PLANT_SETTINGS_EDIT_REFERENCE) return 0;
    const uint8_t channel = edit_channel();
    const PlantPetReference& reference = g_draft.custom_profile.references[channel];
    const float lower = edit_high() ? reference.low + (channel == 1 ? 2.0f : 5.0f)
                                   : channel == 1 ? -20.0f : 0.0f;
    return static_cast<int>(ceilf(channel == 1 ? display_temperature(lower) : lower));
}

int get_plant_settings_encoder_max() {
    if (g_state == PLANT_SETTINGS_MENU) return PLANT_MENU_EXIT_INDEX;
    if (g_state == PLANT_SETTINGS_PROFILE || g_state == PLANT_SETTINGS_CONFIRM_SAVE
        || g_state == PLANT_SETTINGS_CONFIRM_RESET) return 1;
    if (g_state != PLANT_SETTINGS_EDIT_REFERENCE) return 0;
    const uint8_t channel = edit_channel();
    const float domain_high = channel == 1 ? 80.0f : 100.0f;
    const float width = channel == 1 ? 2.0f : 5.0f;
    // Allow moving the whole interval upward in one pass. The lower editor
    // uses the domain ceiling; its partner is adjusted only in the draft.
    const float upper = edit_high() ? domain_high : domain_high - width;
    return static_cast<int>(floorf(channel == 1 ? display_temperature(upper) : upper));
}

int get_plant_settings_encoder_value() {
    switch (g_state) {
        case PLANT_SETTINGS_MENU: return g_menu_index;
        case PLANT_SETTINGS_PROFILE: return g_draft.mode == PlantProfileMode::Custom ? 1 : 0;
        case PLANT_SETTINGS_EDIT_REFERENCE: {
            const float value = current_reference();
            return static_cast<int>(lroundf(edit_channel() == 1 ? display_temperature(value) : value));
        }
        case PLANT_SETTINGS_CONFIRM_SAVE:
        case PLANT_SETTINGS_CONFIRM_RESET: return g_choice;
        default: return 0;
    }
}

void set_plant_settings_encoder_value(int value) {
    const int next = constrain(value, get_plant_settings_encoder_min(), get_plant_settings_encoder_max());
    if (next == get_plant_settings_encoder_value()) return;
    switch (g_state) {
        case PLANT_SETTINGS_MENU: g_menu_index = static_cast<uint8_t>(next); break;
        case PLANT_SETTINGS_PROFILE:
            g_draft.mode = next == 0 ? PlantProfileMode::General : PlantProfileMode::Custom;
            break;
        case PLANT_SETTINGS_EDIT_REFERENCE: {
            PlantPetReference& reference = g_draft.custom_profile.references[edit_channel()];
            const float edited = edit_channel() == 1 ? celsius_temperature(next) : static_cast<float>(next);
            if (edit_high()) reference.high = edited;
            else {
                reference.low = edited;
                const float minimum_high = edited + (edit_channel() == 1 ? 2.0f : 5.0f);
                if (reference.high < minimum_high) reference.high = minimum_high;
            }
            break;
        }
        case PLANT_SETTINGS_CONFIRM_SAVE:
        case PLANT_SETTINGS_CONFIRM_RESET: g_choice = static_cast<uint8_t>(next); break;
        default: return;
    }
    runtime_request_ui_refresh(false);
}

uint8_t handle_plant_settings_button() {
    switch (g_state) {
        case PLANT_SETTINGS_MENU:
            reset_draft();
            if (g_menu_index == 0) change_state(PLANT_SETTINGS_PROFILE);
            else if (g_menu_index == 1) {
                g_draft.mode = PlantProfileMode::Custom;
                g_draft_personalized = true;
                change_state(PLANT_SETTINGS_EDIT_REFERENCE);
            } else if (g_menu_index == PLANT_MENU_RESET_INDEX) {
                change_state(PLANT_SETTINGS_CONFIRM_RESET);
            } else if (g_menu_index == PLANT_MENU_EXIT_INDEX) {
                change_state(PLANT_SETTINGS_NORMAL);
                return 1;
            }
            break;
        case PLANT_SETTINGS_PROFILE:
            g_choice = 0;
            change_state(PLANT_SETTINGS_CONFIRM_SAVE);
            break;
        case PLANT_SETTINGS_EDIT_REFERENCE:
            if (++g_edit_field < 6) {
                runtime_request_ui_refresh(true);
            } else {
                g_choice = 0;
                change_state(PLANT_SETTINGS_CONFIRM_SAVE);
            }
            break;
        case PLANT_SETTINGS_CONFIRM_SAVE:
            if (g_choice == 1) change_state(commit_draft() ? PLANT_SETTINGS_SAVED : PLANT_SETTINGS_ERROR);
            else { reset_draft(); change_state(PLANT_SETTINGS_MENU); }
            break;
        case PLANT_SETTINGS_CONFIRM_RESET:
            if (g_choice == 1) {
                if (reset_plant_settings_store()) {
                    load_plant_settings();
                    reset_draft();
                    change_state(PLANT_SETTINGS_SAVED);
                } else change_state(PLANT_SETTINGS_ERROR);
            } else change_state(PLANT_SETTINGS_MENU);
            break;
        case PLANT_SETTINGS_SAVED:
        case PLANT_SETTINGS_ERROR:
            reset_draft();
            change_state(PLANT_SETTINGS_MENU);
            break;
        default: break;
    }
    return 0;
}

void draw_plant_settings_menu(bool screen_changed) {
    static PlantSettingsMenuState last_state = PLANT_SETTINGS_NORMAL;
    static int last_field = -1;
    static int last_index = -1;
    static int last_value = -9999;
    static int last_choice = -1;
    static bool last_review = false;
    const PlantSettingsMenuState state = g_state;
    const uint8_t field = g_edit_field;
    if (state == PLANT_SETTINGS_NORMAL) return;
    const bool changed = screen_changed || state != last_state
        || (state == PLANT_SETTINGS_EDIT_REFERENCE && field != last_field);
    if (changed) {
        tft.fillScreen(TFT_BLACK);
        drawHeader(L(TIT_LAB_PLANT));
        last_index = -1;
        last_value = -9999;
        last_choice = -1;
    }
    if (state == PLANT_SETTINGS_MENU) {
        const uint8_t selected = g_menu_index;
        if (changed || selected != last_index) {
            const char* items[PLANT_MENU_PRIMARY_COUNT] = {L(PLANT_MENU_PROFILE_KEY), L(PLANT_MENU_PERSONALIZE_KEY)};
            const bool full = changed || last_index < 0;
            drawSettingsGridMenu(items, PLANT_MENU_PRIMARY_COUNT, selected, L(MENU_RESET), L(MENU_EXIT), full, last_index);
            if (full) drawFooterHint(L(INSTR_SEL), tft.width() / 2, LM_MENU_FOOTER_Y);
            last_index = selected;
        }
        const bool review = plant_settings_soil_needs_review();
        if (changed || review != last_review) {
            tft.fillRect(8, 58, 144, 21, TFT_BLACK);
            if (review) {
                tft.setFreeFont(FONT_SMALL);
                tft.setTextDatum(MC_DATUM);
                tft.setTextColor(PB_SOIL_P1, TFT_BLACK);
                tft.drawString(L(PLANT_SOIL_REVIEW_KEY), 80, 67);
                tft.setTextFont(0);
            }
            last_review = review;
        }
    } else if (state == PLANT_SETTINGS_PROFILE || state == PLANT_SETTINGS_EDIT_REFERENCE) {
        const int value = get_plant_settings_encoder_value();
        if (changed || value != last_value) {
            if (state == PLANT_SETTINGS_PROFILE) {
                drawCenteredMenuValueScreen(L(PLANT_MENU_PROFILE_KEY), profile_label(g_draft.mode), PB_LUZ_P1,
                                            MENU_VALUE_FONT_BODY, L(ST_TURN_PUSH), changed);
            } else {
                char title[48];
                char text[24];
                const uint8_t channel = field < 6 ? field / 2 : 2;
                snprintf(title, sizeof(title), "%s: %s", L(channel_key(channel)),
                         L((field & 1u) ? PLANT_REFERENCE_HIGH_KEY : PLANT_REFERENCE_LOW_KEY));
                if (channel == 1) {
                    snprintf(text, sizeof(text), "%.1f %s", display_temperature(current_reference()),
                             g_edit_fahrenheit ? L(ST_UNIT_F_SHORT) : L(ST_UNIT_C_SHORT));
                } else snprintf(text, sizeof(text), "%d%%", value);
                drawCenteredMenuValueScreen(title, text, channel_color(channel), MENU_VALUE_FONT_TIMER, L(ST_TURN_PUSH), changed);
            }
            last_value = value;
        }
    } else if (state == PLANT_SETTINGS_CONFIRM_SAVE) {
        if (changed) draw_save_summary();
        if (changed || g_choice != last_choice) {
            draw_choices(g_choice);
            last_choice = g_choice;
        }
    } else if (state == PLANT_SETTINGS_CONFIRM_RESET) {
        if (changed) drawResetChoicePromptShell(L(MENU_RESET), L(MENU_DEFAULTS), L(PLANT_RESET_SUB_KEY), L(ST_TURN_PUSH));
        if (changed || g_choice != last_choice) {
            updateResetChoiceButtons(L(MENU_NO), L(MENU_YES), g_choice);
            last_choice = g_choice;
        }
    } else if (changed && (state == PLANT_SETTINGS_SAVED || state == PLANT_SETTINGS_ERROR)) {
        drawCenteredMenuSavedScreen(state == PLANT_SETTINGS_SAVED ? L(MENU_SAVED) : L(MENU_ERROR),
                                    profile_label(get_plant_settings_snapshot().settings.mode), PB_LUZ_P1,
                                    MENU_VALUE_FONT_BODY, L(ST_PUSH_MENU));
    }
    last_state = state;
    last_field = field;
}
