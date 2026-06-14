# AGENTS.md

Guía breve para agentes que trabajen en este repo P-Bit.

## Arquitectura Rápida

- Firmware PlatformIO/Arduino para ESP32 con TFT ST7735 160x128, encoder, LED RGB, buzzer y sensores ambientales.
- `src/io.cpp` lee sensores y publica `Reading`; `src/tft_display.cpp` enruta pantallas; `src/rotary.cpp` centraliza encoder; `src/settings_store.cpp` encapsula NVS.
- `SENSOR_ZONE_SCREEN` es la capa común para Temperatura, Humedad, Luz, Sonido, Suelo y Termómetro (`DS18B20` técnico). Sus nombres visibles son `Principal` (sin sufijo), `Dato`, `Curva`, `Rango` y `Ficha`; `Sonido` añade `Sonido VU` y `Sonido Onda`. Los enums internos históricos (`Focus`, `Valor`, `Graph`, `Gauge`, `Card`) pueden seguir existiendo.
- `Sistema` separa dos controles audibles:
  - `Bip` -> `g_sound_enabled` / NVS `sys_sound`: beeps de UI, navegación y confirmaciones.
  - `Alarmas` -> `g_alarm_sound_enabled` / NVS `sys_alarm`: alertas audibles y final de cuenta regresiva del `Timer`.
- La unidad global de temperatura se guarda en NVS `sys_unit_f`; no volver a forzar `g_is_fahrenheit=false` en boot normal.

## Reglas TFT

- Pantalla objetivo: 160x128 px landscape. Cualquier texto largo debe probarse mentalmente contra ese ancho.
- Mantener shell estático separado de campos dinámicos; evitar `fillScreen` en cada tick.
- Usar `L(KEY)`/`LIn(...)` para textos visibles; no introducir strings hardcodeados salvo identificadores técnicos (`DHT11`, `DS18B20`, `LDR`, `GPIO`, etc.).
- Terminología visible: `Sonido`, no `Ruido`; Sensor Zone usa `Dato`, `Curva`, `Rango` y `Ficha`; `Lab` solo para pantallas multisensor/experimento; `Termómetro`/`Termo` para la sonda externa, dejando `DS18B20` como identificador técnico.
- Menús raíz de settings: usar `drawSettingsGridMenu()` 2x3. Máximo 4 opciones primarias; `Reset` abajo izquierda y `Salir` abajo derecha. Solo Suelo usa `Calibrar` como calibración real.
- Contrato encoder-grid: si `drawSettingsGridMenu()` recibe `primary_count=N`, entonces `Reset=N`, `Salir=N+1`, `get_*_encoder_max()` debe devolver `N+1` y `set_*_input_value()` debe limitar con `get_*_encoder_min()/max()`, no con números mágicos. Definir constantes locales para `PRIMARY_COUNT`, `RESET_INDEX` y `EXIT_INDEX`.
- Menús con encoder: cachear estado/índice/valor visible; no redibujar menú completo por giro. `fillScreen()`/`drawHeader()` solo al entrar o cambiar de estado; en navegación interna actualizar solo tiles, botones o valor dinámico.

## Docs Canónicos

- Descripción del producto: `docs/PROJECT.md` — qué es, hardware, capacidades, mapa de docs.
- Manual de usuario: `docs/USER_GUIDE.md` — uso, menús, seguridad, configuración.
- Manual técnico: `docs/TECHNICAL.md` — arquitectura, pinout, código, BLE, NVS, menús completos.
- Diseño visual: `docs/DESIGN_SYSTEM.md` — paleta, iconos, fuentes, reglas de UI.
- Reglas de render: `docs/TFT_RENDER_RULES.md` — anti-flicker, sprites, chrome/data.
- Roadmap: `docs/ROADMAP.md` — pendientes y mejoras futuras.
- Producción: `docs/PRODUCTION_CHECKLIST.md` y `docs/PRODUCTION_RELEASE.md`.
- Visualizador: `visualizer_scenes/README.md`, `visualizer_scenes/SCENE_RULES.md` y escenas por carpeta.
- Cambios relevantes deben quedar en `CHANGELOG.md`.

