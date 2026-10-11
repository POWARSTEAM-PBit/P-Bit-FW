# Production Release P-Bit

Actualizado: 2026-10-12

## Build Local

Usar el entorno reproducible fijado en `platformio.ini`:

```powershell
py -m platformio run -e esp32dev
```

Última verificación local registrada para esta release:

- Fecha: 2026-10-11
- Base Git del código compilado: `3744f04` + cambios locales de mascota y refinamiento visual de Planta Lab, todavía sin commit; el hash base por sí solo no reproduce este candidato.
- Comando: `py -m platformio run -e esp32dev`
- Resultado: `SUCCESS`
- RAM estática: `18.2%` (`59580` bytes de `327680`); no equivale al consumo total en ejecución de stacks, sprites o BLE
- Flash: `74.1%` (`971189` bytes de `1310720`)
- Entorno comprobado: PlatformIO Core `6.1.19`, `espressif32@6.12.0`, Arduino ESP32 `2.0.17`, librerías fijadas en `platformio.ini`
- Advertencias: dos directivas `#undef` con tokens sobrantes en OneWire `2.3.8` (líneas 599–600); aviso de PlatformIO sobre varias instalaciones Core (`6.1.19` frente a `6.2.0`). No impidieron el build; no se cambiaron dependencias ni flags.

Build incremental inicial SUCCESS (20.103 s) tras integrar copos/aire del preview autorizado; regeneración completa final SUCCESS (78.772 s) con los mismos tamaños, tras encontrar vacío el directorio de salida. RAM sin aumento y +184 bytes Flash frente al calor distribuido previo (59580 / 971005). Buffer de mascota 10 260 bytes, sin heap adicional. Se usó PlatformIO del usuario mediante `PYTHONPATH` y caché `C:\.platformio` mediante `PLATFORMIO_CORE_DIR`; flags y dependencias del proyecto sin cambios.

Las builds del `2026-06-03` (RAM `48940`, Flash `945429`) y de base `27e5207` (RAM `49140`, Flash `961709`) quedan como evidencia histórica; no representan este candidato.

## Estado de validación y prioridad actual

- **Observación posterior del usuario:** 2026-10-12: el usuario comunica que la versión cargada funciona muy bien y pide conservar el diseño actual. Se da por cerrado provisionalmente el ajuste visual de producto; esta observación general no verifica individualmente todas las poses, idiomas, recuperación, flicker ni Demo, ni cierra la validación completa por unidad/24 h.
- **Siguiente fase recomendada:** revisión acotada de comportamiento/contrato de perfiles antes de setup; sin ejecución ni implementación nueva.

- **Verificado en esta revisión:** modelo nativo PASS, incluidos perfil térmico/histéresis, intensidad/rampas, desconocidos, recuperación y rollover; raster 1215 combinaciones de pose/fase/transición +101 niveles de Suelo y 4 ausentes/no finitos PASS. Host `-Wall -Wextra -Werror` limpio; 138 comparaciones incremental/completo PASS, con quince temperaturas y diez niveles/límites de símbolos de Luz además de regresiones de K, C/F, suelo, nueve estados y cambios en el mismo tick/dígito. Se incluyen 48 frames adicionales de ondas distribuidas/escalonadas. PNG/GIF desde C++ real en `.pio/plant_pet_review/effects/`; galerías completas térmica/solar y posiciones de calor revisadas por el agente, usuario acepta el preview general de sol/luna y calor distribuido; frío/aire alternados autorizados e integrados. Última integración: 528 comparaciones host PASS y 528 frames idénticos al preview de nueve intensidades y dos combinaciones; raster 1215 casos +101 niveles de Suelo/4 ausentes-no finitos PASS. Artefactos actuales en `.pio/plant_pet_review/ambient_integrated/`. Etiquetas ES/CAT/EN medidas previamente, fuentes/textos sin cambios. Carga de este ajuste completada en COM8; ver registro inferior. Prueba visual/funcional física pendiente.
- **Observación del usuario (2026-10-11):** tras probar la adaptación actual, aprueba colores de planta/medidores y valora favorablemente el efecto del suelo. Pide mejorar sol/luna, intensidad de ondas y facilidad de expresión feliz. Es aceptación puntual del diseño; no cierra poses/idiomas, recuperación, flicker ni Demo. Sol/luna y respuesta térmica gradual ya implementados en la build registrada arriba; Ondas ahora repartidas a distintas alturas/lados con apariciones escalonadas; preview general aceptado por el usuario; comprobación física pendiente. Autoriza integrar los copos y aire distribuidos del preview (ADR-003), ya implementados; falta su prueba TFT. Historial, combinaciones y flexibilidad ambiental siguen propuestas.
- **Evidencia histórica:** entrada/salida de Demo y fixes anti-flicker aprobados puntualmente; `CHANGELOG.md` registra un soak provisional de `16 h 09 min` con BLE activo tras el fix DHT RMT, sin Guru/IWDT/reinicios. Stack mínimo observado en esa prueba: DisplayTask `2108` bytes libres, SensorTask `1944` bytes libres. No valida el candidato actual ni reemplaza el soak final.
- **Comprobaciones específicas aún abiertas (diseño conservado tras observación favorable del usuario):** comprobar en TFT real las barras por sensor, verde persistente, tierra café progresiva, sol/luna y gestos de temperatura/aire; también iconos, K, grado, recuperación, rangos, textos y flicker. La luna representa baja iluminación, no noche confirmada; el modelo todavía marca `POCA LUZ` por debajo de 200 lux. Configuración/presets después. Demo ≥30 s sigue pendiente como regresión. Ver `docs/ROADMAP.md` y `docs/ADR-003-planta-lab.md`.
- **Pendientes aplazados por el usuario:** prueba final de 24 h y validación física completa por unidad. Se conservan los checks abiertos; no se ha aprobado la entrega de producción.
- **Incidencias de junio:** calibración cancelable, Reset con reinicio, selector hasta confirmar idioma y restauración de vistas al salir de Demo están corregidos en código. Ver seguimiento en `docs/HARDWARE_VALIDATION_2026-06-04.md`; las notas originales no son una lista de bugs activos.

