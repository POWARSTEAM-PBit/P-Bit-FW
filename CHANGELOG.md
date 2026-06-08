# Changelog

## 2026-06-08

### Fix UX — Ghost tiles y "Ver límites" circular (M4A)

- **Ghost tiles al rotar rápido:** `drawSettingsGridMenu()` limpia el rectángulo completo del tile antes de redibujarlo y, si el encoder salta más de una posición entre frames, redibuja todos los tiles individualmente sin `clearMenuBands()` ni `fillScreen()`.
- **Snapshot renderizado:** los menús raíz de Temperatura, Humedad, Luz, Sonido, Suelo, Termómetro/DS18B20 y Sistema guardan en `last_menu_index` el índice local que realmente se dibujó, evitando leer un global que pudo cambiar durante el frame.
- **"Ver límites" circular:** `LIGHT_MODE_EDIT_MARKS` y `SOUND_MODE_EDIT_MARKS` entran en las reglas circulares del encoder, igual que otros toggles binarios.
- **Scope acotado:** terminología `Rangos/Niveles/Límites`, cancelación sin guardar y paridad de marcas visuales quedan para M4B/M4C/M4D.
- **Builds:** `esp32dev` SUCCESS — RAM `49116` / Flash `955801`; `esp32dev_debug` SUCCESS — RAM `49156` / Flash `958273`.

### Perf TFT — Editores de valor sin flicker por giro de encoder

- **Card incremental:** `drawCenteredMenuValueScreen()` acepta `force_redraw`. Al entrar al editor dibuja título, footer, card y borde; durante giro de encoder solo refresca el interior del valor y actualiza el borde/acento si cambia el estado.
- **Migrado:** editores numéricos, toggles y selectores de Temperatura, Humedad, Luz, Sonido, Suelo, Termómetro/DS18B20 y Sistema pasan `state_changed` al helper común.
- **Regla canónica:** `docs/TFT_RENDER_RULES.md` y `docs/TECHNICAL.md` documentan el patrón incremental para cards de valor editable.
- **Builds:** `esp32dev` SUCCESS — RAM `49116` / Flash `955793`; `esp32dev_debug` SUCCESS — RAM `49156` / Flash `958269`.

### Perf TFT — Menús raíz 2×3 sin flicker por selección

- **Grid incremental:** `drawSettingsGridMenu()` acepta `force_redraw` y `previous_selected_index`. Al entrar al menú dibuja todos los tiles; durante navegación con encoder solo redibuja el tile anterior y el nuevo.
- **Migrado:** menús raíz de Temperatura, Humedad, Luz, Sonido, Suelo, Termómetro/DS18B20 y Sistema pasan `state_changed`/`last_menu_index` al helper y redibujan el footer solo en redraw completo.
- **Geometría preservada:** `Reset` y `Salir` permanecen fijados en la fila inferior incluso cuando el menú tiene 2 o 3 opciones primarias.
- **Builds:** `esp32dev` SUCCESS — RAM `49116` / Flash `955533`; `esp32dev_debug` SUCCESS — RAM `49156` / Flash `957981`.

### Perf TFT — Reset sin flicker por giro de encoder

- **Shell incremental:** `drawResetChoicePrompt()` se divide en `drawResetChoicePromptShell()` y `updateResetChoiceButtons()`. El fondo rojo, header, panel y descripción se dibujan solo al entrar; cambiar `NO` / `SI` redibuja únicamente los dos botones.
- **Migrado:** confirmaciones de Reset en Temperatura, Humedad, Luz, Sonido, Suelo, Termómetro/DS18B20 y Sistema usan el nuevo patrón incremental.
- **Regla canónica:** `docs/TFT_RENDER_RULES.md`, `docs/DESIGN_SYSTEM.md`, `docs/TECHNICAL.md`, `AGENTS.md` y el skill local `pbit-tft-screen` documentan que menús con encoder son pantallas dinámicas y no deben repintar pantalla completa por giro.
- **Builds:** `esp32dev` SUCCESS — RAM `49116` / Flash `955137`; `esp32dev_debug` SUCCESS — RAM `49156` / Flash `957601`.

### UX/Firmware — Suelo suavizado y calibración con confirmación

- **Promedio móvil:** `src/hw.cpp` reemplaza la EMA de Suelo por una ventana móvil de `10` porcentajes ya calibrados. El filtro se resetea al detectar sensor desconectado o al cambiar la calibración, evitando que valores antiguos contaminen el nuevo mapeo.
- **Calibración protegida:** si `Calibrar sensor` se selecciona con el sensor de Suelo desconectado (`soil_humidity = NaN`), `src/ui_soil.cpp` muestra `Sin sensor` / `Conecta sensor` / `Revisa IO35` y no entra al flujo de captura.
- **Captura reversible:** las pantallas `En aire` y `En agua` muestran botones `Salir` / `Captura` bajo el RAW vivo. `Salir` vuelve al menú sin guardar ni avanzar; `Captura` registra el punto actual.
- **Guardar explícito:** tras capturar `En aire` y `En agua`, la calibración ya no se guarda de inmediato. Ahora muestra un resumen `SECO xxxx` / `MOJADO xxxx` con dos opciones: `Salir` y `Guardar`. `Salir` es la opción inicial y descarta los valores; `Guardar` es la única ruta que escribe NVS.
- **i18n:** `include/languages.h` y `src/lang_select.cpp` añaden `Conecta sensor`, `Captura` y `Guardar` para evitar strings visibles hardcodeados.
- **Builds:** `esp32dev` SUCCESS — RAM `49116` / Flash `955093`; `esp32dev_debug` SUCCESS — RAM `49156` / Flash `957565`.

## 2026-06-07

### Fix — Calibración Suelo cancelable y sin flicker fuerte

- **Cancelación segura:** `src/rotary.cpp` consume pulsación larga mientras la calibración de Suelo está activa y llama a `cancelSoilCalibration()` en `src/ui_soil.cpp`. En subpasos (`En aire`, `En agua`, rangos, alertas y confirmación de reset) vuelve al menú de calibración sin guardar; desde el menú raíz sale a la pantalla normal. La pulsación larga ya no cae al `buttonCallback()` de release, evitando capturas o guardados accidentales.
- **Persistencia protegida:** cancelar descarta RAW temporales, recarga umbrales/alertas persistidos cuando aplica y no llama a `save_soil_calibration()`, `save_soil_thresholds()`, `set_soil_alerts_enabled()` ni `reset_soil_settings()`.
- **Anti-flicker:** el RAW live de calibración se muestrea cada `250 ms` con deadband de `3` cuentas ADC y, tras el shell inicial, solo redibuja la card del valor. Ya no se limpia título/footer/card completa por cada jitter del ADC.
- **Fondo propio:** `draw_soil_calibration_screen()` recibe el `screen_changed` del router y `startSoilCalibration()` solicita redraw completo, evitando residuos de la pantalla de Suelo al reentrar al menú. También se elimina el footer duplicado en la confirmación de Reset de Suelo.
- **Builds:** `esp32dev` SUCCESS — RAM `49052` / Flash `953465`; `esp32dev_debug` SUCCESS — RAM `49092` / Flash `955929`.

### Fix — Reset general reinicia tras limpiar NVS

- **Sistema > Reset:** al confirmar `SI`, `src/ui_system.cpp` borra la configuración con `reset_all_settings()`, muestra overlay `Reset aplicado` / `Reiniciando...` y llama a `esp_restart()`. Esto evita quedarse en una UI con estado en RAM parcialmente actualizado y fuerza que el siguiente arranque use defaults completos.
- **Selector de idioma robusto:** `include/settings_store.h` / `src/settings_store.cpp` añaden `has_language_store()` y `src/main.cpp` muestra el selector mientras la clave `lang` no exista. Además, `fw_stamp` ahora deriva del ELF SHA256 real (`esp_ota_get_app_elf_sha256`) en vez de depender del timestamp de compilación de `main.cpp`, de modo que un binario nuevo limpia NVS aunque el cambio haya ocurrido solo en otra unidad de traducción. Esto evita que el P-Bit arranque silenciosamente en español tras flasheos parciales.
- **Reset en rojo ordenado:** `drawResetChoicePrompt()` usa fondo rojo full-screen más intenso, título superior con línea blanca como el resto de pantallas, panel central rojo oscuro para la descripción y botones `NO` / `SI` legibles. `NO` sigue seleccionado por defecto; `SI` queda visualmente más peligroso al seleccionarlo.
- **Catalán compacto:** textos de sensores externos desconectados pasan de `Sense sensor` / `Comprova IO33/IO35` a `No sensor` / `Revisa IO33/IO35` para evitar desbordes en cards compactas.
- **Documentado:** `docs/DESIGN_SYSTEM.md`, `docs/TECHNICAL.md`, `docs/USER_GUIDE.md`, `docs/PRODUCTION_CHECKLIST.md` y `docs/PRODUCTION_RELEASE.md` aclaran la gramática visual del Reset `danger`, el reinicio tras Reset global, el selector hasta confirmar idioma y la restauración de calibraciones, umbrales, idioma, unidad, Bip, Alarmas y demás valores NVS.

## 2026-06-06

### UX — Fixes Demo Mode (Lote 3 pre-soak final)

