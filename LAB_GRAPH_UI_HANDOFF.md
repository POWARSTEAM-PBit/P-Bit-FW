# Handoff: Pantallas Lab / Graph

Actualizado: 2026-05-16

## Build actual verificada

- Herramienta: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"`
- Resultado: `SUCCESS`
- RAM: `14.1%` (46044 / 327680 bytes)
- Flash: `69.8%` (915281 / 1310720 bytes)
- Fecha: `2026-05-16`

Todo el código de ajustes visuales de las rondas anteriores está commiteado en `ce41581`. El árbol git está limpio (solo el log de debug modificado).

---

## Alcance del build LAB

Las pantallas de laboratorio y gráfica solo se compilan cuando `PBIT_ENABLE_GRAPH_LAB = 1` en `include/config.h`. Ese flag está activo en el build actual.

Archivos principales:

- `src/ui_lab_home_cards.cpp` — HOME CARDS
- `src/ui_lab_dual.cpp` — CLIMA LAB
- `src/ui_lab_widget_showcase.cpp` — TEMP LAB, GAUGE LAB, VALOR LAB
- `src/ui_lab_sound_vu.cpp` — SOUND LAB (STACK + WAVE)
- `src/ui_lab_linear_dash.cpp` — PLANT LAB
- `src/ui_lab_focus.cpp` — SENSOR LAB
- `src/ui_graph.cpp` — GRÁFICA
- `src/ui_lab_sensor_cards.cpp` — TEMP CARD, PROBE CARD, SENSOR CARD
- `src/ui_lab_dash.cpp` — ESTADO LAB (oculto)
- `src/ui_lab_icon_gallery.cpp` — galerías de iconos (ocultas)
- `src/ui_lab_icon_sizes.cpp` — tamaños de iconos (ocultas)
- `src/ui_lab_icon_test.cpp` — icon test (oculto)
- `include/layout.h` — constantes globales de geometría
- `include/languages.h` — claves de texto multilenguaje
- `src/tft_display.cpp` — router de pantallas
- `src/rotary.cpp` — navegación y orden del carrusel

---

## Reglas de geometría globales (layout.h)

### Header común (todas las pantallas)

| Constante | Valor | Descripción |
| --- |---| --- |
| `L_HEADER_Y` | 18 | Baseline del título |
| `L_HEADER_LINE` | 23 | Y de la línea divisoria |
| `L_CONTENT_TOP` | 27 | Primera Y útil bajo el header |
| `LC_MASTER_HEADER_LINE_X` | 4 | X inicial de la línea |
| `LC_MASTER_HEADER_LINE_W` | 152 | Ancho de la línea |

### Cards externas (lab screens)

| Constante | Valor | Descripción |
| --- |---| --- |
| `LC_SCREEN_X` | 2 | Margen lateral común |
| `LC_SCREEN_W` | 156 | Ancho exterior común |
| `LC_SCREEN_BOTTOM` | 126 | Límite inferior común |
| `LC_CARD_RADIUS` | 4 | Radio de esquinas |
| `LC_GAP` | 4 | Gap entre cards/bloques |
| `LC_FOOTER_Y` | 120 | Baseline del footer |

### Cards 2×2 (HOME y familia)

| Constante | Valor | Descripción |
| --- |---| --- |
| `LC_MASTER_CARD_X0` | 2 | X card izquierda |
| `LC_MASTER_CARD_X1` | 82 | X card derecha |
| `LC_MASTER_CARD_Y0` | 27 | Y fila superior |
| `LC_MASTER_CARD_Y1` | 79 | Y fila inferior |
| `LC_MASTER_CARD_W` | 76 | Ancho de cada card |
| `LC_MASTER_CARD_H` | 48 | Alto de cada card |
| `LC_MASTER_CARD_GAP` | 4 | Gap entre cards |
| `LC_MASTER_CARD_BOTTOM` | 126 | Bottom de la fila inferior |

### Gráfica (GRÁFICA / GRAPH_SCREEN)

| Constante | Valor | Descripción |
| --- |---| --- |
| `LG_SENSOR_Y` | 27 | Banda sensor/valor (entre header y gráfica) |
| `LG_GRAPH_X` | 2 | Borde exterior izquierdo |
| `LG_GRAPH_Y` | 46 | Borde exterior superior |
| `LG_GRAPH_W` | 154 | Ancho interior del sprite |
| `LG_GRAPH_H` | 64 | Alto interior del sprite |
| `LG_HINT_Y` | 120 | Footer hint |

