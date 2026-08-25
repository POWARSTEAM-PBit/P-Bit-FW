# 09 Lab Graphs / Product Visuals

Snippets temporales para revisar en el visualizador las pantallas visuales actuales y los renderers reutilizados por Sensor Zone.

Estado de producción:

- Inicio, Clima Lab, Planta Lab condicional y Termo Lab son paradas visibles.
- Sensor Zone reutiliza `Principal`, `Rango`, `Ficha`, `Dato` y `Curva` por sensor.
- `Sonido VU` y `Sonido Onda` son modos de Sonido dentro de Sensor Zone.
- Graph ya no debe documentarse como parada top-level independiente.
- Las escenas antiguas de `sensor_lab_*` son útiles como base, pero ahora representan renderers de Sensor Zone.

Objetivo:

- medir márgenes, cards, tipografías, dials, VU, sparklines y solapes
- revisar pantalla por pantalla sin depender del firmware completo
- devolver ajustes precisos al código real después
- validar que los textos ES/CAT/EN caben en headers, chips y footers

Escenas incluidas actualmente:

- `00_estado_lab_overview.cpp` - legacy/tooling
- `01_sensor_lab_temp.cpp` - Sensor Zone `Principal`, temperatura
- `02_sensor_lab_humidity.cpp` - Sensor Zone `Principal`, humedad
- `03_sensor_lab_light.cpp` - Sensor Zone `Principal`, luz
- `04_sensor_lab_sound.cpp` - Sensor Zone `Principal`, sonido
- `05_sensor_lab_soil_no_sensor.cpp` - Sensor Zone `Principal`, suelo sin sensor
- `06_graph_temp.cpp` - renderer `Curva`, temperatura
- `07_graph_humidity.cpp` - renderer `Curva`, humedad
- `08_clima_lab.cpp` - Clima Lab

Escenas que faltan para cubrir la UI actual:

- Inicio 2x2 cards (`LAB_HOME_CARDS_SCREEN`)
- Planta Lab (`LAB_PLANT_SCREEN`)
- Termo Lab (`LAB_WIDGET_MIX_SCREEN`)
- Sonido VU stack (`LAB_SOUND_VU_STACK_SCREEN`) como modo de Sonido
- Sonido Onda (`LAB_SOUND_VU_WAVE_SCREEN`) como modo de Sonido
- Sensor Zone `Dato` (`SZ_VIZ_VALOR`) para al menos Temperatura/Luz/Sonido
- Sensor Zone `Rango` (`SZ_VIZ_GAUGE`) para al menos Temperatura/Luz/Sonido
- Sensor Zone `Ficha` (`SZ_VIZ_CARD`) para los seis sensores
- `Curva` para Luz con escala `0..8000 lux`

Notas:

- son variantes `visualizer-safe` basadas en el layout actual del firmware
- aquí interesa sobre todo medir geometría y respiración visual
- si hace falta, anota offsets concretos por escena: `label_y -2`, `value_x +4`, `card_h -3`, etc.
- documentar anti-flicker aunque el snippet sea estático: shell/data, caches, sprites y clears acotados
- marcar cualquier escena de laboratorio no visible como `legacy-lab`
