# AGENTS.md

Guía breve para agentes que trabajen en este repo P-Bit.

## Arquitectura Rápida

- Firmware PlatformIO/Arduino para ESP32 con TFT ST7735 160x128, encoder, LED RGB, buzzer y sensores ambientales.
- `src/io.cpp` lee sensores y publica `Reading`; `src/tft_display.cpp` enruta pantallas; `src/rotary.cpp` centraliza encoder; `src/settings_store.cpp` encapsula NVS.
- `SENSOR_ZONE_SCREEN` es la capa común para Temperatura, Humedad, Luz, Sonido, Suelo y Termómetro (`DS18B20` técnico), con modos `Focus`, `Valor`, `Gráfica`, `Dial` y `Card`.
- `Sistema` separa dos controles audibles:
  - `Bip` -> `g_sound_enabled` / NVS `sys_sound`: beeps de UI, navegación y confirmaciones.
  - `Alarmas` -> `g_alarm_sound_enabled` / NVS `sys_alarm`: alertas audibles y final de cuenta regresiva del `Timer`.

## Reglas TFT

- Pantalla objetivo: 160x128 px landscape. Cualquier texto largo debe probarse mentalmente contra ese ancho.
- Mantener shell estático separado de campos dinámicos; evitar `fillScreen` en cada tick.
- Usar `L(KEY)`/`LIn(...)` para textos visibles; no introducir strings hardcodeados salvo identificadores técnicos (`DHT11`, `DS18B20`, `LDR`, `GPIO`, etc.).
- Terminología visible: `Sonido`, no `Ruido`; `Gráfica`, no `Graf`; `Termómetro`/`Termo` para la sonda externa, dejando `DS18B20` como identificador técnico.

## Docs Canónicos

- Estado funcional: `PBIT_FUNCIONAMIENTO_ACTUAL.md`.
- Manual usuario: `MANUAL_DE_USUARIO_PBIT.md`.
- Manual técnico: `MANUAL_TECNICO_PBIT.md`.
- Roadmap: `ROADMAP_PBIT.md`.
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

## Flujo De Verificación

1. Antes de editar: `git status --short` y revisar si hay cambios ajenos.
2. Para búsquedas: preferir `rg`.
3. Si se toca código: compilar con `py -m platformio run -e esp32dev` salvo que el usuario indique lo contrario.
4. Si solo se toca Markdown: revisar términos con `rg` y reportar que no se compiló por no haber cambios de código.
5. Antes de cerrar: resumir archivos editados y puntos clave, mencionando cualquier validación no ejecutada.
