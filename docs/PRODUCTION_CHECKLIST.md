# Checklist de Producción P-Bit

Actualizado: 2026-10-11

Usar esta lista antes de entregar una build o una unidad flasheada.

Estado de esta revisión: build `esp32dev` comprobada el `2026-10-11` sobre base `3744f04` + cambios locales de mascota y refinamiento visual de Planta Lab, con RAM estática `59580` y Flash `971189`. El usuario comunicó una impresión favorable de la primera mascota en P-Bit; después aprobó en P-Bit colores actuales de planta/medidores y valoró favorablemente suelo progresivo, sin completar toda la validación física. Los checks HW marcados conservan evidencia histórica puntual y no certifican el candidato actual; ver `docs/PRODUCTION_RELEASE.md`.

Prioridad acordada: cerrar el refinamiento visual de Planta Lab en TFT real, dejando configuración/presets para después e incluyendo recuperación y regresión de flicker/Demo. La prueba final de 24 h y la validación física completa por unidad quedan pendientes y aplazadas por el usuario. Este checklist sigue abierto y no constituye aprobación de producción.

## 1. Configuración de build

- [x] `py -m platformio run -e esp32dev` compila sin errores.
- [x] Resultado de build registrado: RAM `18.2%` (`59580`/`327680`) y Flash `74.1%` (`971189`/`1310720`).
- [x] `include/config.h`: `PBIT_ENABLE_GRAPH_LAB=1` para el carrusel actual con `Inicio/Clima Lab/Planta Lab/Termo Lab` y `SENSOR_ZONE_SCREEN`.
- [x] `PBIT_ENABLE_SERIAL_PLOTTER=0` para producción.
- [x] `FIRMWARE_DEBUG` sigue comentado.
- [x] `platformio.ini` conserva flags de build silenciosos/optimizados (`-Os`, `CORE_DEBUG_LEVEL=1`, `CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL=1`).
- [ ] No entregar como producción una build con `PBIT_ENABLE_GRAPH_LAB=0`; cualquier build experimental debe documentarse fuera del paquete de producción.

## 2. Alimentación y conectividad de producto

- [ ] Validar arranque y navegación básica con `3 baterías AAA` nuevas; esta es la alimentación recomendada para entrega/aula.
- [ ] Confirmar que el portapilas hace buen contacto y que no hay reinicios al mover suavemente el equipo.
- [ ] Verificar USB-C para programación, monitor serie y alimentación auxiliar 5 V desde PC/powerbank.
- [ ] No presentar USB-C/powerbank como alimentación principal recomendada en material de usuario.
- [ ] Confirmar que la documentación pública describe WiFi/Bluetooth como capacidades del ESP32 sin flujo de configuración en el firmware inicial.
- [ ] Confirmar que ninguna guía pública describe el gesto interno de activación BLE.

## 3. Auditoría estática de UI/i18n

