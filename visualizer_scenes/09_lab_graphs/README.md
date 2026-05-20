# 09 Lab Graphs / Product Visuals

Snippets temporales para revisar en el visualizador las pantallas visuales actuales y los renderers reutilizados por Sensor Zone.

Estado tras producción/i18n:

- Home, Clima, Multi y Sonido VU son paradas visibles.
- Sensor Zone reutiliza focus, valor, graph, dial y card por sensor.
- Graph ya no debe documentarse como parada top-level independiente.
- Las escenas antiguas de `sensor_lab_*` son útiles como base, pero ahora representan renderers de Sensor Zone.

Objetivo:

- medir márgenes, cards, tipografías, dials, VU, sparklines y solapes
- revisar pantalla por pantalla sin depender del firmware completo
- devolver ajustes precisos al código real después
- validar que los textos ES/CAT/EN caben en headers, chips y footers

Escenas incluidas actualmente:

- `00_estado_lab_overview.cpp` - legacy/tooling
- `01_sensor_lab_temp.cpp` - Sensor Zone focus, temp
- `02_sensor_lab_humidity.cpp` - Sensor Zone focus, humedad
- `03_sensor_lab_light.cpp` - Sensor Zone focus, luz
- `04_sensor_lab_sound.cpp` - Sensor Zone focus, sonido
- `05_sensor_lab_soil_no_sensor.cpp` - Sensor Zone focus, suelo sin sensor
- `06_graph_temp.cpp` - renderer graph, temp
- `07_graph_humidity.cpp` - renderer graph, humedad
- `08_clima_lab.cpp` - Clima

Escenas que faltan para cubrir la UI actual:

- Home 2x2 cards (`LAB_HOME_CARDS_SCREEN`)
- Multi / Temp Lab (`LAB_WIDGET_MIX_SCREEN`)
- Sonido VU stack (`LAB_SOUND_VU_STACK_SCREEN`)
- Sonido VU wave (`LAB_SOUND_VU_WAVE_SCREEN`)
- Sensor Zone valor (`SZ_VIZ_VALOR`) para al menos Temp/Luz/Sonido
- Sensor Zone dial (`SZ_VIZ_GAUGE`) para al menos Temp/Luz/Sonido
- Sensor Zone card (`SZ_VIZ_CARD`) para los seis sensores
- Graph para Luz con escala `0..20000 lux`

Notas:

- son variantes `visualizer-safe` basadas en el layout actual del firmware
- aquí interesa sobre todo medir geometría y respiración visual
- si hace falta, anota offsets concretos por escena: `label_y -2`, `value_x +4`, `card_h -3`, etc.
- documentar anti-flicker aunque el snippet sea estático: shell/data, caches, sprites y clears acotados
- marcar cualquier escena de laboratorio no visible como `legacy-lab`
