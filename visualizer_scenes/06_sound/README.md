# 06 Sound

Escenas previstas en este grupo:

- `00_sound_runtime_quiet.cpp`
- `01_sound_runtime_normal.cpp`
- `02_sound_runtime_loud.cpp`
- `03_sound_runtime_very_loud.cpp`
- `04_sound_menu_root.cpp`
- `05_sound_edit_quiet.cpp`
- `06_sound_edit_normal.cpp`
- `07_sound_edit_loud.cpp`
- `08_sound_edit_alerts.cpp`
- `09_sound_reset_confirm.cpp`
- `10_sound_saved_calibration.cpp`
- `11_sound_saved_alerts.cpp`
- `12_sound_saved_reset.cpp`

Notas de estado:

- La parada visible de sonido es `LAB_SOUND_VU_STACK_SCREEN` (Sonido VU), no `SOUND_SCREEN`.
- `LAB_SOUND_VU_WAVE_SCREEN` es sub-vista alternable por pulsación corta desde Sonido VU.
- `SOUND_SCREEN` queda como menú/configuración desde Sensor Zone con `SZ_SOUND` mediante pulsación larga.
- Las escenas runtime antiguas son referencia/config-menu; falta representar VU stack/wave como producción actual.
- El VU debe documentar sprite, historial suavizado y badge con clear acotado.
