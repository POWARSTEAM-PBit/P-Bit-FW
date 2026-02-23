#pragma once

// Carga el idioma desde NVS sin mostrar menú (para deep sleep wake).
void loadLanguage();

// Muestra el menú de selección de idioma (bloqueante).
// Siempre aparece en cold boot. Carga el idioma previo como preselección.
void showLanguageMenu();
