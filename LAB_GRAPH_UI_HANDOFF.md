# Handoff: Pantallas Lab / Graph

Fecha de actualización: 2026-04-10

Este documento resume el estado actual de las pantallas temporales de laboratorio y de la pantalla de gráfica para que otro agente pueda continuar sin perder contexto.

## Alcance

Estas iteraciones afectan solo a pantallas temporales o experimentales:

- `GRAPH_SCREEN`
- `LAB_DASH_OVERVIEW_SCREEN` (`ESTADO LAB`)
- `LAB_SENSOR_FOCUS_SCREEN` (`SENSOR LAB`)
- `LAB_DUAL_TH_SCREEN` (`CLIMA LAB`)
- `LAB_ICON_SET_A/B/C_SCREEN` (`OUTLINE / SOLID / PIXEL`)
- `LAB_GAUGE_TEMP_SCREEN` (`GAUGE LAB`)
- `LAB_VALUE_MODERN_SCREEN` (`VALOR LAB`)
- `LAB_WIDGET_MIX_SCREEN` (`WIDGET LAB`)

No se ha pretendido cerrar todavía el producto final. La idea es seguir usando estas pantallas como banco de pruebas visual y luego decidir qué se integra y qué se elimina.

## Archivos principales

Firmware:

- [src/ui_graph.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_graph.cpp)
- [src/ui_lab_dash.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_lab_dash.cpp)
- [src/ui_lab_focus.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_lab_focus.cpp)
- [src/ui_lab_dual.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_lab_dual.cpp)
- [src/ui_lab_icon_gallery.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_lab_icon_gallery.cpp)
- [src/ui_lab_widget_showcase.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_lab_widget_showcase.cpp)
- [src/ui_icons.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/ui_icons.cpp)
- [include/layout.h](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/include/layout.h)
- [include/languages.h](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/include/languages.h)
- [src/lang_select.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/lang_select.cpp)
- [include/tft_display.h](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/include/tft_display.h)
- [src/tft_display.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/tft_display.cpp)
- [src/rotary.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/src/rotary.cpp)

Snippets del visualizador:

- [visualizer_scenes/09_lab_graphs/00_estado_lab_overview.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/00_estado_lab_overview.cpp)
- [visualizer_scenes/09_lab_graphs/01_sensor_lab_temp.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/01_sensor_lab_temp.cpp)
- [visualizer_scenes/09_lab_graphs/02_sensor_lab_humidity.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/02_sensor_lab_humidity.cpp)
- [visualizer_scenes/09_lab_graphs/03_sensor_lab_light.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/03_sensor_lab_light.cpp)
- [visualizer_scenes/09_lab_graphs/04_sensor_lab_sound.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/04_sensor_lab_sound.cpp)
- [visualizer_scenes/09_lab_graphs/05_sensor_lab_soil_no_sensor.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/05_sensor_lab_soil_no_sensor.cpp)
- [visualizer_scenes/09_lab_graphs/06_graph_temp.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/06_graph_temp.cpp)
- [visualizer_scenes/09_lab_graphs/07_graph_humidity.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/07_graph_humidity.cpp)
- [visualizer_scenes/09_lab_graphs/08_clima_lab.cpp](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/visualizer_scenes/09_lab_graphs/08_clima_lab.cpp)

## Nuevas propuestas de widgets

Se añadió una tanda nueva de pantallas temporales para explorar widgets más expresivos usando el lenguaje visual `SOLID` como base. No usan `TFT_eWidget` todavía; esta tanda está hecha con primitivas `TFT_eSPI` para mantener control fino y bajo riesgo.

### `GAUGE LAB`

- Propuesta de anillo grande para temperatura.
- Icono sólido grande a la izquierda.
- Gauge multicolor tipo frio -> caliente.
- Valor centrado grande dentro del anillo.

Objetivo:

- validar si un gauge circular puede ser una buena pantalla “wow”
- medir legibilidad real del dato en formato radial

### `VALOR LAB`

- Propuesta de pantalla de sensor individual más moderna.
- Card principal con:
  - icono sólido integrado
  - etiqueta de sensor
  - valor grande
  - unidad destacada
  - barra segmentada
  - mini sparkline

Objetivo:

- reinterpretar las pantallas actuales de “dato grande + barra”
- comparar una versión más producto / dashboard

### `WIDGET LAB`

- Propuesta mixta con varios widgets en una sola pantalla:
  - humedad con tanque
  - luz con barra segmentada
  - sonido con VU bars

Objetivo:

- entender el potencial de mezclar tipos de visualización
- detectar qué widgets merecen luego una pantalla propia

## Iconografía

La galería `ICONOS A/B/C` se reconvirtió a familias grandes de iconos:

- `OUTLINE`
- `SOLID`
- `PIXEL`

Ahora muestran 4 iconos grandes por pantalla (`TEMP`, `HUM`, `SUELO`, `SONDA`) en vez de la comparación pequeña antigua.

Importante:

- si el P-Bit no muestra títulos `OUTLINE / SOLID / PIXEL`, no está cargando la build actual


## Estado actual

### 1. `ESTADO LAB`

Situación:

- Ya se rehizo con una lógica de 4 filas reales.
- El valor se redibuja sin repintar toda la fila, para evitar parpadeo del nombre e icono.
- Se subieron visualmente labels y valores:
  - labels: `-1 px`
  - valores: `-2 px`
- Además, se desplazó todo el contenido interno del card `Y - 2 px` para que la última fila no roce el borde inferior.
- Se simplificó la regla de color: `label blanco + un solo acento por sensor`.

Paleta actual:

- Header: `naranja vivo`
- Borde card: azul frío tenue (`0x39CC`)
- `TEMP`: naranja
- `HUM`: cyan
- `LUZ`: amarillo fosforescente
- `MIC`: magenta