Nota de producto: la alimentación recomendada de entrega/aula es `3 baterías AAA`. USB-C queda para programación, monitor serie y alimentación auxiliar 5 V por PC/powerbank; no presentarlo como alimentación principal.

Artefactos generados en `.pio/build/esp32dev/`:

- `firmware.bin`
- `bootloader.bin`
- `partitions.bin`
- `firmware.elf`
- `firmware.map`

`boot_app0.bin` vive en el paquete Arduino ESP32 de PlatformIO:

```text
%USERPROFILE%\.platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin
```

## Carga del candidato actual — 2026-10-11

Carga autorizada por el usuario el 2026-10-11: `py -m platformio run -e esp32dev -t upload --upload-port COM8` SUCCESS (29.848 s), USB-SERIAL CH340, ESP32-D0WD-V3 revisión 3.1. Esptool verificó los hashes de los datos escritos y envió reinicio por RTS. Candidato base `3744f04` + cambios locales de mascota/efectos, RAM 59580 y Flash 971189. Log ignorado: `.pio/plant_pet_review/ambient_integrated/upload.log`. Confirma carga y reinicio solicitado, no observación de pantalla ni cierre de pruebas visuales/funcionales. No se activó debug ni se cambiaron flags/dependencias. La causa del fallo de carga en Visual Studio Code no se reprodujo en esta operación.

## Checks Mínimos

Antes de empaquetar:

```powershell
git status --short
git diff --check
py -m platformio run -e esp32dev
```

Registrar tamaño de firmware, hash de commit, fecha, operador, placa y resultado de flasheo.

Esta revisión comprueba build y código. Las validaciones puntuales de hardware de Demo Mode y anti-flicker/ghosting proceden de sesiones anteriores. El registro completo de flasheo y pruebas debe repetirse aparte por unidad cuando se retome esa validación.

## Auditoría Estática De Producción

Confirmado por lectura de código:

- i18n usa `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)` y `L(...)`.
- Guardar idioma fuerza full redraw mediante `runtime_request_ui_full_redraw()` y la UI consume el evento con `runtime_take_ui_full_redraw()`.
- Textos visibles de UI migrados a `L(...)`/`LIn(...)`; quedan literales directos no lingüísticos como cursores, placeholders, separadores y ticks numéricos.
- Carrusel de producción con `PBIT_ENABLE_GRAPH_LAB=1`: `Inicio -> Clima Lab -> Planta Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`; `Planta Lab` se omite si Suelo no tiene lectura válida.
- Sensor Zone centraliza los seis sensores. Ciclo común: `Principal -> Rango -> Ficha -> Dato -> Curva`; Sonido añade `Sonido VU` y `Sonido Onda` tras `Principal`.
- BLE sale factory-off: `ble_en` carga `false`, `init_ble()` es condicional y el reset por firmware-stamp limpia NVS antes de evaluar BLE en un binario nuevo.
- LDR tiene base técnica y propagación visual implementadas: lux interno en rango `0..8000` por curva empírica v1, `ldr_raw` promediado, conversión `FC = lux / 10.764`, histórico RAW y unidad/valor coherentes en Luz, Sensor Zone, cards, dashboards, gráficas y dials; queda validación en hardware real.
- Estados desconectados de sensores externos implementados en firmware: Suelo usa `Revisa IO35`, Termómetro/DS18B20 usa `Revisa IO33`, ambos con paleta del sensor atenuada y sin persistencia NVS.
- Aviso de conexión/desconexión implementado para sensores externos: si Suelo o Termómetro cambian de estado después del arranque, aparece splash full-screen semafórico de `1500 ms`; verde con `CONECTADO` y rojo con `DESCONECTADO`, seguido de `Sensor Suelo`/`Sensor DS18B20` y `IO35`/`IO33`; no se dispara al boot ni durante Demo Mode.
- Icono `temp` final ajustado a monocromo en small/XL/fallback; solo la versión Dial/XXL con acento conserva detalle multicolor. El icono técnico `probe`/DS18B20 sigue separado y pendiente de validación final.
- `Sistema` separa `Bip` (`sys_sound`) y `Alarmas` (`sys_alarm`) con persistencia independiente; ambos vuelven `OFF` por defecto tras build/reset.
- Unidad global `C/F` persistida en `sys_unit_f`.
- Fixes anti-flicker/ghosting presentes y validados por ahora en dials/gauges, cards, menús/footers, banda de estado en cards y `Sonido VU/Onda`; queda como vigilancia de regresión.
- Modo demo runtime confirmado en hardware: entra en arranque con encoder presionado durante los logos o con pulsación larga desde `Inicio`, muestra splash breve, simula valores solo en el snapshot visual, no persiste sensor/modo en NVS, bloquea reposo mientras está activo y sale con interacción.
- Demo Mode tiene coreografía smooth implementada en firmware: dwell variable, refresco dedicado, curvas por sensor y gráficas sintéticas; queda validación visual final en hardware.
- Demo restaura pantalla/vistas previas al salir y simula Timer y sparklines de Termo Lab/DS18. `kDemoScenes` aún no incluye Planta Lab; incorporarla requiere decidir el alcance de una futura mejora.
- WiFi/Bluetooth existen como capacidades del ESP32. El firmware inicial no incluye flujo WiFi público; BLE queda reservado a fábrica/debug y apagado en producción.

