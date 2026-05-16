# Análisis de Pantallas Experimentales del P-Bit

Actualizado: 2026-05-16

## Estado actual del código

Todo el trabajo de ajuste visual documentado en las rondas anteriores está commiteado en `ce41581` (2026-05-16). El árbol git está limpio. Build verificado:

- `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` → `SUCCESS`
- RAM: `14.1%` — Flash: `69.8%` (915281 bytes)

Hay pendientes tanto de código (galerías incompletas) como de validación en hardware y decisiones de producto. Ver hoja de ruta en sección 10.

---

## 1. Carrusel visible actual

Orden real definido en `src/rotary.cpp` → `kVisibleAppScreens[]`:

1. `HOME` (`LAB_HOME_CARDS_SCREEN`) — aprobada
2. `CLIMA LAB` (`LAB_DUAL_TH_SCREEN`) — aprobada
3. `TEMP LAB` (`LAB_WIDGET_MIX_SCREEN`) — pendiente de hardware
4. `SOUND LAB` (`LAB_SOUND_VU_STACK_SCREEN`) — pendiente de hardware
5. `PLANT LAB` (`LAB_LINEAR_DASH_SCREEN`) — pendiente de hardware
6. `SENSOR LAB` (`LAB_SENSOR_FOCUS_SCREEN`) — referencia visual fuerte
7. `GRÁFICA` (`GRAPH_SCREEN`) — aprobada
8. `TEMP` — pantalla individual real
9. `HUM` — pantalla individual real
10. `LUZ` — pantalla individual real
11. `SONIDO` — pantalla individual real
12. `SUELO` — pantalla individual real
13. `DS18` — pantalla individual real
14. `SISTEMA` — pantalla individual real
15. `TIMER` — pantalla individual real
16. `GAUGE LAB` (`LAB_GAUGE_TEMP_SCREEN`) — pendiente de hardware; biblioteca de widgets
17. `VALOR LAB` (`LAB_VALUE_MODERN_SCREEN`) — pendiente de hardware; plantilla de referencia
18. `SENSOR CARD` (`LAB_SENSOR_CARD_SCREEN`) — pendiente de hardware

### Sub-vistas internas (no entran al carrusel directamente)

- `LAB_SOUND_VU_WAVE_SCREEN` — accesible desde SOUND LAB con pulsación corta
- `LAB_TEMP_CARD_SCREEN` — accesible desde SENSOR CARD con pulsación corta
- `LAB_DS18_CARD_SCREEN` — accesible desde SENSOR CARD con pulsación corta

### Pantallas compiladas y ocultas del carrusel

Definido en `isHiddenRestoreScreen()` en `src/rotary.cpp`:

- `LAB_DASH_OVERVIEW_SCREEN` — ESTADO LAB (4 filas, tooling de referencia)
- `LAB_ICON_SET_A/B/C_SCREEN` — galerías de iconos (OUTLINE/SOLID/PIXEL)
- `LAB_ICON_SIZES_ENV_SCREEN` / `EXT` — tamaños de iconos (S/M/L)
- `LAB_ICON_TEST_SCREEN` — comparativa procedural vs bitmap

---

## 2. Clasificación actual por rol

### Aprobadas como referencia visual estable

Estas pantallas no deben recibir más ajustes sin motivo concreto:

- **`HOME`** (`LAB_HOME_CARDS_SCREEN`) — candidata real de producto como portada. Dashboard 2×2 con Temp, Hum, Luz, Sound. No cubre Suelo ni DS18.
- **`CLIMA LAB`** (`LAB_DUAL_TH_SCREEN`) — candidata condicional de producto. Temp + Hum + banda de estado climático. Solo tiene sentido si el producto adopta navegación por familias.
- **`GRÁFICA`** (`GRAPH_SCREEN`) — candidata real de producto. Gráfica histórica de 6 sensores con pulsación corta para ciclar.

### Pendientes de validación en hardware

Código aplicado y correcto. Solo falta ver en pantalla real:

- **`TEMP LAB`** (`LAB_WIDGET_MIX_SCREEN`) — compara DHT11 vs DS18B20. Barra diferencial izquierda/derecha. Riesgo: textos apretados en idiomas largos.
- **`SOUND LAB`** (`LAB_SOUND_VU_STACK_SCREEN` + WAVE) — VU meter con alternancia por pulsación corta. Footer de estado.
- **`PLANT LAB`** (`LAB_LINEAR_DASH_SCREEN`) — lista de 5 sensores. Candidata para modo técnico/docente.
- **`GAUGE LAB`** (`LAB_GAUGE_TEMP_SCREEN`) — gauge circular con icono blanco y unidad naranja. Biblioteca de widgets.
- **`VALOR LAB`** (`LAB_VALUE_MODERN_SCREEN`) — dato grande + barra + sparkline. Plantilla de referencia de composición.
- **`SENSOR CARD`** (`LAB_SENSOR_CARD_SCREEN`) — galería de TEMP CARD y PROBE CARD. Rail triple, card completa.