- [x] `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)` y `L(...)` presentes en la capa de idioma.
- [x] Cambio de idioma solicita full redraw con `runtime_request_ui_full_redraw()` y el loop de UI lo consume con `runtime_take_ui_full_redraw()`.
- [x] Textos visibles migrados a `L(...)`/`LIn(...)`; literales directos restantes son símbolos/no lingüísticos (`>`, `---`, separadores, ticks numéricos o `ZZZ`).
- [x] Carrusel actual con `PBIT_ENABLE_GRAPH_LAB=1`: `Inicio -> Clima Lab -> Planta Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`; `Planta Lab` se omite si Suelo no tiene lectura válida.
- [x] `SENSOR_ZONE_SCREEN` reutiliza Sensor Zone para los seis sensores y persiste modos visibles `Principal -> Rango -> Ficha -> Dato -> Curva`; `Sonido` añade `Sonido VU` y `Sonido Onda` justo después de `Principal`.
- [x] `Sistema` separa `Bip` y `Alarmas`; la auditoría estática confirma persistencia `sys_sound`, `sys_alarm` y `sys_unit_f` para unidad C/F.
- [x] Anti-flicker/ghosting revisado por código y validado por ahora en dials/gauges, cards, menús/footers y `Sonido VU/Onda`; mantener vigilancia de regresión.
- [x] Modo demo runtime añadido: arranque con encoder presionado durante logo o pulsación larga desde `Inicio`, splash breve de entrada, sin persistir sensor/modo en NVS.
- [x] Menús de settings revisados con helpers comunes: valores centrados con fallback de fuente y summaries más separados del footer.
- [ ] En hardware: arranque frío muestra selector de idioma si la NVS fue limpiada.
- [ ] En hardware: recorrer carrusel completo y verificar legibilidad de todos los modos.
- [ ] En hardware: con Suelo conectado, `Planta Lab` aparece entre `Clima Lab` y `Termo Lab`; sin Suelo, se salta sin dejar posición fantasma.
- [x] Planta Lab software: Modelo nativo, incluidos casos térmicos nuevos, 1215 combinaciones del raster, 101 niveles progresivos de Suelo y 4 lecturas ausentes/no finitas; 138 comparaciones frame parcial/completo PASS. Nueve estados, once niveles de suelo, luz/temperatura y GIF de movimiento revisados por software; 48 frames adicionales verifican ondas repartidas y escalonadas. Usuario acepta el preview general de sol/luna y calor distribuido; frío/aire distribuidos autorizados e integrados: 528 comparaciones host PASS y 528 frames idénticos al preview; etiquetas ES/CAT/EN medidas en el refinamiento previo sin cambiar fuentes/textos. No equivale a validación física.
- [x] Usuario aprueba en P-Bit colores de planta/medidores y valora favorablemente suelo progresivo; aceptación puntual, sin completar el checklist visual.
- [ ] En hardware: `Planta Lab` — nueve poses, verde persistente, movimiento/parpadeo, recuperación y desconocidos; necesidades resaltadas y dominante con doble borde. Barras por sensor y extremos oscurecidos; tierra café que se aclara/agrieta al secarse y se oscurece antes de ampliar manchas de agua. Sol amarillo de cuatro tamaños, luna circular solo <20 lux y ondas térmicas 1..4 repartidas a distintas alturas/lados del terrario, sin invadir lecturas. Comprobar sonrisa a 26/27/30 °C con otros sensores normales, respuesta progresiva 31..36 y margen anti-oscilación; frío simétricamente gradual. Copos 1..3 alternados con descenso y grupos de aire 1..2 con deriva/pausas; máximo cuatro grupos conjuntos. Comprobar frío+aire húmedo y calor+aire seco. Iconos compactos, barras de 48 px, K desde 1000 lux, grado y ES/CAT/EN legibles; C/F coherente, Luz llega a 8000 y tierra/diagnóstico comparten bandas 30/70 % con confirmación temporal, sin parpadeo ni recortes.
- [ ] En hardware: en cada sensor, pulsación corta cambia modo y pulsación larga abre el menú del sensor activo.
- [ ] En hardware: `Timer` responde a corto/largo según diseño.
- [ ] En hardware: cambiar C/F, reiniciar y confirmar persistencia de unidad.
- [ ] En hardware: `Sistema > Bip` silencia beeps de UI sin silenciar `Alarmas`; `Sistema > Alarmas` silencia alertas/timer audibles sin ocultar alertas visuales/RGB.
- [ ] En hardware: reposo visible con `ZZZ`; despierta con interacción del encoder.
- [x] En hardware: encender con encoder presionado durante logo activa Modo demo; pulsación larga desde `Inicio` también lo activa; cualquier interacción posterior sale del demo.
- [x] UX Demo Mode: coreografía/ritmo suavizados en firmware con dwell variable, curvas por sensor y refresco dedicado.
- [ ] UX Demo Mode: validar en hardware que la coreografía smooth no reintroduce flicker ni cambios demasiado lentos.

