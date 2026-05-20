# 04 Humidity

Escenas previstas en este grupo:

- `00_humidity_runtime_dry.cpp`
- `01_humidity_runtime_optimal.cpp`
- `02_humidity_runtime_high_alert.cpp`
- `03_humidity_runtime_no_sensor.cpp`
- `04_humidity_menu_root.cpp`
- `05_humidity_edit_dry.cpp`
- `06_humidity_edit_comfort.cpp`
- `07_humidity_edit_alerts.cpp`
- `08_humidity_reset_confirm.cpp`
- `09_humidity_saved_thresholds.cpp`
- `10_humidity_saved_alerts.cpp`
- `11_humidity_saved_reset.cpp`

Notas de estado:

- `HUMIDITY_SCREEN` ya no es parada directa del carrusel visible.
- Se abre como menú/configuración desde Sensor Zone con `SZ_HUM` mediante pulsación larga.
- La humedad sí aparece en Home y Clima; esas escenas deben vivir en el grupo de pantallas actuales (`09_lab_graphs` mientras no exista una carpeta de producto).
- Textos traducibles deben mapear a `TIT_HUM`, `ST_*`, `MENU_*` y claves de humedad.
