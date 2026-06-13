# Design System — P-Bit TFT

**Actualizado:** 2026-06-13
**Estado:** IMPLEMENTADO — Sensor Zone activa; ghosting/flicker resuelto por ahora; LDR modos y Demo smooth implementados en firmware, pendientes de validación visual final

Referencia canónica para cualquier pantalla nueva, modificación de color, icono o layout en el firmware P-Bit. Leer este documento antes de tocar cualquier color, icono o layout en archivos de producción.

**Hardware objetivo:** ST7735 160 × 128 px, landscape, RGB565, via TFT_eSPI
**Fuentes de verdad en código:** `include/palette.h` (tokens) · `src/sensor_visuals.cpp` (rampas semánticas/RGB) · `include/layout.h` (constantes de posición) · `src/ui_icons.cpp` (iconos)

---

## 1. Tokens de color

### Sistema P1/P2/P3/P4 por sensor

Cada sensor tiene cuatro roles de color. Los valores viven en `include/palette.h` y son la única fuente de verdad. No crear constantes locales duplicadas en archivos de pantalla.

| Sensor | P1 · Primary | Hex | P2 · Secondary | Hex | P3 · Acento cálido | Hex | P4 · Contraste frío | Hex |
|--------|-------------|-----|----------------|-----|--------------------|-----|---------------------|-----|
| **TEMP** | Fucsia eléctrico | `0xF817` | Verde ácido | `0x07E8` | Oro eléctrico | `0xFE45` | Azul hielo | `0x055F` |
| **HUMEDAD** | Cian eléctrico | `0x075F` | Cobalto láser | `0x2A9F` | Aqua brillante | `0x8FFF` | Azul océano | `0x01F4` |
| **LUZ** | Amarillo puro | `0xFFE0` | Ámbar eléctrico | `0xFC40` | Oro neón | `0xFE40` | Dorado oscuro | `0x7A40` |
| **SONIDO** | Naranja cálido | `0xFD40` | Violeta-púrpura | `0xC01F` | Rojo neón | `0xF8A0` | Púrpura oscuro | `0x4011` |
| **SUELO** | Lima ácido | `0x47E8` | Tierra cálida | `0xCC04` | Menta claro | `0x87F0` | Verde oscuro | `0x0300` |
| **TERMÓMETRO** | Naranja vivo | `0xFB80` | Azul láser | `0x045F` | Amatista claro | `0xCC5F` | Cian frío | `0x0659` |

> Los nombres semánticos expresan la intención visual. Los valores exactos en `include/palette.h` son los únicos valores autorizados.
> Suelo aplica además una rampa semántica runtime en `pbit_soil_visual_color()`: amarillo intenso en sequía extrema, verde dentro del rango `Seco..Húmedo` y azul al superar `Húmedo`.

### Uso semántico de los 4 roles

| Rol | Uso en pantalla |
|-----|----------------|
| **P1 · Primary** | Borde de card, dial del gauge, icono activo, línea de gráfica, segmentos llenos |
| **P2 · Secondary** | Sparklines, borde degradado en FOCUS, ring exterior del gauge, highlights de badge |
| **P3 · Acento cálido** | Max-labels en gráfica, pico de VU, highlight de valor extremo |
| **P4 · Contraste frío** | Min-labels en gráfica, referencia de cero, grid sutil, texto de estado neutral |

Acceso en código mediante helpers indexados:

```cpp
pb_primary(sensor_id)       // P1
pb_secondary(sensor_id)     // P2
pb_accent_warm(sensor_id)   // P3
pb_contrast_cool(sensor_id) // P4
```

### Fondos canónicos (no modificar sin motivo explícito)

| Token | Valor | Uso |
|-------|-------|-----|
| `PB_PANEL_NAV_BG` | `tft.color565(8, 12, 18)` | Fondo navy principal de todas las pantallas |
| Fondo de gráfica | `tft.color565(4, 8, 20)` | Área de plot del modo Gráfica |
| Card interior bg | `0x0841` | Interior de card en HOME y pantallas lab |
| Card border | `0x2945` | Borde de card en HOME y pantallas lab |
| Dato numérico principal | `TFT_WHITE` | **Siempre** — sin excepciones |
| Segmentos vacíos / track | `0x1084` | Segmentos apagados, tracks de gauge |

### Reglas de color

