# Paleta e Iconos — Propuesta P-Bit

**Fecha:** 2026-05-16 | **Actualizado:** 2026-05-20
**Estado:** PARCIALMENTE IMPLEMENTADO — paleta canónica activa en Sensor Zone; validación hardware pendiente

---

## Contexto

Este documento recoge el análisis completo de la sesión 2026-05-16 sobre el sistema de color e iconografía del firmware P-Bit. Ya no debe leerse como una propuesta intacta: desde entonces se creó `include/palette.h` y las vistas de Sensor Zone (`FOCUS`, `CARD`, `VALOR`, `GRAPH`, `GAUGE`) ya consumen la paleta P1/P2/P3/P4.

`SENSOR CARD` y `VALOR LAB` ya funcionan como banco de prueba real de esta paleta. Lo pendiente es validar en hardware ST7735 si esos colores se leen bien en condiciones reales y decidir si se extienden a pantallas madre (`HOME`, `CLIMA LAB`) o si esas mantienen su lógica responsiva.

Este documento es el handoff canónico para agentes y sesiones futuras. Antes de tocar cualquier color o icono en archivos de producción, leer la sección "Estrategia de implementación".

## Resumen de estado 2026-05-20

- **Implementado:** `include/palette.h` con P1/P2/P3/P4 y helpers `pb_primary`, `pb_secondary`, `pb_accent_warm`, `pb_contrast_cool`.
- **Implementado:** uso de paleta en `ui_lab_focus.cpp`, `ui_graph.cpp`, `ui_lab_widget_showcase.cpp` y `ui_lab_sensor_cards.cpp`.
- **Implementado:** iconos `humidity`, `probe` y `plant` rediseñados en `src/ui_icons.cpp`; icono Bluetooth añadido para BLE oculto.
- **Pendiente hardware:** validar contraste, diferencias entre sensores, legibilidad de `off` segments y comportamiento real sobre fondos navy.
- **Pendiente técnico menor:** revisar si el termómetro debe recibir fondo como parámetro para eliminar el `TFT_BLACK` interior sobre navy.

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

## Paleta canónica — sistema implementado en Sensor Zone

Un color primario por sensor (identidad) más un color secundario (complemento para efectos dual-color: sparklines, gauge rings, bordes degradados, highlights).

### Tabla maestra de identidad

Actualizada 2026-05-20: la tabla refleja `include/palette.h`. P1/P2/P3/P4 ya no son solo propuesta; son la fuente de verdad para `FOCUS`, `CARD`, `VALOR`, `GRAPH` y `GAUGE`.

| Sensor | P1 · Primary | Hex | P2 · Secondary | Hex | P3 · Acento cálido | Hex | P4 · Contraste frío | Hex |
|--------|-------------|-----|----------------|-----|--------------------|-----|---------------------|-----|
| **TEMP** | Naranja ácido | `0xFA80` | Rosa eléctrico | `0xF814` | Oro eléctrico | `0xFE45` | Azul hielo | `0x055F` |
| **HUMEDAD** | Cian eléctrico | `0x075F` | Cobalto láser | `0x2A9F` | Aqua brillante | `0x8FFF` | Azul océano | `0x01F4` |
| **LUZ** | Amarillo puro | `0xFFE0` | Ámbar eléctrico | `0xFC40` | Oro neón | `0xFE40` | Dorado oscuro | `0x7A40` |
| **SONIDO** | Magenta punk | `0xF81F` | Verde ácido | `0x07E8` | Rojo neón | `0xF8A0` | Púrpura oscuro | `0x4011` |
| **SUELO** | Lima ácido | `0x47E8` | Tierra cálida | `0xCC04` | Menta claro | `0x87F0` | Verde oscuro | `0x0300` |
| **TERMÓMETRO** | Violeta eléctrico | `0xA01F` | Azul láser | `0x045F` | Amatista claro | `0xCC5F` | Cian frío | `0x0659` |

> Los nombres semánticos son la intención visual; los valores exactos viven en `include/palette.h`.

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

### `include/palette.h` actual

```cpp
// P-Bit sensor identity palette — retro arcade edition 2026-05-17
// Fuente de verdad para colores de sensores en todas las viz del sensor zone.
// Aplicar SOLO a: FOCUS, CARD, VALOR, GRAPH, GAUGE.
#pragma once
#include <stdint.h>

constexpr uint16_t PB_TEMP_P1  = 0xFA80;
constexpr uint16_t PB_TEMP_P2  = 0xF814;
constexpr uint16_t PB_TEMP_P3  = 0xFE45;
constexpr uint16_t PB_TEMP_P4  = 0x055F;

// ... idem para HUM, LUZ, SOUND, SOIL y DS18.

inline uint16_t pb_primary(uint8_t sensor_id);
inline uint16_t pb_secondary(uint8_t sensor_id);
inline uint16_t pb_accent_warm(uint8_t sensor_id);
inline uint16_t pb_contrast_cool(uint8_t sensor_id);
```

