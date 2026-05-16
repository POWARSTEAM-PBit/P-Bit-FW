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
- `src/ui_icons.cpp` + `include/ui_icons.h` — biblioteca de iconos procedurales
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
| `LC_CARD_TOP` | 27 | Borde superior de card principal |
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

## Biblioteca de iconos (`ui_icons.cpp` / `ui_icons.h`)

Arquitectura: función `impl_*(cx, cy, color, int s)` por sensor, 3 wrappers públicos por variante.

| Tier | Sufijo | Factor s | Tamaño aprox | Uso |
|---|---|---|---|---|
| Small | `pbit_draw_*_icon` | 1 | ~14×14 px | `SensorIconDrawFn`, cards, listas |
| Large | `pbit_draw_*_icon_large` | 2 | ~28×28 px | Pantallas foco |
| XL | `pbit_draw_*_icon_xl` | 3 | ~42×42 px | Centro del gauge |

Iconos disponibles: `temp`, `probe`, `humidity`, `light`, `sound`, `plant`.

**Bug crítico resuelto (mayo 2026):** ícono humidity — el triángulo de la gota requiere base ±3*s (tangente exacta al círculo, `sqrt(5²−4²)=3`). Con ±5*s o ±7*s la punta se ve separada del círculo. El linter del proyecto revirtió esta corrección varias veces; la versión correcta está en `src/ui_icons.cpp` linea `impl_humidity`.

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
| `LAB_SENSOR_CARD_SCREEN` | Cicla entre los 6 sensores (TEMP→DS18→HUM→LIGHT→SOUND→SOIL) |
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
- Barra diferencial sin etiqueta central "0": escala `+10°C / +18°F` a cada lado (adapta a rango real del delta si supera 10°).
- Dirección: si DHT11 > DS18, llena hacia la izquierda; si DS18 > DHT11, llena hacia la derecha.
- Valor diferencial centrado abajo de la barra, posición Y ajustada `+3 px`.
- Estado: **pendiente de validación en hardware** — especialmente legibilidad de la barra diferencial y textos en idiomas largos.

### SOUND LAB (`LAB_SOUND_VU_STACK_SCREEN` / `LAB_SOUND_VU_WAVE_SCREEN`)

- Pantalla única de entrada: STACK. Pulsación corta alterna a WAVE (sub-vista interna).
- Sin chip/card de MIC; `MIC` queda como etiqueta suelta.
- Sin hint de cambio de vista en pantalla.
- Footer muestra el estado: `Normal`, `Loud`, etc.
- **Ajuste mayo 2026:** todos los elementos desplazados Y-1 (title chip y=25, badge y=24, meter area y=53, footer y=110).
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
- **Ajuste mayo 2026:** ícono XL (s=3, ~42×42 px) centrado; anillo desplazado X+4; etiquetas min/max en color neutro `0x6B6D` (antes eran primary/secondary).
- Icono `TFT_WHITE` cuando sensor válido, `TFT_DARKGREY` cuando no.
- Pulsación corta cicla entre sensores.
- Estado: **pendiente de validación en hardware**; biblioteca de widgets.

### VALOR LAB (`LAB_VALUE_MODERN_SCREEN`)

- Card ampliada: `x=2`, `y=27`, `w=156`, `h=100`, bottom `126`.
- Dato principal grande; unidad junto al valor (no debajo de la sparkline).
- Barra segmentada inferior con doble altura; sparkline con fondo sutil.
- `DHT11` `y-1`; barra inferior `y+1`; gráfica `h+1`; valor/unidad `y+1`.
- Pendiente: el color `off` de los segmentos de la barra puede estar demasiado oscuro (difícil distinguir de fondo).
- Estado: **pendiente de validación en hardware**; plantilla de referencia de composición.

### SENSOR CARD (`LAB_SENSOR_CARD_SCREEN`) — rediseñado mayo 2026

