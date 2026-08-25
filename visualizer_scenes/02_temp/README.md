# 02 Temp

Escenas previstas en este grupo:

- `00_temp_runtime_ok.cpp`
- `01_temp_runtime_low_alert.cpp`
- `02_temp_runtime_high_alert.cpp`
- `03_temp_runtime_no_sensor.cpp`
- `04_temp_menu_root.cpp`
- `05_temp_edit_low.cpp`
- `06_temp_edit_high.cpp`
- `07_temp_edit_unit.cpp`
- `08_temp_edit_alerts.cpp`
- `09_temp_reset_confirm.cpp`
- `10_temp_saved_limits.cpp`
- `11_temp_saved_unit.cpp`
- `12_temp_saved_alerts.cpp`
- `13_temp_saved_reset.cpp`

Notas de estado:

- `TEMP_SCREEN` ya no es parada directa del carrusel visible.
- Se abre como menú/configuración desde Sensor Zone con `SZ_TEMP` mediante pulsación larga.
- Las vistas de detalle visibles para temperatura viven dentro de Sensor Zone: `Principal`, `Rango`, `Ficha`, `Dato` y `Curva` (internamente pueden seguir usando `focus`, `gauge`, `card`, `valor` y `graph`).
- Los snippets runtime de este grupo son útiles como `config-menu` o referencia legacy, no como `Inicio` actual.
- Textos traducibles deben mapear a `TIT_TEMP`, `MENU_*`, `ST_UNIT_*` y claves relacionadas.
