# 08 System

Escenas previstas en este grupo:

- `00_system_runtime_sound_on.cpp`
- `01_system_runtime_sound_off.cpp`
- `02_system_runtime_ble_connected.cpp`
- `03_system_runtime_ble_disconnected.cpp`
- `04_system_menu_root.cpp`
- `05_system_edit_sound.cpp`
- `06_system_edit_sleep.cpp`
- `07_system_edit_language.cpp`
- `08_system_reset_confirm.cpp`
- `09_system_saved_sound.cpp`
- `10_system_saved_sleep.cpp`
- `11_system_saved_language.cpp`
- `12_system_saved_reset.cpp`

Notas de estado:

- `SYSTEM_SCREEN` sigue siendo parada visible top-level.
- Sistema muestra sonido, sleep e idioma; el idioma está centralizado en ES/CAT/EN.
- BLE está factory-off y oculto. Las escenas `02_system_runtime_ble_*` son `hidden-debug`/legacy, no flujo normal visible.
- El desbloqueo BLE real ocurre con pulsación mantenida de 60 s en Sistema y entra en `BLE_TOGGLE_SCREEN`.
- Nuevas escenas de sistema deben priorizar sonido on/off, sleep, idioma y reset.