Los fondos navy siguen siendo compartidos por convención visual, pero no se centralizaron aquí como constantes públicas.

---

## Análisis de iconos — inventario actual

Todos los iconos del sistema son procedurales (dibujados con primitivas de TFT_eSPI en runtime). No hay bitmaps ni sprites. Están definidos principalmente en `ui_icons.cpp` con variantes `_large` y `_xxl` en `ui_lab_widget_showcase.cpp`.

### Problema transversal pendiente

`TFT_BLACK` hardcodeado como color de detalle interior. Esto funciona solo sobre fondo negro puro. Sobre los fondos navy del sistema `(8,12,18)` o `(4,8,20)`, los detalles negros crean manchas visibles y rompen la lectura del icono.

Estado 2026-05-20: el riesgo principal que queda visible en código es el canal interior del termómetro. Los iconos de humedad, planta y sonda ya fueron simplificados/rediseñados.

---

### `pbit_draw_temp_icon` — Termómetro

**Construcción actual:** tubo redondeado 6×10 + bulbo r=4 + `fillRect` interior negro + 2 ticks horizontales (marcas de temperatura).

**Problema:** el `fillRect` interior usa `TFT_BLACK` hardcodeado. Sobre fondo navy, el interior del termómetro aparece negro intenso en lugar de desaparecer con el fondo.

**Veredicto:** Funcional y reconocible como termómetro. Aprobar forma, corregir solo el negro hardcodeado.

---

### `pbit_draw_humidity_icon` — Gota

**Construcción actual:** `fillTriangle` (punta hacia arriba) + `fillCircle` (bulbo inferior) + `fillCircle` negro superpuesto (efecto ring interior). Versión large: triángulo base en `cy-3`, círculo top en `cy-6`.

**Problema histórico 1:** el círculo negro interior que creaba el efecto donut. A tamaños pequeños, a 160×128px, leía como agujero o mancha oscura.

**Problema histórico 2 (versión large):** la transición de 3px entre el borde del triángulo y el círculo era visible. La gota se veía partida en dos piezas.

**Estado actual:** rehecho en `src/ui_icons.cpp`: gota sólida sin donut, triángulo con base ±3*s tangente al círculo.

---

### `pbit_draw_light_icon` — Sol

**Construcción actual:** círculo r=3 + 4 rayos en H/V (`drawFastHLine`/`drawFastVLine`) + 4 rayos diagonales de 1px (`drawLine`).

**Problema:** los 4 rayos diagonales de 1px desaparecen o se ven intermitentes a tamaños de 16px o menos. El sol pierde simetría y parece una cruz, no un sol.

**Veredicto:** Aceptable. Mejorable cambiando diagonales de 1px a 2px de grosor.

---

### `pbit_draw_sound_icon` — Micrófono

**Construcción actual:** cuerpo redondeado 6×9 + pie vertical de 3px + base horizontal de 9px.

**Problema histórico:** la base era demasiado dominante frente al cuerpo.

**Estado actual:** sigue siendo reconocible y se usa en Sensor Zone; validar en hardware si la base actual se lee como micrófono a 1× y XL antes de tocarla otra vez.

---

### `pbit_draw_plant_icon` — Planta / SOIL

**Construcción actual:** base horizontal 11×2 + tallo vertical 2×7 + 2 hojas como `fillTriangle` anguladas a cada lado.

**Problema histórico:** los triángulos agudos de las hojas leían como flechas o puntas a tamaño pequeño. No había ninguna curva que sugiriera hoja vegetal.

**Estado actual:** mejorado en `src/ui_icons.cpp` con hojas redondeadas (`fillRoundRect`) y base/pot redondeado.

---

### `pbit_draw_probe_icon` — Sonda DS18B20

**Construcción actual:** módulo compacto con nodo superior, cuerpo corto y cable recto inferior en `src/ui_icons.cpp`.

**Problema histórico:** la orientación horizontal con conector y cable diagonal hacía que leyera como relé eléctrico, bloque de terminales o conector, no como sonda de temperatura.

**Estado actual:** no final. La versión aplicada evita la silueta cilíndrica problemática, pero el equipo no la considera todavía una identidad visual aprobada. Pendiente rediseñar/probar otra metáfora de sonda externa antes de producción final.

---

### Iconos grandes (`_xl` en `ui_icons.cpp`)

Replican los mismos diseños con factor `s=3` para el centro del gauge. Problemas históricos ya cerrados: gota sin donut y planta con hojas redondeadas. Pendiente: cerrar el icono DS18B20 final, validar termómetro sobre navy y proporciones del micrófono XL en hardware.

