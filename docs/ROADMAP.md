# P-Bit Roadmap

Actualizado: 2026-08-25

Referencia para priorizar el trabajo futuro del firmware P-Bit. Separa deuda técnica de mejoras de producto y ordena iteraciones de menús, sensores y UX.

El historial de lo ya implementado está en `CHANGELOG.md`. Este documento cubre solo lo que queda por hacer.

---

## Estado del firmware (snapshot actual)

| Indicador | Valor |
|---|---|
| Build | ✅ `esp32dev` — sin errores |
| RAM | 15.0 % (`49 140` / `327 680` bytes) |
| Flash | 73.4 % (`961 709` / `1 310 720` bytes) |
| Alimentación | Producto recomendado con `3 baterías AAA`; USB-C para programación/alimentación auxiliar |
| Carrusel activo | 11/12 pantallas con `PBIT_ENABLE_GRAPH_LAB=1` (`PLANTA LAB` aparece solo con Suelo conectado) |
| Idiomas | ES / CAT / EN — i18n completo |
| WiFi/BLE | WiFi sin configuración en firmware inicial; BLE factory-off interno, no público |
| Validación hardware | **En cierre** — ghosting/flicker resuelto por ahora; LDR modos, Demo smooth y estados externos `IO33/IO35` implementados en firmware, pendientes de validación visual final |

---

## Ahora — Cierre en hardware real

Todo el trabajo activo converge aquí. El ghosting/flicker de pantallas queda aprobado por ahora; mantener vigilancia de regresión mientras se validan en hardware los modos LDR, Demo Mode y producción.

### Anti-flicker visual

Estado: resuelto por ahora en hardware real. Usar esta tabla como checklist de regresión si se modifican pantallas, demo o reglas de limpieza.

| Pantalla | Qué verificar |
|---|---|
| `Sonido VU/Onda` | Scroll continuo, idle pulse en silencio, sin congelación periódica |
| `DIAL / GAUGE` | Ring sprite sin vibración; label de unidad restaurado después de `pushSprite()` |
| `SENSOR CARD` | Header, valor, viz y footer sin recortes por clears dinámicos |
| `VALOR` | Sparkline y barra segmentada sin flash negro; contraste suficiente |
| `Graph` | Etiquetas min/max y línea principal legibles en ES/CAT/EN |
| `Inicio` | Cards sin redibujo completo en cada tick |
| `PLANTA LAB` | Barras+valores legibles en 96 px; `ASSEDEGADA` cabe; animación sin parpadeo; azul `AHOGADA` contrasta sobre navy |

### BLE y producción

- [ ] Escaneo BLE externo no ve advertising `PBIT-XXXX` en estado de producción
- [ ] En unidad reflasheada sobre build de desarrollo: primer arranque sin advertising
- [ ] Fila BLE solo visible en `Sistema` cuando `ble_en == true`
- [ ] Confirmar que guías públicas no documentan el flujo interno de activación BLE

### Alimentación

- [ ] Validar arranque, navegación y reposo con `3 baterías AAA` nuevas
- [ ] Confirmar USB-C como programación/auxiliar 5 V, no alimentación principal de producto

### Sensores

- [x] LDR: coherencia `Lux`/`FC`/`Raw ADC` implementada en firmware para Luz, Sensor Zone, cards, dashboards, dials y gráficas
- [x] LDR: curva empírica v1 aplicada desde muestra manual RAW/luxómetro; `FC` deriva de lux con `lux / 10.764`
- [ ] LDR: validar rango `0..8000 lux` plausible en entorno real; RGB permanece apagado en vista de luz
- [ ] DHT11: temperatura y humedad con valores plausibles
- [x] DS18B20/Suelo: estado runtime común de ausencia implementado con `Revisa IO33` / `Revisa IO35` y paleta atenuada
- [ ] DS18B20: detección correcta de presencia/ausencia de sonda en hardware y textos `IO33` legibles en ES/CAT/EN
- [ ] Micrófono: respuesta visible en `Sonido VU`, `Sonido Onda` y pantalla `Sonido`
- [ ] Suelo: detección de ausencia de sensor en hardware, textos `IO35` legibles en ES/CAT/EN y calibración seco/agua funciona
- [ ] Planta Lab: validar aparición condicional con Suelo conectado, barras de rango, texto `ASSEDEGADA`, animación sin parpadeo y contraste del estado `AHOGADA`

### Navegación e i18n