---

## Carrusel visible actual

Definido en `src/rotary.cpp` → `kVisibleAppScreens[]`:

| Posición | Nombre visible | Enum | Archivo |
| --- |---| --- |---|
| 1 | HOME | `LAB_HOME_CARDS_SCREEN` | `ui_lab_home_cards.cpp` |
| 2 | CLIMA LAB | `LAB_DUAL_TH_SCREEN` | `ui_lab_dual.cpp` |
| 3 | TEMP LAB | `LAB_WIDGET_MIX_SCREEN` | `ui_lab_widget_showcase.cpp` |
| 4 | SOUND LAB | `LAB_SOUND_VU_STACK_SCREEN` | `ui_lab_sound_vu.cpp` |
| 5 | PLANT LAB | `LAB_LINEAR_DASH_SCREEN` | `ui_lab_linear_dash.cpp` |
| 6 | SENSOR LAB | `LAB_SENSOR_FOCUS_SCREEN` | `ui_lab_focus.cpp` |
| 7 | GRÁFICA | `GRAPH_SCREEN` | `ui_graph.cpp` |
| 8 | TEMP | `TEMP_SCREEN` | `ui_temp.cpp` |
| 9 | HUM | `HUMIDITY_SCREEN` | `ui_humidity.cpp` |
| 10 | LUZ | `LIGHT_SCREEN` | `ui_light.cpp` |
| 11 | SONIDO | `SOUND_SCREEN` | `ui_sound.cpp` |
| 12 | SUELO | `SOIL_SCREEN` | `ui_soil.cpp` |
| 13 | DS18 | `DS18B20_SCREEN` | `ui_ds18.cpp` |
| 14 | SISTEMA | `SYSTEM_SCREEN` | `ui_system.cpp` |
| 15 | TIMER | `TIMER_SCREEN` | `ui_timer.cpp` |
| 16 | GAUGE LAB | `LAB_GAUGE_TEMP_SCREEN` | `ui_lab_widget_showcase.cpp` |
| 17 | VALOR LAB | `LAB_VALUE_MODERN_SCREEN` | `ui_lab_widget_showcase.cpp` |
| 18 | SENSOR CARD | `LAB_SENSOR_CARD_SCREEN` | `ui_lab_sensor_cards.cpp` |

### Pantallas fuera del carrusel (compiladas, ocultas)

Definido en `isHiddenRestoreScreen()` en `src/rotary.cpp`:

- `LAB_DASH_OVERVIEW_SCREEN` — ESTADO LAB
- `LAB_ICON_SET_A_SCREEN` / `B` / `C` — galerías de iconos
- `LAB_ICON_SIZES_ENV_SCREEN` / `EXT` — tamaños de iconos
- `LAB_ICON_TEST_SCREEN` — test procedural vs bitmap

Pantallas en el enum pero tampoco en el carrusel (sub-vistas internas):

- `LAB_SOUND_VU_WAVE_SCREEN` — accesible desde SOUND LAB con pulsación corta
- `LAB_TEMP_CARD_SCREEN` — accesible desde SENSOR CARD con pulsación corta
- `LAB_DS18_CARD_SCREEN` — accesible desde SENSOR CARD con pulsación corta

### Comportamiento de pulsación corta por pantalla

| Pantalla | Acción |
| --- |---|
| `GRAPH_SCREEN` | Cicla entre los 6 sensores con historial |
| `LAB_SENSOR_FOCUS_SCREEN` | Cicla entre los 6 sensores |
| `LAB_SOUND_VU_STACK_SCREEN` | Alterna a `LAB_SOUND_VU_WAVE_SCREEN` |
| `LAB_SOUND_VU_WAVE_SCREEN` | Vuelve a `LAB_SOUND_VU_STACK_SCREEN` |
| `LAB_SENSOR_CARD_SCREEN` | Cicla entre TEMP CARD y PROBE CARD |
| `LAB_GAUGE_TEMP_SCREEN` | Cicla entre sensores del gauge |

---

## Estado de cada pantalla

### HOME (`LAB_HOME_CARDS_SCREEN`)