**Rediseño completo.** Sustituye el layout izquierda-valor / derecha-tanque por un layout de 3 zonas horizontales sin solapamiento de zonas de borrado.

#### Layout nuevo (card 156×100 px, y=27..126)

```
y=27..43  HEADER (17 px)  icon(s=1) + device_label (color secondary) + unit/status (TR)
                           separador y=44
y=45..81  VALUE  (37 px)  número grande centrado full-width (kValueCx=80)
                           separador y=82
y=83..110 VIZ    (28 px)  visualización horizontal específica por sensor
y=113..126 FOOTER          alert jewel (x=12, y=116)
```

Clave anti-overlap: `draw_card_dynamic` hace un `fillRect` completo antes de dibujar; ningún helper tiene `fillRect` parciales propios.

#### Visualizaciones por sensor

| Sensor | Viz | Rango | Detalle |
|---|---|---|---|
| TEMP (DHT11) | 12 segmentos gradiente azul→rojo | 0–50°C | Labels "0°"/"50°" |
| DS18B20 | 14 segmentos, azul hielo/cálido | -55..+125°C | Tick blanco en 0°, labels "-55°"/"+125°" |
| HUM (DHT11) | 10 gotas-pill en fila, cyan | 0–100% | Highlight dot interior, labels "0%"/"100%" |
| LIGHT (LDR) | 8 barras verticales crecientes | 0–1023 lux | Oscuro→amarillo, estilo ecualizador |
| SOUND (MIC) | 7 columnas VU misma altura | 0–100% | Verde/naranja/rojo por zona |
| SOIL | 3 zonas fijas + marcador | 0–100% | DRY(rojo)/OK(verde)/WET(azul), diamante posición |

- Estado: **pendiente de validación en hardware** — primer build con el nuevo diseño.

#### TEMP CARD / PROBE CARD (sub-vistas internas)

Accesibles desde SENSOR CARD con pulsación corta.
Usan el mismo sistema de vizualización horizontal descrito arriba.

---

## Pendiente de validación en hardware

Estas tareas no requieren código adicional salvo los ajustes que surjan al ver en pantalla real:

1. **SENSOR CARD (nuevo)** — validar toda la nueva geometría: que FONT_VALUE cabe en 37px, `drawSplitDecimalValue` centrado en cx=80, ícono small en header de 17px, marcador de SOIL no se sale del card (y≈124).
2. **VALOR LAB** — off-color segmentos de barra demasiado oscuro; ajustar si no se distingue del fondo.
3. **TEMP LAB** — barra diferencial izquierda/derecha, colores, legibilidad en idiomas con textos largos.
4. **Títulos largos** — confirmar que `TEMPORIZADOR`, `TERMÓMETRO`, `TEMPERATURA` no se recortan.
5. **SOUND LAB** — posición y claridad del texto de estado con el ajuste Y-1.
6. **GAUGE LAB** — ícono XL en centro, legibilidad del valor dentro del anillo, contraste.
7. Confirmar ausencia de parpadeo en redraw rápido (especialmente SENSOR LAB y ESTADO LAB).

### Decisión de producto pendiente

- Decidir si `TEMP CARD`, `PROBE CARD` y `TEMP LAB` se mantienen visibles en el carrusel o se ocultan.
- Decidir si `GAUGE LAB`, `VALOR LAB` y `SENSOR CARD` quedan como pantallas de laboratorio permanentes o se promueven.
- Decidir paleta general de pantallas madre (revisión pendiente de colores de fondo, acento y contraste global).

---

## Regla de documentación

Cada ajuste visual nuevo debe dejar una nota en este documento o en `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md`:

- archivo tocado
- motivo del cambio
- estado de build
- qué queda pendiente de validar en pantalla real

No rehacer pantallas desde cero sin pedirlo explícitamente. La base visual está conseguida; lo que corresponde ahora es retoque de microgeometría y validación en hardware.
