# P-Bit Roadmap

Actualizado: 2026-06-03

Referencia para priorizar el trabajo futuro del firmware P-Bit. Separa deuda técnica de mejoras de producto y ordena iteraciones de menús, sensores y UX.

El historial de lo ya implementado está en `CHANGELOG.md`. Este documento cubre solo lo que queda por hacer.

---

## Estado del firmware (snapshot actual)

| Indicador | Valor |
|---|---|
| Build | ✅ `esp32dev` — sin errores |
| RAM | 14.9 % (`48 940` / `327 680` bytes) |
| Flash | 72.1 % (`945 429` / `1 310 720` bytes) |
| Carrusel activo | 12 pantallas con `PBIT_ENABLE_GRAPH_LAB=1` |
| Idiomas | ES / CAT / EN — i18n completo |
| BLE | Factory-off — gesto secreto 30 s en `Sistema` |
| Validación hardware | **En cierre** — ghosting/flicker resuelto por ahora; LDR modos, Demo smooth y estados externos `IO33/IO35` implementados en firmware, pendientes de validación visual final |

---

## Ahora — Cierre en hardware real

Todo el trabajo activo converge aquí. El ghosting/flicker de pantallas queda aprobado por ahora; mantener vigilancia de regresión mientras se validan en hardware los modos LDR, Demo Mode y producción.

### Anti-flicker visual

Estado: resuelto por ahora en hardware real. Usar esta tabla como checklist de regresión si se modifican pantallas, demo o reglas de limpieza.

| Pantalla | Qué verificar |
|---|---|
| `Sound VU` (STACK + WAVE) | Scroll continuo, idle pulse en silencio, sin congelación periódica |
| `DIAL / GAUGE` | Ring sprite sin vibración; label de unidad restaurado después de `pushSprite()` |
| `SENSOR CARD` | Header, valor, viz y footer sin recortes por clears dinámicos |
| `VALOR` | Sparkline y barra segmentada sin flash negro; contraste suficiente |
| `Graph` | Etiquetas min/max y línea principal legibles en ES/CAT/EN |
| `HOME` | Cards sin redibujo completo en cada tick |

### BLE y producción

- [ ] Escaneo BLE externo no ve advertising `PBIT-XXXX` en estado de producción
- [ ] En unidad reflasheada sobre build de desarrollo: primer arranque sin advertising
- [ ] Fila BLE solo visible en `Sistema` cuando `ble_en == true`

### Sensores

- [x] LDR: coherencia `Lux`/`FC`/`Raw ADC` implementada en firmware para Luz, Sensor Zone, cards, dashboards, dials y gráficas
- [x] LDR: curva empírica v1 aplicada desde muestra manual RAW/luxómetro; `FC` deriva de lux con `lux / 10.764`
- [ ] LDR: validar rango `0..8000 lux` plausible en entorno real; RGB permanece apagado en vista de luz
- [ ] DHT11: temperatura y humedad con valores plausibles
- [x] DS18B20/Suelo: estado runtime común de ausencia implementado con `Revisa IO33` / `Revisa IO35` y paleta atenuada
- [ ] DS18B20: detección correcta de presencia/ausencia de sonda en hardware y textos `IO33` legibles en ES/CAT/EN
- [ ] Micrófono: respuesta visible en `Sound VU` y pantalla `Sonido`
- [ ] Suelo: detección de ausencia de sensor en hardware, textos `IO35` legibles en ES/CAT/EN y calibración seco/agua funciona

### Navegación e i18n

- [ ] Carrusel completo recorrido — legibilidad de todos los modos
- [ ] `SENSOR_ZONE_SCREEN`: encoder cambia sensor, pulsación corta cambia vista, larga abre menú del sensor activo
- [ ] Cabeceras localizadas de Sensor Zone no se recortan en ES/CAT/EN (`TERMÓMETRO`, `TEMPORIZADOR`, `ALARMAS`)
- [ ] Selector de idioma aparece en cold boot tras NVS limpiada
- [ ] `Timer`: corto/largo responden según diseño; alarma audible al finalizar si `Alarmas` activo
- [ ] `Sistema > Bip` silencia beeps de UI sin silenciar alertas; `Alarmas` silencia alertas/timer audibles sin ocultar alertas visuales/RGB
- [ ] Reposo visible con `ZZZ`; despierta con encoder
- [x] Modo demo: entrada desde logos con encoder presionado, entrada desde `Home` con pulsación larga, señal visual y salida con giro/pulsación
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
| **Paleta HOME/CLIMA** | Decidir si `HOME` y `CLIMA LAB` migran a `include/palette.h` o mantienen colores responsivos propios. | Validación Sensor Zone en HW |
| **P3/P4 en hardware** | Verificar legibilidad de labels min/max y segmentos apagados sobre ST7735 real. | Validación hardware |

