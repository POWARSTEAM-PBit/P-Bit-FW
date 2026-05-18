# Handoff: Pantallas Lab / Graph

Actualizado: 2026-05-17

## Build actual verificada

- Herramienta: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"`
- Resultado: `SUCCESS`
- RAM: `14.1%` (46044 / 327680 bytes)
- Flash: `70.4%` (922773 / 1310720 bytes)
- Fecha: `2026-05-17`

Sesión 2026-05-17: implementada arquitectura Sensor Zone completa (carrusel plano de 12 posiciones, `sensor_zone.h/cpp`, persistencia NVS, routing en `tft_display.cpp`, navegación en `rotary.cpp`). Ajustes visuales: medidor TEMP LAB ampliado 2px arriba, vizualizaciones Sensor Card bajadas 2px. Pending: naming system en idiomas, sz_is_active() para fix de title flash, paleta canónica P3/P4.

---

## Alcance del build LAB

Las pantallas de laboratorio y gráfica solo se compilan cuando `PBIT_ENABLE_GRAPH_LAB = 1` en `include/config.h`. Ese flag está activo en el build actual.

Archivos principales:

- `src/sensor_zone.cpp` / `include/sensor_zone.h` — **NUEVO** estado central de la zona de sensores
- `src/settings_store.cpp` / `include/settings_store.h` — persistencia NVS sensor zone
- `src/ui_lab_home_cards.cpp` — HOME CARDS
- `src/ui_lab_dual.cpp` — CLIMA LAB
- `src/ui_lab_widget_showcase.cpp` — TEMP LAB, GAUGE LAB, VALOR LAB
- `src/ui_lab_sound_vu.cpp` — SOUND LAB (STACK + WAVE)
- `src/ui_lab_linear_dash.cpp` — PLANT LAB
- `src/ui_lab_focus.cpp` — sub-renderer FOCUS (panel valor + mini-gráfica)
- `src/ui_graph.cpp` — sub-renderer GRÁFICA
- `src/ui_lab_sensor_cards.cpp` — sub-renderer CARD
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

## Carrusel actual — arquitectura plana de 12 posiciones

Definido en `src/rotary.cpp` → `kCarousel[]` (struct `CarouselEntry{Screen, int8_t sensor}`).

| Pos | Nombre header | Screen enum | Sensor |
|-----|--------------|-------------|--------|
| 1 | HOME | `LAB_HOME_CARDS_SCREEN` | — |
| 2 | CLIMA LAB | `LAB_DUAL_TH_SCREEN` | — |
| 3 | TEMP LAB | `LAB_WIDGET_MIX_SCREEN` | — |
| 4 | SOUND LAB | `LAB_SOUND_VU_STACK_SCREEN` | — |
| 5 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_TEMP` |
| 6 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_HUM` |
| 7 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_LIGHT` |
| 8 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_SOUND` |
| 9 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_SOIL` |
| 10 | *(viz del sensor)* | `SENSOR_ZONE_SCREEN` | `SZ_DS18` |
| 11 | TIMER | `TIMER_SCREEN` | — |
| 12 | SISTEMA | `SYSTEM_SCREEN` | — |

Las posiciones 5–10 comparten el mismo `Screen` (`SENSOR_ZONE_SCREEN`) pero cada una fija el sensor activo vía `sz_set_sensor()` al aterrizar en ella. El encoder navega linealmente entre las 12 posiciones sin niveles secundarios.

### Pantallas fuera del carrusel (compiladas, ocultas)

- `LAB_DASH_OVERVIEW_SCREEN` — ESTADO LAB
- `LAB_ICON_SET_A/B/C_SCREEN` — galerías de iconos
- `LAB_ICON_SIZES_ENV/EXT_SCREEN` — tamaños de iconos
- `LAB_ICON_TEST_SCREEN` — test procedural vs bitmap
- `LAB_SENSOR_FOCUS_SCREEN` — sub-renderer de `SENSOR_ZONE_SCREEN` (viz FOCUS)
- `LAB_GAUGE_TEMP_SCREEN` — sub-renderer de `SENSOR_ZONE_SCREEN` (viz GAUGE)
- `LAB_VALUE_MODERN_SCREEN` — sub-renderer de `SENSOR_ZONE_SCREEN` (viz VALOR)
- `LAB_SENSOR_CARD_SCREEN` — sub-renderer de `SENSOR_ZONE_SCREEN` (viz CARD)
- `GRAPH_SCREEN` — sub-renderer de `SENSOR_ZONE_SCREEN` (viz GRAF)
- `LAB_SOUND_VU_WAVE_SCREEN` — sub-vista interna de SOUND LAB (pulsación corta)

### Comportamiento de pulsación por pantalla

| Pantalla / contexto | Pulsación corta | Pulsación larga |
|---------------------|-----------------|-----------------|
| `SENSOR_ZONE_SCREEN` | Cicla viz mode del sensor activo | Abre menú de config del sensor |
| `LAB_SOUND_VU_STACK_SCREEN` | Alterna a WAVE | — |
| `LAB_SOUND_VU_WAVE_SCREEN` | Vuelve a STACK | — |
| Resto del carrusel | Sin acción | — |

Después de salir de un menú de config (TEMP_SCREEN, HUMIDITY_SCREEN, etc.), `configure_app_rotary_bounds` detecta que la pantalla activa no está en el carrusel y vuelve automáticamente a `SENSOR_ZONE_SCREEN` con el sensor correcto.

---

## Arquitectura Sensor Zone

### Módulo `sensor_zone.h / sensor_zone.cpp`

Estado central para la zona de los 6 sensores individuales. Desacopla la navegación (rotary.cpp) del rendering (tft_display.cpp).

#### Enums

```cpp
enum SzSensorId : uint8_t {
    SZ_TEMP=0, SZ_HUM, SZ_LIGHT, SZ_SOUND, SZ_SOIL, SZ_DS18, SZ_SENSOR_COUNT
};