- Consultar `color-expert` antes de añadir cualquier valor RGB565 nuevo al código.
- Verificar contraste sobre fondo navy `0x1082` antes de aprobar cualquier color.
- **No usar `TFT_BLACK` como color de detalle interior** — sobre fondo navy crea manchas visibles. Usar `PB_PANEL_NAV_BG` en su lugar.
- Los valores en `include/palette.h` son la fuente de verdad. No duplicar constantes locales por pantalla.
- Pendiente de decisión: si `HOME` y `CLIMA LAB` migran a paleta canónica o mantienen colores responsivos propios (ver `docs/ROADMAP.md`).

---

## 2. Sistema de iconos

Todos los iconos son procedurales (dibujados con primitivas TFT_eSPI en runtime). No hay bitmaps. Definidos en `src/ui_icons.cpp`.

### Tamaños disponibles

| Función | Factor `s` | Tamaño aprox. | Uso |
|---------|-----------|---------------|-----|
| `pbit_draw_*_icon(cx, cy, color)` | s=1 | ~14×14 px | Sensor cards, headers, jewels (`SensorIconDrawFn`) |
| `pbit_draw_*_icon_xl(cx, cy, color)` | s=3 | ~42×42 px | Centro de Rango / Principal |
| `pbit_draw_*_icon_xxl(cx, cy, color, accent)` | s=3 | ~42×42 px | Pantalla Rango — con color de acento separado |

> **Nota:** No existe API `_large` (s=2). `_xl` y `_xxl` comparten escala s=3; `_xxl` añade color de acento.

### Inventario y estado

| Icono | Sensor | Estado | Deuda / Notas |
|-------|--------|--------|---------------|
| `temp` — Termómetro | TEMP | ✅ **FINAL** | Geometría v17: silueta centrada en `cx` con tubo `fillRoundRect(cx-2s, cy-7s, 4s, 11s)`, canal vacío `fillRect(cx-s, cy-6s, 2s, 4s, bg)`, bulbo `fillCircle(cx, cy+4s, 3s+1)` y ticks `(4s)/3`. `pbit_draw_temp_icon`, `_xl` y el fallback `_xxl(cx, cy, color)` son **monocromos**: no dibujan `TFT_WHITE` ni acentos internos. Solo `pbit_draw_temp_icon_xxl(cx, cy, color, accent)`, usado por Rango, conserva detalle multicolor/mercurio. Canal usa `kIconCardBg` (constante `0x1082` en `src/ui_icons.cpp`). |
| `humidity` — Gota | HUMEDAD | ✅ Aprobado en código | Silueta sólida sin donut interior. Pendiente validación en hardware |
| `light` — Sol | LUZ | ✅ Funcional | Rayos diagonales de 1 px pueden ser poco visibles a s=1. Mejorable a 2 px si hardware lo confirma |
| `sound` — Micrófono | SONIDO | ✅ Funcional | No tocar sin captura de hardware previa. Si base excesiva, reducir solo en hardware |
| `plant` — Planta | SUELO | ✅ Aprobado en código | Hojas con `fillRoundRect`; base redondeada. Pendiente validación en hardware |
| `probe` — Sonda | TERMÓMETRO | ❌ No final | Identidad visual no aprobada. Pendiente rediseño o validación de metáfora alternativa |

### Normas de implementación de iconos

- Los iconos se dibujan **siempre al final**, después de todos los `fillRect` de limpieza de datos (chrome-last rule).
- No usar `TFT_BLACK` para detalles interiores — pasar el color de fondo como parámetro `bg`.
- Antes de rediseñar un icono, validar la propuesta en hardware con `LAB_ICON_TEST_SCREEN`.
- Para nuevos iconos, consultar `pixel-art-sprites` y `8-bit-pixel-art-patterns`.

---

## 3. Anatomía de pantalla

### Zonas canónicas (160 × 128 px, landscape)

```
Y=0..19    HEADER   — título drawHeader(); solo chrome, nunca datos dinámicos
Y=20..26   GAP      — respiración bajo header
Y=27..126  CONTENT  — toda la UI de datos (LC_CARD_TOP=27, LC_SCREEN_BOTTOM=126)
Y=118..126 FOOTER   — hints, alert jewel (atención: solapamiento con CONTENT inferior)
```

### Constantes de layout (`include/layout.h`)

| Constante | Valor | Uso |
|-----------|-------|-----|
| `L_CONTENT_TOP` | 27 | Primera Y útil bajo header |
| `LC_SCREEN_X` | 2 | Margen lateral de cards |
| `LC_SCREEN_W` | 156 | Ancho de card full-width |
| `LC_SCREEN_BOTTOM` | 126 | Límite inferior de cards |
| `LC_CARD_TOP` | 27 | Top de card principal |
| `LC_CARD_RADIUS` | 4 | Radio de esquinas de cards |

