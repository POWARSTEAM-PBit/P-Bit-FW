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
- `10_sound_saved_calibration.cpp` - nombre legacy; corresponde a límites guardados
- `11_sound_saved_alerts.cpp`
- `12_sound_saved_reset.cpp`

Notas de estado:

- La parada visible de sonido es el slot `SENSOR_ZONE_SCREEN` con `SZ_SOUND`, no `SOUND_SCREEN`.
- La pulsación corta en Sonido recorre `Principal -> Sonido VU -> Sonido Onda -> Rango -> Ficha -> Dato -> Curva`.
- `LAB_SOUND_VU_STACK_SCREEN` y `LAB_SOUND_VU_WAVE_SCREEN` son renderers/modos de Sonido dentro de Sensor Zone.
- `SOUND_SCREEN` queda como menú/configuración desde Sensor Zone con `SZ_SOUND` mediante pulsación larga.
- Las escenas runtime antiguas son referencia/config-menu; representar VU stack/wave como modos de producción dentro de Sonido.
- El VU debe documentar sprite, historial suavizado y badge con clear acotado.
