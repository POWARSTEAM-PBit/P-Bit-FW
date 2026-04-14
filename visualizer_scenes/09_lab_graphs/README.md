# 09_lab_graphs

Snippets temporales para revisar en el visualizador las pantallas lab y la pantalla de gráfica.

Objetivo:
- medir mejor márgenes, cards, tipografías y solapes
- revisar pantalla por pantalla sin depender del firmware completo
- devolver ajustes precisos al código real después

Escenas incluidas:
- `00_estado_lab_overview.cpp`
- `01_sensor_lab_temp.cpp`
- `02_sensor_lab_humidity.cpp`
- `03_sensor_lab_light.cpp`
- `04_sensor_lab_sound.cpp`
- `05_sensor_lab_soil_no_sensor.cpp`
- `06_graph_temp.cpp`
- `07_graph_humidity.cpp`
- `08_clima_lab.cpp`

Notas:
- son variantes `visualizer-safe` basadas en el layout actual del firmware
- aquí interesa sobre todo medir geometría y respiración visual
- si hace falta, anota offsets concretos por escena: `label_y -2`, `value_x +4`, `card_h -3`, etc.
