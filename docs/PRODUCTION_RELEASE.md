# Production Release P-Bit

Actualizado: 2026-05-20

## Build Local

Usar el entorno reproducible fijado en `platformio.ini`:

```powershell
py -m platformio run -e esp32dev
```

Última verificación local de esta revisión:

- Fecha: 2026-05-20
- Comando: `py -m platformio run -e esp32dev`
- Resultado: `SUCCESS`
- RAM: `14.7%` (`48028` bytes de `327680`)
- Flash: `70.6%` (`925873` bytes de `1310720`)

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

En esta revisión solo queda verificado el build local y la auditoría estática. El flasheo y las pruebas con hardware real deben registrarse aparte por unidad.

## Auditoría Estática De Producción

Confirmado por lectura de código:

- i18n usa `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)` y `L(...)`.
- Guardar idioma fuerza full redraw mediante `runtime_request_ui_full_redraw()` y la UI consume el evento con `runtime_take_ui_full_redraw()`.
- Textos visibles de UI migrados a `L(...)`/`LIn(...)`; quedan literales directos no lingüísticos como cursores, placeholders, separadores y ticks numéricos.
- BLE sale factory-off: `ble_en` carga `false`, `init_ble()` es condicional y el reset por build-hash limpia NVS antes de evaluar BLE en una build nueva.
- LDR entrega lux en rango `0..20000`, conserva `ldr_raw` y soporta modo visible `Raw ADC`.
- Sensor Zone centraliza los seis sensores y sus modos visuales persistidos.
- Fixes anti-flicker presentes en dials/gauges, cards, menús/footers y `Sound VU`.

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

`logs/xvba_debug.log` aparece como log local modificado en el árbol de trabajo; no es pertinente para el commit de documentación/release salvo decisión explícita del usuario.

## Validación De Unidad

Después de flashear una unidad:

- [ ] Confirmar selector de idioma en primer arranque si la build limpió NVS.
- [ ] Confirmar BLE apagado por defecto y sin publicidad `PBIT-XXXX`.
- [ ] Recorrer carrusel completo y probar una pulsación corta/larga en un sensor.
- [ ] Verificar lecturas plausibles de DHT11, LDR, micrófono, suelo y DS18B20.
- [ ] Verificar que LDR responde a sombra/luz y que el modo `Raw ADC` muestra lectura cruda.
- [ ] Confirmar que `Sound VU`, cards y dials no presentan flicker visible en uso normal.
- [ ] Dejar 30 minutos en reposo visible y confirmar que despierta con el encoder.
- [ ] Para lote piloto, hacer prueba de 24 h con BLE apagado y otra con BLE activado.