### Referencia visual fuerte (no promover como pantalla extra)

- **`SENSOR LAB`** (`LAB_SENSOR_FOCUS_SCREEN`) — carrusel interno de 6 sensores con sparkline. La mejor plantilla para una futura v2 de pantallas individuales. Hoy visible en el carrusel como posición 6 para facilitar evaluación.

### Tooling interno (bien ocultas)

- `ESTADO LAB` (`LAB_DASH_OVERVIEW_SCREEN`) — 4 filas sin Suelo/DS18. Referencia de layout de lista.
- `LAB_ICON_SET_A/B/C_SCREEN` — familias de iconos.
- `LAB_ICON_SIZES_*_SCREEN` — tamaños de iconos.
- `LAB_ICON_TEST_SCREEN` — comparativa de pipeline de iconos.

---

## 3. Reglas visuales vigentes en código

Todas definidas en `include/layout.h`:

**Header (todas las pantallas):**

- Baseline título: `L_HEADER_Y = 18`
- Línea divisoria: `L_HEADER_LINE = 23`
- Primera Y útil: `L_CONTENT_TOP = 27`
- Línea: `x=4`, `w=152`
- Títulos con `C_BASELINE` (no `TC_DATUM`)

**Cards externas (lab screens):**

- `LC_SCREEN_X=2`, `LC_SCREEN_W=156`, `LC_SCREEN_BOTTOM=126`, `LC_CARD_RADIUS=4`, `LC_GAP=4`

**Cards 2×2 (HOME y familia):**

- `LC_MASTER_CARD_X0=2`, `LC_MASTER_CARD_X1=82`
- `LC_MASTER_CARD_Y0=27`, `LC_MASTER_CARD_Y1=79`
- `LC_MASTER_CARD_W=76`, `LC_MASTER_CARD_H=48`, `LC_MASTER_CARD_GAP=4`
- Bottom común: `126`

**Gráfica:**

- Banda sensor/valor: `LG_SENSOR_Y=27`
- Marco exterior: `x=2`, `y=46`, `w=156`, `h=66`, radio `4`
- Sprite interior: `154×64 px`

---

## 4. Lo que está resuelto (no volver a tocar)

- Dead code eliminado: las 6 funciones `draw_icon_*_big` ya no existen en `ui_lab_widget_showcase.cpp`.
- Colores GAUGE LAB: icono `TFT_WHITE`, unidad `kWarmOrange` cuando sensor válido — ya aplicado.
- Pantalla gris de startup: `kShowStartupLayoutValidation = false` — ya aplicado.
- Footer `Vista experimental` retirado de pantallas donde no aportaba acción.
- `LAB_SOUND_VU_WAVE_SCREEN` ya no es una parada del carrusel — es sub-vista interna de SOUND LAB.

---

## 5. Solapes detectados (sin resolver aún)

Estos solapes no requieren código ahora, pero condicionan decisiones de producto:

**Tres candidatos a overview** — no deben convivir en el producto final:

- `HOME` (2×2, solo ambientales) — recomendado
- `PLANT LAB` (lista, 5 sensores) — modo técnico
- `ESTADO LAB` (4 filas, tooling) — oculto

**Tres candidatos a detalle de sensor** — solo uno debe ser pantalla de producto:

- Pantallas individuales actuales (TEMP, HUM, etc.)
- `SENSOR LAB` (carrusel interno unificado)
- `VALOR LAB` / `GAUGE LAB` (plantillas)

**Tendencia** — esto sí puede convivir si los roles quedan claros:

- `GRÁFICA` como pantalla de análisis dedicado
- Sparklines en `SENSOR LAB` y `VALOR LAB` como contexto rápido

---

## 6. Pendiente de validación en hardware

Solo flashear y anotar — no requiere código:

1. Distancia línea–card `3 px` y bottom `126` — confirmar en todas las pantallas.
2. **TEMP LAB** — barra diferencial izquierda/derecha; colores; textos en idioma inglés (`Sense sensor` vs `Sonda`).
3. **Títulos largos** — `TEMPORIZADOR`, `TERMÓMETRO`, `TEMPERATURA` sin recorte con el header aprobado.
4. **SOUND LAB** — texto de estado, posición del indicador de límites.
5. **GAUGE LAB** — legibilidad del valor dentro del anillo; contraste icono blanco.
6. **VALOR LAB** — distinción visual sparkline/barra; tamaño del dato. (Nota: VALOR LAB aún no cicla por sensores; esta validación aplica solo al estado actual con temperatura fija.)
7. **SENSOR CARD / TEMP CARD / PROBE CARD** — rail, escala, marca de cero. (Nota: SENSOR CARD aún solo cicla entre TEMP y DS18; esta validación aplica al estado actual.)
8. Parpadeo en redraw rápido (SENSOR LAB, ESTADO LAB).

