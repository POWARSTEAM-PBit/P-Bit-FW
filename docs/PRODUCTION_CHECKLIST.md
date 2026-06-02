# Checklist de Producción P-Bit

Actualizado: 2026-05-28

Usar esta lista antes de entregar una build o una unidad flasheada.

Estado de esta revisión: build y auditoría estática completados; Demo Mode y anti-flicker/ghosting validados puntualmente en hardware; pruebas completas por unidad pendientes.

## 1. Configuración de build

- [x] `py -m platformio run -e esp32dev` compila sin errores.
- [x] Resultado de build registrado: RAM `14.7%` (`48156`/`327680`) y Flash `71.5%` (`936653`/`1310720`).
- [x] `include/config.h`: `PBIT_ENABLE_GRAPH_LAB=1` para el carrusel actual con `Home/Clima/Multi/Sonido VU` y `SENSOR_ZONE_SCREEN`.
- [x] `PBIT_ENABLE_SERIAL_PLOTTER=0` para producción.
- [x] `FIRMWARE_DEBUG` sigue comentado.
- [x] `platformio.ini` conserva flags de build silenciosos/optimizados (`-Os`, `CORE_DEBUG_LEVEL=1`, `CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL=1`).
- [ ] Si `PBIT_ENABLE_GRAPH_LAB=0` en una build futura, documentar explícitamente que se entrega el carrusel clásico `TEMP_SCREEN -> GRAPH_SCREEN`.

## 2. Auditoría estática de UI/i18n

- [x] `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)` y `L(...)` presentes en la capa de idioma.
- [x] Cambio de idioma solicita full redraw con `runtime_request_ui_full_redraw()` y el loop de UI lo consume con `runtime_take_ui_full_redraw()`.
- [x] Textos visibles migrados a `L(...)`/`LIn(...)`; literales directos restantes son símbolos/no lingüísticos (`>`, `---`, separadores, ticks numéricos o `ZZZ`).
- [x] Carrusel actual con `PBIT_ENABLE_GRAPH_LAB=1`: `Home -> Clima -> Multi -> Sonido VU -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`.
- [x] `SENSOR_ZONE_SCREEN` reutiliza Sensor Zone para los seis sensores y persiste modos `Focus -> Valor -> Gráfica -> Dial -> Card`.
- [x] `Sistema` separa `Bip` y `Alarmas`; la auditoría estática confirma persistencia `sys_sound`, `sys_alarm` y `sys_unit_f` para unidad C/F.
- [x] Anti-flicker/ghosting revisado por código y validado por ahora en hardware en dials/gauges, cards, menús/footers y `Sound VU`; mantener vigilancia de regresión.
- [x] Modo demo runtime añadido: arranque con encoder presionado durante logo o pulsación larga desde `Home`, splash breve de entrada, sin persistir sensor/modo en NVS.
- [x] Menús de settings revisados con helpers comunes: valores centrados con fallback de fuente y summaries más separados del footer.
- [ ] En hardware: arranque frío muestra selector de idioma si la NVS fue limpiada.
- [ ] En hardware: recorrer carrusel completo y verificar legibilidad de todos los modos.
- [ ] En hardware: en cada sensor, pulsación corta cambia modo y pulsación larga abre el menú del sensor activo.
- [ ] En hardware: `Timer` responde a corto/largo según diseño.
- [ ] En hardware: cambiar C/F, reiniciar y confirmar persistencia de unidad.
- [ ] En hardware: `Sistema > Bip` silencia beeps de UI sin silenciar `Alarmas`; `Sistema > Alarmas` silencia alertas/timer audibles sin ocultar alertas visuales/RGB.
- [ ] En hardware: reposo visible con `ZZZ`; despierta con interacción del encoder.
- [x] En hardware: encender con encoder presionado durante logo activa Modo demo; pulsación larga desde `Home` también lo activa; cualquier interacción posterior sale del demo.
- [x] UX Demo Mode: coreografía/ritmo suavizados en firmware con dwell variable, curvas por sensor y refresco dedicado.
- [ ] UX Demo Mode: validar en hardware que la coreografía smooth no reintroduce flicker ni cambios demasiado lentos.

## 3. BLE

- [x] Estado esperado de fábrica por código: BLE `OFF`.
- [x] `load_ble_enabled_store()` usa default `false` para `ble_en`.
- [x] Reset por build-hash limpia el namespace NVS antes de evaluar BLE en una build nueva.
- [x] `init_ble()` solo se ejecuta si `load_ble_enabled_store()` devuelve `true`.
- [x] La pantalla `Sistema` no muestra fila BLE cuando `ble_en == false`.
- [x] La pantalla oculta BLE solo aparece manteniendo el encoder 30 s en `Sistema` según auditoría estática.
- [ ] Un escaneo BLE externo no ve publicidad `PBIT-XXXX` en estado de producción.
- [ ] Si se activó BLE para pruebas, volver a `OFF`, confirmar reinicio y repetir escaneo.
- [ ] En unidad física reflasheada sobre desarrollo, confirmar primer arranque sin publicidad `PBIT-XXXX`.

## 4. Sensores y feedback

- [x] LDR base técnica revisada por código: lux interno limitado a `0..8000`, curva empírica v1 desde RAW promediado, `FC = lux / 10.764` y `Raw ADC` disponible para recalibración.
- [ ] DHT11 muestra temperatura y humedad plausibles.
- [ ] LDR responde a sombra/luz y queda razonablemente cercano al luxómetro tras reflashear; el RGB permanece apagado en la vista de luz.
- [x] LDR propaga unidad y valor visible de forma coherente según modo (`lux`, `FC`, `raw`) en Luz, Sensor Zone, cards, dashboards, gráficas y dials.
- [ ] LDR muestra variación visible del icono de luz en el tramo bajo de luz ambiental.
- [ ] Micrófono responde en `Sonido VU` y `Sonido`.
- [ ] Suelo detecta ausencia de sensor, muestra `Revisa IO35` sin solapes en ES/CAT/EN y permite calibración seco/agua.
- [ ] Termómetro (`DS18B20`) detecta ausencia/presencia de sonda y muestra `Revisa IO33` sin solapes en ES/CAT/EN.
- [ ] Conexión/desconexión Suelo: al cambiar de estado después del arranque aparece splash verde/rojo `CONECTADO/DESCONECTADO / Sensor Suelo / IO35` y vuelve a la pantalla anterior.
- [ ] Conexión/desconexión Termómetro: al cambiar de estado después del arranque aparece splash verde/rojo `CONECTADO/DESCONECTADO / Sensor DS18B20 / IO33` y vuelve a la pantalla anterior.
- [x] Icono `temp` final: small/XL/fallback monocromos; solo Dial/XXL con acento mantiene detalle multicolor.
- [ ] Icono técnico `probe`/DS18B20 revisado en hardware; estado actual no final.
- [ ] Alertas visuales, RGB y audio de `Alarmas` se prueban al menos en un sensor crítico.

## 5. Entrega y repo hygiene

- [x] No incluir `.pio/`, logs locales, `.zip`, `.bin`, `.elf`, `.map`, `.exe` ni artefactos temporales nuevos en esta actualización documental.
- [x] Revisar `git status --short` antes de cerrar la build.
- [x] Registrar cambios de documentación en `CHANGELOG.md`.
- [x] `logs/xvba_debug.log` identificado como no pertinente para este commit salvo decisión del usuario.
- [ ] Si se genera un paquete de release, separar binarios/ejecutables grandes de la documentación cuando se haga la pasada de higiene planificada.
