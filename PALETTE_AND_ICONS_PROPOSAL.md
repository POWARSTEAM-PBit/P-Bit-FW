# Paleta e Iconos — Propuesta P-Bit

**Fecha:** 2026-05-16
**Estado:** PROPUESTA — pendiente de validación en hardware

---

## Contexto

Este documento recoge el análisis completo de la sesión 2026-05-16 sobre el sistema de color e iconografía del firmware P-Bit. Los cambios descritos aquí **NO están implementados en producción**. El código activo sigue usando la paleta inventariada en la sección "Paleta actual".

Las nuevas entradas de pantalla SENSOR CARD y VALOR LAB son el banco de prueba de esta propuesta: son código nuevo que se implementa directamente con la paleta propuesta, de modo que si se ve bien en hardware, la paleta queda validada sin riesgo de regresión en las pantallas existentes.

Este documento es el handoff canónico para agentes y sesiones futuras. Antes de tocar cualquier color o icono en archivos de producción, leer la sección "Estrategia de implementación".

---

## Paleta actual (inventario)

Qué colores usa exactamente cada pantalla hoy, tal como está en el código fuente.

### HOME — `ui_lab_home_cards.cpp`

| Elemento | Valor | RGB888 |
|----------|-------|--------|
| TEMP card accent | `TFT_ORANGE` | (255, 165, 0) |
| HUM card accent | `TFT_CYAN` | (0, 255, 255) |
| LUZ card accent | `0xFFE0` | (255, 252, 0) |
| SOUND card accent | `TFT_MAGENTA` | (255, 0, 255) |
| Card interior bg | `0x0841` | — |
| Card frame | `0x2945` | — |

### SENSOR LAB — `ui_lab_focus.cpp` (primario + secundario por sensor)

| Sensor | Primary | Secondary |
|--------|---------|-----------|
| TEMP | `TFT_ORANGE` | `TFT_MAGENTA` |
| HUM | `TFT_CYAN` | `(168,96,255)` → `0xAB1F` aprox |
| DS18 | `(180,100,255)` | `TFT_CYAN` |
| LUZ | `TFT_YELLOW` | `TFT_CYAN` |
| SOUND | `TFT_MAGENTA` | `TFT_GREEN` |
| SOIL | `TFT_GREEN` | `TFT_CYAN` |

Fondos: navy `(8,12,18)` y navy oscuro `(4,8,20)`.

### GAUGE LAB — `ui_lab_widget_showcase.cpp` (constantes locales)

```cpp
kWarmOrange   = TFT_ORANGE
kCoolBlue     = TFT_CYAN
kHotPink      = TFT_MAGENTA
kNeonGreen    = 0x3FE8   // ≈ (56, 252, 64)
kElectricBlue = 0x35FF   // ≈ (49, 190, 255)
kRoyalBlue    = 0x21D9   // ≈ (33, 57, 206)
kNeonYellow   = 0xFFE0
kDeepPurple   = 0x881F   // ≈ (139, 0, 255)
```

Pares por sensor en gauge:

| Sensor | Ring primary | Ring secondary |
|--------|-------------|----------------|
| TEMP | kWarmOrange | kHotPink |
| HUM | kCoolBlue | kElectricBlue |
| LUZ | kNeonYellow | kCoolBlue |
| SOUND | kHotPink | kNeonGreen |
| SOIL | kNeonGreen | kCoolBlue |
| DS18 | kElectricBlue | kHotPink |

### CLIMA LAB — `ui_lab_dual.cpp` (temperatura y humedad responsivas)

**Temperatura** (color de shell según valor):

| Rango | RGB888 |
|-------|--------|
| ≤ 15 °C | (0, 180, 255) |
| ≤ 22 °C | (80, 255, 188) |
| ≤ 27 °C | (255, 232, 0) |
| ≤ 32 °C | (255, 144, 0) |
| > 32 °C | (255, 48, 168) |

**Humedad** (color de shell según valor):

