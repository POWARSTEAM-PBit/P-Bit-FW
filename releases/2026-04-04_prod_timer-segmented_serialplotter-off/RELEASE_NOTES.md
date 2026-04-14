# Release Notes

## Identificación

- Release ID: `2026-04-04_prod_timer-segmented_serialplotter-off`
- Fecha de build: `2026-04-04`
- Git short SHA: `9079548`
- Estado del árbol al compilar: `dirty worktree`

## Cambios destacados

- `Serial Plotter` desactivado por defecto para una build más limpia de producción.
- Optimización del `Timer`:
  - el valor grande ya no se repinta como un bloque único en cada tick
  - el render se hace por segmentos para reducir flicker en dígitos que no cambian
- `Timer v2` mantiene:
  - cronómetro en `00:00:00`
  - cuenta regresiva con editor `HH:MM:SS`
  - formato adaptativo `MM:SS:CC` / `HH:MM:SS`
- Las pantallas temporales de calibración ya no forman parte del firmware.

## Comportamiento esperado

- Selector de idioma en cada cold boot
- Reposo visible con `ZZZ`
- `TEMP/DS18` arrancan siempre en `Celsius`
- El cambio a `Fahrenheit` sigue siendo válido durante la sesión, pero no persiste tras reinicio

## Recursos

- RAM: `38,540 bytes`
- Flash: `872,249 bytes`

## Observaciones operativas

- La carpeta `visualizer_scenes/` no entra en el firmware.
- Solo se compilan estas fuentes reales:
  - `Roboto_Regular7pt8b`
  - `Roboto_Medium10pt8b`
  - `Roboto_Light6pt8b`
  - `IBMPlexSans_Regular9pt8b`
  - `IBMPlexMono_Regular12pt8b`
  - `IBMPlexMono_Regular24pt8b`