---

## 7. Pendientes de código

### Galerías de sensores incompletas

El plan es que GAUGE LAB, VALOR LAB y SENSOR CARD sean galerías que ciclan por todos los sensores con pulsación corta. Estado actual:

| Pantalla | Archivo | Estado galería |
| --- | --- | --- |
| **GAUGE LAB** | `ui_lab_widget_showcase.cpp` | ✅ Completo — cicla 6 sensores: Temp, Hum, Luz, Sound, Suelo, DS18 |
| **SENSOR CARD** | `ui_lab_sensor_cards.cpp` | ⚠️ Parcial — solo `CARD_TEMP` ↔ `CARD_DS18`. Faltan: Hum, Luz, Sound, Suelo |
| **VALOR LAB** | `ui_lab_widget_showcase.cpp` | ❌ No implementado — fijo en temperatura DHT11. Shell y lógica de ciclo pendientes |

Lo que requiere SENSOR CARD: ampliar `LabSensorCardId` con 4 sensores más y sus specs en `kCardSpecs[]`.

Lo que requiere VALOR LAB: añadir enum de sensor activo (al estilo de `GaugeLabSensor`), adaptar `draw_lab_value_shell()` para icono/etiqueta/colores por sensor, y conectar `lab_value_cycle_sensor()` en `rotary.cpp`.

### Pantallas madre (TEMP, HUM, LUZ, SOUND, SUELO, DS18)

Las pantallas individuales actuales recibieron ajustes experimentales (cards, chips de icono, etc.) durante las rondas de lab. Antes de cerrar el producto, hay que revisar si estas pantallas quedaron en su estado original limpio o si acumularon cambios que no forman parte del diseño aprobado. No hacer ahora — es el paso 2 de la hoja de ruta.

---

## 8. Decisiones de producto pendientes

Antes de mover más pantallas o hacer más ajustes, conviene cerrar estas:

1. **¿TEMP CARD, PROBE CARD y TEMP LAB se mantienen visibles en el carrusel o se ocultan?**
2. **¿GAUGE LAB, VALOR LAB y SENSOR CARD quedan como laboratorio permanente o se promueven?**
3. **¿La portada cubre solo sensores ambientales integrados o también los externos (Suelo y DS18)?**
4. **¿SENSOR LAB es candidata a sustituir las pantallas individuales en v2 o solo referencia de diseño?**
5. **¿Qué hacer con las pantallas madre?** — ver paso 2 de la hoja de ruta.

---

## 8. Estructura de navegación recomendada

### Opción actual (mínimo cambio, árbol claro)

```text
HOME → CLIMA LAB → TEMP LAB → SOUND LAB → PLANT LAB → SENSOR LAB →
GRÁFICA → TEMP → HUM → LUZ → SONIDO → SUELO → DS18 → SISTEMA → TIMER →
GAUGE LAB → VALOR LAB → SENSOR CARD
```

Condición: aceptar que HOME resume solo sensores ambientales integrados.

### Opción futura v2 (menos pantallas, más unificado)

```text
HOME → SENSOR LAB (detalle genérico) → GRÁFICA → TIMER → SISTEMA
```

Coste: requiere refactor real de navegación y menús contextuales por sensor.

---

## 9. Hoja de ruta acordada

Orden de trabajo definido. No avanzar al siguiente paso sin cerrar el anterior:

### Paso 1 — Completar galerías de sensores (código pendiente)

- Ampliar SENSOR CARD para ciclar por los 6 sensores (no solo TEMP y DS18).
- Implementar ciclo de sensores en VALOR LAB (hoy fijo en temperatura DHT11).
- GAUGE LAB ya está completo — no tocar.
- Validar build y hacer prueba básica en hardware antes de pasar al paso 2.

### Paso 2 — Revisar y definir pantallas madre

- Revisar el estado actual de TEMP, HUM, LUZ, SOUND, SUELO y DS18.
- Identificar si acumularon cambios experimentales no aprobados (cards, chips, etc.).
- Decidir si se revierten a su estado original o si los cambios se aprueban.
- No implementar cambios hasta tener una decisión clara por pantalla.

### Paso 3 — Reorganizar el carrusel

- Decidir qué pantallas lab sobreviven como pantallas de producto y cuáles se ocultan.
- Ajustar `kVisibleAppScreens[]` en `src/rotary.cpp` con el orden final aprobado.
- Cerrar las decisiones pendientes de la sección 8.

### Paso 4 — Revisión final de idioma y funcionalidad