Cards 2×2 (HOME y familia): `X0=2`, `X1=82`, `Y0=27`, `Y1=79`, `W=76`, `H=48`

---

## 4. Patrones de componentes

### SENSOR CARD (patrón moderno canónico)

Tres zonas horizontales con separadores, sin solapamiento de erase zones:

```
y=27..43   HEADER (17px)  icon(s=1, cx=15, cy=40) + device_label(P2) + unit/status(TR)
           separador y=44
y=45..81   VALUE  (37px)  dato grande centrado full-width, TFT_WHITE, cx=80
           separador y=82
y=83..110  VIZ    (28px)  visualización horizontal específica por sensor
y=113..126 FOOTER         alert jewel (x=12, y=116)
```

Clave: `draw_card_dynamic` hace `fillRect` completo del área antes de dibujar → no se necesitan clear rects parciales en helpers.

### Visualizaciones por sensor

Cada sensor tiene una visualización única y propia. No reutilizar la misma viz para dos sensores distintos.

| Sensor | Tipo de viz | Características |
|--------|------------|-----------------|
| TEMP | 12 segmentos gradiente | azul→rojo, 0–50°C, labels extremos |
| DS18 | 14 segmentos, split en 0° | azul hielo/cálido, tick blanco en cero, −55..+125°C |
| HUMEDAD | 10 gotas-pill en fila | cyan gradient, highlight dot, estilo "burbuja" |
| LUZ | 8 barras verticales crecientes | oscuro→amarillo, equalizer, altura proporcional |
| SONIDO | 7 columnas VU | misma altura = nivel, verde/naranja/rojo por zona |
| SUELO | 3 zonas fijas + marcador | DRY/OK/WET, diamante de posición |

### Rangos Visuales Canónicos

Todas las visualizaciones de un sensor deben usar el mismo rango base. No reutilizar `0–50°C` del DHT11 para el Termómetro/DS18B20.

| Sensor | Rango visual |
|---|---|
| TEMP DHT11 | `0..50°C` / `32..122°F` |
| DS18B20 | `-55..+125°C` / `-67..+257°F` |
| HUMEDAD | `0..100%` |
| LUZ | `0..8000 lux` |
| SONIDO | `0..100%` |
| SUELO | `0..100%` |

Nota LUZ: `0..8000 lux` es el rango ambiental base actual para categorías, alertas y escala visual tras la calibración empírica LDR v1. La presentación visible usa helper común para `Lux`, `FC` y `Raw ADC`; Sensor Zone, cards, dashboards, dials y gráficas deben mostrar valor/unidad según el modo activo.

El LED RGB debe seguir el color semántico de la visualización activa mediante `sensor_visuals.*`. Timer usa el color de estado visible; cualquier vista de solo Luz mantiene RGB apagado para no influir en el LDR.

Regla de marcas en diales:

- TEMP DHT11, HUMEDAD y SUELO: marcas visibles por defecto porque sus rangos son interpretables con más confianza.
- Todos los sensores con dial tienen opción `Marcas`.
- TEMP, HUMEDAD, SUELO y TERMÓMETRO: marcas activas por defecto.
- LUZ y SONIDO: marcas ocultas por defecto para evitar saturación visual.
- TERMÓMETRO/DS18B20: marca fija de `0 °C`; no mostrar límites alto/bajo por defecto.
- Luz usa progresión visual logarítmica en el dial para que los cambios bajos no queden comprimidos al inicio.

### Menú raíz de settings 2×3

Todos los menús raíz de configuración usan `drawSettingsGridMenu()`:

```
[ opción 1 ] [ opción 2 ]
[ opción 3 ] [ opción 4 ]
[  Reset  ] [  Salir  ]
```

Reglas:

- Máximo 4 opciones primarias; `Reset` y `Salir` viven siempre en la última fila.
- Si un menú tiene solo 2 o 3 opciones primarias, los slots no usados quedan vacíos.
- `Reset` usa acento amarillo/naranja; `Salir` usa rojo/magenta.
- El texto visible debe venir de `L(KEY)`; no usar strings hardcodeados.
- Solo Suelo usa `Calibrar` como calibración real. Luz y Sonido usan límites interpretativos.
- El grid 2×3 es también contrato anti-flicker: al entrar se puede dibujar completo, pero durante navegación con encoder solo deben refrescarse el tile anterior y el tile nuevo. Header, línea y footer no cambian por giro.

