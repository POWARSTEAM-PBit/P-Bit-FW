# Analisis de Pantallas Experimentales del P-Bit

Actualizado: 2026-05-16

## Objetivo

Este documento resume el estado de las pantallas experimentales que hoy conviven con el firmware principal, para tomar decisiones de producto sin mezclar:

- pantallas realmente candidatas a entrar en el flujo principal
- pantallas que sirven como plantilla visual o libreria de widgets
- pantallas que son tooling para iconos o comparacion tecnica

La idea no es volver a discutir microajustes de layout, sino ordenar el arbol de navegacion y reducir repeticiones innecesarias.

## Nota de continuidad 2026-05-15

Se inició una pasada profunda para unificar la gramatica visual de todas las pantallas.

Regla visual actual aplicada en código:

- Header común: `L_HEADER_Y=18`, `L_HEADER_LINE=23`, `L_CONTENT_TOP=27`.
- Los títulos se dibujan con `C_BASELINE` para evitar diferencias entre pantallas.
- La línea del header usa `x=4`, `w=152`.
- Regla común de contenido: `LC_SCREEN_X=2`, `LC_SCREEN_W=156`, `LC_SCREEN_BOTTOM=126`, `LC_CARD_RADIUS=4`.
- Las cards 2x2 aprobadas usan `LC_MASTER_CARD_X0=2`, `LC_MASTER_CARD_Y0=27`, `LC_MASTER_CARD_W=76`, `LC_MASTER_CARD_H=48`, `LC_MASTER_CARD_GAP=4`.
- El texto `Vista experimental` se está retirando de pantallas visibles cuando no es un hint accionable.

Estado de esta pasada:

- Compila correctamente con `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"`.
- No se considera aprobada visualmente hasta validar de nuevo en hardware.
- El objetivo inmediato es decidir qué pantallas sobreviven al carrusel y cuáles quedan como laboratorio oculto.

Clasificación provisional:

- Aprobadas como referencia visual: `LAB_HOME_CARDS_SCREEN`, `LAB_DUAL_TH_SCREEN`, `GRAPH_SCREEN`.
- Mantener y ajustar menor: `LAB_LINEAR_DASH_SCREEN`, `LAB_SOUND_VU_STACK_SCREEN`, `LAB_SOUND_VU_WAVE_SCREEN`, `SYSTEM_SCREEN`, `TIMER_SCREEN`.
- Mantener como demo/plantilla: `LAB_GAUGE_TEMP_SCREEN`, `LAB_VALUE_MODERN_SCREEN`.
- Mantener visible como referencia fuerte: `LAB_SENSOR_FOCUS_SCREEN`.
- Revisar con lupa antes de aprobar: `LAB_TEMP_CARD_SCREEN`, `LAB_DS18_CARD_SCREEN`, `LAB_WIDGET_MIX_SCREEN`.
- No promover todavía a producto: pantallas de icon gallery y tamaños de iconos.

Protocolo para siguientes agentes:

- No reescribir pantallas desde cero sin pedirlo.
- Primero respetar el contrato de header y zonas seguras.
- Documentar cada ajuste con: archivo, motivo, build, pendiente de hardware.
- Si una pantalla sigue pisando texto o bordes tras dos microajustes, proponer ocultarla del carrusel en lugar de seguir acumulando parches.

### Ajuste de continuidad 2026-05-15

Ultima ronda aplicada y compilada:

