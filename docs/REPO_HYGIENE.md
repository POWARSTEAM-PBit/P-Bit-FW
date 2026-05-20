# Repo Hygiene P-Bit

Actualizado: 2026-05-20

Esta pasada no mueve carpetas grandes ni elimina artefactos ya versionados. Solo deja documentado qué conviene ordenar después.

## Estado

- `.gitignore` ya cubre cachés locales de PlatformIO, logs, zips locales, binarios/ELF/mapas/ejecutables nuevos y ruido de editor/sistema.
- Los archivos ya trackeados no dejan de estar trackeados por aparecer en `.gitignore`.
- `py -m platformio run -e esp32dev` genera/actualiza `.pio/`, pero no debe entrar en el commit.
- `logs/xvba_debug.log` aparece modificado en el árbol de trabajo y no es pertinente para el commit de documentación/release salvo decisión explícita del usuario.
- El repo contiene cambios ajenos en código y documentación fuera de este ownership; no se revierten ni se normalizan en esta pasada.

## Estado De Release Revisado

- Build local verificado: `SUCCESS`.
- RAM reportada: `14.7%` (`48028`/`327680`).
- Flash reportada: `70.6%` (`925873`/`1310720`).
- Cambios documentales limitados a: `MANUAL_TECNICO_PBIT.md`, `CHANGELOG.md`, `docs/PRODUCTION_CHECKLIST.md`, `docs/PRODUCTION_RELEASE.md`, `docs/REPO_HYGIENE.md`.
- Validación de hardware real pendiente; no mezclarla con checks completados por compilación/auditoría estática.

## Candidatos a mover o desversionar después

- `logs/`: logs locales de ejecución o herramientas.
- `platformio.zip`: paquete local generado; conviene moverlo a almacenamiento de artefactos.
- `listado_completo.txt`: parece salida generada de inventario; validar si sigue siendo fuente útil.
- `releases/*/bin/*.bin`, `releases/*/bin/*.elf` y `releases/*/bin/*.map`: útiles para flasheo/depuración, pero pesados para historia Git.
- `releases/*/flash_download_tool_*`: herramienta externa grande; candidata clara a almacenamiento externo o instrucciones de descarga.
- `releases/*/dl_temp/`: temporales del flash tool; no deberían crecer dentro del repo.

## Propuesta de limpieza futura

1. Definir qué artefactos de release deben vivir en Git y cuáles en almacenamiento externo.
2. Mantener en Git `README.md`, `RELEASE_NOTES.md`, `manifest.json` y comandos de flasheo.
3. Mover binarios y herramientas externas solo con aprobación explícita.
4. Si se decide desversionar algo, usar `git rm --cached` con rutas concretas y revisar el diff antes de commit.
5. Decidir explícitamente si `logs/xvba_debug.log` se descarta, se conserva o se mueve fuera del repo antes de incluirlo en cualquier commit.
