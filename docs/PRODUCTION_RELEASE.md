# Production Release P-Bit

Actualizado: 2026-10-12

## Build Local

Usar el entorno reproducible fijado en `platformio.ini`:

```powershell
py -m platformio run -e esp32dev
```

Última verificación local registrada para este candidato:

- Fecha: 2026-10-12.
- Base Git: `23fdb57` + cambios locales de comportamiento y setup V1 de Planta Lab; sin commit al compilar. El hash base por sí solo no reproduce este candidato.
- Comando: `py -m platformio run -e esp32dev`.
- Resultado: `SUCCESS`; build completa inicial 81.990 s y final incremental 22.711 s tras ajustar el editor, etiqueta del tile y reinicio explícito del modelo por revisión de perfil.
- RAM estática: `18.4%` (`60372/327680` bytes); no equivale al consumo total de ejecución de stacks, sprites o BLE.
- Flash: `74.7%` (`979701/1310720` bytes).
- Incremento frente al candidato visual anterior (59580 / 971189): +792 bytes RAM y +8512 bytes Flash.
- Entorno: PlatformIO Core `6.1.19`, `espressif32@6.12.0`, Arduino ESP32 `2.0.17`, librerías fijadas en `platformio.ini`. Se usó `PLATFORMIO_CORE_DIR=C:\.platformio` y `PYTHONPATH` del usuario; flags/dependencias del proyecto sin cambios.
- Advertencias conocidas: dos `#undef` con tokens sobrantes en OneWire `2.3.8` (líneas 599–600) en la build completa; aviso de varias instalaciones Core (`6.1.19` frente a `6.2.0`). No impidieron build/carga.
- Logs ignorados: `.pio/plant_pet_review/profile_v1/build.log` y `build-final.log`.

La build/carga del 2026-10-11 sobre base `3744f04` + mascota/efectos locales (RAM 59580, Flash 971189) queda como antecedente visual; ese código ya estaba versionado al auditar `23fdb57`. Las builds de junio (48940 / 945429) y de base `27e5207` (49140 / 961709) tampoco representan el candidato V1 actual.

## Estado de validación y prioridad actual

- **Diseño conservado:** el usuario aprobó planta verde, barras por sensor y suelo café; el 2026-10-12 comunicó que la versión anterior funcionaba muy bien y pidió conservarla. Es cierre visual provisional, sin comprobar individualmente poses, idiomas, recuperación, flicker ni Demo.
- **Auditoría e implementación:** la revisión acotada sobre `23fdb57` reprodujo cuatro trazas y definió el contrato. Después el usuario autorizó todas las correcciones y setup V1. [Informe con resolución F1–F5](PLANTA_LAB_BEHAVIOR_REVIEW.md). No es una auditoría global ni validación de rangos biológicos.
- **V1 implementado:** poca luz contextual/`REPOSO`; adquisición identificada y frescura por canal; Suelo/T/Aire graduales; mezcla de poses continua. General/Personalizado con seis referencias educativas, borrador/Guardar/Cancelar, C/F, NVS versionada/validada y Reset propio. Recalibración de Suelo personalizado pide revisión; barras usan perfil activo y T amplía escala cuando hace falta. Real/Demo separados, Demo General. Sin presets, historial diario/persistente, RTC, consejos sostenidos ni detección de sol directo/calor propio.
- **Adquisición nativa:** 28 checks PASS contra `src/io.cpp`: valores iguales con secuencia nueva, retención DHT/TOO_SOON, invalidez, reconexión, canales parciales, rollover y tiempo cero. Host C++11 `-Wall -Wextra -Werror`; se suprime solo el parámetro histórico no usado de la tarea en este harness, sin cambiar flags firmware.
- **Modelo/setup nativos:** suites PASS con C++11 `-Wall -Wextra -Werror`: copias sin evidencia, nuevas muestras idénticas, caducidad por canal, desconocidos, referencias válidas/corruptas, intensidad, recuperación interrumpida, rollover y revisión forzada del perfil. Persistencia: borrador sin NVS, Cancelar predeterminado, guardado único, General/Personalizado, calibración, C/F, separación, intervalos elevados en una pasada, fallo de escritura, blob corrupto/incompatible y Reset aislado.
- **Render/pantallas:** raster 1215 combinaciones +101 niveles de Suelo +4 ausentes/no finitos PASS; continuidad exacta de los píxeles de la planta al interrumpir recuperación PASS. Pantalla principal: 143 comparaciones incremental/completo PASS. Setup: 30 comparaciones PASS en ES/CAT/EN; etiqueta `Ajustar`/`Edit profile` cabe en tile. Previews desde código/fuentes y semántica TFT_eSPI 2.5.43 emulada; no reemplazan ST7735 real. Artefactos ignorados en `.pio/plant_pet_review/profile_v1/`; SHA256 de fuentes del preview en `setup_preview/test-results.json`.
- **Carga actual:** COM8 SUCCESS el 2026-10-12, hashes de escritura verificados y reinicio RTS enviado; registro inferior. No confirma observación del arranque, uso del menú ni persistencia física.
- **Pruebas específicas abiertas:** usar Perfil/Ajustar, Guardar/Cancelar, cancelación larga, Reset y C/F; reiniciar, recalibrar/reconfirmar Suelo y alternar perfiles con valores iguales. Verificar reposo con otra necesidad, datos ausentes/antiguos, recuperación/interrupción, textos/efectos y flicker en TFT. Demo ≥30 s sigue pendiente como regresión. [Roadmap](ROADMAP.md), [ADR-003](ADR-003-planta-lab.md) y [checklist](PRODUCTION_CHECKLIST.md).
- **Evidencia histórica:** entrada/salida Demo y fixes anti-flicker aprobados puntualmente; soak de `16 h 09 min` con BLE activo tras fix DHT RMT, sin Guru/IWDT/reinicios. Stack mínimo libre: DisplayTask 2108 bytes, SensorTask 1944 bytes. No valida este candidato.
- **Pendientes aplazados:** prueba final de 24 h y validación física completa por unidad; no se iniciaron ni se aprobó entrega de producción. Incidencias de junio deben contrastarse con el seguimiento de [HARDWARE_VALIDATION_2026-06-04](HARDWARE_VALIDATION_2026-06-04.md), no tratarse automáticamente como bugs vigentes.

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