- Header unificado: se aumento `1 px` la distancia titulo-linea y se fijo `3 px` entre linea y primer borde de contenido.
- Se igualaron gaps tipo `HOME` en las pantallas revisadas: `CLIMA LAB`, `TEMP LAB`, `PLANT LAB`, `SENSOR LAB`, `GRAPH`, `VALOR LAB`, `GAUGE LAB`, `TEMP CARD` y `PROBE CARD`.
- Se redistribuyo espacio sobrante: cards mas altas en `HOME`; paneles y footer alineados en `CLIMA LAB`; filas mas altas en `PLANT LAB`; graficas mas grandes en `SENSOR LAB` y `GRAPH`.
- `CLIMA LAB` sube barras internas `3 px` y el texto `Optimo` `1 px`; `TEMP LAB` mueve la unidad `C/F` junto a `TEMP/SONDA` y estira la card inferior hasta bottom `126`.
- `VALOR LAB` corrige el montaje unidad/sparkline; `GAUGE LAB` alinea min/max con el gauge; `TEMP CARD`/`PROBE CARD` ganan card completa `x=2,y=27,w=156,h=100` y rail mas alto.
- Build verificado: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`.
- Pendiente: validar en pantalla real y capturar feedback puntual por pantalla antes de aprobar u ocultar candidatas.

### Ajuste puntual de microdiagramacion 2026-05-15

Ultima ronda aplicada y compilada:

- `HOME`: icono, etiqueta y dato de las 4 cards se movieron `x+2,y+2`.
- `CLIMA LAB`: iconos internos `x+2`, unidades `x-1`, barras `y-4`, barras mas altas (`h=9`) y hint de estado `y-1`.
- `TEMP LAB`: cards superiores centradas y nombradas `DHT11`/`DS18`; dato `y+1`; card inferior convertida en barra comparativa centrada de diferencia con etiqueta multilenguaje `LAB_TEMP_DIFF`.
- `GRAPH`: banda superior `TEMP/dato` sube `1 px` con `LG_SENSOR_Y=28`.
- `SENSOR LAB`: summary y grafica reducen `1 px` de alto, conservando top y gap.
- Build verificado: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`.
- Pendiente: flashear y validar especialmente la claridad de la nueva barra diferencial de `TEMP LAB`.

### Ajuste amplio con multiples agentes 2026-05-15

Ultima ronda aplicada y compilada:

- Correccion posterior: `LAB_SENSOR_FOCUS_SCREEN` vuelve al carrusel visible y a restauracion de sueño; es el carretel de sensores que se quiere conservar.
- La pantalla gris inicial de validacion de layout en `setup()` queda apagada con `kShowStartupLayoutValidation = false`; esa prueba de 4 cards antes de los logos no forma parte del carrusel.
- Build verificado tras la correccion: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`, Flash usada `914233 bytes`.
- `IDIOMAS`: las tres opciones suben `2 px`.
- `HOME`: contenido interno baja `2 px`, texto/valor `x+1`, tanque lateral al doble de ancho creciendo hacia la izquierda.
- `CLIMA LAB`: unidades `C/F/%` bajan `1 px`.
- `PLANT LAB`: etiquetas de sensores `x+1`.
- `GRAPH`: nombres completos por sensor (`Temperatura aire`, `Humedad suelo`, `Temperatura sonda`, etc.) y banda superior en `LG_SENSOR_Y=27`.
- `TEMPERATURA`: card principal adopta limites comunes de la familia de cards.
- `TEMP LAB`: paleta mas cercana a Sensor Lab/Grafica/Home, `DS18B20` completo, nombre/unidad en colores distintos y barra diferencial `0..10C` / `0..18F` ampliable.
- `SOUND LAB`: sin `STACK/WAVE` ni hint de cambio de vista; estado usado como footer, chip/valor/grafica redistribuidos.
- `GAUGE LAB`, `VALOR LAB`, `TEMP CARD` y `PROBE CARD` recibieron ajustes de escala, rails, dato y paleta para aprovechar el espacio.
- Build verificado: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, Flash usada `914793 bytes`.
- Pendiente: flashear y capturar fotos; revisar especialmente HOME con tanque doble, Graph con textos largos y Temp Card con rail triple.

### Ajuste puntual y aprobaciones 2026-05-15

Ultima ronda aplicada y compilada:

- Aprobadas como referencia visual: `HOME`, `CLIMA LAB` y `GRÁFICA`.
- `TEMP LAB`: se elimina `DIF TEMP`; valor diferencial al centro inferior; barra diferencial `y-3`; `0` y los extremos `+10C/+18F` suben; DHT11 caliente llena hacia la izquierda y DS18B20 caliente hacia la derecha.
- `SOUND LAB`: se elimina el chip/card de `MIC`; `MIC` queda libre, valor `y-3` e indicador de límites abajo a la izquierda.
- `GRÁFICA`: la retícula horizontal de humedad se suaviza para no tener líneas blancas dominantes.
- `GAUGE LAB`: icono blanco `y+3`; unidad `C/F` naranja.
- `VALOR LAB`: barra inferior `y+1`, `DHT11 y-1`, gráfica `h+1`, valor/unidad `y+1`.
- `TEMP CARD` / `PROBE CARD`: indicador `x-3,y+4`; nombre de sensor sube y pasa a blanco para no quedar cortado por clears.
- Build verificado: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.1%`, Flash `69.7%`, Flash usada `914169 bytes`.
- Pendiente: flashear y validar `TEMP LAB`, `SOUND LAB`, `GAUGE LAB`, `VALOR LAB`, `TEMP CARD` y `PROBE CARD` en hardware.