- **Timer simulado:** `src/timer.cpp` usa `demo_mode_simulated_timer_ms()` cuando Demo Mode está activo en `TIMER_SCREEN`, mostrando un ciclo 0..59 s relativo al inicio del demo sin tocar `userTimerRunning`, `userTimerStart` ni `userTimerElapsed`.
- **Sparklines demo:** `src/ui_lab_widget_showcase.cpp` usa `demo_mode_graph_values()` como fuente sintética durante Demo Mode antes de leer buffers históricos reales; Termo Lab/DS18 y el resto de sensores muestran líneas demo aunque no haya historial físico.
- **Estado pre-demo restaurado:** `include/sensor_zone.h` y `src/sensor_zone.cpp` añaden `SzRuntimeSnapshot` runtime-only para preservar sensor activo, los 6 modos `g_viz[]` y los sub-renderers sincronizados por Sensor Zone (Focus, Graph, Gauge, Valor/Termo Lab y Sensor Card). `src/demo_mode.cpp` restaura ese snapshot y `active_screen` al salir, evitando que las escenas demo contaminen la selección del usuario. No toca NVS.
- **Alineación desconectados:** `src/ui_lab_focus.cpp` evita depender de datums verticales tipo `MC/MR/ML` en los textos externos desconectados; usa Y top calculado desde el centro visual de las cards para alinear mejor `Sin sensor` y `Revisa IO33/IO35`. Tras revisar `glyph_ab` real de TFT_eSPI/FreeFonts, `Revisa IO33/IO35` baja 1 px para coincidir con el centro matemático del card inferior. Los textos de alerta (`Sin sensor`, `Revisa IO33/IO35`) usan color vivo del sensor mientras fondos, bordes e iconos permanecen desaturados.
- **Alineación conectados:** `src/ui_lab_focus.cpp` normaliza la Y de los nombres conectados (`TEMP.`, `AIRE`, `LUZ`, `MIC`, `SUELO`, `DS18B20`) con la misma fórmula FreeFont usada en desconectados; el valor principal y su unidad mantienen su composición visual original. Los iconos pequeños de Temperatura y Humedad suben 1 px porque su centro de masa visual cae ~1 px por debajo del centro de bbox.
- **Temp Lab multisensor:** `src/ui_lab_widget_showcase.cpp` baja 2 px los valores válidos de las dos cards superiores (`DHT11` y `DS18B20`). Cuando DS18 está desconectado, el header normal se omite y la card superior derecha usa un bloque centrado de 3 líneas (`DS18B20` / `Sin sensor` / `Revisa IO33`) con `Sin sensor` y `Revisa IO33` en color vivo de alerta.
- **Builds:** `esp32dev` RAM `49044` / Flash `951737` (+24 RAM / +576 Flash vs post-A+B). `esp32dev_debug` RAM `49084` / Flash `954177` (+24 RAM / +528 Flash).

## 2026-06-05

### Estabilidad — Vendorización driver DHT RMT (P0 IWDT fix)

- **Problema:** sesión de validación HW 2026-06-04 detectó 3 panics `Interrupt wdt timeout on CPU0` durante 24 h con Adafruit DHT 1.4.6. Backtrace en `DHT::readHumidity()` → `DHT::read()`. Causa raíz: la librería usa `noInterrupts()` durante el polling de pulsos del sensor, bloqueando IRQs por encima del límite del Interrupt WDT (~300 ms) en peor caso.
- **Spike validador (17h09m):** se probó `htmltiger/dhtESP32-rmt@1.0.0` (MIT, RMT peripheral). Resultado: 0 Guru, 0 IWDT, 0 panics. Hipótesis confirmada.
- **Fix:** driver local vendorizado en `include/pbit_dht.h` + `src/pbit_dht.cpp`, adaptado de `htmltiger/dhtESP32-rmt v1.0` (MIT, atribución completa preservada) con cinco parches del P-Bit:
  - `uint8_t data[5] = {0}` en el buffer de bits (elimina UB del shift-left sobre memoria sin inicializar, y dimensión exacta porque solo se accede a [0..4]).
  - Singleton estático con `static uint32_t last_read_ms` para el rate-limit, eliminando la tabla dinámica de pines con `new pin_grp[1]` (zero heap allocation).
  - Validación `tot_items >= 1` antes de leer `rx_items[0]` (fix UB en caso de ringbuffer vacío).
  - Suma de pulsos en `uint16_t` en lugar de `uint8_t` para evitar truncamiento silencioso.
  - Cleanup explícito (`rmt_rx_stop` + `rmt_driver_uninstall`) en todos los error paths intermedios, no solo en el éxito.
  - Estados prefijados `PBIT_DHT_*` para evitar colisiones con otras definiciones globales.
- **`platformio.ini`:** eliminadas dependencias `adafruit/DHT sensor library@1.4.6` y `adafruit/Adafruit Unified Sensor@1.1.15`. NO se añade `htmltiger/dhtESP32-rmt` como dependencia externa: el driver es ahora local.
- **`src/io.cpp`:** lectura DHT pasa de alternar humedad/temperatura cada segundo a una sola llamada `pbit_dht11_read(t, h)` que devuelve ambos por RMT capture. Manejo explícito de `PBIT_DHT_TOO_SOON` (no cuenta como fallo). Contadores `[DHT] OK / TOO_SOON / ERR` añadidos bajo `FIRMWARE_DEBUG` para futuras sesiones de soak.
- **Limitación de validación HW:** el DHT11 está soldado al PCB del P-Bit. No se prueba desconexión física. Los caminos de error del driver están auditados en código: todos pasan por cleanup uniforme antes de retornar.
- **Decisión técnica:** se descartó `beegee-tokyo/DHTesp` (archivada, GPL-3.0, sigue usando `portENTER_CRITICAL`). Se descartó aumentar `CONFIG_ESP_INT_WDT_TIMEOUT_MS` (anti-patrón). RMT scheduler-friendly es la solución arquitecturalmente correcta y la implementación local con parches elimina los riesgos de calidad detectados en la librería externa.
- **Soak provisional 16h09m (2026-06-05 18:40 → 2026-06-06 10:46) con BLE activo:** 0 Guru, 0 IWDT, 0 reinicios post-boot. Contadores DHT al cierre: `OK:56804 / TOO_SOON:0 / ERR:12` (error rate 0.021%, todos recuperables). Stack worst: DisplayTask 2108 bytes libres; SensorTask 1944 bytes libres. Soak final 24h sobre firmware candidato a producción se ejecutará tras cerrar lotes pre-soak restantes.
- **Builds (DHT RMT aislado, sin pulidos visuales encima):** `esp32dev` RAM `49020` (+96 vs baseline pre-fix) / Flash `950845` (+5280). `esp32dev_debug` RAM `49060` / Flash `953337`.

### Pulido visual — ajustes pre-soak final

- **HOME/Luz:** en `src/ui_lab_home_cards.cpp`, la tarjeta pequeña muestra `Lux` como etiqueta superior en modo Lux y elimina la unidad inferior; valores `>= 1000` se compactan como `1.2k` para evitar invasión de la barra/card y restos visuales al volver a 3 dígitos.
- **Termómetro tarjeta:** en `src/ui_lab_sensor_cards.cpp`, el estado desconectado de `LAB_DS18_CARD_SCREEN` sube `---` y `Revisa IO33` 3 px, sin mover Suelo ni otros sensores.
- **LDR dial:** en `src/ui_lab_widget_showcase.cpp`, el icono de Luz arranca casi gris/negro a 0 lux y aumenta brillo de forma más progresiva, alineado con la lectura visual del gauge.
- **Suelo defaults:** en `src/hw.cpp`, la calibración por defecto pasa a `dry=3550` / `wet=1855`, basada en 5 P-Bits medidos (`Aire`: 3560, 3490, 3520, 3640, 3530; `Agua`: 1889, 1830, 1840, 1918, 1805).
- **Sensor Lab Focus:** en `src/ui_lab_focus.cpp`, los estados desconectados de Suelo/DS18 alinean icono, nombre y `Sin sensor` sobre el mismo centro Y en la card superior; `Revisa IO33/IO35` queda centrado geométricamente en la card inferior con clear redondeado para preservar esquinas. El hint inferior ahora dice `Cambia vista de sensor` / `Change sensor view`, con fallback automático de fuente si el ancho lo requiere.
- **Builds combinados (DHT RMT + pulidos visuales):** `esp32dev` RAM `49020` / Flash `951161` (+316 Flash vs DHT-only por los pulidos). `esp32dev_debug` RAM `49060` / Flash `953649`.

## 2026-06-04

### Pulido visual — Estados desconectados Suelo/DS18 en focus (Lote 1B post-validación HW)

- **`src/ui_lab_focus.cpp`:** en `LAB_SENSOR_FOCUS_SCREEN` con sensor externo (DS18B20/Suelo) desconectado, ya no se duplica "Sin Sensor" + "Revisa IOxx" en ambas cards. Ahora:
  - Card superior (summary panel) -> solo "Sin Sensor", Y de `34` -> `36` (Y+2).
  - Card inferior (graph panel) -> solo "Revisa IO33" / "Revisa IO35", Y de `LF_GRAPH_Y+28` -> `LF_GRAPH_Y+24` (Y-4).