### Confirmación de Reset

Todas las confirmaciones de `Reset` usan el patrón `drawResetChoicePromptShell()` + `updateResetChoiceButtons()` como pantalla `danger` común:

- Fondo rojo full-screen para indicar acción destructiva.
- Header estándar: título superior (`Reset` o `Reset total`) y línea blanca en `L_HEADER_LINE`.
- Panel central rojo oscuro con dos líneas descriptivas de la acción (`Por defecto` / `de temperatura`, `Restaurar` / `fábrica`, etc.).
- Botones `NO` / `SI` dentro del panel; `NO` es la selección por defecto y se dibuja con fondo amarillo, mientras `SI` queda como acción peligrosa al seleccionarse.
- El hint inferior permanece en la posición de menú (`LM_MENU_FOOTER_Y - 4`) y conserva el texto de interacción.
- No usar esta pantalla para acciones reversibles ni para confirmaciones informativas: es exclusiva de resets destructivos.
- Anti-flicker: el fondo rojo, header, panel y descripción son shell. Al alternar `NO` / `SI` con el encoder, solo se redibujan los botones mediante `updateResetChoiceButtons()`.

### Selectores y valores de menú

Los estados de edición (`ON/OFF`, `Celsius/Fahrenheit`, límites numéricos, modos de lectura) usan `drawCenteredMenuValueScreen()`, que dibuja el valor dentro de una card central:

- Card `x=20, y=58, w=120, h=38`; valor centrado visualmente en `y=75`.
- Borde del color activo del valor (`ON` verde, `OFF` rojo, límites por color semántico).
- Fondo oscuro estable para evitar ghosting sobre la banda del menú.
- Título arriba y hint abajo fuera de la card.
- Cuando el contenido sigue siendo texto sin card, el frame común dibuja un separador sutil sobre el hint inferior y evita que los colores del contenido repitan el cyan de instrucción.

### Checklist de pantalla nueva

Antes de escribir código para una pantalla nueva:

```
□ ¿Cuáles son las dimensiones exactas del card? (X, Y, W, H)
□ ¿Qué elementos son chrome (estáticos) y qué son dinámicos?
□ ¿Qué variables de caché necesito (una por campo dinámico)?
□ ¿Cuál es la zona exclusiva del icono?
□ ¿El clear rect de cada campo está clampado al card?
□ ¿La viz del sensor es apropiada para ese sensor, no genérica?
□ ¿El dato numérico usa TFT_WHITE?
□ ¿La unidad usa P1 del sensor?
□ ¿El device label usa P2 (secondary)?
□ ¿Ninguna zona de borrado pisa otra zona?
□ ¿El icono se dibuja AL FINAL, después de todos los fillRect?
```

---

## 5. Reglas de uso

### Chrome vs. datos (anti-flicker)

Toda pantalla divide su contenido en dos capas:

| Capa | Elementos | Cuándo redibuja |
|------|-----------|-----------------|
| **Chrome** (estática) | Borde, icono, label sensor, jewel de alerta, título | Solo si `chrome_dirty` o `screen_changed` |
| **Datos** (dinámica) | Valor numérico, barra/tank fill, sparkline, ring gauge | En cada `sensor_data_changed` |

Ver `docs/TFT_RENDER_RULES.md` para el protocolo anti-flicker completo (sprites, clears localizados, chrome-last rule).

### Terminología visible en pantalla

| Elemento | Término correcto | ❌ Evitar |
|----------|-----------------|-----------|
| Sensor externo DS18B20 | `Termómetro` / `Termo` | `DS18`, `Sonda`, `Probe` |
| Modo de tendencia de Sensor Zone | `Curva` | `Graf`, `Graph`, `Gráfica` como sufijo |
| Modo gauge/dial de Sensor Zone | `Rango` | `Dial`, `Gauge`, `Medidor` |
| Modo resumen de Sensor Zone | `Ficha` (`Info` en EN) | `Tarjeta`, `Card` como término visible |
| Modo numérico de Sensor Zone | `Dato` | `Valor`, `Value`, `Lab` |
| Sensor de sonido | `Sonido` | `Ruido`, `Sound` |
| Textos de UI general | `L(KEY)` / `LIn(...)` | Strings en español hardcodeados |
| Identificadores técnicos | `DHT11`, `DS18B20`, `LDR`, `GPIO` | — (estos sí pueden estar hardcodeados) |