### Corrección de código 2026-05-16

Ajuste puntual tras revisión de discrepancias entre el handoff y el código real:

- `GAUGE LAB` (`src/ui_lab_widget_showcase.cpp`): el icono de sensor pasó a `TFT_WHITE` cuando el sensor es válido. La ronda anterior lo documentaba como hecho, pero el código seguía usando `primary`.
- `GAUGE LAB` (`src/ui_lab_widget_showcase.cpp`): la unidad (`C/F`, `%`, etc.) pasó a `kWarmOrange` cuando el sensor es válido. Misma situación que el icono.
- `src/ui_lab_widget_showcase.cpp`: eliminadas 6 funciones `draw_icon_*_big` (dead code). Eran wrappers de iconos que nunca se llamaban y causaban errores de compilación por forward-reference al definirse antes que las funciones que invocaban (`draw_solid_drop_large`, `draw_solid_light_large`, `draw_solid_mic_large`).
- Build verificado: `SUCCESS`, RAM `14.1%`, Flash `69.8%`, Flash `915057 bytes`.
- Pendiente: igual que la ronda anterior — flashear y validar en hardware.

## 1. Inventario real actual

### Pantallas experimentales visibles hoy en el carrusel

Estas pantallas se pueden recorrer con el encoder porque no estan ocultas por la logica de `resolveVisibleAppScreen(...)`:

- `LAB_DUAL_TH_SCREEN`
- `LAB_WIDGET_MIX_SCREEN`
- `LAB_SOUND_VU_STACK_SCREEN`
- `LAB_LINEAR_DASH_SCREEN`
- `GRAPH_SCREEN`
- `LAB_GAUGE_TEMP_SCREEN`
- `LAB_VALUE_MODERN_SCREEN`
- `LAB_TEMP_CARD_SCREEN`
- `LAB_DS18_CARD_SCREEN`
- `LAB_HOME_CARDS_SCREEN`

Nota: `LAB_SOUND_VU_WAVE_SCREEN` no está como entrada directa; se alterna desde `LAB_SOUND_VU_STACK_SCREEN` con pulsación corta.

### Pantallas experimentales compiladas pero ocultas

Estas existen en firmware, pero el carrusel las salta a proposito:

- `LAB_ICON_SET_A_SCREEN`
- `LAB_ICON_SET_B_SCREEN`
- `LAB_ICON_SET_C_SCREEN`
- `LAB_ICON_SIZES_ENV_SCREEN`
- `LAB_ICON_SIZES_EXT_SCREEN`
- `LAB_DASH_OVERVIEW_SCREEN`
- `LAB_SENSOR_FOCUS_SCREEN`
- `LAB_ICON_TEST_SCREEN`

### Lectura general

Ahora mismo el firmware mezcla tres capas distintas dentro del mismo build:

- producto real
- laboratorio visual de pantallas
- laboratorio de iconografia / activos

Eso explica por que, despues de `Timer`, aparecen pantallas con roles muy distintos entre si.

## 2. Lectura por familias

### A. Pantallas de tendencia o analitica

#### `GRAPH_SCREEN`

Archivo principal:
- `src/ui_graph.cpp`

