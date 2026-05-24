# Changelog

## 2026-05-24

### Firmware — Rediseño iconos sensor

- **Rediseñado:** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp` — icono DS18B20 reemplazado por silueta chip TO-92: dos barras de emisión térmica arriba, cuerpo en forma D (cúpula + cara plana), tres pines GND/DATA/VDD abajo. Diseño inequívoco con valor educativo. Elimina la forma anterior que era confundible con figuras inapropiadas.
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
