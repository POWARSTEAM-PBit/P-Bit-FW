# Changelog

## 2026-05-24

### Firmware/UI

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