Rol:
- grafica a pantalla completa
- banda superior con sensor y valor actual
- foco en historico, no en configuracion

Fortalezas:
- rol claro y distinto
- aporta una funcion real, no solo estetica
- ya usa buffers historicos reales

Debilidades:
- ya rota por los 6 sensores con historico, pero falta validar que todos se lean igual de bien en hardware
- parte de su funcion ya aparece insinuada como sparkline en otras pantallas experimentales

Veredicto:
- candidata real de producto
- debe sobrevivir solo si se consolida como pantalla analitica generica

#### `LAB_SENSOR_FOCUS_SCREEN`

Archivo principal:
- `src/ui_lab_focus.cpp`

Rol:
- vista de detalle de un sensor
- valor grande arriba
- sparkline abajo
- ciclo interno entre 6 sensores: `Temp`, `Humidity`, `Light`, `Sound`, `Soil`, `DS18`

Fortalezas:
- es la propuesta mas fuerte de detalle unificado
- cubre los 6 sensores reales
- ya reutiliza los buffers historicos
- separa bien shell y contenido

Debilidades:
- hoy es pantalla de lectura, no de configuracion
- si entra como pantalla adicional junto a las pantallas actuales de cada sensor, duplica demasiado

Veredicto:
- no la recomiendo como pantalla extra en el arbol principal actual
- si la recomendaria como plantilla prioritaria para una futura v2 de las pantallas individuales

### B. Pantallas de resumen / overview

#### `LAB_DASH_OVERVIEW_SCREEN`

Archivo principal:
- `src/ui_lab_dash.cpp`

Rol:
- panel compacto de 4 filas
- `Temp`, `Hum`, `Light`, `Sound`

Fortalezas:
- muy limpia
- buena lectura rapida
- redraw fino por fila

Debilidades:
- se queda corta para el universo completo del P-Bit
- no muestra `Soil` ni `DS18`
- su funcion ya compite con `HOME CARDS` y `SENSOR LIST`

Veredicto:
- util como referencia de layout
- no la promoveria a producto si ya existe `HOME CARDS` o `LINEAR DASH`

#### `LAB_HOME_CARDS_SCREEN`

Archivo principal:
- `src/ui_lab_home_cards.cpp`

Rol:
- dashboard 2x2
- `Temp`, `Hum`, `Light`, `Sound`

Fortalezas:
- es la mejor candidata visual a "home" o "pantalla inicial"
- escaneo muy rapido
- mas amable y pedagogica que una lista densa

Debilidades:
- tampoco incluye `Soil` ni `DS18`
- si el producto exige que la portada represente todo el ecosistema, se queda parcial

Veredicto:
- recomendada como mejor candidata a `HOME` si se acepta que la portada cubra solo sensores ambientales integrados

#### `LAB_LINEAR_DASH_SCREEN`

Archivo principal:
- `src/ui_lab_linear_dash.cpp`

Rol:
- lista compacta por filas
- `Temp`, `Hum`, `Light`, `Sound`, `Soil`

Fortalezas:
- es la vista resumen mas completa
- muy eficiente para lectura tecnica o de docente
- barras segmentadas ayudan a comparar rapido

Debilidades:
- visualmente menos "producto" y menos amigable que `HOME CARDS`
- sigue faltando `DS18`
- en esta resolucion, una sexta fila empezaria a tensar mucho la pantalla

Veredicto:
- buena candidata para modo tecnico, diagnostico o docente
- no la pondria como home infantil por defecto

### C. Pantallas agrupadas por familia semantica

#### `LAB_DUAL_TH_SCREEN`

Archivo principal:
- `src/ui_lab_dual.cpp`

Rol:
- vista doble `Temperatura + Humedad`

Fortalezas:
- agrupacion semantica muy coherente
- lectura comparativa inmediata
- muy buena candidata si se quiere una familia "clima"
- ahora aprovecha la franja inferior con un resumen compacto de clima (`muy seco`, `fresco`, `óptimo`, `cálido`, `riesgo de moho`) en lugar de dejarla vacía

Debilidades:
- si se mantiene junto a `Temp`, `Humidity`, `Graph` y un posible `Home`, aumenta mucho la repeticion