### Deuda técnica post-auditoría (2026-06-03)

Hallazgos de la revisión profunda de junio 2026 (Fases 0–5). Ninguno es bug ni bloqueante para producción; se documentan para abordar en ciclos futuros cuando se toquen los archivos por otra razón.

| Ítem | Descripción | Severidad | Disparador sugerido |
|---|---|---|---|
| ~~**Magic colors `0x1082`**~~ | ✅ **Resuelto 2026-06-03.** `src/ui_icons.cpp` usa `kIconCardBg = 0x1082` (constante local documentada). | — | — |
| ~~**Magic colors `0x0841`**~~ | ✅ **Resuelto 2026-06-03.** `src/ui_lab_home_cards.cpp` usa `kCardBg = 0x0841` (constante añadida al namespace). | — | — |
| ~~**Rename `render_global_alert_badge`**~~ | ✅ **Resuelto 2026-06-03.** Renombrada a `update_rgb_led_state()` en `src/tft_display.cpp`. | — | — |
| **`struct Cache` canónico** | `src/ui_lab_focus.cpp` y `src/ui_graph.cpp` usan `static last_*` sueltas en lugar del patrón `struct *Cache` documentado en `docs/TFT_RENDER_RULES.md` Nivel 0. Las pantallas clásicas (`ui_humidity`, `ui_light`, `ui_sound`, `ui_soil`, `ui_temp`, `ui_ds18`) usan `meta_dirty` que es aceptado por la regla. No hay flicker activo. | Baja (estilo) | Próximo refactor de Focus/Graph |
| **RGB mapping extraíble** | `src/tft_display.cpp` contiene ~170 líneas de `apply_*_rgb` que lógicamente pertenecen a `led_control.cpp`. Acoplado a `active_screen` y `g_ui_readings_snapshot`. Extracción requiere refactor de interfaz. | Baja | Si `tft_display.cpp` crece más allá de ~1100 líneas |
| ~~**NVS fuera de `settings_store.cpp`**~~ | ✅ **Resuelto 2026-06-03.** `src/lang_select.cpp` delega en `load_language_store()` / `save_language_store()`. NVS 100% encapsulado. | — | — |
| **Stack HWM validación HW** | `switch_screen` y `sensor_reading_task` usan stack de `4096` bytes. Suficiente para el código actual. La instrumentación ya existe bajo `FIRMWARE_DEBUG` (`src/tft_display.cpp:940`, `src/io.cpp:118`). Pendiente: activar `FIRMWARE_DEBUG` en build de validación y registrar el HWM real en condiciones de uso normal. | Media (validación) | Sesión de validación 24 h con `FIRMWARE_DEBUG=1` |

> **Política**: estos ítems NO se abordan en commits aislados. Cada uno se resuelve cuando se edite el archivo por otra razón (feature, fix, refactor planeado), siempre dentro del mismo PR para mantener trazabilidad. La excepción es el stack HWM, que requiere build dedicado con flag de debug.

---

## Después — Iteraciones de producto

Ideas acordadas con valor educativo y de producto; sin urgencia técnica.

### UX y feedback

- **Demo guion educativo**: añadir secuencias temáticas futuras (aula, planta, exterior) sobre la coreografía smooth ya implementada.
- **Gamificación de alertas**: arcoíris rápido + sonido feliz al pasar a estado óptimo desde alerta.
- **Alertas globales en pantalla**: la lógica (`AlertEngine`) y el RGB ya funcionan. Falta decidir una posición de layout que no invada ninguna pantalla y reactivar la capa visual.
- **Calibración real de sonido**: el menú actual usa `Niveles` interpretativos. Decidir si más adelante se añade una calibración acústica real del entorno.

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