Pendientes:

- Validar en hardware si el centrado visual de las 4 filas ya se siente homogéneo.
- Confirmar si esta regla `blanco + un acento` ya convence más que la mezcla anterior.
- Confirmar si sigue existiendo cualquier microflicker cuando cambia solo un dato.

### 2. `SENSOR LAB`

Situación:

- `TEMP` sigue siendo el patrón bueno de referencia.
- Se unificó la geometría principal alrededor del patrón de `TEMP`:
  - mismo `title x/y`
  - mismo `value y`
  - mismo `main font` para todos los valores
  - iconos centrados sobre una misma caja visual
- Se implementó mezcla de dos colores por sensor:
  - `TEMP`: naranja + magenta
  - `HUM`: cyan + morado
  - `LIGHT`: amarillo + cyan
  - `SOUND`: magenta + verde
  - `SOIL`: verde + cyan
  - `DS18`: morado + cyan
- Se separó redraw de shell y contenido.
- La gráfica inferior usa retícula uniforme y verticales más visibles que horizontales.

Offsets ya previstos en firmware:

- `TEMP`: valor `y + 1`
- `HUM`: icono más a la izquierda y valor `y + 2`
- `LIGHT`: icono más a la izquierda y valor `y + 2`
- `SOUND`: icono más a la izquierda y valor `y + 3`
- `SOIL`: icono más a la izquierda, valor `y + 2`, “Sin sensor” superior e inferior recolocados
- `DS18`: icono más a la izquierda, valor `y + 1`, “Sin sensor” superior e inferior recolocados

Pendientes:

- Validar en hardware que la unificación ya dejó todos los valores con el mismo peso visual que `TEMP`.
- Revisar si el contenido de `SOUND` todavía vibra más de la cuenta por la naturaleza de la señal.
- Valorar si `unit` debería ganar un poco más de contraste en `LIGHT` y `DS18`.

### 3. `CLIMA LAB`

Situación:

- El layout actual gusta y se conserva.
- `TEMP` y `HUM` ahora tienen unidades con más presencia visual.
- Los clears del valor ya no deberían comerse el icono, especialmente el termómetro.
- Se separó shell/contenido para evitar refresco completo.

Paleta actual:

- `TEMP`: naranja vivo + acento magenta
- `HUM`: cyan + acento morado
- Fondo negro con estructura gris oscura

Pendientes:

- Revisar en hardware si `C/F` y `%` ya tienen el peso visual correcto o si conviene agrandarlos un paso más.
- Confirmar si el clear del valor de `TEMP` ya no invade el termómetro en ningún frame.
- Confirmar que el panel derecho (`HUM`) se siente igual de rico que el izquierdo.

### 4. `GRAPH`

Situación:

- La banda superior ya no se repinta en cada frame: solo cuando cambia el valor visible o el sensor.
- La línea del gráfico ya usa `graph_line_color(...)`.
- La retícula ya es uniforme y forma cuadrícula.
- Las verticales son más visibles que las horizontales.
- Las horizontales ahora están tematizadas:
  - `TEMP`: bandas más frías a cálidas
  - `HUM`: bandas de azul

Paleta actual:

- `GRAPH TEMP`:
  - línea verde brillante
  - verticales moradas oscuras
  - horizontales en bandas frías->cálidas atenuadas
  - label de banda superior en cyan
- `GRAPH HUM`:
  - línea cyan
  - verticales violeta suave
  - horizontales en azules más desaturados
  - máximo en blanco
  - mínimo azul claro

Pendientes:

- Confirmar en hardware si las horizontales aún compiten demasiado con la curva.
- Confirmar que la cuadrícula realmente se percibe como “cuadros”.
- Decidir si la etiqueta superior de `HUM` se queda en magenta o vuelve a cyan.

## Redraw / flicker

La dirección actual es esta:

- `ESTADO LAB`: solo redibujar el valor de la fila cuando cambia el dato.
- `SENSOR LAB`: redibujar shell solo si cambian sensor/paleta; redibujar contenido si cambia dato.
- `CLIMA LAB`: shell por panel solo si cambia color/unidad/estado; contenido por panel cuando cambia valor.
- `GRAPH`: banda superior y borde no se repintan salvo cambio real; el sprite de la gráfica se actualiza cuando cambia sensor o histórico.

Pendiente global:

- Verificación hardware real. El código ya separa bastante mejor redraw estático y dinámico, pero la comprobación final de vibración sigue dependiendo del P-Bit real.

## Build verificada

Última compilación correcta:

- RAM: `45,652 bytes`
- Flash: `895,425 bytes`

Comando usado:

```powershell
py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"
```

## Siguiente paso recomendado

Hacer una ronda corta de validación en hardware real, en este orden:

1. `ESTADO LAB`
2. `SENSOR LAB / HUM`
3. `SENSOR LAB / LIGHT`
4. `SENSOR LAB / SOUND`
5. `SENSOR LAB / SOIL`
6. `SENSOR LAB / DS18`
7. `CLIMA LAB`
8. `GRAPH TEMP`
9. `GRAPH HUM`

Y devolver feedback corto tipo:

- `Estado Lab fila 3 bajar 1`
- `Hum unit x -1`
- `Sound sigue vibrando`
- `Clima % ok`
- `Graph temp horizontales aún muy visibles`

## Nota importante para otro agente

No rehacer estas pantallas desde cero.

La base visual ya está bastante conseguida. Lo correcto ahora es:

- retocar microgeometría en hardware
- validar paletas
- y cerrar flicker residual

No abrir cambios grandes de arquitectura salvo que el usuario lo pida explícitamente.