---

## 6. Estado de implementación y pendientes

### Implementado y activo en código

- `include/palette.h` con P1/P2/P3/P4 y helpers `pb_primary()`, `pb_secondary()`, `pb_accent_warm()`, `pb_contrast_cool()` por índice de sensor
- Sensor Zone completa (`FOCUS`, `CARD`, `VALOR`, `GRAPH`, `GAUGE`) consume paleta canónica; sus nombres visibles son `Principal`, `Ficha`, `Dato`, `Curva` y `Rango`
- Icono `temp` final aplicado a small/XL/XXL: small/XL/fallback son monocromos y solo Rango/XXL con acento conserva detalle multicolor; no mezclar este estado con el icono técnico `probe`/DS18B20
- Estados externos desconectados para `SZ_DS18` y `SZ_SOIL`: textos `Revisa IO33` / `Revisa IO35`, helper runtime común y paleta del sensor atenuada en clásicas, Sensor Zone y Lab
- Iconos `humidity` y `plant` rediseñados en `src/ui_icons.cpp`; `probe` se mantiene separado de `temp` y no está aprobado como final
- Icono Bluetooth añadido en `src/ui_icons.cpp` para pantalla `BLE_TOGGLE_SCREEN`

### Pendiente visual / vigilancia en hardware (ST7735 real)

- Contraste real de P1/P2/P3/P4 sobre fondos navy — especialmente HUM vs. DS18 como identidades visualmente distinguibles
- Legibilidad de P3/P4 en labels min/max y segmentos apagados sobre ST7735 real (vs. simulador)
- LDR: coherencia visual `Lux / FC / Raw ADC` implementada en firmware; validar en hardware real
- DS18/Suelo desconectados: validar en hardware real que `Sin sensor` + `Revisa IO33/IO35` caben en ES/CAT/EN y que la paleta atenuada comunica ausencia sin parecer alarma roja
- Icono `light`: rayos diagonales de 1 px en s=1 — confirmar si son visibles o requieren 2 px
- Icono `sound`: proporciones de base horizontal en s=1, s=2, s=3
- Demo Mode: coreografía smooth implementada; validar que no reintroduzca flicker por cambios de datos demasiado rápidos

### Decisiones de diseño abiertas

**Paleta:**
- [ ] Aprobar en hardware TEMP / HUM / DS18 como identidades visualmente distinguibles
- [ ] Validar en ST7735 real la rampa de Suelo amarillo→verde→azul contra los rangos configurados
- [ ] Decidir si `HOME` y `CLIMA LAB` migran a paleta canónica o mantienen colores responsivos propios

**Iconos:**
- [ ] Rediseño y aprobación final del icono `probe` (DS18B20) en hardware
- [ ] Confirmar si diagonales del icono `light` necesitan ajuste a 2 px tras validación hardware

---

## 7. Historial de decisiones de diseño

*Síntesis de decisiones tomadas durante el diseño (sesión 2026-05-16, refinamientos hasta 2026-05-24).*

**Problemas resueltos en esta iteración:**

| Problema | Solución aplicada |
|----------|------------------|
| HUM y DS18 usaban el mismo par cian/púrpura intercambiado | Identidades independientes en paleta canónica |
| SUELO y HUMEDAD compartían cian | Paletas separadas: SUELO usa lima ácido como P1 |
| `TFT_ORANGE` (255,165,0) leía como ámbar/dorado, no naranja ácido | Reemplazado por `0xFA80` en paleta canónica |
| Constantes de color definidas localmente por cada pantalla | Centralizadas en `include/palette.h` |
| Gota de humedad con círculo negro interior (efecto donut) | Rediseñada como silueta sólida |
| Planta con hojas triangulares (leían como flechas a s=1) | Rediseñada con `fillRoundRect` para hojas ovales |
| Sin `palette.h` canónico — drift de colores entre pantallas | `include/palette.h` creado; Sensor Zone lo consume |

**Orden de migración de paleta (aplicado y en progreso):**
1. ✅ Pantallas nuevas (`SENSOR CARD`, `VALOR LAB`) — banco de prueba histórico sin tocar producción; nombres visibles actuales: `Ficha` y `Dato`
2. ✅ Sensor Zone completa — paleta canónica en todos los modos
3. ⏳ Pantallas madre (`HOME`, `CLIMA`) — solo después de validar Sensor Zone en hardware real