| Rango | RGB888 |
|-------|--------|
| < 35 % | (180, 96, 255) |
| < 70 % | (0, 230, 255) |
| ≥ 70 % | (96, 170, 255) |

Footer bg: `(8, 12, 18)`.

---

## Problemas detectados

### 1. HUM y DS18 son el mismo par de colores, intercambiado

En SENSOR LAB:
- HUM = `TFT_CYAN` (primary) + purple (secondary)
- DS18 = purple (primary) + `TFT_CYAN` (secondary)

Son literalmente el mismo par con los roles swapeados. Un niño mirando ambas pantallas no puede distinguir qué sensor es qué. Es el problema más grave del sistema actual.

### 2. SOIL y HUM comparten cian

`TFT_CYAN` aparece como primary de HUM y como secondary de SOIL. Los dos sensores "hídricos" del dispositivo (humedad del aire y humedad del suelo) se ven demasiado parecidos. Esto confunde la lectura semántica del color: el cian debería pertenecer a uno solo.

### 3. TFT_ORANGE es dorado, no ácido

`TFT_ORANGE = (255, 165, 0)` — el canal verde en 165 hace que lea como ámbar/dorado, no como naranja encendido. Para la estética Nintendo punk de P-Bit, `(255, 100, 0)` es más ácido, más saturado y más correcto. La diferencia es notable en la pantalla ST7735.

### 4. Sin archivo canónico de paleta

Cada pantalla define sus propias constantes locales (`kWarmOrange`, `kHotPink`, etc. solo existen en `ui_lab_widget_showcase.cpp`). No hay un `include/palette.h` como fuente de verdad. El resultado es drift: el mismo sensor puede tener colores ligeramente distintos en pantallas distintas. Si se quiere cambiar el color de TEMP, hay que modificar cuatro archivos.

### 5. Inconsistencia entre pantallas para el mismo sensor

DS18 usa purple como primary en SENSOR LAB pero electricBlue+hotPink en GAUGE LAB. No existe un color de identidad fijo por sensor. Esto hace imposible que el usuario construya asociación color-sensor a través del uso.

---

## Paleta propuesta — sistema canónico

Un color primario por sensor (identidad) más un color secundario (complemento para efectos dual-color: sparklines, gauge rings, bordes degradados, highlights).

### Tabla maestra de identidad

Actualizada 2026-05-17: añadidos P3 (acento cálido) y P4 (contraste frío), extraídos de los colores ya presentes en `ui_graph.cpp` (max-label, min-label, band-label). Son colores **existentes en el código** — no inventados. La columna "Fuente" indica el módulo de origen.

| Sensor | P1 · Primary | Hex | P2 · Secondary | Hex | P3 · Acento cálido | Hex | P4 · Contraste frío | Hex |
|--------|-------------|-----|----------------|-----|--------------------|-----|---------------------|-----|
| **TEMP** | Naranja ácido | `0xFB20` | Magenta punk | `0xF81F` | Amarillo cálido *(graph max-label)* | `0xFFD8` | Azul frío *(graph min-label)* | `0x069F` |
| **HUMEDAD** | Cian eléctrico | `0x069F` | Cobalto | `0x3BDF` | Rosa *(graph band-label)* | `0xFBAE` | Azul marino *(graph grid-v)* | `0x0318` |
| **LUZ** | Amarillo | `0xFFE0` | Ámbar | `0xFDA0` | Dorado claro *(graph band-label)* | `0xFF86` | Dorado oscuro *(graph border)* | `0x7E60` |
| **SONIDO** | Magenta punk | `0xF81F` | Verde neón | `0x07E0` | Naranja *(graph max-label)* | `0xFD20` | Púrpura oscuro *(graph grid-v)* | `0x5C2C` |
| **SUELO** | Verde cálido | `0x2F85` | Cian menta *(focus secondary)* | `0x07FF` | Menta claro *(graph band-label)* | `0x6FD6` | Verde oscuro *(graph border)* | `0x0300` |
| **TERMÓMETRO** | Violeta eléctrico | `0xA1FF` | Azul eléctrico *(gauge primary)* | `0x35FF` | Lila claro *(graph band-label)* | `0xCABF` | Cian frío *(graph min-label)* | `0x07FF` |