- Verificar que todas las claves de `include/languages.h` tienen traducción completa en todos los idiomas.
- Comprobar que todas las pantallas visibles funcionan correctamente (lectura de sensores, caché, redraw, sleep/wake).
- Validar en hardware el resultado completo antes de declarar el ciclo cerrado.

---

## 10. Protocolo para siguientes sesiones

- No reescribir pantallas desde cero sin pedirlo.
- Respetar el contrato de header y zonas seguras de `include/layout.h`.
- Documentar cada ajuste con: archivo, motivo, build, pendiente de hardware.
- Si una pantalla sigue pisando texto o bordes tras dos microajustes, proponer ocultarla del carrusel en lugar de seguir acumulando parches.
- Cada ajuste nuevo debe quedar reflejado en `LAB_GRAPH_UI_HANDOFF.md` con: archivo tocado, motivo, estado de build y qué queda pendiente de validar en pantalla real.

---

## 11. Análisis de paleta de colores e iconos — 2026-05-16

### Estado
Análisis completado. Propuesta documentada en `PALETTE_AND_ICONS_PROPOSAL.md`. Ningún cambio aplicado en producción. Las entradas nuevas de SENSOR CARD y VALOR LAB usarán la paleta propuesta como banco de prueba.

### Problemas detectados en paleta

**HUM y DS18 indistinguibles (crítico):**
En SENSOR LAB, HUM = cyan(primario)+purple(secundario) y DS18 = purple(primario)+cyan(secundario). Mismo par de colores con roles intercambiados. Un niño no puede distinguirlos.

**SOIL y HUM comparten cian:**
HUM primary = TFT_CYAN. SOIL secondary = TFT_CYAN. Dos sensores hídricos con el mismo color dominante.

**TFT_ORANGE es dorado, no ácido:**
TFT_ORANGE = (255,165,0) — el canal G=165 lo hace amber/dorado. Para el estilo punk/Nintendo, (255,100,0) es más encendido.

**Sin fuente de verdad:**
Cada pantalla define sus constantes locales. Hay drift de color entre screens para el mismo sensor.

### Paleta canónica propuesta

| Sensor | Primary propuesto | Hex | Secondary propuesto | Hex |
|--------|-----------------|-----|---------------------|-----|
| TEMP | Naranja ácido (255,100,0) | 0xFB20 | Magenta punk | 0xF81F |
| HUM | Cian eléctrico (0,210,255) | 0x069F | Cobalto (60,120,255) | 0x3BDF |
| LUZ | Amarillo (sin cambio) | 0xFFE0 | Ámbar (255,180,0) | 0xFDA0 |
| SOUND | Magenta (sin cambio) | 0xF81F | Verde neón (sin cambio) | 0x07E0 |
| SOIL | Verde cálido (40,240,40) | 0x2F85 | Tierra (200,130,60) | 0xCC07 |
| DS18 | Violeta (160,60,255) | 0xA1FF | Magenta punk | 0xF81F |

Los fondos navy `(8,12,18)` y `(4,8,20)` **no cambian** — están bien y son parte del carácter visual del sistema.

### Problemas detectados en iconos

| Icono | Problema | Decisión |
|-------|---------|---------|
| HUM | Ring negro interior crea efecto donut. En versión large, triángulo y círculo no se unen suavemente (brecha 3px) | Confirmado: REHACERLO |
| DS18 | Sonda horizontal con cables lee como conector eléctrico, no sonda de temperatura | Confirmado: REDISEÑAR como sonda vertical |
| SOIL | Hojas triangulares agudas leen como flechas a pequeño tamaño | Confirmado: MEJORAR con hojas ovales |
| SOUND | Base de micrófono de 9px sobre cuerpo de 6px — parece mesa | Pendiente: propuesta de 7px |
| LUZ | Rayos diagonales de 1px desaparecen a tamaños pequeños | Pendiente: rayos de 2px |
| TEMP | TFT_BLACK hardcodeado en detalle interior — se rompe sobre fondos navy | Pendiente: parámetro bg_color |

### Estrategia de validación

**No cambiar producción sin validación en hardware.** Proceso aprobado:

1. Implementar variantes `_v2` en `LAB_ICON_TEST_SCREEN` (ya existe)
2. Comparar en hardware: icono actual vs propuesto lado a lado
3. Aprobar uno a uno — nunca en batch
4. Solo tras aprobación, reemplazar función original en `ui_icons.cpp`

Para paletas: las entradas nuevas de SENSOR CARD (HUM, LUZ, SOUND, SUELO) y el ciclo de sensores de VALOR LAB usan la paleta propuesta. Si se ven bien en hardware → paleta aprobada.

### Documentación completa

Ver `PALETTE_AND_ICONS_PROPOSAL.md` en este mismo directorio para:
- Inventario completo de colores por pantalla
- Pseudocódigo de redesign de cada icono
- Propuesta de `include/palette.h`
- Lista de decisiones pendientes de producto