---

## Propuestas de rediseño de iconos

### HUM — gota corregida (aplicado)

Cambios a aplicar:
- Eliminar el `fillCircle` negro interior. La gota queda como silueta sólida, limpia.
- Agregar highlight de agua: 2 píxeles en color blanco o muy claro en posición superior-izquierda del bulbo.
- En versión large: cambiar `cy - 18` a `cy - 15` en el triángulo superior para que el solapamiento con el círculo cierre la gota sin costura visible.

Estado actual: aplicado como silueta sólida; queda validación visual en hardware.

### DS18 — sonda externa (no final)

```
Cuerpo cilíndrico vertical + punta metálica hacia abajo + conector corto arriba

Versión pequeña:
  fillRoundRect(cx-3, cy-8, 6, 12, 3, color)   // cuerpo
  fillTriangle(cx-3, cy+4, cx+3, cy+4, cx, cy+9, color)  // punta
  drawFastVLine(cx, cy-11, 4, color)             // conector arriba

Versión large/xxl: escalar proporcionalmente
```

Estado actual: la orientación vertical/cápsula fue descartada por lectura ambigua en hardware. La versión actual usa un módulo compacto con nodo y cable recto, pero sigue marcada como no final.

### SOIL/PLANT — hojas redondeadas (aplicado)

Ya se reemplazaron hojas triangulares por `fillRoundRect`, convirtiendo la hoja a forma oval reconocible. Mantener base, tallo y posición XY general salvo hallazgo en hardware.

### SOUND — proporciones corregidas (todas las tallas)

Estado actual: no tocar sin captura de hardware. Si se ve demasiado pesado, reducir la base manteniendo cuerpo y pie.

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

### Orden de implementación actualizado

**Fase 1 — Banco de prueba en código nuevo (hecho)**

Las pantallas SENSOR CARD y VALOR LAB ya usan la paleta canónica. Si se ven bien en hardware, la paleta queda validada en contexto real sin haber tocado pantallas madre.

**Fase 2 — Prototipos de iconos en pantalla de test (parcial)**

Variantes HUM, DS18 y SOIL ya están aplicadas en la biblioteca común. `LAB_ICON_TEST_SCREEN` sigue siendo útil para comparar en hardware si se quiere tocar SOUND, TEMP o LUZ.

**Fase 3 — Migración a producción (pendiente de decisión)**

Solo los cambios aprobados en hardware, un sensor a la vez, actualizando HOME → SENSOR LAB → GAUGE LAB en ese orden para cada sensor. Verificar cada pantalla después de cada cambio.

**Fase 4 — Crear `include/palette.h` (hecho)**

El archivo canónico ya existe y Sensor Zone lo consume. Lo pendiente es decidir si se amplía su uso a pantallas madre o se mantiene acotado.

### Por qué las pantallas nuevas primero

- SENSOR CARD y VALOR LAB ya usan color de identidad por sensor.
- Son el lugar correcto para validar la paleta sin romper pantallas existentes.
- Si algo no funciona en hardware, ajustar primero Sensor Zone antes de migrar la paleta a más pantallas.

---

## Decisiones pendientes de producto

Ver también `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md` sección 8 y `LAB_GRAPH_UI_HANDOFF.md` para decisiones de navegación relacionadas.

### Paleta — decisiones abiertas

- [x] Crear `include/palette.h` y helpers por índice de sensor.
- [x] Aplicar paleta a Sensor Zone (`FOCUS`, `CARD`, `VALOR`, `GRAPH`, `GAUGE`).
- [ ] Aprobar en hardware TEMP/HUM/DS18 como identidades realmente distinguibles.
- [ ] Validar si `PB_SOIL_P2` tierra cálida funciona mejor que cian menta en ST7735 real.
- [ ] Decidir si `HOME` y `CLIMA LAB` migran a paleta canónica o mantienen colores responsivos propios.
- [ ] Verificar P3/P4 en hardware, especialmente labels min/max y segmentos apagados.

### Iconos — decisiones abiertas

- [ ] Rediseño final de DS18B20 aprobado en hardware.
- [x] Rediseño de HUM como gota limpia sin donut.
- [x] Mejora de SOIL con hojas redondeadas.
- [ ] Corrección de proporciones de SOUND (base horizontal) solo si hardware lo pide.
- [ ] ¿Firma de `pbit_draw_temp_icon` con parámetro `bg` para eliminar TFT_BLACK hardcodeado? — decisión de API interna

---

*Documento generado en sesión 2026-05-16 y reclasificado el 2026-05-20. Próxima acción: validar Sensor Zone en hardware real antes de ampliar la paleta o tocar más iconos.*