- [ ] Carrusel completo recorrido — legibilidad de todos los modos
- [ ] `SENSOR_ZONE_SCREEN`: encoder cambia sensor, pulsación corta cambia vista, larga abre menú del sensor activo
- [ ] Cabeceras localizadas de Sensor Zone no se recortan en ES/CAT/EN (`TERMÓMETRO`, `TEMPORIZADOR`, `ALARMAS`)
- [ ] Selector de idioma aparece en cold boot tras NVS limpiada
- [ ] `Timer`: corto/largo responden según diseño; alarma audible al finalizar si `Alarmas` activo
- [ ] `Sistema > Bip` silencia beeps de UI sin silenciar alertas; `Alarmas` silencia alertas/timer audibles sin ocultar alertas visuales/RGB
- [ ] Reposo visible con `ZZZ`; despierta con encoder
- [x] Modo demo: entrada desde logos con encoder presionado, entrada desde `Inicio` con pulsación larga, señal visual y salida con giro/pulsación
- [x] Modo demo: suavizar ritmo, intención y coreografía visual para evitar sensación de cambios bruscos o sin propósito
- [ ] Modo demo: validación visual final en hardware para confirmar que la nueva cadencia no reintroduce flicker

### Checklist formal

Firmar `docs/PRODUCTION_CHECKLIST.md` completo antes de entregar build o unidad.

---

## Próximo — Mejoras técnicas post-validación

Listas para implementar una vez que el hardware esté validado.

### Hardening técnico

| Ítem | Descripción | Prioridad |
|---|---|---|
| **TWDT** | Task Watchdog Timer para `sensor_reading_task` y `switch_screen`. Timeout sugerido: 10 s. Requiere test de 24 h continuas en hardware. Usar el Modo demo runtime como base del ciclo continuo. | Alta |
| **LDR calibración HW** | Validar `Lux / FC / Raw ADC` contra luz/sombra reales y ajustar la curva empírica v1 si el rango `0..8000 lux` no se siente plausible. | Alta |
| **Demo validación HW** | Probar la coreografía smooth en ST7735 real y ajustar dwell/refresco si aparece flicker o cambios demasiado lentos. | Alta |
| **Renombrar flag** | `PBIT_ENABLE_GRAPH_LAB` → `PBIT_ENABLE_FULL_NAV` (u otro nombre representativo). Limpiar referencias `LAB_` en enums y constantes de `tft_display.h` y `rotary.cpp`. | Media |

### Design System

| Ítem | Descripción | Dependencia |
|---|---|---|
| **Paleta Inicio/Clima** | Decidir si `Inicio` y `Clima Lab` migran a `include/palette.h` o mantienen colores responsivos propios. | Validación Sensor Zone en HW |
| **P3/P4 en hardware** | Verificar legibilidad de labels min/max y segmentos apagados sobre ST7735 real. | Validación hardware |

### Deuda técnica post-auditoría (2026-06-03)

Hallazgos de la revisión profunda de junio 2026 (Fases 0–5). Ninguno es bug ni bloqueante para producción; se documentan para abordar en ciclos futuros cuando se toquen los archivos por otra razón.

| Ítem | Descripción | Severidad | Disparador sugerido |
|---|---|---|---|
| ~~**Magic colors `0x1082`**~~ | ✅ **Resuelto 2026-06-03.** `src/ui_icons.cpp` usa `kIconCardBg = 0x1082` (constante local documentada). | — | — |
| ~~**Magic colors `0x0841`**~~ | ✅ **Resuelto 2026-06-03.** `src/ui_lab_home_cards.cpp` usa `kCardBg = 0x0841` (constante añadida al namespace). | — | — |
| ~~**Rename `render_global_alert_badge`**~~ | ✅ **Resuelto 2026-06-03.** Renombrada a `update_rgb_led_state()` en `src/tft_display.cpp`. | — | — |
| ~~**`struct Cache` canónico**~~ | ✅ **Resuelto 2026-06-03.** `src/ui_lab_focus.cpp` usa `struct FocusCache g_cache`. `src/ui_graph.cpp` usa `struct GraphCache cache` (locale a la función). Las pantallas clásicas siguen con `meta_dirty` aceptado por la regla Nivel 0. | — | — |
| **RGB mapping extraíble** | `src/tft_display.cpp` contiene ~289 líneas de mapping LED RGB (`set_rgb565`, `apply_*_visual_rgb`, `apply_global_alert_rgb`, `update_rgb_led_state`). **NO mover a `led_control.cpp`** — invertiría capas (driver dependiendo de UI/sensor_zone/alert_engine). Si en el futuro se extrae, hacerlo a un módulo nuevo `ui_led_feedback.cpp/.h` con interfaz pública única `update_rgb_led_state()`. Decisión documentada en ADR-001 (2026-06-03): se evaluó mover ahora, se descartó porque (a) el trigger no ha saltado, (b) es pre-flash y hardware-visible, (c) `g_is_fahrenheit` sin header propio convierte el move en diseño no trivial. | Baja | Si `tft_display.cpp` crece más allá de ~1100 líneas |
| ~~**NVS fuera de `settings_store.cpp`**~~ | ✅ **Resuelto 2026-06-03.** `src/lang_select.cpp` delega en `load_language_store()` / `save_language_store()`. NVS 100% encapsulado. | — | — |
| **Stack HWM validación HW** | `switch_screen` y `sensor_reading_task` usan stack de `4096` bytes. Suficiente para el código actual. **Infraestructura lista (2026-06-04):** env `esp32dev_debug` definido en `platformio.ini` con `-DFIRMWARE_DEBUG`, instrumentación periódica añadida en `src/tft_display.cpp` y `src/io.cpp` (muestreo cada `1000 ms`, log cuando el worst empeora o cada `60 s`, valores en bytes). Procedimiento documentado en `docs/TECHNICAL.md` § A.5. **Pendiente:** flashear con `pio run -e esp32dev_debug -t upload`, correr en uso real prolongado, registrar el peor HWM observado en `docs/PRODUCTION_RELEASE.md`. El ítem queda abierto hasta tener esa medida real en hardware. | Media (validación) | Ejecutar `esp32dev_debug` y registrar HWM tras sesión de uso prolongado |

