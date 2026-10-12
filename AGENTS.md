# AGENTS.md

Guía breve para agentes que trabajen en este repo P-Bit.

## Estado y prioridad de trabajo — 2026-10-12

- Última build comprobada (2026-10-12): `esp32dev`, base `23fdb57` + cambios locales V1 de Planta Lab, `SUCCESS` (72.82 s, tras fix de acceso al setup y resumen compacto); RAM estática `60372/327680` (18.4 %), Flash `979797/1310720` (74.8 %). Evidencia y advertencias en `docs/PRODUCTION_RELEASE.md`; no atribuir esta build solo al commit base.
- Diseño de Planta Lab conservado: verde A persistente, barras B por sensor y tierra café progresiva; sol/luna y efectos distribuidos con máximo cuatro grupos conjuntos. La versión visual anterior cargada en P-Bit recibió valoración favorable el 2026-10-12 y cierre visual provisional de producto. Esa aceptación no valida automáticamente comportamiento/setup V1 ni todas las poses, idiomas, recuperación, flicker o Demo.
- V1 implementada: Luz <200 lux permite REPOSO contextual sin competir como atención; identidad/fecha de adquisición por canal y caducidad local (Suelo/Luz >1000 ms, DHT >3000 ms); gesto gradual de Suelo/Temperatura/Aire y mezcla de poses continua al interrumpir recuperaciones. Textos observacionales SUELO SECO/HÚMEDO; no diagnóstico de daño. Revisión de origen: `docs/PLANTA_LAB_BEHAVIOR_REVIEW.md`; contrato vigente: ADR-003.
- Configuración V1 implementada: General/Personalizado; larga 1200 ms abre y larga dentro cancela. Grid Perfil/Ajustar, Reset=2 y Salir=3 (dos primarias). Ajustar edita seis referencias Suelo/Temperatura/Aire en borrador, con Cancelar inicial y Guardar explícito. Temperatura usa C/F global y guarda °C; NVS `plant_cfg` versionada con checksum. General conserva personalizado; Reset restaura únicamente Planta Lab. Personalizado de Suelo ligado a referencias RAW: al cambiar calibración, referencia neutral/número blanco y REVISAR SUELO hasta revisar/guardar, sin cambiar alarmas ni calibración. Demo usa General con instancia separada y sin escribir perfil; Planta Lab sigue fuera de `kDemoScenes`.
- Candidato V1 cargado el 2026-10-12 en COM8 (hashes verificados). Primera prueba de uso: la pulsación larga no abría el setup (`LAB_PLANT_SCREEN` sin umbral en `poll_rotary_aux()`) y el resumen se veía apretado; ambos corregidos y el usuario informa que funcionan. Siguen sin comprobar caso a caso persistencia tras reinicio, Reset, C/F, revisión de calibración, comportamiento, poses, idiomas, flicker y Demo. Siguiente paso: completar esa prueba V1, conservando diseño; presets por especie, historial/combinaciones y consejos sostenidos siguen posteriores.
- Referencias General: Suelo 30..70 %, temperatura 15..30 °C y Aire 40..70 %, educativas sin especie. Referencias editables, escala gráfica, dominio instrumental y anclajes ambientales están separados; barras/modelo comparten perfil activo. Temperatura gráfica conserva General 0..40 °C y se amplía para referencias personalizadas fuera de ese tramo. Efectos conservan anclajes absolutos: frío <15 hacia 8 °C, calor desde 27/30/33/36, Aire <40/>70 %. Amplitudes de gesto e histéresis son reglas provisionales de mascota, no óptimos ni daño biológico. Suelo es relativo a calibración; 8000 lux es techo instrumental.
- Investigación preliminar: `docs/PLANTA_LAB_RESEARCH.md`. Reposo contextual y General/Personalizado ya implementados; candidatas por especie no son presets validados. `millis()` permite duraciones de sesión, pero no hay historial diario/persistente, RTC, detección de sol directo ni corrección por calor propio.
- La prueba final de 24 h y la validación física completa por unidad siguen pendientes, aplazadas por decisión del usuario. No iniciarlas ni activar debug por inferencia de un checklist histórico.
- `docs/HARDWARE_VALIDATION_2026-06-04.md` es evidencia histórica. Antes de tratar una anomalía como bug vigente, consultar su tabla de seguimiento, `CHANGELOG.md` y el código actual.
- El cierre visual vigente y sus preguntas están en `docs/ROADMAP.md`; diseño/implementación de Planta Lab en `docs/ADR-003-planta-lab.md`. No marcar validaciones HW como completadas basándose solo en lectura de código o build.

## Arquitectura Rápida

- Firmware PlatformIO/Arduino para ESP32 con TFT ST7735 160x128, encoder, LED RGB, buzzer y sensores ambientales.
- `src/io.cpp` lee sensores y publica `Reading`; `src/tft_display.cpp` enruta pantallas; `src/rotary.cpp` centraliza encoder; `src/settings_store.cpp` encapsula NVS.
- `src/plant_pet.cpp` interpreta lecturas con identidad/frescura también fuera de Planta Lab; `src/plant_pet_art.cpp` dibuja en buffer fijo RGB565 y mantiene mezcla efectiva de poses. `src/ui_plant_settings.cpp` gestiona perfil/borrador y `settings_store.cpp` su blob NVS. Instancias real/Demo separadas (Demo General); referencias activas compartidas entre modelo y barras, desconocidos sin penalización.
- Carrusel de producción: `Inicio -> Clima Lab -> Planta Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`; `Planta Lab` se omite si Suelo no entrega lectura válida.
- `SENSOR_ZONE_SCREEN` es la capa común para Temperatura, Humedad, Luz, Sonido, Suelo y Termómetro (`DS18B20` técnico). Su ciclo visible común es `Principal -> Rango -> Ficha -> Dato -> Curva`; `Sonido` añade `Sonido VU` y `Sonido Onda` justo después de `Principal`. Los enums internos históricos (`Focus`, `Valor`, `Graph`, `Gauge`, `Card`) pueden seguir existiendo.
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

- Escribir guías y documentos desde la versión de producción. No presentar pantallas legacy, pantallas de laboratorio, `GRAPH_SCREEN` clásico, `SOUND_SCREEN` clásico ni `DS18B20_SCREEN` como paradas visibles; si se mencionan, marcarlas como implementación interna, menú/configuración o visualizador.
- Guía base completa: `docs/PBIT_KNOWLEDGE_BASE.md` — fuente principal para producto, uso, energía, pantallas, sensores, pinout, conectividad y soporte.
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