- El render de sensores no externos (DHT/LDR/mic) en `LAB_SENSOR_FOCUS_SCREEN` no cambia.
- Color de los textos sin cambios (se mantiene la paleta `pbit_external_dim_*` consensuada). Cualquier ajuste de "color más vivo/entonado" se evaluará en un lote posterior.
- Build verificado: ambos envs `SUCCESS`.

### Pulido visual — Timer status text Y-2 (Lote 1A post-validación HW)

- **Ajustado:** `LT_STATUS_TEXT_Y` en `include/layout.h:71` pasa de `LT_CARD_Y + 8` a `LT_CARD_Y + 6` (-2 px). El texto de estado del Timer (`LISTO`/`EN CURSO`/`PAUSADO`/`TERMINADO`) sube 2 px para mejor alineación dentro de la card del cronómetro, según observación de Pablo en `docs/HARDWARE_VALIDATION_2026-06-04.md` § Fase 1.4.
- **Sin cambio de tamaño de build:** `LT_STATUS_TEXT_Y` es `constexpr int` evaluado en compile time. Builds con el mismo uso de memoria que el commit anterior: producción `48924`/`945565`, debug `48964`/`947861`.
- Build verificado: ambos envs `SUCCESS`.

### Build de depuración — env `esp32dev_debug` + instrumentación periódica de Stack HWM

- **Añadido:** `[env:esp32dev_debug]` en `platformio.ini` que extiende `esp32dev` y añade `-DFIRMWARE_DEBUG`. El env de producción `esp32dev` queda intacto. `include/config.h` mantiene `FIRMWARE_DEBUG` comentado para que la activación pase exclusivamente por build flag del env separado.
- **Mejorado:** instrumentación `#ifdef FIRMWARE_DEBUG` en `src/tft_display.cpp` (DisplayTask) y `src/io.cpp` (SensorTask). Antes reportaba una sola vez por tarea; ahora muestrea cada `1000 ms` y emite log cuando el peor caso histórico empeora o cada `60 s`. Valores reportados directamente en **bytes**: en ESP32 + `framework-arduinoespressif32`, `uxTaskGetStackHighWaterMark()` ya devuelve bytes, no words (a diferencia del vanilla FreeRTOS). Cero overhead cuando `FIRMWARE_DEBUG` no está definido — todo el bloque se compila fuera.
- **Documentado:** `docs/TECHNICAL.md` § A.5 explica cómo compilar/subir/monitorizar con el env debug y qué buscar en Serial.
- **Documentado:** `docs/ROADMAP.md` ítem "Stack HWM validación HW" actualizado: infraestructura lista, pendiente medir en hardware real en sesión prolongada.
- **Autorización:** activación de `FIRMWARE_DEBUG` realizada bajo petición explícita del usuario per `AGENTS.md`. Plan revisado y aprobado por code review cruzado (Claude + Codex) antes de aplicar.
- **Sin impacto en producción:** `py -m platformio run -e esp32dev` sigue dando el binario producción intacto. Solo `py -m platformio run -e esp32dev_debug` activa la build de validación.

## 2026-06-03

### Decisión operativa — Stack HWM diferido a sesión de validación HW

- **Contexto:** ítem "Stack HWM validación HW" del ROADMAP § "Deuda técnica post-auditoría". La instrumentación ya existe en `src/tft_display.cpp:940` y `src/io.cpp:118` bajo `#ifdef FIRMWARE_DEBUG`.
- **Decisión: NO activar `FIRMWARE_DEBUG` desde esta auditoría.** Razones:
  1. `AGENTS.md` prohíbe explícitamente cambiar flags de build (`FIRMWARE_DEBUG`, `PBIT_ENABLE_SERIAL_PLOTTER`) sin petición explícita del usuario.
  2. Esto NO es refactor — es ejecución de un procedimiento de validación de hardware (flashear, correr 24 h, leer Serial, registrar).
  3. Activar el flag desde un agente sin petición explícita rompería la política, aunque sea en un env separado.
- **Acción aplicada:** entrada del ROADMAP refinada para clarificar que se mantiene abierta hasta que Pablo solicite explícitamente activar el flag o añadir un env `esp32dev_debug` aislado para la sesión de validación.
- **Resultado:** la deuda no se cierra ni se ignora — se reposiciona como **paso operativo de validación HW**, no como deuda de refactor.

### Decisión arquitectural — ADR-001: diferir extracción de RGB LED mapping

- **Contexto:** `src/tft_display.cpp` (946 líneas) contiene ~289 líneas de mapping LED RGB (`set_rgb565`, `apply_*_visual_rgb` ×6, `apply_global_alert_rgb`, `update_rgb_led_state`). El ítem "RGB mapping extraíble" del ROADMAP § "Deuda técnica post-auditoría" sugería extraer.
- **Evaluadas 3 opciones:** (A) mover a `led_control.cpp` — **descartada**, invierte capas (driver dependiendo de UI/sensor_zone/alert_engine); (B) crear módulo nuevo `ui_led_feedback.cpp/.h` con move puro — atractivo pero pre-flash; (C) diferir.
- **Decisión: opción C — diferir.** Razones:
  1. El trigger documentado (`tft_display.cpp > 1100 líneas`) **no se ha cumplido**: está en 946.
  2. El cambio es **hardware-visible**: regresiones del LED RGB el compilador no las detecta; requieren validación en placa.
  3. NO es un move puro total: `g_is_fahrenheit` se usa con `extern` manual en muchos `.cpp` sin header propio. Extraerlo limpio requiere diseño de contrato, no solo traslado.
  4. El beneficio es organizativo, no funcional; las deudas de menor riesgo ya están cerradas.
- **Política futura:** si se extrae más adelante, hacerlo a `ui_led_feedback.cpp/.h` (nombre acordado por code review cruzado Claude+Codex). NO a `led_control.cpp`.
- **Resultado:** ítem queda **abierto en ROADMAP** con su trigger original (>1100 líneas). ADR-001 documenta la evaluación y la decisión para que no se reabra en futuras sesiones.

### Refactor — `struct Cache` canónico en `ui_lab_focus` y `ui_graph`

- **`src/ui_lab_focus.cpp`:** 5 variables `static g_last_summary_*` sueltas reemplazadas por `struct FocusCache` con campos nombrados (`sensor`, `valid`, `key`, `unit_mode`, `light_mode`). Instancia `g_cache` static a nivel namespace. La actualización de la cache se concentra al final del frame en un solo bloque etiquetado, siguiendo el contrato Nivel 0 de `docs/TFT_RENDER_RULES.md`.
- **`src/ui_graph.cpp`:** 4 variables `static` de cache locales a `draw_graph_screen()` (`last_sensor`, `last_band_value`, `last_band_valid`, `last_band_sensor`) reemplazadas por `struct GraphCache` (`sensor`, `band_sensor`, `band_valid`, `band_value[24]`). Locale a la función — la pantalla GRAPH es singleton y no requiere caché compartida fuera. `data_buf[GRAPH_BUFFER_SIZE]` sigue siendo `static` local (no es cache, es scratch buffer del frame actual).
- **Sin cambio funcional.** Los registros de "dirty" y los redibujados se comportan exactamente igual; solo se agrupan los flags por archivo.
- **Resultado:** cierra el ítem "`struct Cache` canónico" de la deuda técnica post-auditoría. Las pantallas con cache canónica ahora cumplen Nivel 0 explícitamente — auditable por `rg -n 'struct.*Cache' src -g 'ui_*.cpp'`.
- Build verificado: `SUCCESS` — RAM 14.9% (`48924` bytes, **-16** vs commit anterior), Flash 72.1% (`945565` bytes, **-36**). El delta negativo es esperado: agrupar campos en struct mejora padding/alineamiento.

### Refactor — NVS de idioma encapsulado en `settings_store`

- **Añadido:** `include/settings_store.h` declara `load_language_store()` y `save_language_store(uint8_t lang)`.
- **Añadido:** `src/settings_store.cpp` implementa ambas funciones usando el patrón `ScopedPrefs` consistente con el resto del módulo.
- **Refactorizado:** `src/lang_select.cpp` elimina su uso directo de `Preferences`. `loadLanguage()`, `saveLanguage()` y `showLanguageMenu()` ahora delegan en las funciones del store. Quitado `#include <Preferences.h>`.
- **Resultado:** NVS queda 100% encapsulado en `settings_store` (cierra la excepción documentada en ROADMAP § "Deuda técnica post-auditoría").
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes, sin cambio), Flash 72.1% (`945601` bytes, **-904 bytes** vs commit anterior por eliminación de dos copias inline del patrón `Preferences::begin/end`).

### Mantenimiento — Limpieza de magic numbers y rename (deuda técnica)

- **Corregido:** `src/ui_icons.cpp` — los 4 literales `0x1082` en código reemplazados por `kIconCardBg` (constante local documentada). Los comentarios explican el workaround de transparencia de icono. Centraliza el valor y reduce duplicación.
- **Corregido:** `src/ui_lab_home_cards.cpp` — los literales `0x0841` reemplazados por `kCardBg` (constante añadida al bloque de constantes del archivo). Consistencia con el patrón del resto de lab screens.
- **Renombrado:** `render_global_alert_badge()` → `update_rgb_led_state()` en `src/tft_display.cpp`. El nombre anterior sugería renderizar un badge; en realidad aplica estado LED RGB cada loop. Función `static` interna, sin impacto en la API.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes, sin cambio), Flash 72.2% (`946505` bytes, sin cambio).