Veredicto:
- candidata condicional
- solo tiene sentido si el producto adopta navegacion por familias

### D. Pantallas de widget / lenguaje visual

#### `LAB_GAUGE_TEMP_SCREEN`

Archivo principal:
- `src/ui_lab_widget_showcase.cpp`

Rol:
- prueba de gauge circular protagonista para temperatura

Veredicto:
- muy valiosa como biblioteca visual
- no la recomiendo como pantalla propia en navegacion principal

#### `LAB_VALUE_MODERN_SCREEN`

Archivo principal:
- `src/ui_lab_widget_showcase.cpp`

Rol:
- tarjeta moderna de sensor individual
- icono
- valor grande
- barra segmentada
- sparkline

Fortalezas:
- probablemente la mejor base para redisenar pantallas individuales
- lenguaje mas producto que las pantallas actuales

Debilidades:
- hoy solo aterrizada para temperatura
- como pantalla adicional duplica con `Temp` y con `Sensor Focus`

Veredicto:
- plantilla prioritaria
- no dejarla como pantalla extra; usarla para redisenar, no para sumar pantallas

#### `LAB_TEMP_CARD_SCREEN`

Archivo principal:
- `src/ui_lab_sensor_cards.cpp`

Rol:
- variante en card de la pantalla principal de temperatura
- mantiene valor protagonista
- sustituye el tanque por una torre segmentada compacta dentro de una tarjeta

Fortalezas:
- permite comparar directamente "pantalla actual vs card moderna"
- conserva legibilidad del dato
- introduce un lenguaje visual mas producto sin romper del todo la semantica actual

Debilidades:
- aun no demuestra si la card aporta mas que una buena pantalla de detalle unificada
- puede quedarse a medio camino entre refactor real y experimento visual

Veredicto:
- candidata de comparacion
- sirve para decidir si `Temp` merece migrar a un layout en card en una futura v2

#### `LAB_DS18_CARD_SCREEN`

Archivo principal:
- `src/ui_lab_sensor_cards.cpp`

Rol:
- variante en card de la pantalla principal de sonda externa
- conserva la lectura de temperatura y el hito visual de `0`

Fortalezas:
- aterriza el concepto de card en un sensor externo real
- permite comparar si la sonda tambien funciona mejor en tarjeta o si debe mantener un lenguaje distinto

Debilidades:
- necesita validacion visual en hardware, sobre todo por la escala del rail y la marca de cero

Veredicto:
- candidata de comparacion
- util para decidir si `DS18` debe alinearse visualmente con `Temp` o seguir con identidad propia

#### `LAB_WIDGET_MIX_SCREEN`

Archivo principal:
- `src/ui_lab_widget_showcase.cpp`

Rol:
- ahora visible como `TEMP LAB`
- compara temperatura ambiente contra sonda externa
- usa dos cards superiores y una grafica diferencial inferior

Veredicto:
- util para decidir si la comparacion `TEMP` vs `DS18` merece una pantalla propia
- sigue siendo una de las pantallas con mas riesgo de textos apretados; validar antes de promover

#### `LAB_SOUND_VU_STACK_SCREEN`

Archivo principal:
- `src/ui_lab_sound_vu.cpp`

Rol:
- propuesta de sonido tipo columnas LED segmentadas

Veredicto:
- candidata clara para evaluar un lenguaje mas vivo para `Sound`
- no pretende ser FFT real; es una visualizacion expresiva del nivel global

#### `LAB_SOUND_VU_WAVE_SCREEN`

Archivo principal:
- `src/ui_lab_sound_vu.cpp`

Rol:
- propuesta de sonido tipo pulso/onda centrada

Veredicto:
- candidata clara si se busca una estetica mas moderna y menos "ecualizador clasico"
- igual que la anterior, es interpretacion visual del nivel, no analisis espectral

### E. Pantallas de iconografia y activos

#### `LAB_ICON_SET_A/B/C_SCREEN`

Archivo principal:
- `src/ui_lab_icon_gallery.cpp`