- Layout 2×2 con Temp, Hum, Luz, Sound.
- Geometría validada: cards en `x=2/82`, `y=27/79`, `w=76`, `h=48`, gap `4`, bottom `126`.
- Icono, etiqueta y valor de cada card en `x+2, y+2` del origen de la card.
- Tanque lateral de humedad: doble ancho, crece hacia la izquierda.
- Footer `Vista experimental` eliminado.
- Bug conocido: artefacto en esquina superior derecha de la card MIC al arrancar desde el selector de idioma. No bloqueante para validación.
- Estado: **aprobada como referencia visual**.

### CLIMA LAB (`LAB_DUAL_TH_SCREEN`)

- Dos panels superiores Temp + Hum en `x=2/82`, `y=27`, `w=76`, `h=73`.
- Footer de estado climático en `x=2`, `y=104`, `w=156`, `h=23`, bottom `126`.
- Estado en footer: `Muy seco`, `Fresco`, `Óptimo`, `Cálido`, `Riesgo de moho` según umbrales reales.
- Barras internas en `y=89`, altura `9 px`.
- Texto `Óptimo` centrado en `y=115`.
- Iconos internos `x+2`, unidades `C/F/%` `x-1`, `y+1`.
- Estado: **aprobada como referencia visual**.

### TEMP LAB (`LAB_WIDGET_MIX_SCREEN`)

- Dos cards superiores en `x=2/82`, `y=27`, `w=76`, `h=50`: `DHT11` (izquierda) y `DS18B20` (derecha).
- Fondos diferenciados, nombre en color y unidad `C/F` en esquina superior derecha de cada card.
- Card inferior de diferencia en `x=2`, `y=81`, `w=156`, `h=46`, bottom `126`.
- Barra diferencial sin texto "DIF TEMP": `0` centrado, escala `+10 C / +18 F` a cada lado.
- Dirección: si DHT11 > DS18, llena hacia la izquierda; si DS18 > DHT11, llena hacia la derecha.
- Valor diferencial centrado abajo de la barra, `y` ajustado `+3 px` respecto a ronda anterior.
- Estado: **pendiente de validación en hardware** — especialmente legibilidad de la barra diferencial y textos en idiomas largos.

### SOUND LAB (`LAB_SOUND_VU_STACK_SCREEN` / `LAB_SOUND_VU_WAVE_SCREEN`)

- Pantalla única de entrada: STACK. Pulsación corta alterna a WAVE (sub-vista interna).
- Sin chip/card de MIC; `MIC` queda como etiqueta suelta.
- Sin hint de cambio de vista en pantalla.
- Footer muestra el estado: `Normal`, `Loud`, etc.
- Valor `y-3` respecto a posición anterior; indicador de límites en esquina inferior izquierda.
- Estado: **pendiente de validación en hardware**.

### PLANT LAB (`LAB_LINEAR_DASH_SCREEN`)

- Lista compacta: Temp, Hum, Luz, Suelo, DS18 (sin Mic).
- Filas en `y=27/47/67/87/107`, alto `16`, gap visual `4`.
- Nombres de sensores `x+1`.
- Estado: **pendiente de validación en hardware**.

### SENSOR LAB (`LAB_SENSOR_FOCUS_SCREEN`)

- Carrusel interno de 6 sensores: Temp, Hum, Luz, Sound, Suelo, DS18.
- Pulsación corta cicla entre ellos.
- Visible en el carrusel principal (posición 6).
- Reutiliza buffers históricos reales.
- Summary en `x=2`, `y=27`, `w=156`, `h=42`; gráfica en `x=2`, `y=73`, `w=156`, `h=42`.
- Estado: **referencia visual fuerte para futura v2 de pantallas individuales**.

### GRÁFICA (`GRAPH_SCREEN`)

- Gráfica a pantalla completa con banda superior sensor/valor.
- Banda superior: `LG_SENSOR_Y=27`.
- Marco exterior: `x=2`, `y=46`, `w=156` (borde incluido), `h=66`, radio `4`.
- Sprite interior: `LG_GRAPH_W=154`, `LG_GRAPH_H=64`.
- Pulsación corta cicla entre 6 sensores: Temp aire, Hum aire, Luz, Sonido, Hum suelo, DS18.
- Líneas horizontales de humedad: progresión azul suave (sin llegar a blanco).
- Etiquetas largas por sensor via claves `GRAPH_LABEL_*`.
- Estado: **aprobada como referencia visual**.

### GAUGE LAB (`LAB_GAUGE_TEMP_SCREEN`)

