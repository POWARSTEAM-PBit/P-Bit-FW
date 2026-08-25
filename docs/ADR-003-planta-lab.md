# ADR-003 - Planta Lab: pantalla multisensor de salud de planta

Fecha: 2026-06-15

Estado: Aceptado

## Contexto

El P-Bit ya tiene pantallas Lab para clima múltiple (`CLIMA LAB`) y temperatura de sonda (`TERMO LAB`). Una de las capacidades más potentes del dispositivo para el aula es la combinación del sensor de suelo con los sensores ambientales (temperatura, humedad del aire, luz), que juntos permiten evaluar las condiciones de vida de una planta real.

Sin embargo, no existía una pantalla que integrara esos cuatro sensores en un lenguaje de cuidado de planta: el niño tenía que saltar entre pantallas individuales sin contexto. Esto pierde la oportunidad pedagógica central: que una planta depende de un sistema de variables, no de una sola.

La taxonomía ya cierra la regla: `Lab = pantalla multisensor, experimento o actividad educativa`. `PLANTA LAB` cumple esta definición.

## Decisión

Se crea `PLANTA LAB` como pantalla Lab global en el carrusel principal, independiente del stack de `Suelo`.

La pantalla integra cuatro sensores:
- Humedad de suelo (%)
- Temperatura del aire (°C)
- Humedad del aire (%)
- Luz (lx)

La planta es el protagonista visual. No es un panel de datos: es un diagnóstico en tiempo real del estado de la planta mediante un icono que cambia de color según la condición dominante.

## Diseño de V1

### Estructura visual

Layout de "terrario + filas de rango". Panel vertical a la izquierda (terrario) donde
la planta crece sembrada en tierra; a la derecha, cuatro filas-card (una por sensor), cada
una con icono + barra de rango + valor, y debajo una card de estado ancha.

```
┌──────────────────────────────────────────┐
│              PLANTA  LAB                  │  título grande (fuente header)
├───────────┬──────────────────────────────┤
│  terrario │ 💧 ▓▓│███│░  25%  ← Suelo (!) │
│   bulbo   │ 🌡 ▓│████│░  22°   ← Temp     │
│   hojas   │ 💨 ▓▓│███│░  55%  ← Aire      │
│   tallo   │ ☀ ▓▓▓│██│░ 180lx ← Luz       │
│  ▓tierra▓ │ ┌──────────────────┐          │
│           │ │     SEDIENTA     │          │  card de estado
└───────────┴─┴──────────────────┴──────────┘
```

- **Panel terrario** (izquierda): planta vertical sembrada en una banda de tierra **plana**
  al fondo. La tierra **es** el sensor de Suelo (seca/sana/encharcada). Coordenadas exactas
  en la sección "Layout V1 — coordenadas de implementación".
- **Cuatro filas de rango** (derecha): cada fila es una card con icono (color de identidad
  del sensor) + barra de zonas (baja/óptima/alta) con marcador en el valor actual + valor
  numérico **blanco**. La fila del sensor que dispara el diagnóstico lleva borde de acento
  (color de estado). La barra da el doble valor: número exacto + posición en el rango sano.
- **Card de estado** (ancha, debajo de las filas): el diagnóstico en texto, con borde y
  texto en color de estado. Enmarcado como las demás — sin texto flotante.
- **Jerarquía tipográfica:** el título usa la fuente de header (grande); el estado, fuente
  menor. Así el estado cabe en el ancho de la columna derecha incluso en catalán.
- Temperatura se muestra **sin decimal** (`22°`, no `22.4°`) — el DHT11 no justifica el
  decimal y ahorra ancho.

### Estados de diagnóstico

Cuatro estados en V1:

| Estado | Condición dominante | Color | Forma (postura) |
|---|---|---|---|
| `BIEN` | Todos los sensores en rango óptimo | Verde | erguida, hojas arriba, yema visible |
| `SEDIENTA` | Suelo por debajo del umbral bajo | Amarillo | hojas caídas, tierra seca/agrietada |
| `AHOGADA` | Suelo por encima del umbral alto | Azul | hojas pesadas + gotas, tierra encharcada |
| `ESTRÉS` | Temperatura, luz o humedad de aire fuera de rango | Naranja | hojas muy caídas, una hoja caída al suelo |