> Los hex565 de P3/P4 son aproximaciones de los valores `tft.color565(r,g,b)` que ya usa `ui_graph.cpp`. Los valores exactos se calculan en runtime — ver las funciones `graph_band_label_color()`, `graph_max_label_color()`, `graph_min_label_color()` en ese archivo.

### Uso semántico de los 4 colores

| Color | Uso principal |
|-------|--------------|
| P1 · Primary | Borde de card, dial del gauge, icono activo, línea de gráfica, segmentos llenos |
| P2 · Secondary | Sparklines, borde degradado en FOCUS, ring exterior del gauge, highlights de badge |
| P3 · Acento cálido | Max-labels en gráfica, pico de VU, highlight de valor extremo |
| P4 · Contraste frío | Min-labels en gráfica, referencia de cero, grid sutil, texto de estado neutral |

### Backgrounds — sin cambio

| Uso | Color | Notas |
|-----|-------|-------|
| Panel navy principal | `tft.color565(8, 12, 18)` | No cambiar — funciona perfectamente |
| Fondo de gráfica | `tft.color565(4, 8, 20)` | No cambiar |
| Card interior bg | `0x0841` | Mantener |
| Card border | `0x2945` | Mantener |

### Lo que NO cambia respecto al estado actual

- `TFT_YELLOW` para LUZ: ya está bien, es el correcto
- `TFT_MAGENTA` para SOUND primary: ya está bien
- `TFT_GREEN` para SOUND secondary: ya está bien
- Todos los fondos navy: son la mejor decisión del sistema actual, no tocar

### Propuesta de `include/palette.h`

```cpp
// P-Bit sensor identity palette — propuesta 2026-05-16
// PENDIENTE DE APROBACIÓN EN HARDWARE
// Este archivo no existe aún en el proyecto — es la propuesta canónica.
#pragma once
#include <TFT_eSPI.h>

// ── Sensor identity: primary colors ──────────────────────────────────────────
constexpr uint16_t PB_TEMP_PRIMARY    = 0xFB20;  // naranja ácido (255,100,0)
constexpr uint16_t PB_HUM_PRIMARY     = 0x069F;  // cian eléctrico (0,210,255)
constexpr uint16_t PB_LUZ_PRIMARY     = 0xFFE0;  // amarillo (255,255,0)
constexpr uint16_t PB_SOUND_PRIMARY   = 0xF81F;  // magenta punk (255,0,255)
constexpr uint16_t PB_SOIL_PRIMARY    = 0x2F85;  // verde cálido (40,240,40)
constexpr uint16_t PB_DS18_PRIMARY    = 0xA1FF;  // violeta eléctrico (160,60,255)

// ── Sensor identity: secondary colors ────────────────────────────────────────
constexpr uint16_t PB_TEMP_SECONDARY  = 0xF81F;  // magenta punk
constexpr uint16_t PB_HUM_SECONDARY   = 0x3BDF;  // cobalto (60,120,255)
constexpr uint16_t PB_LUZ_SECONDARY   = 0xFDA0;  // ámbar (255,180,0)
constexpr uint16_t PB_SOUND_SECONDARY = 0x07E0;  // verde neón (0,255,0)
constexpr uint16_t PB_SOIL_SECONDARY  = 0xCC07;  // tierra (200,130,60)
constexpr uint16_t PB_DS18_SECONDARY  = 0xF81F;  // magenta punk

// ── Shared UI colors ──────────────────────────────────────────────────────────
constexpr uint16_t PB_UNIT_COLOR      = 0xFD20;  // kWarmOrange — etiquetas de unidades
constexpr uint16_t PB_CARD_BG         = 0x0841;  // card interior dark
constexpr uint16_t PB_CARD_BORDER     = 0x2945;  // dark blue-grey border
constexpr uint16_t PB_PANEL_NAV_BG    = 0x0862;  // (8,12,18) navy oscuro principal
constexpr uint16_t PB_GRAPH_BG        = 0x0042;  // (4,8,20) navy más oscuro para gráficas
```

