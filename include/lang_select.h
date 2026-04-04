#pragma once
#include "languages.h"

// Load the language from NVS without showing the selector again.
void loadLanguage();

// Save and apply the active language.
void saveLanguage(Language language);

// Show the blocking boot language selector.
// It always appears on cold boot and preselects the last saved language.
void showLanguageMenu();