Rol:
- comparar familias `OUTLINE`, `SOLID`, `PIXEL`

Veredicto:
- tooling puro
- bien ocultas

#### `LAB_ICON_SIZES_ENV_SCREEN` y `LAB_ICON_SIZES_EXT_SCREEN`

Archivo principal:
- `src/ui_lab_icon_sizes.cpp`

Rol:
- comparar tamanos `S/M/L`

Veredicto:
- tooling puro
- bien ocultas

#### `LAB_ICON_TEST_SCREEN`

Archivo principal:
- `src/ui_lab_icon_test.cpp`

Rol:
- comparar icono procedural vs bitmap RGB565

Fortalezas:
- util para decidir pipeline de iconos

Debilidades:
- hoy queda oculto del carrusel
- no aporta nada al usuario final

Veredicto:
- ya debe tratarse como pantalla interna de laboratorio

## 3. Solapes detectados

### Resumen general

Hoy hay tres candidatos distintos a "overview":

- `LAB_DASH_OVERVIEW_SCREEN`
- `LAB_HOME_CARDS_SCREEN`
- `LAB_LINEAR_DASH_SCREEN`

No deberian convivir en el producto final. Hay que elegir uno.

### Detalle de sensor

Hoy compiten por el mismo territorio:

- pantallas reales actuales por sensor
- `LAB_SENSOR_FOCUS_SCREEN`
- `LAB_VALUE_MODERN_SCREEN`
- `LAB_GAUGE_TEMP_SCREEN`

Conclusión:
- solo una debe ser pantalla de producto
- las otras deben quedar como fuente de lenguaje visual o plantilla

### Tendencia

Hoy la tendencia aparece en dos niveles:

- `GRAPH_SCREEN` como funcion principal
- `Sensor Focus` y `Value Lab` como sparkline integrada

Esto si puede convivir, pero solo si queda claro que:

- la sparkline es contexto rapido
- la grafica completa es analitica dedicada

### Iconografia

Hoy hay tres niveles distintos de trabajo de iconos:

- familia visual
- tamano
- formato tecnico procedural vs bitmap

Eso esta bien para laboratorio, pero no debe contaminar el arbol principal.

## 4. Que ya no hace falta crear

Con lo que existe hoy, no veo necesario crear mas pantallas nuevas para seguir explorando conceptos.

El problema ya no es falta de ideas visuales. El problema ahora es de arquitectura:

- elegir un solo overview
- decidir si habra familias agrupadas o solo sensores individuales
- decidir si `Sensor Focus` sustituye o no a las pantallas individuales en una futura v2
- decidir si `Graph` sera una pantalla generica de todos los sensores o solo de clima

La unica necesidad potencial de una pantalla nueva seria esta:

- una portada que resuma de forma coherente los 6 sensores reales sin saturar la pantalla

Pero incluso eso solo hace falta si se decide que la portada debe cubrir tambien los sensores externos (`Soil` y `DS18`).

## 5. Recomendacion de clasificacion

### Mantener como candidatas reales de producto

- `GRAPH_SCREEN`
- `LAB_HOME_CARDS_SCREEN`
- `LAB_DUAL_TH_SCREEN` solo si se adopta navegacion por familias

### Mantener como plantillas prioritarias para redisenar pantallas

- `LAB_SENSOR_FOCUS_SCREEN`
- `LAB_VALUE_MODERN_SCREEN`

### Mantener como biblioteca de widgets / referencia visual

- `LAB_GAUGE_TEMP_SCREEN`
- `LAB_WIDGET_MIX_SCREEN`
- `LAB_DASH_OVERVIEW_SCREEN`

### Mantener como laboratorio de iconografia o assets

- `LAB_ICON_SET_A_SCREEN`
- `LAB_ICON_SET_B_SCREEN`
- `LAB_ICON_SET_C_SCREEN`
- `LAB_ICON_SIZES_ENV_SCREEN`
- `LAB_ICON_SIZES_EXT_SCREEN`
- `LAB_ICON_TEST_SCREEN` solo como herramienta oculta

### Recomendacion directa