> Nota sobre `PB_PANEL_NAV_BG`: el valor `0x0862` es la aproximación RGB565 de `(8,12,18)`. Verificar contra el valor calculado por `tft.color565(8,12,18)` en hardware antes de hacer canónico.

---

## Análisis de iconos — inventario actual

Todos los iconos del sistema son procedurales (dibujados con primitivas de TFT_eSPI en runtime). No hay bitmaps ni sprites. Están definidos principalmente en `ui_icons.cpp` con variantes `_large` y `_xxl` en `ui_lab_widget_showcase.cpp`.

### Problema transversal a todos los iconos

`TFT_BLACK` hardcodeado como color de detalle interior. Esto funciona solo sobre fondo negro puro. Sobre los fondos navy del sistema `(8,12,18)` o `(4,8,20)`, los detalles negros crean manchas visibles y rompen la lectura del icono.

---

### `pbit_draw_temp_icon` — Termómetro

**Construcción actual:** tubo redondeado 6×10 + bulbo r=4 + `fillRect` interior negro + 2 ticks horizontales (marcas de temperatura).

**Problema:** el `fillRect` interior usa `TFT_BLACK` hardcodeado. Sobre fondo navy, el interior del termómetro aparece negro intenso en lugar de desaparecer con el fondo.

**Veredicto:** Funcional y reconocible como termómetro. Aprobar forma, corregir solo el negro hardcodeado.

---

### `pbit_draw_humidity_icon` — Gota

**Construcción actual:** `fillTriangle` (punta hacia arriba) + `fillCircle` (bulbo inferior) + `fillCircle` negro superpuesto (efecto ring interior). Versión large: triángulo base en `cy-3`, círculo top en `cy-6`.

**Problema 1:** el círculo negro interior que crea el efecto donut. A tamaños pequeños, a 160×128px, lee como agujero o mancha oscura, no como brillo de gota.

**Problema 2 (versión large):** la transición de 3px entre el borde del triángulo y el círculo es visible. La gota se ve partida en dos piezas.

**Veredicto:** REHACERLO. Confirmado por producto.

---

### `pbit_draw_light_icon` — Sol

**Construcción actual:** círculo r=3 + 4 rayos en H/V (`drawFastHLine`/`drawFastVLine`) + 4 rayos diagonales de 1px (`drawLine`).

**Problema:** los 4 rayos diagonales de 1px desaparecen o se ven intermitentes a tamaños de 16px o menos. El sol pierde simetría y parece una cruz, no un sol.

**Veredicto:** Aceptable. Mejorable cambiando diagonales de 1px a 2px de grosor.

---

### `pbit_draw_sound_icon` — Micrófono

**Construcción actual:** cuerpo redondeado 6×9 + pie vertical de 3px + base horizontal de 9px.

**Problema:** la base de 9px es más ancha que el cuerpo de 6px. El resultado visual es una mesa o un seta, no el pie de un micrófono. La proporción correcta sería base de 5–7px máximo.

**Veredicto:** Reconocible pero con proporciones incorrectas. Corregir ancho de base.

---

### `pbit_draw_plant_icon` — Planta / SOIL

**Construcción actual:** base horizontal 11×2 + tallo vertical 2×7 + 2 hojas como `fillTriangle` anguladas a cada lado.

**Problema:** los triángulos agudos de las hojas leen como flechas o puntas a tamaño pequeño. No hay ninguna curva que sugiera hoja vegetal.

**Veredicto:** MEJORAR. Confirmado por producto.

