# Visualizer Scenes

Biblioteca auxiliar de escenas del P-Bit para el visualizador TFT.

Estado de referencia: revisión de producción/i18n de 2026-08-25.

Objetivo:
- guardar snippets de pantalla listos para pegar en la workstation/web visualizer
- organizar pantallas y subestados por grupos
- mantener este material fuera de `src/` e `include/` para que no afecte al firmware

UI visible actual de producción:
1. Inicio 2x2 (`LAB_HOME_CARDS_SCREEN`)
2. Clima Lab (`LAB_DUAL_TH_SCREEN`)
3. Planta Lab (`LAB_PLANT_SCREEN`, solo si Suelo tiene lectura válida)
4. Termo Lab (`LAB_WIDGET_MIX_SCREEN`)
5. Sensor Zone para Temperatura, Humedad, Luz, Sonido, Suelo y Termómetro
6. Timer
7. Sistema

En Sensor Zone, la pulsación corta avanza `Principal -> Rango -> Ficha -> Dato -> Curva`. `Sonido` añade `Sonido VU` y `Sonido Onda` justo después de `Principal`; no son paradas globales independientes.

Reglas:
- esta carpeta no forma parte del build del P-Bit
- los archivos aquí pueden borrarse más adelante sin romper el firmware
- cada escena debe vivir en un archivo independiente
- los snippets deben ser planos, pensados para el visualizador: sin `setup()`, sin `loop()`, sin `#include`
- si una escena es una variante del firmware real, indicarlo en un comentario al inicio
- los textos traducibles deben referenciar la `LangKey` equivalente o indicar el idioma simulado
- LDR se documenta como `0..8000 lux` con curva empírica v1; raw ADC `0..4095` queda para calibración/debug
- BLE es factory-off y oculto; no tratar sus escenas como flujo visible normal

Convención recomendada:
- un archivo por escena
- nombres con prefijo numérico para mantener orden estable
- extensión `.cpp` para conservar sintaxis y coloreado
- etiquetar cada escena en comentarios como `production-current`, `sensor-zone-renderer`, `config-menu`, `hidden-debug` o `legacy-lab`

Orden de trabajo propuesto:
1. `00_global_and_calibration`
2. `09_lab_graphs` para Inicio/Clima Lab/Planta Lab/Termo Lab/Sensor Zone
3. `05_light`
4. `06_sound`
5. `02_temp`
6. `03_ds18`
7. `04_humidity`
8. `07_soil`
9. `01_timer`
10. `08_system`

Flujo:
1. generar o revisar grupo por grupo
2. confirmar i18n ES/CAT/EN contra `include/languages.h`
3. revisar geometría, cards, dials, VU y clears anti-flicker
4. copiar al visualizador
5. ajustar visualmente
6. traducir cambios al firmware real solo si se aprueban