La **forma** comunica el estado aunque no se distinga el color (regla de accesibilidad
`color-only-meaning`: ~8% de daltonismo + squint test). El color refuerza, no es el
único portador del significado.

El estado `ESTRÉS` agrupa los tres sensores ambientales porque en V1 no se diferencia visualmente entre calor, frío, poca luz o aire seco. El texto puede variar si se quiere más detalle (ver extensión posible abajo), pero el color del icono es único para el estado.

### Prioridad de diagnóstico

Cuando más de un sensor está fuera de rango, se muestra el de mayor prioridad:

```text
suelo > temperatura > humedad aire > luz
```

Razón: suelo y temperatura afectan la planta en minutos; la luz y la humedad del aire tienen un impacto más lento.

### Thresholds V1 (perfil "interior genérico" hardcodeado)

| Sensor | Bajo (alerta) | Óptimo | Alto (alerta) |
|---|---|---|---|
| Suelo (%) | < 25 | 30 – 70 | > 80 |
| Temperatura (°C) | < 12 | 18 – 26 | > 30 |
| Humedad aire (%) | < 30 | 40 – 70 | > 85 |
| Luz (lx) | < 100 | 200 – 10000 | > 30000 |

Estos valores corresponden a una planta de interior genérica. No se exponen al usuario en V1. Se ajustarán antes del commit si hay datos empíricos de aula disponibles.

### Animación V1

Animación **continua en todos los estados** pero acotada (Nivel A + B + partículas). Las
5 hojas se balancean con fase propia (sway desordenado), y cada estado añade un gesto:
`BIEN` rápido + pulso de yema, `SEDIENTA` lento, `AHOGADA` lento + gotas que caen,
`ESTRÉS` tiembla. Detalle de implementación en "Layout V1 → Animación".

**No hay animación de transición entre estados.** El cambio de estado es instantáneo
(puede cambiar rápido; una transición se vería nerviosa). La caída/crecimiento animado
queda diferido a V3.

Solo se redibuja la zona del icono (tallo + hojas + yema); la tierra es chrome estático.
El icono es < 20 formas, así que en principio **no necesita sprite** (Nivel 4 de
`docs/TFT_RENDER_RULES.md`): clear localizado + redibujo. Si el icono animado parpadea en
HW, envolver solo su rect en un `TFT_eSprite` pequeño.

### Visibilidad en el carrusel

`PLANTA LAB` solo aparece en el carrusel si el sensor de suelo está conectado y da lecturas válidas. Si no está disponible, la pantalla se omite del ciclo de navegación.

### Posición en el carrusel

```text
Inicio
Clima Lab
Planta Lab     ← aquí
Termo Lab
Temperatura
Humedad
Luz
Sonido
Suelo
Termómetro
Temporizador
Sistema
```

Agrupa con los otros Labs globales por coherencia de producto.

## Nombre visible y traducciones

| Idioma | Nombre visible |
|---|---|
| Español | `PLANTA LAB` |
| Català | `PLANTA LAB` |
| English | `PLANT LAB` |

El nombre en catalán y español es idéntico. Se usa singular porque la pantalla observa una planta concreta, no plantas en general.

Textos de diagnóstico:

| Estado | Español | Català | English |
|---|---|---|---|
| Bien | `BIEN` | `BÉ` | `OK` |
| Sedienta | `SEDIENTA` | `ASSEDEGADA` | `THIRSTY` |
| Ahogada | `AHOGADA` | `OFEGADA` | `SOGGY` |
| Estrés | `ESTRÉS` | `ESTRÈS` | `STRESSED` |

Longitud estimada en la fase de layout: con fuente 1 (~6 px/char) `ASSEDEGADA` (10
chars ≈ 60 px) cabe en el panel derecho de diagnóstico (~92 px). No se prevé fallback
corto. Reverificar en hardware real por si la fuente final difiere.

## Icono de planta

Se crea un **icono nuevo** para el héroe, distinto del icono del sensor de Suelo (que es
una planta en maceta). El héroe es una planta **sembrada en tierra**: banda de suelo al
fondo + tallo vertical recto + 5 hojas asimétricas + yema. Lenguaje de relleno
(`fillEllipse` + `fillRect`), sin contornos, coherente con el resto de iconos del firmware.

Razón del icono nuevo: el sensor de Suelo ya usa una planta en maceta. Maceta vs tierra
son dos siluetas que no se confunden ni a tamaño pequeño. No se reutiliza
`impl_plant_detail`.