---

### `pbit_draw_probe_icon` — Sonda DS18B20

**Construcción actual:** cuerpo horizontal 9×5 + conector derecho 4×3 + cable en diagonal hacia la izquierda.

**Problema:** la orientación horizontal con conector y cable diagonal hace que lea como relé eléctrico, bloque de terminales o conector, no como sonda de temperatura. Un DS18B20 físico es un componente vertical cilíndrico con punta.

**Veredicto:** REDISEÑAR completamente. Confirmado por producto.

---

### Iconos grandes (`_large`, `_xxl` en `ui_lab_widget_showcase.cpp`)

Replican los mismos diseños con dimensiones ampliadas. Los mismos problemas se escalan: el negro hardcodeado en el termómetro es más visible, el donut de la gota es más obvio, la base ancha del micro es más notoria, la sonda horizontal no mejora con el tamaño.

---

## Propuestas de rediseño de iconos

### HUM — gota corregida (todas las tallas)

Cambios a aplicar:
- Eliminar el `fillCircle` negro interior. La gota queda como silueta sólida, limpia.
- Agregar highlight de agua: 2 píxeles en color blanco o muy claro en posición superior-izquierda del bulbo.
- En versión large: cambiar `cy - 18` a `cy - 15` en el triángulo superior para que el solapamiento con el círculo cierre la gota sin costura visible.

Resultado esperado: gota reconocible a cualquier tamaño, sin artefactos negros.

### DS18 — sonda vertical con punta (todas las tallas)

```
Cuerpo cilíndrico vertical + punta metálica hacia abajo + conector corto arriba

Versión pequeña:
  fillRoundRect(cx-3, cy-8, 6, 12, 3, color)   // cuerpo
  fillTriangle(cx-3, cy+4, cx+3, cy+4, cx, cy+9, color)  // punta
  drawFastVLine(cx, cy-11, 4, color)             // conector arriba

Versión large/xxl: escalar proporcionalmente
```

Esto lee como sonda tipo TO-92 o sensor de inserción, que es exactamente lo que es el DS18B20. Reconocible también para niños sin conocimiento técnico porque es "la aguja que va en el agua/tierra".

### SOIL/PLANT — hojas redondeadas (todas las tallas)

Reemplazar cada `fillTriangle` de hoja por `fillRoundRect` horizontal pequeño con radio de esquina igual a la mitad del alto. Esto convierte la hoja de triángulo agudo a forma oval reconocible como hoja vegetal. Mantener base, tallo y posición XY general sin cambios.

### SOUND — proporciones corregidas (todas las tallas)

Reducir la base horizontal de 9px a 5–7px (centrada bajo el pie). El cuerpo 6×9 y el pie de 3px permanecen sin cambios. Solo el ancho de la base cambia.

### TEMP — eliminar TFT_BLACK hardcodeado

```cpp
// Antes:
tft.fillRect(cx - 1, cy - 6, 2, 6, TFT_BLACK);

// Después (opción A — pasar bg como parámetro):
void pbit_draw_temp_icon(int cx, int cy, uint16_t color, uint16_t bg = PB_PANEL_NAV_BG);
tft.fillRect(cx - 1, cy - 6, 2, 6, bg);

// Después (opción B — si el fondo siempre es nav bg):
tft.fillRect(cx - 1, cy - 6, 2, 6, PB_PANEL_NAV_BG);
```

Aplicar el mismo patrón a todas las variantes `_large` y `_xxl`.

### LUZ — rayos diagonales de 2px

```cpp
// Antes (1px invisible a tamaños pequeños):
tft.drawLine(cx-4, cy-4, cx-2, cy-2, color);

// Después (par de líneas paralelas para 2px efectivo):
tft.drawLine(cx-4, cy-4, cx-2, cy-2, color);
tft.drawLine(cx-3, cy-4, cx-1, cy-2, color);  // línea paralela +1px en X
```

Aplicar el mismo patrón a los 4 rayos diagonales.