### Firmware — BLE scan response con nombre del dispositivo

- **Corregido:** `src/ble.cpp` añade el nombre del dispositivo (`PBIT-XXXX`) al `scan response` mediante `adv->setScanResponse(true)` + `scanData.setName(dev_name)` antes de `adv->start()`. Sin esto, clientes BLE que filtran por `namePrefix=PBIT-` no resolvían el dispositivo aunque el advertising fuera visible. No afecta producción porque BLE sigue factory-off (`ble_en=false`); el fix mejora la detección cuando se activa el gesto secreto en `Sistema` para debug/diagnóstico.
- **Documentado:** `docs/TECHNICAL.md` § 10 (Modo BLE activo) menciona el `scan response` con `namePrefix=PBIT-`.
- **Origen del fix:** rama remota antigua `feat/lux-ldr` (commit `2e4e961` de 2025-10-26 por Rafata). Detectado durante auditoría de ramas remotas previo a su limpieza. Sin este port, la rama no podría borrarse sin perder el cambio.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes, sin cambio), Flash 72.2% (`946505` bytes, `+1076`).

### Documentación — Reglas TFT y firmware reforzadas

- **Reforzado:** `docs/TFT_RENDER_RULES.md` con Nivel 0 (cache canónica), sub-sección `sz_set_active`, Nivel 5 (Demo Mode), "Verificación pre-claim" con 10 proofs y "Banderas rojas grep-ables".
- **Reforzado:** `docs/TECHNICAL.md` con Apéndice A: stack real (Arduino+PlatformIO vs ESP-IDF), 9 banderas rojas grep firmware, 8 proofs pre-claim, GPIO traps.

### Mantenimiento — Limpieza de fonts y archivos basura

- **Limpiado:** `include/fonts.h` reducido a las 6 fuentes que realmente se compilan (`Roboto_Regular7pt8b`, `Roboto_Light6pt8b`, `Roboto_Medium10pt8b`, `IBMPlexSans_Regular9pt8b`, `IBMPlexMono_Regular12pt8b`, `IBMPlexMono_Regular24pt8b`) + los alias `FONT_*`. Las ~30 declaraciones `extern` muertas se eliminaron; los archivos `.h` quedan en `include/` como librería disponible para reactivar.
- **Eliminado del tracking:** `platformio.zip` y `logs/xvba_debug.log` (ya cubiertos por reglas existentes en `.gitignore`); `listado_completo.txt` y `preview_probe_icon.html` (entradas añadidas a `.gitignore` en este commit para evitar re-tracking).
- **Eliminado del repo:** `wokwi.toml`, `diagram.json`, carpeta `archive/`.
- **Movido:** `tools/layout_validation_snippet.cpp` → `docs/`.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes), Flash 72.1% (`945429` bytes).

### Mantenimiento — Corrección documental post-limpieza

- **Corregido:** referencias rotas tras borrar `archive/` — `AGENTS.md` (línea histórica eliminada) y `visualizer_scenes/09_lab_graphs/informe_ux_ui_pbit_tft.md` (apunta ahora a `docs/TFT_RENDER_RULES.md` y `docs/DESIGN_SYSTEM.md`).
- **Actualizado:** `docs/PROJECT.md`, `docs/TECHNICAL.md`, `docs/PRODUCTION_CHECKLIST.md` y `docs/PRODUCTION_RELEASE.md` con tamaños de build vigentes (`48940`/`945429`, fecha 2026-06-03).
- **Actualizado:** documentación del visualizador para el rango LDR vigente `0..8000 lux` con curva empírica v1.
- **Mejorado:** todos los comandos `rg` de `docs/TFT_RENDER_RULES.md` y `docs/TECHNICAL.md` reescritos con `src -g 'ui_*.cpp'` para que funcionen en PowerShell/cmd sin requerir expansión de glob por shell. Añadida nota explicativa en cada bloque.
- **Matizado:** Nivel 0 de `docs/TFT_RENDER_RULES.md` aclara cuándo es obligatorio `struct *Cache` (≥2 campos dinámicos) y cuándo `meta_dirty` es aceptable (1 campo + jewel), citando `ui_temp.cpp`/`ui_humidity.cpp`/`ui_ds18.cpp` como ejemplos válidos.
- **Higiene:** quitada blank line al EOF en `docs/TECHNICAL.md` y `docs/TFT_RENDER_RULES.md` (limpia warnings de `git diff --check`).

### Documentación — Cierre de auditoría profunda (Fases 0–5)

- **Añadido:** `docs/ROADMAP.md` § "Deuda técnica post-auditoría (2026-06-03)" con 7 ítems no bloqueantes detectados en la revisión y su disparador sugerido (próxima edición del archivo correspondiente). Política: no abordar en commits aislados, sino dentro de PRs que ya toquen esos archivos.
- **Auditadas (read-only, sin cambios de código):**
  - Fase 2 Pantallas TFT: 7 banderas rojas grep-ables aplicadas sobre `src/ui_*.cpp`. Sin bugs de flicker activos. Sprites con lazy-init correcto, `TFT_DARKGREY` reservado para sensores internos, `pbit_external_dim_*` correctamente migrado para DS18/Suelo.
  - Fase 3 Firmware ESP32: NVS encapsulado (excepción documentada en `lang_select.cpp`), BLE factory-off confirmado por cadena `load_ble_enabled_store()` → `init_ble()`, Bip vs Alarmas separados sin cruces, 7 mutexes inventariados sin hallazgos bloqueantes (`g_ble_tx_mutex` documentado como excepción por payload JSON + notify), core pinning correcto (UI Core 1, Sensor Core 0), sin delays largos en loops críticos.
  - Fase 4 Arquitectura: `tft_display.cpp` (946 líneas) coherente como orquestador de UI. `SENSOR_ZONE_SCREEN` con sub-dispatch propio bien aislado. RGB LED mapping identificado como deuda extraíble pero no se mueve en este ciclo.
  - Fase 5 Comentarios: zero TODO/FIXME en `src/` e `include/`. Calidad alta de comentarios "por qué" sobre "qué".
- **Confirmado por code review cruzado:** las fases 0–5 pasaron revisión doble (Claude Code + Codex) con correcciones de wording incorporadas.

## 2026-05-29

### Firmware — Estados externos desconectados + temp mono

- **Añadido:** `external_sensor_state.*` centraliza el estado runtime de ausencia para sensores externos: DS18B20/Termómetro falta con `temp_ds18b20 < -100` y Suelo falta con `soil_humidity = NaN`.
- **Ajustado:** textos visibles de desconexión pasan a referencias de PCB visibles: `Revisa IO33` / `Revisa IO33` / `Check IO33` para Termómetro y `Revisa IO35` / `Revisa IO35` / `Check IO35` para Suelo.
- **Ajustado:** pantallas clásicas, Sensor Zone, Graph y vistas Lab usan paleta del sensor atenuada para DS18/Suelo desconectados, evitando gris plano o rojo dominante y manteniendo retorno runtime al reconectar.
- **Refinado:** el estado desconectado queda menos desaturado para mejorar legibilidad; los textos accionables `Sin sensor` / `Revisa IO33/IO35` ganan prioridad visual y se ajustan en Y en Focus y Temp Lab.
- **Añadido:** splash semafórico de conexión/desconexión para sensores externos: al conectar muestra fondo verde con `CONECTADO / Sensor Suelo|Sensor DS18B20 / IO35|IO33`; al desconectar muestra fondo rojo con `DESCONECTADO / Sensor Suelo|Sensor DS18B20 / IO35|IO33`. Dura `1500 ms`, no se dispara al boot ni durante Demo Mode.
- **Ajustado:** color visual de Suelo ahora sigue umbrales: `0%` y tramo bajo se ven amarillo intenso, `Seco..Húmedo` se mantiene verde y por encima de `Húmedo` vira progresivamente a azul. La rampa se comparte en clásica, Focus, Card, Gauge, Plant Lab, iconos y RGB.
- **Corregido:** en Focus/Sensor Zone, el clear dinámico de la card superior respeta el ancho real del título para que `DS18B20` no quede recortado por el área del valor; el label DS18B20 baja 2 px para mejorar alineación.
- **Ajustado:** icono `temp` small/XL/fallback queda monocromo; solo la versión Dial/XXL con argumento de acento conserva detalle multicolor. `probe`/DS18B20 queda separado y sin aprobación final.
- **Documentado:** `USER_GUIDE`, `TECHNICAL`, `PROJECT`, `DESIGN_SYSTEM`, `TFT_RENDER_RULES`, producción y roadmap reflejan `IO33/IO35`, estado runtime no persistente y criterios de validación.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes), Flash 72.1% (`945429` bytes).

### Firmware — LDR curva empírica v1