- Gauge circular protagonista: radio `r=32`, valor centrado.
- Icono de sensor grande, centrado verticalmente, color `TFT_WHITE` cuando el sensor es válido.
- Etiqueta del sensor centrada junto al icono.
- Unidad `C/F/%` etc. en `kWarmOrange` cuando el sensor es válido.
- `TFT_DARKGREY` para icono y unidad cuando el sensor no es válido.
- Pulsación corta cicla entre sensores.
- Estado: **pendiente de validación en hardware**; biblioteca de widgets.

### VALOR LAB (`LAB_VALUE_MODERN_SCREEN`)

- Card ampliada: `x=2`, `y=27`, `w=156`, `h=100`, bottom `126`.
- Dato principal grande; unidad junto al valor (no debajo de la sparkline).
- Barra segmentada inferior con doble altura; sparkline con fondo sutil.
- `DHT11` `y-1`; barra inferior `y+1`; gráfica `h+1`; valor/unidad `y+1`.
- Estado: **pendiente de validación en hardware**; plantilla de referencia de composición.

### SENSOR CARD (`LAB_SENSOR_CARD_SCREEN`)

- Galería de cards de sensor; pulsación corta cicla entre TEMP CARD y PROBE CARD.
- Enums internos: `LAB_TEMP_CARD_SCREEN` y `LAB_DS18_CARD_SCREEN` (no entran directamente al carrusel).

#### TEMP CARD (`LAB_TEMP_CARD_SCREEN`)

- Card única: `x=2`, `y=27`, `w=156`, `h=100`, bottom `126`.
- Rail en `x=140`, `y=52`, `w=12`, `h=62`.
- Nombre del sensor arriba, color blanco.
- Indicador de límites `x-3, y+4` respecto a ronda anterior.
- Estado: **pendiente de validación en hardware**.

#### PROBE CARD (`LAB_DS18_CARD_SCREEN`)

- Misma estructura que TEMP CARD pero para DS18B20.
- Marca de cero en el rail: pendiente de confirmar escala en hardware.
- Estado: **pendiente de validación en hardware**.

### ESTADO LAB (`LAB_DASH_OVERVIEW_SCREEN`) — oculto

- 4 filas: Temp, Hum, Luz, Sound.
- Redraw por fila (solo cambia lo que cambia).
- No muestra Suelo ni DS18.
- Oculto del carrusel; útil como referencia de layout de lista.

### Galerías de iconos — ocultas

- `LAB_ICON_SET_A/B/C_SCREEN` — familias OUTLINE / SOLID / PIXEL.
- `LAB_ICON_SIZES_ENV_SCREEN` / `EXT` — tamaños S/M/L.
- `LAB_ICON_TEST_SCREEN` — comparativa procedural vs bitmap.
- Tooling interno. No forman parte de ningún flujo de producto.

---

## Pendiente de validación en hardware

Estas tareas no requieren código adicional. Son verificación visual en pantalla real:

1. Distancia línea–card de `3 px` y bottom común `126` — confirmar en todas las pantallas.
2. **TEMP LAB** — barra diferencial izquierda/derecha, colores, legibilidad. Revisar en idiomas con textos largos (`Sonda` → `Sense sensor`).
3. **Títulos largos** — confirmar que `TEMPORIZADOR`, `TERMÓMETRO`, `TEMPERATURA` no se recortan.
4. **SOUND LAB** — posición y claridad del texto de estado; comportamiento del indicador de límites.
5. **GAUGE LAB** — legibilidad del valor dentro del anillo; contraste icono blanco sobre fondo.
6. **VALOR LAB** — lectura del dato grande; distinción sparkline/barra.
7. **TEMP CARD / PROBE CARD** — rail, escala y marca de cero.
8. Confirmar ausencia de parpadeo en redraw rápido (especialmente SENSOR LAB y ESTADO LAB).

### Decisión de producto pendiente

- Decidir si `TEMP CARD`, `PROBE CARD` y `TEMP LAB` se mantienen visibles en el carrusel o se ocultan.
- Decidir si `GAUGE LAB`, `VALOR LAB` y `SENSOR CARD` quedan como pantallas de laboratorio permanentes o se promueven.

---

## Regla de documentación

Cada ajuste visual nuevo debe dejar una nota en este documento o en `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md`:

- archivo tocado
- motivo del cambio
- estado de build
- qué queda pendiente de validar en pantalla real

No rehacer pantallas desde cero sin pedirlo explícitamente. La base visual está conseguida; lo que corresponde ahora es retoque de microgeometría y validación en hardware.