## Carga del candidato V1 actual — 2026-10-12

Usuario confirma conexión para cargar la versión implementada. `py -m platformio run -e esp32dev -t upload --upload-port COM8` SUCCESS (34.229 s), USB-SERIAL CH340, ESP32-D0WD-V3 revisión 3.1. Tamaños previos a carga: RAM 60372 y Flash 979701, base `23fdb57` + cambios locales V1. Esptool verificó hashes de los datos escritos y envió reinicio RTS. Log ignorado: `.pio/plant_pet_review/profile_v1/upload.log`. Confirma programación y reinicio solicitado, sin prueba visual/funcional física por el agente; no se activó debug ni se cambiaron flags/dependencias.

### Carga histórica del diseño — 2026-10-11

Mismo comando/puerto, SUCCESS (29.848 s); hashes verificados y reinicio RTS. Base `3744f04` + mascota/efectos locales, RAM 59580 y Flash 971189. Log ignorado: `.pio/plant_pet_review/ambient_integrated/upload.log`. El usuario valoró favorablemente ese diseño después. La causa del fallo previo de carga desde Visual Studio Code no se reprodujo en la operación.

## Checks Mínimos

Antes de empaquetar:

```powershell
git status --short
git diff --check
py -m platformio run -e esp32dev
```

Registrar tamaño de firmware, hash de commit, fecha, operador, placa y resultado de flasheo.

Esta revisión comprueba build, pruebas nativas/host y carga. Las validaciones puntuales de hardware de Demo Mode y anti-flicker/ghosting proceden de sesiones anteriores. El registro completo de flasheo y pruebas debe repetirse aparte por unidad cuando se retome esa validación.

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
- [ ] Revisar Planta Lab según `docs/ADR-003-planta-lab.md`: nueve poses con rostro, recuperación gradual, necesidades simultáneas y datos desconocidos; aparición condicional y textos ES/CAT/EN. Luz llega al extremo a 8000 lux y no genera necesidad adversa; tierra/gesto usan referencias del perfil activo (General 30/70 %), con confirmación por muestras nuevas. Comprobar que el buffer no pisa bordes ni produce parpadeo.
- [ ] Planta Lab: abrir con pulsación larga ~1.2 s; recorrer Perfil/Ajustar/Reset/Salir, editar seis referencias y comprobar Cancelar predeterminado, Guardar y cancelación larga. Confirmar persistencia tras reinicio, C/F sin alterar °C guardados, Reset aislado y REVISAR SUELO tras recalibrar Personalizado; reconfirmar en Ajustar.
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