- **Ajustado:** `src/io.cpp` reemplaza el modelo teórico GL5528 por la primera curva empírica tomada con luxómetro: `lux = 10 * ((4095 - raw) / (raw + 150))^2`.
- **Ajustado:** el rango visual/base de Luz pasa de `0..20000 lux` a `0..8000 lux`, más coherente con las muestras reales (`5 raw -> ~7400 lux`) y con el techo de la nueva fórmula.
- **Ajustado:** el menú y la validación NVS de rangos de Luz limitan `Max brillante` al nuevo techo de `8000 lux`.
- **Ajustado:** `FC` sigue derivándose del lux calibrado con `lux / 10.764`; no existe una curva independiente para foot-candle.
- **Ajustado:** Demo Mode reduce la amplitud de luz simulada para no recortar en el nuevo techo y calcula `Raw ADC` con la inversa de la misma curva empírica.
- **Documentado:** `docs/TECHNICAL.md` deja trazable la muestra usada, la corrección del punto invertido `120 - 800` como `800 - 120`, la fórmula de ajuste y la aproximación elegida para firmware.
- Build verificado: `SUCCESS` — RAM 14.9% (`48900` bytes), Flash 71.8% (`941425` bytes).

## 2026-05-28

### Sensor Cards — paleta aplicada a TEMP/DS18; fix solape icono-valor

- **Corregido:** `temp_accent()` usa `PB_TEMP_P1` (fucsia) en lugar de `getTempColor()` — la tarjeta TEMP del Sensor Zone ahora refleja la identidad de paleta correcta.
- **Corregido:** `ds18_accent()` usa `PB_DS18_P1` en lugar del gradiente anterior — la tarjeta Termómetro/DS18 usa su color de identidad.
- **Paleta DS18:** `PB_DS18_P1` `0xA01F` (violeta) → `0xFB80` rgb(255, 121, 0) **naranja vivo** — evita la confusión visual con TEMP fucsia en el carrusel. P2 `0x045F` azul láser se mantiene (buen complemento frío/cálido).
- **Fix layout:** `kValueClearY` = `kValueTopY - 1` (y=45) → `kValueTopY + 3` (y=49) — el icono llega hasta cy+7=49; la zona de borrado del valor ya no pisa los últimos 4px del icono.
- **Sin cambio:** colores de alerta (azul/rojo), `temp_bar_color` gradient, demo mode y pantalla `LAB_DUAL_TH`.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 1 fila TERMÓMETRO actualizada.

### Paleta — TEMP y SONIDO: identidades visuales rediseñadas

- **TEMP P1:** Naranja ácido `0xFA80` → **Fucsia eléctrico** `0xF817` rgb(255, 0, 184). Temperatura deja de evocar alarma; pasa a identidad neutra/vibrante estilo GBC.
- **TEMP P2:** Rosa eléctrico `0xF814` → **Verde ácido** `0x07E8` rgb(0, 255, 64). Contraste complementario de alta tensión con el fucsia.
- **SONIDO P1:** Magenta punk `0xF81F` → **Naranja cálido** `0xFD40` rgb(255, 168, 0). Naranja encaja semánticamente con energía/vibración sonora.
- **SONIDO P2:** Verde ácido `0x07E8` → **Violeta-púrpura** `0xC01F` rgb(197, 0, 255). Desplazado hacia el morado para diferenciarse del fucsia de TEMP en pantallas multi-sensor.
- **Sin cambio:** P3/P4 de ambos sensores y resto de paleta. Rojo neón `0xF8A0` SONIDO P3 se mantiene para picos VU/alarmas.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 1 tabla de tokens actualizada.

### Documentación — Estado post-validación visual

- **Cerrado por ahora:** ghosting/flicker en pantallas revisadas queda aprobado tras la última validación; se mantiene como vigilancia de regresión, no como bloqueador general.
- **Cerrado:** icono de temperatura `temp` queda como versión final propagada a tamaños small/XL/XXL; no confundir con el icono técnico `probe`/DS18B20.
- **Actualizado:** LDR `Lux / FC / Raw ADC` queda propagado en firmware: valor/unidad ya usan un helper común en Luz, Sensor Zone, cards, dials, dashboards y gráficas; queda validación visual en hardware.
- **Actualizado:** Modo demo entra desde logos con encoder presionado y desde `Home` con pulsación larga; ahora usa escenas con dwell variable, curvas simuladas suaves, RAW coherente con luz y gráficas sintéticas; queda validación visual en hardware.

### Firmware — LDR coherente y Demo smooth

- **Añadido:** `include/light_display.h` / `src/light_display.cpp` centralizan la presentación de Luz (`Lux`, `FC`, `raw`) y la conversión `lux / 10.764`.
- **Propagado:** Sensor Zone `Card`, `Valor`, `Focus`, `Gráfica`, `Dial`, Home cards, dashboards y pantalla clásica de Luz usan el modo LDR activo para valor/unidad visible.
- **Añadido:** `g_graph_light_raw` guarda histórico RAW ADC para que gráficas y sparklines muestren RAW real cuando el modo `Raw ADC` está activo.
- **Ajustado:** Demo Mode pasa a escenas con duración variable, refresco dedicado de 220 ms, curvas suaves por sensor y valores RAW inversamente correlacionados con la luz simulada.
- **Añadido:** `demo_mode_graph_values(...)` genera datos sintéticos para `Graph` durante demo, evitando que las gráficas dependan del histórico físico mientras se muestra la coreografía.
- Build verificado: `SUCCESS` — RAM 14.9% (`48900` bytes), Flash 71.8% (`941497` bytes).

### Firmware — Calibración LDR: RAW estable y modo FC

- **Ajustado:** `io.cpp` estabiliza `ldr_raw` con media móvil de 10 lecturas ADC y calcula lux desde ese RAW promediado, eliminando el EMA posterior sobre lux.
- **Ajustado:** `Luz > Modo` cambia a `Lux / FC / Raw ADC`; `FC` muestra `lux / 10.764`, usa clave i18n `ST_FC_UNIT` y `Raw ADC` muestra la lectura cruda promediada para calibración con luxómetro.
- **Corregido:** `Sensor Cards` amplía 2 px adicionales hacia abajo el clear del valor para eliminar restos inferiores persistentes.
- **Documentado:** `docs/USER_GUIDE.md`, `docs/TECHNICAL.md` y `docs/PROJECT.md` reflejan los modos `Lux / FC / Raw ADC` y el uso del RAW promediado para calibración.
- Build verificado: `SUCCESS` — RAM 14.7% (`48252` bytes), Flash 71.6% (`938497` bytes).

### Firmware — Icono termómetro v16: diseño final propagado a todas las escalas

- **Definido como FINAL:** `impl_temp_detail` (s=3, XXL/Dial) es el diseño de referencia aprobado.
- **Propagado a XL:** `pbit_draw_temp_icon_xl` ahora llama `impl_temp_detail(cx, cy, c, c, 3)` — íconos de gauge/Focus visualmente idénticos al Dial.
- **Corregido:** `ui_icons.cpp` declara `impl_temp_detail(...)` antes de usarlo en la API XL, cerrando el fallo de compilación de la propagación v16.
- **Propagado a small (s=1):** `impl_temp` actualizado con canal parcial (4s) + rect de mercurio blanco. Sin círculo interior blanco: a s=1, `s+3 == 3s+1` (cubre todo el bulbo).
- **Todas las escalas** comparten: eje central `cx`, bulbo único centrado, mercurio `TFT_WHITE`, ticks `(4s)/3`.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 2 — tabla de tamaños corregida (eliminado `_large` s=2 inexistente), fila `temp` marcada ✅ FINAL con geometría por escala.

### Firmware — Icono termómetro v15: alineación central completa y ticks recortados

- **Ajustado (v15):** `impl_temp` e `impl_temp_detail` en `src/ui_icons.cpp` — todos los elementos comparten eje `cx`: bulbo simplificado a un único `fillCircle(cx, cy+4s, 3s+1)` (eliminados los dos círculos asimétricos en cx/cx-1), mercurio rect en `cx-s…cx+s`, círculo blanco interior en `cx`. Ticks recortados de `2s` a `(4s)/3` (−1/3).

### Firmware — Icono termómetro v14: círculo bulbo más grande y centrado

- **Ajustado (v14):** `impl_temp_detail` en `src/ui_icons.cpp` — círculo blanco interior del bulbo cambia de `fillCircle(cx-1, cy+4s, s+1)` a `fillCircle(cx, cy+4s, s+3)`: diámetro +4 px y centrado exacto con el rectángulo de mercurio.

### Firmware — Icono termómetro v9: bulbo reducido, ratio 1:2