Reglas de forma:

- **Postura por estado:** hojas **arriba** de su inserción = sana; hojas **caídas** (cada
  vez más abajo) = enferma. Como `fillEllipse` no rota, la postura se codifica por el
  desplazamiento vertical `dy` del centro de la hoja, no por ángulo. Ver tabla de estados.
- **Hojas asimétricas:** 5 hojas en posiciones y tamaños desordenados (no pares espejo),
  para que se vea natural.
- **Tallo recto** en todos los estados. La postura la cargan solo las hojas (dy + tamaño).
- **Parámetros del dibujo**, derivables de variables de estado: dy de hoja, escala de
  hoja, altura y grosor del tallo, grosor de la yema, color de la tierra.

## Mapeo sensor → canal visual (roadmap, NO V1)

Norte de diseño para cuando se profesionalice el card: cada sensor controla una variable
visual distinta y la planta es la **suma** de todas (Ley del Mínimo hecha imagen). En V1
NO se implementa el mapeo fino — se simplifica a los 4 estados de diagnóstico. La
introducción será escalonada (un canal a la vez) para no saturar la lectura.

| Sensor | Canal visual | Bajo | Óptimo | Alto |
|---|---|---|---|---|
| Suelo | la **tierra** del terrario | clara/gris/grietas | café oscuro sano | muy oscura + charco/gotas |
| Temperatura | **postura/vigor** de hojas | hojas encogidas (frío) | hojas abiertas arriba | hojas muy caídas (calor) |
| Humedad aire | **partículas** en el aire | aire vacío | alguna gotita | muchas gotitas / vaho |
| Luz | un **sol** en la esquina | sol pequeño y gris | sol amarillo medio | sol blanco intenso |
| Tiempo / cuidado | **altura** y nº de hojas | brote | crece | alta, frondosa |
| Salud global | **tamaño** de hoja | hojas chicas | medianas | grandes |

Riesgo a vigilar: si todos los canales se mueven a la vez, el niño no sabe qué leer. Por
eso V1 = 4 estados (un color + postura), y la combinación fina llega después.

## Layout V1 — coordenadas de implementación (para Codex)

Referencia: 160×128 (rotation=1). Constantes en `include/layout.h`. Archivo nuevo:
`src/ui_lab_plant.cpp` (+ `include/ui_lab_plant.h`). Colores nuevos vía
`tft.color565(r,g,b)`; los de sensor usan los tokens de `include/palette.h`.

### Zonas y cards (px exactos)

| Elemento | X | Y | W | H | Notas |
|---|---|---|---|---|---|
| Título header | — | base 18 (`L_HEADER_Y`) | — | — | `drawHeader("PLANTA LAB")` |
| Divisor header | 4 | 23 (`L_HEADER_LINE`) | 152 | — | línea |
| Terrario (card) | 2 | 27 | 56 | 99 | `drawRoundRect` r=4, fondo `0x0841`, borde idle `0x2945` |
| Tierra (banda plana) | 4 | 110 | 52 | 14 | dentro del terrario; **SIN elipses** |
| Fila Suelo | 62 | 27 | 96 | 17 | icono+barra+valor; borde acento si culpable |
| Fila Temp | 62 | 46 | 96 | 17 | |
| Fila Aire | 62 | 65 | 96 | 17 | |
| Fila Luz | 62 | 84 | 96 | 17 | |
| Card Estado | 62 | 104 | 96 | 22 | borde + texto = color de estado |

Verificación: terrario 2+56=58, gap `LC_GAP`=4, columna derecha 62, 62+96=158 = 2+`LC_SCREEN_W`(156) ✓.
Vertical derecha: filas en 27/46/65/84 (h17, gap ~2) → 101; estado 104..126 = `LC_SCREEN_BOTTOM` ✓.

### Interior de cada fila de sensor (96×17)

Tres zonas horizontales. `Y` = top de la fila.

- **Icono:** centro `(70, Y+8)`, ~12 px, color de identidad del sensor.
- **Barra de rango:** `x=80, y=Y+5, w=42, h=7`, r=2. Pista `color565(22,36,46)`. Tres
  zonas contiguas: baja `color565(122,64,48)`, óptima `color565(46,125,74)`, alta
  `color565(42,90,122)`; anchos ∝ thresholds del sensor. Marcador: línea blanca de 1 px
  de ancho × 11 px de alto en `x = 80 + 42 * frac`, con `frac` = posición del valor en el
  rango total (clamp 0..1).
