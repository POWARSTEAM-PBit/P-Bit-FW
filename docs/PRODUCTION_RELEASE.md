# Production Release P-Bit

Actualizado: 2026-08-25

## Build Local

Usar el entorno reproducible fijado en `platformio.ini`:

```powershell
py -m platformio run -e esp32dev
```

Última verificación local registrada para esta release:

- Fecha: 2026-06-03
- Comando: `py -m platformio run -e esp32dev`
- Resultado: `SUCCESS`
- RAM: `14.9%` (`48940` bytes de `327680`)
- Flash: `72.1%` (`945429` bytes de `1310720`)

Nota 2026-08-25: esta pasada actualiza documentación de producción; no sustituye la última verificación local registrada ni aporta nuevos tamaños de firmware.

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

## Checks Mínimos

Antes de empaquetar:

```powershell
git status --short
git diff --check
py -m platformio run -e esp32dev
```

Registrar tamaño de firmware, hash de commit, fecha, operador, placa y resultado de flasheo.

En esta revisión quedan verificados el build local, la auditoría estática y una validación puntual en hardware de Demo Mode y anti-flicker/ghosting. El registro completo de flasheo y pruebas debe repetirse aparte por unidad.

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

Después de flashear una unidad:

- [ ] Confirmar selector de idioma en primer arranque si la build limpió NVS.
- [ ] Confirmar BLE apagado por defecto y sin publicidad `PBIT-XXXX`.
- [ ] Recorrer carrusel completo y probar una pulsación corta/larga en un sensor.
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