- **Ajustado (v13):** `impl_temp_detail` — mercurio conectado visualmente: `fillRect` blanco ahora se dibuja DESPUÉS de los `fillCircle` del bulbo, creando una tira blanca continua que atraviesa el centro del bulbo y conecta con el `fillCircle` blanco interior. Columna de mercurio unificada tubo→bulbo.
- **Ajustado (v12):** `impl_temp_detail` — añadido `fillCircle(cx-1, cy+4s, s+1, TFT_WHITE)` dentro del bulbo: mercurio concentrado visible en el bulbo, conecta visualmente con la columna blanca del tubo.
- **Ajustado (v11):** bulbo asimétrico via dos `fillCircle` superpuestos (cx r=3s+1 + cx-1 r=3s+1) → +2px izquierda / +1px derecha. Mercurio cambiado de acento a `TFT_WHITE`.
- **Redimensionado (v10):** `impl_temp` / `impl_temp_detail` — ícono ahora llena el canvas ±7s igual que todos los demás íconos. Tubo extendido de 9s a 11s (cy-7s→cy+4s), bulbo movido a cy+4s (bottom=cy+7s). Orden de render corregido: tubo→canal→bulbo. En `_detail`: mercurio extendido a 6s (cy-2s→cy+4s), bulbo dibujado al final para quedar limpio.
- **Ajustado:** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — ticks reducidos de `3*s` a `2*s` (de 9 px a 6 px en Dial). Pequeñas marcas de escala.
- **Ajustado:** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — radio del bulbo reducido de **r=5s a r=3s**.
  - Ratio ancho:alto corregido de ~1:1.4 a **1:2** exacto (6s × 12s; a s=3 Dial → 18 px Ø × 36 px alto).
  - Aplica a todas las escalas: s=1 (home), s=3 (XL y Dial).
  - Tubo (4s ancho), canal (2s), ticks y mercurio sin cambios.

## 2026-05-27

### Firmware — Anti-flicker Labs, Sensor Cards y Sonido Lab

- **Optimizado:** `Clima Lab` mantiene paneles, bordes y footer-card como shell estático; el footer ahora usa cache de estado y solo limpia el texto/dot cuando cambia.
- **Optimizado:** `Temp Lab` separa cards superiores y delta-card en shell + datos; en ticks normales actualiza solo valores, barra delta y labels necesarios.
- **Corregido:** `Sensor Cards` deja de repintar icono, jewel, estado y visualización completa cuando solo cambia el valor; los estados (`Óptimo`, `Muy húmedo`, `Muy fuerte`, etc.) tienen clear dedicado para evitar fantasmas.
- **Corregido:** `Sensor Cards` separa físicamente carril de estado, valor grande y visualización inferior: estado sube 2 px, valor/unidad bajan 3 px y las barras inferiores recuperan 16 px bajando a Y+2 para conservar `Suelo Tarjeta`; el clear de visualización queda acotado a la barra para no borrar números ni esquinas inferiores.
- **Corregido:** `Sensor Cards` guarda el ancho/posición del valor anterior y limpia la unión entre caja anterior y actual, evitando fantasmas laterales cuando el dato baja de dos/tres dígitos a uno.
- **Corregido:** `Sensor Cards` amplía el carril horizontal de limpieza del valor grande para cubrir el inicio real de números anchos y eliminar restos laterales al volver a valores cortos.
- **Corregido:** `Sensor Cards` amplía 3 px hacia abajo el clear del valor grande para borrar residuos en los últimos píxeles inferiores de la fuente.
- **Ajustado:** `Sonido Lab` elimina el badge bajo el valor y muestra un número limpio en la esquina superior derecha con clear rect pequeño y fallback de fuente.
- **Ajustado:** `Sound Lab` usa `FONT_HEADER` como fuente menor del valor con fallback a `FONT_BODY`, sube el dato 1 px, baja el icono `MIC` 3 px y el texto `MIC` 2 px.
- **Ajustado:** `Clima Lab` elimina la línea vertical gris entre los cards de temperatura y humedad.
- **Ajustado:** `Sistema` recorta 1 px el card inferior desde arriba para equilibrar la distancia vertical entre cards.
- **Ajustado:** `Sistema` sube 1 px los labels `Bip` y `Alarmas` del card inferior.
- **Refinado:** `Temp Lab` ajusta clears de valores/delta para no tapar `DS18B20`, la barra inferior ni los labels `+10`.
- Build verificado: `SUCCESS` — RAM 14.7% (`48204` bytes), Flash 71.6% (`938437` bytes).

### Firmware — Icono termómetro v8: geometría unificada 4s/2s para todos los tamaños

- **Rediseñado (v8):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — fórmula única para s=1, s=2 y s=3:
  - Tubo: **4s de ancho** → paredes **1s** cada lado, canal interior **2s**. Altura: **9s** (igual que v6)
  - Bulbo: **r=5s**, centro cy+2s → base en cy+7s (borde exacto del canvas ±7s)
  - `impl_temp_detail` (Dial): misma base + `fillRect(cx-s, cy-2s, 2s, 4s, accent)` para mercurio
  - A **s=3 (Dial):** paredes 3 px | canal/mercurio **6 px** | paredes 3 px = 12 px total
  - A **s=1 (home):** 1 px | 2 px | 1 px = 4 px total (1 px menos que v6 — imperceptible)
- **Eliminado:** branch `s==1` especial de v7 — una sola fórmula para toda la familia de tamaños

## 2026-05-26

### Firmware — Defaults silenciosos, BLE 30 s y Demo simulado

- **Ajustado:** gesto secreto BLE baja de 60 s a 30 s.
- **Ajustado:** `Bip` y `Alarmas` vuelven apagados por defecto tras build/reset; BLE sigue factory-off.
- **Añadido:** Demo Mode aplica una capa visual de valores simulados sobre `g_ui_readings_snapshot` sin modificar sensores reales, NVS ni BLE.
- **Añadido:** fuente común de color en `sensor_visuals.*` para que gauges/diales y LED RGB usen la misma lógica cromática por sensor/estado.
- **Añadido:** Luz y Sonido incorporan opción persistente `Ver límites` en sus menús; se guarda en NVS y se restaura con reset del sensor.
- **Ajustado:** diales muestran marcas de rango según criterio de confianza: Temp. DHT, Humedad y Suelo visibles por defecto; Luz y Sonido solo si `Ver límites` está activo; Termómetro/DS18B20 muestra solo la marca fija de `0 °C`.
- **Ajustado:** el dial de Termómetro/DS18B20 usa escala completa `-55..+125 °C` (`-67..+257 °F`) y rampa térmica amplia: blanco hielo/cian/azul → amarillo/naranja/rojo.
- **Ajustado:** LED RGB sincronizado con la visualización activa de sensor/timer: el color físico sigue el color semántico mostrado en pantalla; cualquier vista de solo Luz mantiene RGB apagado para no contaminar el LDR.
- **Ajustado:** `Sonido Lab` baja/recorta 1 px el borde superior de la card y Sensor Cards sube 2 px el grupo numérico para evitar cortes con las gráficas inferiores.
- **Refinado:** textos abreviados de temperatura usan punto (`TEMP.`) y los mensajes `Guardado` de menús pasan a magenta, 3 px más arriba, para no confundirse con valores verdes.
- Build verificado: `SUCCESS` — RAM 14.7% (`48156` bytes), Flash 71.5% (`936653` bytes).

### Firmware — Icono termómetro v6: proporciones reales, tubo 5s paredes 1px

- **Rediseñado (v6):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo: vuelve a **5s** de ancho (proporciones de termómetro real, no fat)
  - Paredes: **1s** cada lado → interior **3s** (vs 1s en v4, vs 5s en v5 que era demasiado)
  - Bulbo: **r=5s** — equator 11px vs tubo 5px = ratio 1:2.2; claramente más ancho que el tubo
  - Altura: **9s** (cy-7s a cy+7s = canvas completo)
  - Air: `fillRect(cx-s, cy-6s, 3s, 4s, 0x1082)` — 3px wide × 4 rows
  - Mercury (Dial): `fillRect(cx-s, cy-2s, 3s, 4s, a)` — 3px wide × 4 rows en acento temperatura
  - Ticks: desde cx+3s (1px tras la pared), 3s de largo
- **Razonamiento:** v5 (7s ancho) se veía "gordo"; v6 recupera la silueta estilizada con interior 3× más visible que v4
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.3% (`934929` bytes).

### Firmware — Icono termómetro v5: tubo grande, paredes 1px, interior amplio

- **Rediseñado (v5):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo: **5s → 7s** de ancho (usa el canvas completo en horizontal)
  - Altura: 8s → **9s** (cy-7s..cy+2s, toca el borde superior del canvas)
  - Paredes: **2s → 1s** cada lado — el espacio ganado pasa al interior
  - Canal de aire: **1s×3s → 5s×4s** — 20× más área visible
  - Mercurio (Dial): **1s×4s → 5s×4s** — mismo salto
  - Bulbo: r=4s → **r=5s** — proporcional al tubo más ancho, toca cy+7s (canvas edge)
  - Ticks: desde cx+4s (1s tras la pared), longitud 3s
- **Resultado a s=1:** `[pared 1px][aire/merc 5px][pared 1px]` — todo visible sin lupa
- **Resultado a s=3 (Dial):** aire 15×12px + mercurio 15×12px dentro de paredes 3px — efecto muy dramático
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.3% (`934913` bytes).

### Firmware — Iconos: unificación de familia (estructura interior en todos)