## 4. BLE

- [x] Estado esperado de fábrica por código: BLE `OFF`.
- [x] `load_ble_enabled_store()` usa default `false` para `ble_en`.
- [x] Reset por firmware-stamp limpia el namespace NVS antes de evaluar BLE en un binario nuevo.
- [x] `init_ble()` solo se ejecuta si `load_ble_enabled_store()` devuelve `true`.
- [x] La pantalla `Sistema` no muestra fila BLE cuando `ble_en == false`.
- [x] La pantalla oculta BLE solo aparece manteniendo el encoder 30 s en `Sistema` según auditoría estática.
- [ ] Un escaneo BLE externo no ve publicidad `PBIT-XXXX` en estado de producción.
- [ ] Si se activó BLE para pruebas, volver a `OFF`, confirmar reinicio y repetir escaneo.
- [ ] En unidad física reflasheada sobre desarrollo, confirmar primer arranque sin publicidad `PBIT-XXXX`.

## 5. Sensores y feedback

- [x] LDR base técnica revisada por código: lux interno limitado a `0..8000`, curva empírica v1 desde RAW promediado, `FC = lux / 10.764` y `Raw ADC` disponible para recalibración.
- [ ] DHT11 muestra temperatura y humedad plausibles.
- [ ] LDR responde a sombra/luz y queda razonablemente cercano al luxómetro tras reflashear; el RGB permanece apagado en la vista de luz.
- [x] LDR propaga unidad y valor visible de forma coherente según modo (`lux`, `FC`, `raw`) en Luz, Sensor Zone, cards, dashboards, gráficas y dials.
- [ ] LDR muestra variación visible del icono de luz en el tramo bajo de luz ambiental.
- [ ] Micrófono responde en `Sonido`, `Sonido VU` y `Sonido Onda`.
- [ ] Suelo detecta ausencia de sensor, muestra `Revisa IO35` sin solapes en ES/CAT/EN y permite calibración seco/agua.
- [ ] Termómetro (`DS18B20`) detecta ausencia/presencia de sonda y muestra `Revisa IO33` sin solapes en ES/CAT/EN.
- [ ] Conexión/desconexión Suelo: al cambiar de estado después del arranque aparece splash verde/rojo `CONECTADO/DESCONECTADO / Sensor Suelo / IO35` y vuelve a la pantalla anterior.
- [ ] Conexión/desconexión Termómetro: al cambiar de estado después del arranque aparece splash verde/rojo `CONECTADO/DESCONECTADO / Sensor DS18B20 / IO33` y vuelve a la pantalla anterior.
- [x] Icono `temp` final: small/XL/fallback monocromos; solo Rango/XXL con acento mantiene detalle multicolor.
- [ ] Icono técnico `probe`/DS18B20 revisado en hardware; estado actual no final.
- [ ] Alertas visuales, RGB y audio de `Alarmas` se prueban al menos en un sensor crítico.

## 6. Entrega y repo hygiene

- [x] No incluir `.pio/`, logs locales, `.zip`, `.bin`, `.elf`, `.map`, `.exe` ni artefactos temporales nuevos en esta actualización documental.
- [x] Revisar `git status --short` antes de cerrar la build.
- [x] Registrar cambios de documentación en `CHANGELOG.md`.
- [x] `logs/xvba_debug.log` queda fuera del tracking e ignorado como log local.
- [ ] Si se genera un paquete de release, separar binarios/ejecutables grandes de la documentación cuando se haga la pasada de higiene planificada.


### Observación de uso — 2026-10-12

2026-10-12: el usuario comunica que la versión cargada funciona muy bien y pide conservar el diseño actual. Se da por cerrado provisionalmente el ajuste visual de producto; esta observación general no verifica individualmente todas las poses, idiomas, recuperación, flicker ni Demo, ni cierra la validación completa por unidad/24 h. No se marcan checks adicionales de hardware por esta observación general.