enum SzVizMode : uint8_t {
    SZ_VIZ_FOCUS=0,  // PENDIENTE: cambiar default de CARD a FOCUS
    SZ_VIZ_CARD,
    SZ_VIZ_VALOR,
    SZ_VIZ_GRAPH,
    SZ_VIZ_GAUGE,
    SZ_VIZ_COUNT
};
```

#### API pública

| Función | Descripción |
|---------|-------------|
| `sz_init()` | Carga sensor y viz de NVS al arranque |
| `sz_get_sensor()` | Sensor activo actual |
| `sz_get_viz()` | Viz mode del sensor activo |
| `sz_set_sensor(uint8_t)` | Salta a sensor por índice, persiste, solicita redraw |
| `sz_next_sensor()` | Avanza sensor (wraps), persiste |
| `sz_prev_sensor()` | Retrocede sensor (wraps), persiste |
| `sz_next_viz()` | Cicla viz mode del sensor activo, persiste |
| `sz_sync_renderer(bool force)` | Sincroniza sub-renderer; tracking interno evita calls redundantes |
| `sz_sensor_name(SzSensorId)` | Nombre display del sensor (actualmente hardcoded ES — **pendiente: usar LangKey**) |
| `sz_sensor_rgb(SzSensorId, r, g, b)` | Color LED para el sensor activo |

#### Persistencia NVS

| Clave | Tipo | Descripción |
|-------|------|-------------|
| `"sz_sen"` | `UChar` | Sensor activo (0–5) |
| `"sz_v0"` .. `"sz_v5"` | `UChar` | Viz mode por sensor (0–4) |

#### Routing en tft_display.cpp

```cpp
case SENSOR_ZONE_SCREEN:
    sz_sync_renderer(screen_changed);
    switch (sz_get_viz()) {
        case SZ_VIZ_CARD:  draw_lab_sensor_card_screen(...); break;
        case SZ_VIZ_VALOR: draw_lab_value_modern_screen(...);
                           if (screen_changed) drawHeader(sz_sensor_name(...)); break;
        case SZ_VIZ_FOCUS: draw_lab_focus_screen(...);
                           if (screen_changed) drawHeader(sz_sensor_name(...)); break;
        case SZ_VIZ_GRAPH: draw_graph_screen(...);
                           if (screen_changed) drawHeader(sz_sensor_name(...)); break;
        case SZ_VIZ_GAUGE: draw_lab_gauge_temp_screen(...);
                           if (screen_changed) drawHeader(sz_sensor_name(...)); break;
    }
    break;