- **Rediseñado `impl_humidity`**: añade `fillCircle(cx-2s, cy-s, s, 0x1082)` — punto de brillo reflectante en cuadrante superior-izquierdo de la gota. Convierte la masa sólida en "vidrio de agua" con estructura interior, igual que el termómetro.
- **Rediseñado `impl_sound`**: añade `drawFastHLine(cx-3s, cy-3s, 6s, 0x1082)` — franja de membrana en el tercio superior de la cápsula. Convierte el bloque sólido en "cápsula con diafragma visible".
- **Principio aplicado** (pixel-art-sprites skill): todos los iconos ahora tienen UN elemento de estructura interior en color bg (`0x1082`). Ningún icono es masa pura. Unifica el set como familia visual.

| Icono | Estructura interior |
|-------|-------------------|
| Humidity | Punto de brillo (gloss) |
| Sound | Línea de membrana |
| Light | Rayos crean espacios negativos |
| Plant | Tallo vs hojas (implícito) |
| Probe | Gap collar/cuerpo + seam (detail) |
| Temp | Canal de aire + mercurio |

### Firmware — Icono termómetro v4: paredes 2px, familia unificada

- **Rediseñado (v4 final):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo pasa de **3s a 5s** de ancho → paredes de **2px a s=1** (igual que cable sonda, tallo planta, rayos luz)
  - Cap integrado via `fillRoundRect` con `r=s` — sin círculo separado
  - Bulbo pasa de **r=3s a r=4s** — proporcional al tubo más ancho, mantiene el saliente visible
  - Ticks pasan de `cx+2s` a **`cx+3s`** (borde exterior del tubo 5s)
  - Canal de aire 1s centrado en cx — conservado: paredes 2s|aire 1s|paredes 2s
  - `impl_temp_detail` (Dial s=3): tubo 15px, paredes 6px, canal 3px, mercurio 3px×12px
- **Intención:** grosor visual equiparado con resto de iconos; espacio negativo del vidrio conservado
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.2% (`933781` bytes).

## 2026-05-25

### Firmware — Modo demo runtime y ajustes finos Sistema/reposo

- **Añadido:** `include/demo_mode.h` / `src/demo_mode.cpp` con Modo demo runtime activable al encender con el encoder presionado. Recorre una banda de pantallas, bloquea reposo mientras está activo y sale con cualquier interacción posterior del encoder.
- **Protegido:** Demo Mode usa setters runtime de Sensor Zone (`sz_set_sensor_runtime`, `sz_set_viz_runtime`) para no persistir sensor/modo en NVS.
- **Ajustado:** reposo visible — las `Z` suben 4 px y el clear rect acompaña la nueva posición.
- **Ajustado:** pantalla `Sistema` — ID/valor y cards medias quedan 1 px más bajos tras la última validación visual.
- **Ajustado:** pantalla `Sistema` mantiene estables las cards medias e inferiores cuando BLE está habilitado; BLE pasa a badge compacto en la card superior de ID y el bloque de audio conserva su ancho completo.
- **Mejorado:** activación de Modo demo — `run_boot_sequence(true)` muestrea el encoder durante la animación de arranque; una pulsación larga desde `Home` activa el demo manualmente; al entrar se muestra un splash breve `MODO DEMO`.
- **Ajustado:** Modo demo rota todas sus escenas cada 6 segundos.
- **Corregido:** icono de humedad en Home usa el icono común actualizado y `Clima Lab` eleva la gota 2 px para evitar el recorte inferior.
- **Corregido:** Sensor Cards amplía la limpieza del indicador superior (`Óptimo`, `Seco`, `Húmedo`, etc.) y desplaza el valor para evitar ghosting/solapes.
- **Refinado:** encabezados compuestos de Sensor Zone usan abreviaturas con punto (`HUM. TARJETA`, `TERMO. LAB`) para evitar recortes.
- **Revisado:** menús de configuración con multiagentes; los helpers comunes ahora reducen fuente si el valor no cabe, separan mejor los summaries guardados del footer y `Luz > Modo lectura` pasa a `Luz > Modo`.
- **Documentado:** manual de usuario, manual técnico, checklist/release, proyecto y roadmap reflejan activación, salida y restricciones del Modo demo.
- Build verificado: `SUCCESS` — RAM 14.7% (`48148` bytes), Flash 71.2% (`932717` bytes).

### Firmware — Icono termómetro v3: canal de aire + mercurio cromático en Dial

- **Rediseñado (v3 final):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo estrecho 3s + cap redondeado (`fillCircle`) + bulbo r=3s + ticks en `cx+2s`
  - **`impl_temp` (card, monocromo):** canal de aire en `0x1082` — mercurio implícito en `c`
  - **`impl_temp_detail` (Dial):** canal de aire + columna de mercurio explícita en **color acento `a`** coordinado con temperatura: `mix3_565(PB_TEMP_P4→azul, kNeonYellow, TFT_RED→rojo, amount)`
  - Intención: icono monocromo a resolución pequeña (card); mercurio cromático en el Dial
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.2%.

### Firmware — Icono DS18B20 v6: aprobado + Dial más grande

- **Diseño aprobado.** Icono DS18B20 marcado como final en `docs/ROADMAP.md` (eliminado de pendientes).
- **Rediseñado (v6):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp`:
  - Cable: 1px → 2px (añadido `drawFastVLine(cx+1, ...)`) en ambas funciones
  - Collar: 7s → 5s de ancho (proporcional al cuerpo de 3s)
  - Cuerpo: 5s×5s → 3s×6s (más estrecho y alargado verticalmente)
  - Punta: `fillCircle(cx, cy+5s, s)` — r=s, ajustada al cuerpo de 3s
  - `impl_probe_detail`: V-highlight ampliado de 3s → 4s alto
- **Dial más grande:** `pbit_draw_probe_icon_xxl` pasa de s=3 a s=4 — el icono en el centro del Dial ocupa más área dentro del clear rect disponible (60×62px). Resto de sensores sin cambio.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.1%.

### Firmware — Icono DS18B20 v5: gap collar/cuerpo + punta alineada al borde

- **Rediseñado:** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp`:
  - Cable: 4s → 3s (hace hueco al gap visual)
  - Collar: desplazado a `cy-4s` — deja 1s de gap físico entre collar y cuerpo
  - Cuerpo: 6s → 5s alto, sigue siendo 5s de ancho — proporciones más limpias
  - Punta: `fillCircle(cx, cy+4s, 2s)` — centro en borde inferior del cuerpo; diámetro ecuatorial = ancho del cuerpo
  - `impl_probe_detail`: groove eliminado (reemplazado por gap físico), V-highlight ajustado a 3s alto
- **Skills activos:** `pbit-tft-screen`, `pixel-art-sprites`, `8-bit-pixel-art-patterns`, `icon-design`, `retro`
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

### Firmware — Icono DS18B20 v4: simetría vertical garantizada

- **Corregido:** `impl_probe` en `src/ui_icons.cpp` — collar ampliado de 5s a 7s de ancho (igual que `impl_probe_detail`): `fillRect(cx-(7s)/2, cy-3s, 7s, 2s)`. El collar sobresale 1s a cada lado del housing (5s) en todas las escalas.
- **Resultado:** simetría axial vertical perfecta — silueta idéntica a izquierda y derecha del eje cx. El V-highlight izquierdo (color/brillo) se mantiene asimétrico por diseño; solo la forma es simétrica.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

### Firmware — Icono DS18B20 v3: cable vertical centrado

- **Rediseñado (v3):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp` — tercera iteración: el cable pasa de diagonal (corner superior-izquierdo) a **vertical centrado** (sale del centro del collar hacia arriba). La silueta es ahora: cable recto ↑ + collar horizontal + cuerpo cilíndrico + punta redondeada ↓. Versión detallada (XXL) usa dos `drawFastVLine` adyacentes en color acento para cable 2px de grosor.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

## 2026-05-24

### Firmware — Rediseño iconos sensor

- **Rediseñado (v2):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp` — segunda iteración del icono DS18B20: ahora representa la probeta waterproof real (cápsula metálica con punta redondeada). Estructura: cable diagonal al corner superior-izquierdo → collar horizontal (anillo de conexión) → cuerpo cilíndrico recto → punta redondeada inferior. El cable en diagonal rompe toda simetría vertical. Versión XXL añade cable de 2px (acento), collar con groove, highlights metálicos blancos en housing y punta.
- **Ajustado:** `impl_sound` / `impl_sound_detail` — cápsula micrófono de 10s×10s (casi circular, r=4s) a 8s×10s (r=3s): más oval, reconocible como cápsula a tamaños pequeños.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.1%.

### Documentación — Auditoría Diataxis y mejoras por documento

- **Reescrito:** `docs/ROADMAP.md` — reestructurado con framework Now/Next/Later (`product-management:roadmap-update`); sección "Hecho" eliminada (contenido en `CHANGELOG.md`); nueva sección "Estado del firmware" con snapshot de métricas; tablas de validación hardware explícitas por área.
- **Reescrito:** `docs/DESIGN_SYSTEM.md` — reestructurado de formato análisis/propuesta a referencia canónica con `design:design-system`; nueva estructura: Tokens → Iconos → Anatomía → Patrones → Reglas → Estado → Historial; secciones "Problemas detectados" y "Propuestas" comprimidas en "Historial de decisiones".
- **Actualizado:** `docs/TFT_RENDER_RULES.md` — fecha footer actualizada a 2026-05-24.
- **Corregidas** referencias huérfanas en `docs/TECHNICAL.md` (l.728), `docs/ROADMAP.md` (l.35-37, 64) y `docs/DESIGN_SYSTEM.md` (l.381) apuntando a archivos renombrados o archivados.
- **Ampliado:** `docs/USER_GUIDE.md` sección 4 — reemplazada "Primer encendido" (4 bullets) por "Primeros pasos — guía de inicio rápido" completa (`user-guide-writing`): 7 pasos con prerequisitos, success check y tabla de siguientes pasos sugeridos.
- **Añadido:** `docs/TECHNICAL.md` — tabla de contenidos con 18 secciones enlazadas al inicio del documento (`engineering:documentation`).
- **Activados** skills: `embedded-systems`, `esp32-firmware-engineer`, `pbit-tft-screen`, `diataxis`, `user-guide-writing`, `design:design-system`, `product-management:roadmap-update`, `technical-roadmaps`, `ui-design-system`.