## Qué No Tocar Sin Permiso

- No editar código si el usuario pidió solo documentación.
- No revertir cambios ajenos ni limpiar archivos modificados que no sean parte de la tarea.
- No activar `FIRMWARE_DEBUG`, `PBIT_ENABLE_SERIAL_PLOTTER` ni cambiar flags de build sin una petición explícita.
- No versionar artefactos de build, logs, `.pio/`, binarios ni zips.

## BLE Secreto

- BLE sale factory-off mediante `ble_en=false` y reset por build-hash.
- La activación BLE es una función interna de fábrica/debug. No documentarla en el manual de usuario ni en guías de aula.
- Si hay que hablar del gesto o de `BLE_TOGGLE_SCREEN`, hacerlo solo en documentación técnica o handoff para agentes.

## Matriz de documentación — qué actualizar cuando cambia X

Antes de cerrar cualquier tarea que toque código, consultar esta tabla. Si el archivo modificado aparece en la columna izquierda, los docs de la derecha deben revisarse y actualizarse si es necesario.

| Si modificas… | Revisa y actualiza si aplica… |
|---|---|
| `include/palette.h` | `docs/DESIGN_SYSTEM.md` § 1 (Tokens de color) |
| `include/layout.h` | `docs/DESIGN_SYSTEM.md` § 3 (Anatomía) · `docs/TFT_RENDER_RULES.md` |
| `src/ui_icons.cpp` | `docs/DESIGN_SYSTEM.md` § 2 (Iconos) |
| `src/ui_*.cpp` — menús, opciones, textos de pantalla | `docs/TECHNICAL.md` § 17 · `docs/USER_GUIDE.md` §§ 8–15 |
| `src/rotary.cpp` — gestos del encoder | `docs/TECHNICAL.md` §§ 8 y 17 |
| `src/tft_display.cpp` — enrutado del carrusel | `docs/TECHNICAL.md` § 8 · `docs/USER_GUIDE.md` § 6 |
| `src/settings_store.cpp` / `include/hw.h` — claves NVS | `docs/TECHNICAL.md` § 9 |
| `src/ble.cpp` / `include/settings_store.h` — BLE | `docs/TECHNICAL.md` § 10 |
| `include/languages.h` / `src/lang_select.cpp` — i18n | `docs/TECHNICAL.md` § 12 |
| `src/alert_engine.cpp` — lógica de alertas | `docs/TECHNICAL.md` § 13 · `docs/USER_GUIDE.md` (tablas de alertas) |
| `src/io.cpp` — lectura de sensores | `docs/TECHNICAL.md` §§ 6 y 7 · `docs/PROJECT.md` § 3 |
| `platformio.ini` / `include/config.h` — flags de build | `docs/PRODUCTION_CHECKLIST.md` |
| `src/timer.cpp` / `src/ui_timer.cpp` | `docs/USER_GUIDE.md` § 14 · `docs/TECHNICAL.md` § 8 |
| Cualquier cambio de firmware | `CHANGELOG.md` — siempre, sin excepción |

**Regla de propagación docs ↔ docs**: si se actualiza `docs/TECHNICAL.md` § 17 (menús), verificar que `docs/USER_GUIDE.md` §§ 8–15 siguen siendo consistentes, y viceversa.

## Flujo De Verificación

1. Antes de editar: `git status --short` y revisar si hay cambios ajenos.
2. Para búsquedas: preferir `rg`.
3. Si se toca código: compilar con `py -m platformio run -e esp32dev` salvo que el usuario indique lo contrario.
4. Si solo se toca Markdown: revisar términos con `rg` y reportar que no se compiló por no haber cambios de código.
5. **Revisar la Matriz de documentación** — identificar qué docs afectan los archivos modificados y actualizarlos.
6. Añadir entrada en `CHANGELOG.md` con los cambios relevantes.
7. Antes de cerrar: resumir archivos editados y puntos clave, mencionando cualquier validación no ejecutada.