```

> **Bug conocido**: el `drawHeader` posterior al sub-renderer causa flash visible (dos draws en el mismo frame). Fix pendiente: flag `sz_is_active()` para que los sub-renderers salten su `drawHeader` interno.

---

## Sistema de nombres de visualización — Sensor Zone

### Tabla definitiva de títulos header

El sensor activo determina el prefijo; el viz mode añade el sufijo. El viz FOCUS (principal, default) usa solo el nombre del sensor sin sufijo.

| Sensor | FOCUS (default) | CARD | VALOR | GRAPH | GAUGE |
|--------|----------------|------|-------|-------|-------|
| SZ_TEMP | `Temperatura` | `Temp Card` | `Temp Lab` | `Temp Graf` | `Temp Dial` |
| SZ_HUM | `Humedad` | `Hum Card` | `Hum Lab` | `Hum Graf` | `Hum Dial` |
| SZ_LIGHT | `Luz` | `Luz Card` | `Luz Lab` | `Luz Graf` | `Luz Dial` |
| SZ_SOUND | `Sonido` | `Sonido Card` | `Sonido Lab` | `Sonido Graf` | `Sonido Dial` |
| SZ_SOIL | `Suelo` | `Suelo Card` | `Suelo Lab` | `Suelo Graf` | `Suelo Dial` |
| SZ_DS18 | `Termómetro` | `Termómetro Card` | `Termómetro Lab` | `Termómetro Graf` | `Termómetro Dial` |

> **Nota hardware**: `"Termómetro Card"` / `"Termómetro Graf"` son los títulos más largos (~15 chars). Verificar que no se recorten en pantalla real con `FONT_HEADER` a 160px de ancho.

### Implementación pendiente

El sistema de nombres requiere:
1. Añadir LangKeys `SZ_NAME_*` en `include/languages.h` y `src/lang_select.cpp` para que `sz_sensor_name()` retorne texto localizado (actualmente hardcoded ES).
2. Añadir LangKeys `SZ_VIZ_SUFFIX_CARD`, `SZ_VIZ_SUFFIX_LAB`, `SZ_VIZ_SUFFIX_GRAF`, `SZ_VIZ_SUFFIX_DIAL` para los sufijos.
3. `sz_sensor_name()` compondrá `sensor_name + " " + viz_suffix` según el viz activo (excepto FOCUS que retorna solo el nombre del sensor).
4. Cambiar el default de `g_viz[i]` de `SZ_VIZ_CARD` a `SZ_VIZ_FOCUS`.
5. Cambiar `GRAPH_PUSH_SENSOR` → nuevo LangKey `SZ_PUSH_CHANGE_VIZ` = `"Pulsa: cambiar vista"` en el footer de FOCUS.

### Resolución de duplicaciones de título

Los sub-renderers muestran internamente labels cortos del sensor (TEMP, HUM, SUELO…) que duplican el header cuando se llaman desde `SENSOR_ZONE_SCREEN`. El enfoque aprobado: flag `sz_is_active()` en `sensor_zone.h`. Cada sub-renderer lo chequea y sustituye su label interno por la **unidad de medida** (`°C`, `%`, `lux`) en lugar de suprimirlo — mantiene el peso visual sin duplicar información.

Pantallas afectadas y qué cambiar:

| Sub-renderer | Label interno actual | Reemplazar por |
|-------------|---------------------|----------------|
| `ui_lab_focus.cpp` `draw_summary_shell()` | `sensor_title()` = "TEMP" | unidad del sensor |
| `ui_lab_widget_showcase.cpp` `draw_lab_gauge_dynamic()` | `L(spec.label_key)` = "TEMP" | unidad del sensor |
| `ui_lab_widget_showcase.cpp` `draw_lab_value_dynamic()` | badge text `L(label_key)` = "TEMP" | solo icono (sin texto en el badge) |
| `ui_graph.cpp` `draw_graph_screen()` | `graph_sensor_label()` = "Temperatura aire" | unidad del sensor |
| `ui_lab_sensor_cards.cpp` | device_label "DHT11"/"LDR" | sin cambio — es info de hardware, no duplica |

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