### Documentación — Reestructura completa

- Nueva estructura de documentación: raíz limpia con 3 archivos (`README.md`, `CHANGELOG.md`, `AGENTS.md`) y todos los docs en `docs/`.
- **Nuevo:** `docs/PROJECT.md` — biblia del producto: hardware, capacidades, usos, mapa de documentación.
- **Reescrito:** `docs/USER_GUIDE.md` — manual de producto con avisos de seguridad, precauciones, especificaciones, menús completos y solución de problemas.
- **Actualizado:** `docs/TECHNICAL.md` — absorbió contenido de `Menues.MD` (sección 17 con flujos de encoder y menús completos) y actualizó referencias internas.
- **Renombrado:** `PALETTE_AND_ICONS_PROPOSAL.md` → `docs/DESIGN_SYSTEM.md` (ya no es propuesta, es canon implementado).
- **Movidos a `docs/`:** `ROADMAP_PBIT.md` → `docs/ROADMAP.md`, `MANUAL_TECNICO_PBIT.md` → `docs/TECHNICAL.md`, `MANUAL_DE_USUARIO_PBIT.md` → `docs/USER_GUIDE.md`.
- **Archivados en `archive/`:** `LAB_GRAPH_UI_HANDOFF.md`, `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md`, `docs/DISPLAY_AUDIT.md`.
- **Eliminados:** `PBIT_FUNCIONAMIENTO_ACTUAL.md` (contenido distribuido en `PROJECT.md` y `USER_GUIDE.md`), `Menues.MD` (absorbido en `TECHNICAL.md`), `TFT_RENDER_RULES.md` de raíz (duplicado de `docs/`), `docs/REPO_HYGIENE.md` (efímero).
- **Actualizados:** `README.md` (simplificado a landing de 1 página), `AGENTS.md` (mapa de docs corregido).

### Firmware/UI

- Unificados los menús raíz de sensores y Sistema con `drawSettingsGridMenu()` en grid 2×3: opciones primarias arriba, `Reset` siempre abajo izquierda y `Salir` abajo derecha.
- Añadida card central para selectores/edición de menús (`ON/OFF`, `C/F`, límites y modos), con borde semántico por valor.
- Ajustada alineación vertical de valores dentro de cards de menú y diferenciados los resúmenes de texto frente al hint inferior mediante color/separador.
- Refinada pantalla `Sistema`: panels medio/inferior reposicionados, `ID` justificado izquierda/derecha y borde inferior de audio unificado para evitar lectura de doble card.
- Renombradas opciones raíz para evitar falsas calibraciones: `Luz > Rangos`, `Sonido > Niveles`, `Humedad > Rangos` y `Termómetro > Corrección / Límites / Unidad / Alertas`.
- Simplificada la edición de rangos de Suelo a dos umbrales (`Seco` y `Húmedo`); `Muy seco`, `Óptimo` y `Muy húmedo` se derivan automáticamente.
- Añadida persistencia NVS de la unidad global `C/F` mediante `sys_unit_f`; las pulsaciones cortas en Temperatura/Termómetro y los menús de unidad sobreviven reinicios.
- Reorganizada la pantalla `Sistema`: card principal con panels internos para `ID`, `Tiempo`, `Idioma`, audio y BLE condicional; el menú raíz pasa a grid 2×3.
- Ocultación BLE reforzada en `Sistema`: si `ble_en == false`, no se dibuja panel BLE ni estado `OFF`.
- Ajustada la progresión visual del icono de luz en diales con más pasos en el tramo bajo `0..1000 lux`.
- Corregidas zonas de limpieza/capa en cards para evitar que valores dinámicos tapen labels técnicos como `DS18B20`.
- Subidos los indicadores mínimo/máximo de diales y mantenido `DS18B20` como icono no final pendiente de rediseño/validación.

### Documentación

- Documentada la separación de `Sistema` entre `Bip` (`sys_sound`, beeps de UI) y `Alarmas` (`sys_alarm`, alertas y timer audibles).
- Actualizados términos visibles: `Sonido` en lugar de `Ruido`, `Gráfica` en lugar de abreviaturas `Graf`, y `Termómetro`/`Termo` para la sonda externa, manteniendo `DS18B20` como identificador técnico.
- Mantenida la activación BLE secreta fuera del manual de usuario; queda tratada solo como documentación técnica/agent handoff.
- Actualizados manuales, roadmap, checklist/release y escenas de Sistema del visualizador. Añadido `AGENTS.md` para futuros agentes.

## 2026-05-20

### Revisión de producción/i18n

- Build local verificado con `py -m platformio run -e esp32dev`: `SUCCESS`.
- Tamaño reportado por PlatformIO: RAM `14.7%` (`48028` bytes de `327680`) y Flash `70.6%` (`925873` bytes de `1310720`).
- Auditoría estática de i18n: `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)`, `L(...)` y full redraw al cambiar idioma presentes.
- Textos visibles migrados al diccionario de idiomas; la búsqueda de literales directos solo deja símbolos/no lingüísticos como cursores, separadores, placeholders y ticks numéricos.
- BLE confirmado como factory-off por defecto: `ble_en` carga `false`, `init_ble()` queda condicionado y el reset por build-hash limpia NVS en una build nueva.
- LDR documentado con rango lux `0..20000`, saturación alta a `20000 lux`, `ldr_raw` y modo `Raw ADC`.
- Sensor Zone consolidado como capa común para los seis sensores, con modos `Focus`, `Valor`, `Gráfica`, `Dial` y `Card`.
- Fixes anti-flicker documentados para dials/gauges, cards, menús/footers y `Sound VU`.
- `logs/xvba_debug.log` marcado como log local/no pertinente para este commit salvo decisión explícita del usuario.

### Documentación

- Actualizados `MANUAL_TECNICO_PBIT.md`, `CHANGELOG.md`, `docs/PRODUCTION_CHECKLIST.md`, `docs/PRODUCTION_RELEASE.md` y `docs/REPO_HYGIENE.md` con el estado de build, auditoría estática y pendientes de hardware real.
- Actualizados `README.md`, `MANUAL_DE_USUARIO_PBIT.md` y `PBIT_FUNCIONAMIENTO_ACTUAL.md` para reflejar el carrusel actual, Sensor Zone, BLE oculto, i18n completo, LDR `0..20000 lux`, Timer y reposo visible.
- Actualizados `ROADMAP_PBIT.md`, `docs/DISPLAY_AUDIT.md`, `LAB_GRAPH_UI_HANDOFF.md`, `PALETTE_AND_ICONS_PROPOSAL.md` y reglas TFT para separar trabajo resuelto, deuda de producto y validación pendiente en hardware.
- Actualizada documentación secundaria de visualizer (`ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md` y `visualizer_scenes/**/*.md`) para no arrastrar el carrusel antiguo ni rangos de LDR obsoletos.

## 2026-05-19

### Documentación

- Sincronizado el estado real del carrusel con `PBIT_ENABLE_GRAPH_LAB=1`: pantallas lab/producto iniciales y seis slots de sensor sobre `SENSOR_ZONE_SCREEN`.
- Documentada la gráfica como modo por sensor, no como pantalla separada del carrusel actual.
- Reforzado que BLE está oculto y desactivado por defecto, con acceso interno por gesto de 60 s en `Sistema`.
- Añadido checklist de producción en `docs/PRODUCTION_CHECKLIST.md`.
- Añadidas notas de repo hygiene en `docs/REPO_HYGIENE.md`.

### Firmware

- El arranque limpia NVS por build-hash antes de cargar BLE y settings, evitando que una unidad reflasheada anuncie BLE con estado anterior.
- BLE usa buffers fijos, bitmap de validez y servicio rate-limited desde la tarea de sensores; se evita `String` en el camino caliente y la notificación desde callbacks.
- LDR usa lectura ADC protegida, arranca con valores inválidos explícitos y mantiene el rango de luz documentado para producción.
- Luz y sonido ignoran lecturas inválidas en alertas y UI, evitando falsos estados de sol directo o sonido crítico.
- Lecturas ADC compartidas pasan por un guard común para reducir carreras entre calibración UI y tarea de sensores.
- El buzzer deja de llamar LEDC dentro de secciones críticas; usa mutex de tarea.
- Alert jewels de Temp/Humedad/DS18/Luz/Sonido se repintan al cambiar pantalla y se reducen clears que podían pisarlos.
