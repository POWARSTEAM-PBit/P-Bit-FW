# 00 Global

Escenas previstas en este grupo:

- `00_boot_language_selector.cpp`
- `01_overlay_sleep_warning.cpp`
- `02_overlay_restarting.cpp`
- `03_overlay_blackout.cpp`

Notas de estado:

- `00_boot_language_selector.cpp` debe reflejar i18n centralizado ES/CAT/EN (`LANG_ES`, `LANG_CAT`, `LANG_EN`).
- El selector aparece en primer arranque tras borrado de NVS por cambio de build; en arranques normales se carga el idioma guardado.
- Los overlays son globales y no pertenecen al carrusel.
- No incluir BLE en este grupo: el desbloqueo BLE es pantalla oculta/debug desde Sistema.