> **Política**: estos ítems NO se abordan en commits aislados. Cada uno se resuelve cuando se edite el archivo por otra razón (feature, fix, refactor planeado), siempre dentro del mismo PR para mantener trazabilidad. La excepción es el stack HWM, que requiere build dedicado con flag de debug.

---

## Después — Iteraciones de producto

Ideas acordadas con valor educativo y de producto; sin urgencia técnica.

### UX y feedback

- **Demo guion educativo**: añadir secuencias temáticas futuras (aula, planta, exterior) sobre la coreografía smooth ya implementada.
- **Gamificación de alertas**: arcoíris rápido + sonido feliz al pasar a estado óptimo desde alerta.
- **Alertas globales en pantalla**: la lógica (`AlertEngine`) y el RGB ya funcionan. Falta decidir una posición de layout que no invada ninguna pantalla y reactivar la capa visual.
- **Calibración real de sonido**: el menú actual usa `Límites` interpretativos, no una calibración acústica real. Decidir si más adelante se añade una calibración del entorno.

### Timer y laboratorio

- **Timer orientado a laboratorio**: automatizaciones básicas o flujos de experimento. La v2 (cronómetro + editor `HH:MM:SS`) ya es producto completo; la v3 es una decisión de producto, no técnica.

### Idiomas

- **Cuarto idioma**: la arquitectura `LANG_COUNT` ya lo soporta. Requiere principalmente `include/languages.h` y `src/lang_select.cpp`; validar textos largos en 160 px.

---

## Roadmap paralelo: Visualizador / TFT Workstation

Herramienta de desarrollo externa para iterar layouts antes de tocar hardware real. No es firmware embebido.

### Estado actual

- Biblioteca de escenas canon activa; interpreta variables, expresiones, `textWidth()`, mutaciones y strings con ceros a la izquierda.
- Flujo de inspección completo: inspector editable, drag-and-drop, nudges, checkpoints, compare mode, undo/redo.

### Próximas mejoras de mayor valor

| Mejora | Objetivo | Prioridad |
|---|---|---|
| Helpers del firmware | Soportar `drawHeader()`, `drawCard()`, `L(KEY)` directamente en snippets | Alta |
| Variables de escena editables | Cambiar valores de runtime sin duplicar escenas | Alta |
| Pipeline de iconos | `drawBitmap()`, `pushImage()`, importación PNG → array C | Media |
| Diff visual / golden screenshots | Comparar render actual vs captura aprobada | Media |
| Suite de calibración visual | Escenas de test por tipo: tipografía, sprites, primitivas, widgets | Media |

### Clasificación de fallos visuales

| Categoría | Cuándo aplica |
|---|---|
| `Firmware` | Mismo fallo en hardware real Y en visualizador |
| `Visualizador` | Fallo solo en el visualizador |
| `Escena` | La prueba no aísla bien el problema |
| `Mixto` | Debilidad en la escena + fallo real de motor o firmware |

---

## Principios de trabajo

1. Mantener estables los menús ya existentes antes de añadir más complejidad.
2. Resolver solo una iteración nueva por pantalla cuando haya una necesidad clara.
3. Revisar primero si el sensor admite calibración real o solo umbrales de interpretación.
4. Consolidar visualmente el modo Gráfica actual antes de ampliar sensores o modos.
5. No volver a planificar como futuro lo que ya está implementado en código.
6. Si una pantalla tiene subestados, persistencia en NVS y feedback consistente → base estable.
7. Antes de añadir una mejora nueva, verificar que no afecta al encoder, al sleep ni a la legibilidad visual del conjunto.