- **Valor:** fuente 1, alineado a la derecha en `x=156`, baseline `Y+12`, **BLANCO**.

| Fila | icono | color icono (token) | rango total → zonas bajo/ópt/alto |
|---|---|---|---|
| Suelo | gota | `PB_SOIL_P1` (0x47E8) | 0–100% → <25 / 30–70 / >80 |
| Temp | termómetro | `PB_TEMP_P1` (0xF817) | 0–40 °C → <12 / 18–26 / >30 |
| Aire | nube | `PB_HUM_P1` (0x075F) | 0–100% → <30 / 40–70 / >85 |
| Luz | sol | `PB_LUZ_P1` (0xFFE0) | log 0–30000 lx → <100 / 200–10000 / >30000 |

El icono mantiene su color de identidad; el "culpable" se marca solo con el **borde** de
su fila en color de estado. Temp **sin decimal**. Si el valor de Luz tiene ≥5 dígitos y se
aprieta, reducir la barra a `w=38` en esa fila.

### Card de estado (62, 104, 96, 22)

Texto centrado `cx=110`, baseline ~119, fuente 1, color = color de estado. Borde = color
de estado (2 px). Fondo `0x0841`. Catalán `ASSEDEGADA` (10 ch × 6 px = 60 px) cabe en 96 px ✓.

### Icono héroe — parámetros (cx=30, tierra_y=110)

`fillRect`/`fillEllipse`, **ejes alineados** (TFT_eSPI `fillEllipse` NO rota: la postura
se codifica por la **Y del centro** de cada hoja respecto a su punto de inserción, no por
rotación). Todo en color de estado.

- **Tallo** (recto siempre): `fillRect(cx-1, tallo_top, 3, tierra_y - tallo_top)`.
- **Bulbo:** `fillEllipse(cx, tallo_top-1, 4, 6)`. En `BIEN`, punto claro:
  `fillCircle(cx, tallo_top-4, 2, color_claro)`.
- **Hojas:** **5 hojas asimétricas** (no pares espejo), `fillEllipse(cx+dx, cy, rx, ry)`.
  Posición y tamaño base fijos y desordenados; el estado aplica un desplazamiento vertical
  `dy` y una escala global `esc`.

Set base (asimétrico) — por hoja `(dx, cy_base, rx, ry)`:

| # | dx | cy_base | rx | ry |
|---|---|---|---|---|
| 1 | -12 | 88 | 8 | 3 |
| 2 | +11 | 80 | 7 | 3 |
| 3 | -9 | 72 | 6 | 2 |
| 4 | +10 | 64 | 6 | 2 |
| 5 | -6 | 58 | 5 | 2 |

Por estado: `cy = cy_base + dy`, `rx,ry ×= esc`:

| Estado | tallo_top | dy hojas | esc | Extra |
|---|---|---|---|---|
| `BIEN` | 44 | -4 (arriba) | 1.0 | yema con punto claro |
| `SEDIENTA` | 46 | +4 (caídas) | 0.85 | — |
| `AHOGADA` | 46 | +6 | 0.9 | 2 gotas que caen (animadas) |
| `ESTRÉS` | 46 | +7 | 0.7 | solo hojas 1–3; + hoja caída `fillEllipse(44,118,7,3)` naranja apagado |

### Tierra por banda de Suelo (independiente del estado)

Relleno de la banda + línea superior (2 px) + grietas; color por banda de humedad de suelo:

| Banda suelo | Relleno | Línea superior | Grietas |
|---|---|---|---|
| Seco (<25%) | `color565(110,74,36)` | `color565(134,96,46)` | 2× `drawLine` `color565(74,48,22)` |
| Óptimo (30–70%) | `color565(74,48,22)` | `color565(110,74,36)` | — |
| Encharcado (>80%) | `color565(46,36,24)` | brillo = color AHOGADA | — |

La tierra es una **banda plana**: rect + línea fina superior + grietas (`drawLine`).
**NUNCA dibujar elipses sobre la tierra** — causaban un bulto marrón con mala lectura.

### Colores de estado (planta + texto/borde card estado + borde culpable)

| Estado | `tft.color565()` |
|---|---|
| `BIEN` | `(46, 204, 90)` |
| `SEDIENTA` | `(232, 162, 42)` |
| `AHOGADA` | `(63, 163, 224)` |
| `ESTRÉS` | `(232, 84, 42)` |

