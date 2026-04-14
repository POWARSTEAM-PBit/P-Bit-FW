# Comandos de Flasheo

## Opción 1: PlatformIO

Desde la raíz del proyecto:

```powershell
py -3 -m platformio run --target upload
```

Si necesitas indicar puerto:

```powershell
py -3 -m platformio run --target upload --upload-port COM3
```

Sustituye `COM3` por el puerto real de tu P-Bit.

## Opción 2: esptool.py con binarios de esta release

```powershell
py -3 -m esptool --chip esp32 --port COM3 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bin/bootloader.bin 0x8000 bin/partitions.bin 0x10000 bin/firmware.bin
```

Sustituye `COM3` por el puerto real.

## Flujo recomendado

1. Flashear un P-Bit piloto
2. Verificar:
   - selector de idioma
   - navegación completa
   - `Timer` corriendo / pausado / editor
   - reposo `ZZZ`
3. Si todo va bien, repetir con el resto usando exactamente estos binarios

## Nota

`firmware.elf` y `firmware.map` se incluyen como referencia técnica, pero no son necesarios para el flasheo normal.