- mantener `LAB_ICON_TEST_SCREEN` fuera del carrusel visible
- no promover `LAB_DASH_OVERVIEW_SCREEN` si `HOME CARDS` o `LINEAR DASH` sobreviven
- no dejar `GAUGE LAB`, `VALUE LAB` y `WIDGET LAB` como pantallas extra permanentes

## 6. Estructura de navegacion recomendada

### Opcion recomendada ahora: cambio minimo y arbol claro

Esta es la opcion mas coherente si se quiere avanzar sin rehacer todo el firmware de navegacion:

1. `HOME`
2. `TEMP`
3. `HUM`
4. `LIGHT`
5. `SOUND`
6. `SOIL`
7. `DS18`
8. `GRAPH`
9. `TIMER`
10. `SYSTEM`

Recomendaciones para esta opcion:

- `HOME` deberia ser `LAB_HOME_CARDS_SCREEN`
- `GRAPH` sigue como pantalla de analitica, no como resumen
- `Sensor Focus`, `Value Lab`, `Gauge Lab`, `Widget Lab`, `Dash Overview`, `Linear Dash` y todos los labs de iconos quedan fuera del arbol principal

Ventajas:

- muy poco confusa
- solo introduce una repeticion intencional: la portada-resumen
- conserva intactos los menus actuales por sensor

Condicion:

- aceptar que `HOME` resume los sensores ambientales integrados, no necesariamente todos los externos

### Opcion recomendada a futuro: menos pantallas, mas lenguaje unificado

Esta opcion solo tiene sentido si se quiere una v2 de interfaz mas moderna y menos fragmentada:

1. `HOME`
2. `SENSOR FOCUS`
3. `GRAPH`
4. `TIMER`
5. `SYSTEM`

Donde:

- `HOME` presenta resumen
- `SENSOR FOCUS` se convierte en detalle generico de cualquier sensor
- la configuracion se abre contextualmente segun el sensor activo

Ventajas:

- elimina mucha duplicacion
- unifica lenguaje visual
- aprovecha mejor el trabajo de `Sensor Focus` y `Value Lab`

Coste:

- requiere refactor real de navegacion y de entrada a menus
- no es el siguiente paso mas barato

## 7. Que usaria en que momento

### Para producto real ahora

- `HOME CARDS` como portada
- pantallas actuales por sensor para operacion y configuracion
- `GRAPH` como analitica
- `TIMER`
- `SYSTEM`

### Para redisenar sin añadir mas pantallas

- `SENSOR FOCUS` como referencia principal de detalle
- `VALUE LAB` como referencia de composicion
- `GAUGE LAB` como fuente de widget hero
- `WIDGET LAB` como biblioteca de microcomponentes

### Para trabajo interno de diseño

- `ICON SET A/B/C`
- `ICON SIZES`
- `ICON TEST` oculto

## 8. Decision recomendada antes de seguir

Antes de crear mas pantallas, conviene cerrar estas tres decisiones:

1. Si la portada debe resumir solo sensores ambientales integrados o tambien externos.
2. Si `Graph` va a ser una pantalla analitica generica de todos los sensores o solo una vista de clima.
3. Si `Sensor Focus` es una futura sustitucion de las pantallas individuales o solo una referencia de diseño.

## 9. Conclusión

El material experimental ya es suficiente y valioso. No falta exploracion visual; falta ordenar roles.

La recomendacion mas sana es esta:

- elegir una sola portada
- no meter mas de una pantalla resumen en el flujo principal
- usar `Sensor Focus` y `Value Lab` como base de evolucion, no como duplicados permanentes
- mantener las pantallas de iconos como laboratorio interno

En otras palabras:

- no hacen falta mas pantallas
- hace falta decidir la arquitectura final de las que ya existen

## 10. Addendum 2026-04-23

Despues de este analisis se implementaron cuatro prototipos acotados para poder decidir con mas criterio visual en hardware:

- `LAB_SOUND_VU_STACK_SCREEN`
- `LAB_SOUND_VU_WAVE_SCREEN`
- `LAB_TEMP_CARD_SCREEN`
- `LAB_DS18_CARD_SCREEN`

