# 05 Light

Escenas previstas en este grupo:

- `00_light_runtime_lux.cpp`
- `01_light_runtime_log_pct.cpp`
- `02_light_runtime_raw_adc.cpp`
- `03_light_runtime_dark.cpp`
- `04_light_runtime_dim.cpp`
- `05_light_runtime_indoor.cpp`
- `06_light_runtime_bright.cpp`
- `07_light_runtime_sunlight.cpp`
- `08_light_menu_root.cpp`
- `09_light_edit_dim.cpp`
- `10_light_edit_indoor.cpp`
- `11_light_edit_bright.cpp`
- `12_light_edit_display.cpp`
- `13_light_edit_alerts.cpp`
- `14_light_reset_confirm.cpp`
- `15_light_saved_calibration.cpp` - nombre legacy; corresponde a límites/modo guardados
- `16_light_saved_display.cpp`
- `17_light_saved_alerts.cpp`
- `18_light_saved_reset.cpp`

Notas de estado:

- `LIGHT_SCREEN` ya no es parada directa del carrusel visible.
- Se abre como menú/configuración desde Sensor Zone con `SZ_LIGHT` mediante pulsación larga.
- La magnitud de usuario es lux calibrado `0..8000`; el valor raw ADC `0..4095` es solo calibración/debug.
- Inicio, Sensor Zone `Dato`, `Rango`, `Ficha` y `Curva` deben usar escala `0..8000 lux`.
- Mantener la nota de producto: el RGB se apaga en luz para no contaminar la lectura LDR.