Fondo navy: `color565(8,12,18)`. Card interior `0x0841`. Borde idle `0x2945`.

### Chrome vs data (anti-flicker, ver `docs/TFT_RENDER_RULES.md`)

- **Chrome** (1 vez en `screen_changed`): header + divisor, borde del terrario, bordes
  idle de las 4 filas, iconos de sensor, pista vacía de cada barra de rango, fondo de la
  card de estado.
- **Data** (al cambiar la cache): banda de tierra (si cambia la banda), planta
  (tallo+hojas+bulbo; animación continua, ver abajo), zonas+marcador de cada barra,
  4 valores, borde de la fila culpable, texto + borde de la card de estado.

### Cache struct

```cpp
struct PlantLabCache {
    uint8_t  state;        // 0xFF init → fuerza primer dibujo
    uint8_t  soil_band;    // 0=seco 1=óptimo 2=encharcado (color de tierra)
    uint8_t  culprit;      // card resaltada: 0=Suelo 1=Temp 2=Aire 3=Luz, 0xFF=ninguna
    int16_t  soil_x10;     // últimos valores mostrados (×10 para detectar cambio)
    int16_t  temp_x10;
    int16_t  hum_x10;
    uint32_t lux;
    uint8_t  anim_phase;   // fase de animación (avanza cada frame, todos los estados)
    bool     chrome_done;
};
static PlantLabCache g_plant_cache = { 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, false };
```

### Animación (Nivel A + B + partículas)

Animación **continua en todos los estados** (responde a "la planta necesita más vida"),
pero acotada para no exceder coste de render.

- **Nivel A — idle sway (siempre):** cada una de las 5 hojas se balancea con fase propia.
  Como `fillEllipse` no rota, el sway se aproxima moviendo `cx` de la hoja ±1–2 px (y/o
  alternando `rx` ±1) según `anim_phase` + un offset por hoja, de modo que NO se muevan
  todas igual (desordenado = natural). Precalcular 3–4 pasos por hoja.
- **Nivel B — gesto por estado:** `BIEN` sway rápido + pulso de yema; `SEDIENTA` sway
  lento; `AHOGADA` sway lento + gotas; `ESTRÉS` tiembla (offset X ±1 px de todo el grupo
  hoja+tallo).
- **Partículas:** `AHOGADA` 1–2 gotas (`fillEllipse` 2×3) que caen (avanzan en Y por
  `anim_phase`, reaparecen arriba). En `BIEN`, destello opcional en la yema.

Render: cada frame se redibuja **solo** la zona del icono (clear localizado del interior
del terrario por encima de la tierra; la tierra es chrome y no se toca). Respetar el
refresh de Demo Mode (~220 ms) y los clears localizados. **Sin transición** entre estados
(cambio instantáneo, puede ser rápido). Total < 20 formas → **sin sprite** (Nivel 4); si
en HW el parpadeo del icono animado fuese visible, envolver SOLO el rect del icono en un
`TFT_eSprite` pequeño (no toda la pantalla).

### Lógica de diagnóstico

Prioridad `suelo > temperatura > humedad aire > luz`. El estado es el del primer sensor
fuera de rango por prioridad; si todos están en óptimo → `BIEN`. `culprit` = índice de
ese sensor (Suelo=0, Temp=1, Aire=2, Luz=3); en `BIEN`, `culprit = 0xFF`. Thresholds en
la sección "Thresholds V1".

### Integración en el firmware

- **Enum:** añadir `LAB_PLANT_SCREEN` en `include/tft_display.h` (junto a los `LAB_*_SCREEN`).
- **Render dispatch:** añadir el caso en el switch principal de render que enruta pantallas,
  llamando al render de `ui_lab_plant`.
- **Carrusel** (`src/rotary.cpp`, `kCarousel[]`): insertar `{ LAB_PLANT_SCREEN, -1 }` entre
  `{ LAB_DUAL_TH_SCREEN, -1 }` (CLIMA LAB) y `{ LAB_WIDGET_MIX_SCREEN, -1 }` (TERMO LAB).
- **Visibilidad condicional:** omitir del ciclo si el sensor de suelo no da lectura válida
  (`isnan(soil_humidity)`); ver `include/external_sensor_state.h`.