## Artefactos De Release

Para GitHub Releases o paquete externo, incluir:

- `firmware.bin`
- `bootloader.bin`
- `partitions.bin`
- `boot_app0.bin`
- `SHA256SUMS.txt`
- `manifest.json` o notas con commit, entorno PlatformIO y versión de librerías
- `FLASH_COMMANDS.md`

No versionar nuevos `.bin`, `.elf`, `.map`, `.exe`, logs o temporales dentro del repo activo.

`logs/xvba_debug.log` queda fuera del tracking e ignorado como log local; no incluirlo en paquetes ni commits de release.

## Validación De Unidad

Después de flashear una unidad, cuando se retome la validación física completa (actualmente aplazada):

- [ ] Confirmar arranque y navegación básica con `3 baterías AAA` nuevas.
- [ ] Confirmar USB-C para programación/alimentación auxiliar 5 V, sin presentarlo como alimentación principal.
- [ ] Confirmar selector de idioma en primer arranque si la build limpió NVS.
- [ ] Confirmar BLE apagado por defecto y sin publicidad `PBIT-XXXX`.
- [ ] Recorrer carrusel completo y probar una pulsación corta/larga en un sensor.
- [ ] Revisar Planta Lab según `docs/ADR-003-planta-lab.md`: nueve poses con rostro, recuperación gradual, necesidades simultáneas y datos desconocidos; aparición condicional y textos ES/CAT/EN. Luz llega al extremo a 8000 lux; tierra/diagnóstico comparten las bandas de Suelo 30/70 %, con confirmación temporal. Comprobar que el buffer no pisa bordes ni produce parpadeo.
- [ ] Verificar lecturas plausibles de DHT11, LDR, micrófono, suelo y Termómetro (`DS18B20`).
- [ ] Verificar sensores externos desconectados: Suelo muestra `Revisa IO35`, Termómetro muestra `Revisa IO33`, con pantalla atenuada y retorno a paleta normal al reconectar sin reiniciar.
- [ ] Verificar splash de conexión/desconexión: `CONECTADO/DESCONECTADO / Sensor Suelo / IO35` y `CONECTADO/DESCONECTADO / Sensor DS18B20 / IO33`, fondo verde/rojo correcto y sin aviso inicial al arrancar con sensores ya conectados o ya desconectados.
- [ ] Verificar que `Bip OFF` silencia beeps de UI y que `Alarmas OFF` silencia alertas/timer audibles.
- [ ] Cambiar `C/F`, reiniciar y confirmar que la unidad permanece guardada.
- [ ] Validar LDR en hardware: verificar respuesta a sombra/luz, comparar contra luxómetro y confirmar que al cambiar modo todos los valores/unidades visibles son coherentes entre `lux`, `FC` y `raw`.
- [ ] Validar Demo smooth en hardware: confirmar que la coreografía no reintroduce flicker y que los cambios de gráficas/dials/cards se sienten intencionales.
- [ ] Verificar que el icono de luz cambia con varias etapas visibles en el tramo bajo de luz ambiental.
- [ ] Revisar icono DS18B20 en hardware; no tratarlo como identidad final hasta aprobarlo visualmente.
- [ ] Vigilancia no bloqueante de regresión: revisar que `Sonido VU/Onda`, cards y dials no recuperen flicker/ghosting visible en uso normal.
- [ ] Dejar 30 minutos en reposo visible y confirmar que despierta con el encoder.
- [x] Activar Modo demo encendiendo con encoder presionado durante los logos y también con pulsación larga desde `Inicio`; salida confirmada con interacción.
- [ ] Para lote piloto, hacer prueba de 24 h con BLE apagado y otra con BLE activado.
