# Release P-Bit

Esta carpeta contiene una build lista para flasheo repetido en varios P-Bits.

## Contenido

- `bin/firmware.bin`: binario principal del firmware
- `bin/bootloader.bin`: bootloader ESP32 generado por PlatformIO
- `bin/partitions.bin`: tabla de particiones
- `bin/firmware.elf`: ELF de referencia para depuración
- `bin/firmware.map`: mapa de enlace de referencia
- `manifest.json`: metadatos de build y hashes
- `RELEASE_NOTES.md`: cambios y comportamiento esperado
- `FLASH_COMMANDS.md`: comandos de flasheo recomendados
- `DEVICE_FLASH_LOG.csv`: plantilla para registrar qué equipos fueron flasheados

## Estado de esta build

- Fecha: `2026-04-04`
- Entorno: `esp32dev`
- `Serial Plotter`: OFF
- `Timer`: render segmentado para reducir flicker/ghosting en dígitos que no cambian
- `C/F`: arranque por defecto en `Celsius`
- Selector de idioma: aparece en cada cold boot

## Recomendación de uso

1. Probar primero un P-Bit piloto.
2. Verificar arranque, idioma, navegación, `Timer` y reposo `ZZZ`.
3. Si el piloto va bien, usar exactamente estos mismos binarios para el resto.