- **i18n:** añadir título y strings de estado (ES/CAT/EN) al sistema de strings del proyecto.
- **Alert jewel:** `L_ALERT_JEWEL` (14,118) cae dentro del terrario. En esta pantalla NO se
  dibuja el alert jewel (la planta comunica el estado). Si el runtime lo fuerza, suprimirlo
  en esta pantalla.

## Alcance de V1

### Sí entra en V1
- Pantalla única `PLANTA LAB` con layout terrario (planta sembrada izq) + cuatro filas
  de rango (Suelo, Temp, Aire, Luz) + card de diagnóstico.
- Icono héroe nuevo (planta en tierra), distinto del icono de Suelo.
- Cuatro estados por postura + color con lógica de prioridad.
- Tallo recto en todos los estados.
- La tierra del terrario refleja el estado de suelo (seca / sana / encharcada).
- Animación continua acotada en todos los estados (sway + gesto de estado + partículas en
  `AHOGADA`); sin transición entre estados.
- Thresholds hardcodeados (perfil interior genérico).
- Visibilidad condicional según sensor de suelo disponible.
- Strings i18n en ES / CAT / EN.
- Posición en el carrusel.

### No entra en V1
- Mapeo fino sensor→canal visual (cada sensor controla una variable distinta del dibujo).
- Selector de perfiles de planta (tropical, cactus, interior, personalizado).
- Persistencia NVS de perfil seleccionado.
- Segunda pantalla del Lab (vista de rangos por sensor).
- Variación de forma del icono por crecimiento (tallo que crece, hojas que aparecen).
- Animación de transición entre estados (hojas cayendo).
- Submodos de navegación dentro de `PLANTA LAB`.
- Actividades o misiones guiadas.
- Notificaciones BLE de cambio de estado.

## Roadmap de versiones

### V2 — Perfiles de planta
- Selector de perfil al entrar a `PLANTA LAB` (o desde el menú).
- Perfiles: interior, tropical, cactus, mucha agua, poca agua, mucha luz, poca luz, personalizado.
- Guardado en NVS con namespace propio.
- El nombre del perfil activo aparece como subtítulo: `PLANTA LAB · Tropical`.
- Segunda pantalla del Lab: lista de los cuatro sensores con barra de rango que muestra zona baja / óptima / alta y posición del valor actual. Usa la misma paleta de zonas que `Sensor Zone Rango` para coherencia visual.

### V3 — Planta viva
- Mapeo fino sensor→canal visual completo (ver sección "Mapeo sensor → canal visual"):
  tierra por suelo, postura por temperatura, partículas por humedad de aire, sol por luz.
- Animación de transición entre estados (hojas que caen al enfermar).
- La planta tiene historia: empieza pequeña (tallo corto, una hoja) y crece según condiciones acumuladas.
- Con buenas condiciones: el tallo se alarga, aparecen más hojas.
- Con estrés prolongado: las hojas caen, el tallo se dobla.
- Sistema de frames de pixel art para los estados de crecimiento.
- Actividades guiadas: misiones con objetivo y seguimiento de condiciones.
- Comparación entre experimentos.
- Posible integración con app companion vía BLE.

## Consecuencias

`PLANTA LAB` convierte el sensor de suelo en algo significativo para niños que de otro modo no sabrían qué hacer con un porcentaje de humedad de tierra. La pantalla enseña que el bienestar de una planta depende de un sistema de variables, no de una sola lectura.

La decisión de crear un icono héroe nuevo separa la metáfora de cuidado de planta del icono técnico de Suelo. La decisión de hardcodear los thresholds permite validar los rangos en el aula antes de exponerlos como configurables.

El mecanismo de visibilidad condicional (solo aparece si hay sonda de suelo) mantiene la coherencia del carrusel para usuarios sin ese sensor.

## Plan de aplicación

1. Mantener este ADR como fuente de decisión para `PLANTA LAB`.
2. Diseñar el layout con coordenadas exactas sobre 160×128 px.
3. Implementar la pantalla en un archivo nuevo (`ui_lab_plant.cpp` o equivalente).
4. Strings i18n, posición en carrusel, lógica de visibilidad condicional.
5. Verificar en hardware: colores de estado, texto de diagnóstico en las tres lenguas, animación continua, filas de rango y aparición condicional por sensor de Suelo.
6. Actualizar CHANGELOG y ROADMAP del firmware.
