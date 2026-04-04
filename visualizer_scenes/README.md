# Visualizer Scenes

Biblioteca auxiliar de escenas del P-Bit para el visualizador TFT.

Objetivo:
- guardar snippets de pantalla listos para pegar en la workstation/web visualizer
- organizar pantallas y subestados por grupos
- mantener este material fuera de `src/` e `include/` para que no afecte al firmware

Reglas:
- esta carpeta no forma parte del build del P-Bit
- los archivos aquí pueden borrarse más adelante sin romper el firmware
- cada escena debe vivir en un archivo independiente
- los snippets deben ser planos, pensados para el visualizador: sin `setup()`, sin `loop()`, sin `#include`
- si una escena es una variante del firmware real, indicarlo en un comentario al inicio

Convención recomendada:
- un archivo por escena
- nombres con prefijo numérico para mantener orden estable
- extensión `.cpp` para conservar sintaxis y coloreado

Orden de trabajo propuesto:
1. `00_global_and_calibration`
2. `01_timer`
3. `02_temp`
4. `03_ds18`
5. `04_humidity`
6. `05_light`
7. `06_sound`
8. `07_soil`
9. `08_system`

Flujo:
1. generar grupo por grupo
2. revisar snippets aquí
3. copiar al visualizador
4. ajustar visualmente
5. traducir cambios al firmware real