---

## Estrategia de implementación

### Regla crítica: no cambiar producción sin validación en hardware

La pantalla ST7735 a 160×128px tiene comportamiento diferente al simulador y al ojo humano analizando código hexadecimal. Un color que parece bien en papel puede verse apagado, demasiado similar a otro, o perder contraste sobre el fondo navy en condiciones reales de iluminación.

### Orden de implementación

**Fase 1 — Banco de prueba en código nuevo (sin riesgo de regresión)**

Las pantallas SENSOR CARD y VALOR LAB son entradas nuevas que aún no existen en producción. Se implementan directamente con la paleta propuesta. Si se ven bien en hardware, la paleta está validada en contexto real sin haber tocado ningún archivo existente.

**Fase 2 — Prototipos de iconos en pantalla de test**

Agregar las variantes rediseñadas (HUM, DS18, SOIL, SOUND, TEMP, LUZ) como pantalla `LAB_ICON_TEST_SCREEN` que muestre ambas versiones en paralelo: actual vs propuesta. Validar en hardware sensor por sensor.

**Fase 3 — Migración a producción**

Solo los cambios aprobados en hardware, un sensor a la vez, actualizando HOME → SENSOR LAB → GAUGE LAB en ese orden para cada sensor. Verificar cada pantalla después de cada cambio.

**Fase 4 — Crear `include/palette.h`**

Solo después de que todos los colores estén validados. Crear el archivo canónico y migrar las constantes locales de cada `.cpp` para que usen las constantes de `palette.h`.

### Por qué las pantallas nuevas primero

- SENSOR CARD necesita iconos y colores para HUM, LUZ, SOUND y SOIL (entradas nuevas)
- VALOR LAB necesita shell per-sensor con icono y color de identidad
- Son código nuevo: usar la paleta propuesta desde el inicio es la implementación correcta
- Si se ve bien en hardware, la paleta queda aprobada sin riesgo de romper nada existente
- Si algo no funciona, se ajusta solo en el código nuevo antes de migrar

---

## Decisiones pendientes de producto

Ver también `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md` sección 8 y `LAB_GRAPH_UI_HANDOFF.md` para decisiones de navegación relacionadas.

### Paleta — decisiones abiertas

- [ ] ¿Aprobar naranja ácido `(255,100,0)` para TEMP P1? — requiere ver en hardware vs TFT_ORANGE actual
- [ ] ¿Aprobar cobalto `(60,120,255)` como P2 de HUMEDAD? — clave para diferenciar HUMEDAD vs TERMÓMETRO
- [ ] ¿Aprobar violeta eléctrico `(160,60,255)` para TERMÓMETRO P1? — diferenciación vs HUM y purple actual
- [ ] ¿Tierra `(200,130,60)` como P2 de SUELO o mantener cian menta? — decisión estética
- [ ] ¿Crear `include/palette.h`? — Fase 4 según estrategia. Bloqueada hasta validar P1/P2 en hardware.
- [ ] P3/P4 añadidos a tabla (2026-05-17): verificar valores hex565 exactos vs `tft.color565()` en hardware.

### Iconos — decisiones abiertas

- [ ] ¿Rediseño de DS18 como sonda vertical (TO-92)? — requiere prototipo en `LAB_ICON_TEST_SCREEN`
- [ ] ¿Rediseño de HUM como gota limpia sin donut? — requiere prototipo
- [ ] ¿Mejora de SOIL con hojas redondeadas? — requiere prototipo
- [ ] ¿Corrección de proporciones de SOUND (base horizontal)? — cambio menor, bajo riesgo
- [ ] ¿Firma de `pbit_draw_temp_icon` con parámetro `bg` para eliminar TFT_BLACK hardcodeado? — decisión de API interna

---

*Documento generado en sesión 2026-05-16. Próxima acción: implementar SENSOR CARD y VALOR LAB con paleta propuesta y validar en hardware.*