Tambien se probo una capa de chips con icono sobre las pantallas principales actuales, pero la prueba en hardware mostro dos problemas:

- montaba el icono sobre el titulo
- repetia visualmente el nombre del sensor y hacia la cabecera mas pesada

Por ese motivo, esa prueba ya no se toma como direccion recomendada y los chips se retiraron de las pantallas habituales.

### Estado de las pruebas tras revision en hardware

- `LAB_ICON_TEST_SCREEN` queda oculto del carrusel por ahora
- `LAB_SOUND_VU_STACK_SCREEN` y `LAB_SOUND_VU_WAVE_SCREEN` siguen como pruebas activas de lenguaje para `Sound`
- `TEMP` y `HUM` pasan a probar una card superior para agrupar hint + valor sin rehacer toda la pantalla
- `LIGHT`, `SOUND`, `SOIL`, `DS18` y `GRAPH` vuelven a su cabecera limpia sin chip experimental
- `LAB_TEMP_CARD_SCREEN` y `LAB_DS18_CARD_SCREEN` se simplifican para priorizar lectura antes que ornamento

### Interpretacion recomendada de este addendum

- los VU son pruebas de lenguaje visual especificas para `Sound`
- las cards simplificadas de laboratorio sirven para evaluar legibilidad y jerarquia, no para cerrar aun una UI final
- la prueba mas util ahora mismo en pantallas reales es la card superior de `TEMP` y `HUM`
- los chips con icono no pasan esta ronda de evaluacion en hardware

## 11. Estructura activa 2026-04-23

Tras la siguiente ronda de ajustes, el carrusel visible deja de depender del orden bruto del enum y pasa a responder a una secuencia de producto mas clara.

### Orden visible recomendado que queda implementado ahora

1. `HOME`
2. `CLIMA LAB`
3. `TEMP LAB`
4. `SOUND LAB`
5. `PLANT LAB`
6. `SENSOR LAB`
7. `GRAPH`
8. pantallas individuales actuales por sensor
9. `SYSTEM`
10. `TIMER`
11. resto de labs todavia visibles para validacion puntual

### Decisiones aplicadas en esta ronda

- `HOME CARDS` pasa a llamarse solo `HOME`
- `SENSOR LIST` pasa a llamarse `PLANT LAB`
- `WIDGET LAB` se reutiliza como `TEMP LAB`
- `PLANT LAB` deja fuera `MIC` y queda con `TEMP`, `HUM`, `LUZ`, `SUELO` y `DS18`
- `GRAPH` ya no se limita a clima: con pulsacion corta rota por los 6 sensores con historico
- `TEMP LAB` entra como tercera pantalla visible y compara ambiente vs sonda
- `SOUND LAB` entra como cuarta pantalla y se presenta como una sola parada del carrusel
- `PLANT LAB` queda como quinta pantalla, todavia en formato lista mientras se valida una futura version en cards
- `SENSOR LAB` se mantiene como sexta pantalla de exploracion
- dentro de `SOUND LAB`, la vista inicial es `STACK` y la pulsacion corta alterna a `WAVE`
- `LAB_SOUND_VU_WAVE_SCREEN` deja de comportarse como pantalla separada del carrusel y pasa a ser una vista interna
- `ESTADO LAB` sale del flujo visible por ahora

### Ajustes visuales asociados

- `HOME` y `PLANT LAB` dejan de mostrar el footer `Vista experimental`
- `CLIMA LAB` usa ahora la franja inferior para una banda-resumen de estado climático, apoyada en umbrales reales de humedad y temperatura
- `TEMP LAB` pasa a usar dos cards superiores y una grafica diferencial inferior con `0` centrado para comparar `TEMP` y `DS18`
- `TEMP CARD` y `PROBE CARD` se recentran y se acercan mas al lenguaje de tarjeta usado en otras pantallas con cards
- `PLANT LAB` aun no pasa a cards en esta ronda para no mezclar reordenado y rediseño completo a la vez
- la logica de cards sigue siendo aun prototipo; la refactorizacion completa a templates reutilizables se pospone para una siguiente fase
