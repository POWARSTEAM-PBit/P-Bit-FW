# TFT Render Rules — P-Bit Firmware

**Protocolo anti-flicker para ST7735 160×128 px via SPI (TFT_eSPI)**

Fecha: 2026-06-03 | Versión consolidada post-Fase B/C/D + auditoría de reglas-fuente.

> Este documento es **la fuente única** de patterns de render del P-Bit. Cualquier otro skill, doc o memoria que describa patterns anti-flicker debe limitarse a apuntar aquí, no a duplicar las reglas — duplicación garantiza desincronización.

---

## Estado actual

Las reglas de este documento ya están aplicadas en las pantallas de mayor riesgo del firmware actual: dials/gauges, cards, `Sound VU`, `Graph`, `Valor`, `Home` y pantallas de sensor clásicas. La validación puntual en hardware deja ghosting/flicker cerrado por ahora; la deuda activa es mantener checks de regresión cuando se toquen pantallas, Demo Mode, modos LDR o reglas de limpieza.

Cambios cerrados desde la auditoría original:
- `Sound VU`: sprite, EWMA asimétrico, scroll continuo e idle pulse.
- `DIAL/GAUGE`: ring sprite 64×64 y chrome/data split.
- `VALOR`: sparkline sprite y clears acotados.
- `SENSOR CARD`: chrome-last rule para evitar recorte por overhang de glyphs.
- `HOME` y cards lab: `draw_card_chrome` separado de `draw_card_value_and_tank`.
- `LDR`: `Lux`, `FC` y `Raw ADC` usan helper común de presentación; Sensor Zone/cards/gráficas/dials deben seguir valor/unidad del modo activo.
- `Externos`: DS18B20/Termómetro y Suelo usan estado runtime común de ausencia; al desconectar se dibujan con paleta atenuada y textos `Revisa IO33` / `Revisa IO35`, sin persistencia en NVS.

## Por qué importa

El ST7735 no tiene doble buffer. Cada `fillRect` o `drawString` sobre el TFT es una secuencia de comandos SPI síncronos que el panel pinta fila a fila mientras el CPU sigue ejecutando el loop. Si en el mismo frame borramos una zona y luego la redibujamos, el ojo humano ve el negro intermedio como "flash" o "flicker", especialmente en sensores que actualizan a >10 Hz (sonido, luz).

---

## Nivel 0 — Plantilla canónica de cache (primera línea de defensa)

Cada pantalla con datos dinámicos declara un struct cache *en la traducción del archivo*, no en el header. Sin cache → no hay forma de detectar "nada cambió" → cada tick redibuja → flicker garantizado.

```cpp
// src/ui_my_sensor.cpp — al inicio del archivo, ANTES de la función pública
namespace {
struct MyScreenCache {
    bool     valid       = false;        // marcador "primer dibujo hecho"
    int      value_key   = INT_MIN;      // valor cuantizado: (int)roundf(v*10) para 1 decimal
    uint8_t  alert_code  = ALERT_CODE_OFF;
    bool     alerts_en   = true;         // último estado de alertas globales
    bool     unit_mode_f = false;        // último C/F; o último LDR mode (Lux/FC/Raw)
    bool     no_sensor   = false;        // último estado conectado/desconectado (externos)
    uint16_t accent      = 0;            // último color accent (cambia con sensor o estado)
};
MyScreenCache g_cache;
}  // anonymous namespace
```

### Reglas de la clave (`*_key`)

| Formato de display | Cómo cuantizar |
|---|---|
| `%.0f` (entero) | `(int)roundf(v)` — **nunca** `(int)v`, trunca |
| `%.1f` (1 decimal) | `(int)roundf(v * 10.0f)` |
| `%.2f` (2 decimales) | `(int)roundf(v * 100.0f)` |
| String semántico (`Óptimo`, `Seco`, ...) | enum id, no `strcmp` cada frame |
| String libre | `strncmp` con buffer cacheado |

### Cómo se usa la cache (template)

```cpp
void draw_my_screen(bool screen_changed, bool sensor_data_changed) {
    // 1. Snapshot y key
    const float val   = g_ui_readings_snapshot.my_value;
    const bool  valid = !isnan(val);
    const int   key   = valid ? (int)roundf(val * 10.0f) : INT_MIN;

    // 2. Detección de cambios
    const bool chrome_dirty = !g_cache.valid
                           || screen_changed
                           || (valid != !g_cache.no_sensor)
                           || (g_alerts_enabled != g_cache.alerts_en)
                           || (current_alert_code() != g_cache.alert_code)
                           || (g_is_fahrenheit != g_cache.unit_mode_f);
    const bool data_dirty   = chrome_dirty || (key != g_cache.value_key);

    if (!sensor_data_changed && !data_dirty) return;     // nada que hacer

    // 3. Chrome primero solo si toca
    if (chrome_dirty) draw_my_chrome();

    // 4. Data siempre que data_dirty
    if (data_dirty) {
        clear_value_zone();        // SOLO el rectángulo del valor
        draw_value(val, valid);
    }

    // 5. ÚLTIMO: redibujar elementos chrome que tocan zona de datos (icono, label adyacente)
    if (chrome_dirty) draw_chrome_overlap_last();

    // 6. Cache update — al final, una sola vez
    g_cache = { true, key, current_alert_code(), g_alerts_enabled,
                g_is_fahrenheit, !valid, current_accent() };
}
```

> **Regla — cuándo Cache vs cuándo meta_dirty**:
>
> - **Pantallas con ≥2 campos dinámicos independientes** (ej. Focus con valor + barra + sparkline; cards multi-sensor; gauge con ring + label central): OBLIGATORIO `struct *Cache` propio con todos los campos actualizados al final del redraw. Sin Cache, la pantalla está rota aunque "se vea bien" — el flicker aparece en hardware con cargas reales, no en simulador.
> - **Pantallas con 1 campo + jewel de alerta** (ej. `ui_temp.cpp`, `ui_humidity.cpp`, `ui_ds18.cpp`): aceptable `meta_dirty` (ver Nivel 1) sin struct, siempre que el clear esté acotado al ancho exacto del campo y el chrome se redibuje al final.
>
> En ambos casos vale la consigna anti-flicker: nunca repintar chrome cuando solo cambió el valor.

---

## Nivel 1 — Separación chrome / data (OBLIGATORIO en toda pantalla)

Toda pantalla divide su contenido en dos capas lógicas:

| Capa | Elementos | Cuándo se redibuja |
|---|---|---|
| **Chrome** (estática) | Borde, ícono, label sensor, jewel de alerta, título | Solo si `chrome_dirty` o `screen_changed` |
| **Data** (dinámica) | Valor numérico, barra/tank fill, sparkline, ring gauge | En cada `sensor_data_changed` |

### Definición canónica de chrome_dirty

```cpp
bool chrome_dirty = !cache.valid
                 || sensor_valid_changed      // sensor conectado/desconectado
                 || alert_code_changed        // cambio de nivel de alerta
                 || alerts_enabled_changed    // toggle alertas globales
                 || accent_changed;           // ej. F/C toggle, sensor switch
```

### meta_dirty (pantallas simples sin struct de cache)

```cpp
bool meta_dirty = screen_changed
               || alert_changed
               || no_sensor_changed
               || alerts_en_changed
               || unit_mode_changed;
```

### Plantilla de loop

```cpp
void draw_my_screen(bool screen_changed, bool sensor_data_changed) {
    const bool chrome_dirty = !g_cache.valid || detect_chrome_changes();
    const bool data_dirty   = chrome_dirty   || detect_data_changes();

    if (sensor_data_changed && data_dirty) {
        if (chrome_dirty) draw_my_chrome();   // borde, ícono, label
        draw_my_data();                        // valor, barra
        update_cache();
    }
}
```

---

## Nivel 2 — Clears localizados (OBLIGATORIO)

### Regla: nunca limpiar más de lo necesario

| Anti-patrón | Reemplazo correcto |
|---|---|
| `fillRect(0, y, 160, h, BG)` para limpiar valor | `fillRect(value_x, y, max_value_width, font_h + 2, BG)` |
| `fillRoundRect` de toda la card en cada update | Separar en `draw_chrome()` + `draw_data()` |
| `fillRect` antes de `pushSprite()` | El sprite hace su propio `fillSprite()` — el pre-clear es redundante y crea flash |

### Cálculo del ancho máximo del valor

Para strings de ancho variable (`"100%"` vs `"5%"`), limpiar siempre el ancho del caso más largo:

```cpp
// Opción A: medir en tiempo de compilación
constexpr int kMaxValueW = 60;   // px suficientes para "100.0"

// Opción B: medir el peor caso en runtime (una vez)
tft.setFreeFont(FONT_BODY);
int max_w = tft.textWidth("100%");   // cachear, no recalcular cada frame
```

---

## Nivel 2.5 — Labels de estado y cards lab

Los labels semánticos que cambian por estado (`Óptimo`, `Muy húmedo`, `Muy fuerte`, `Cálido`, etc.) no deben depender del clear del valor numérico ni del clear de la gráfica inferior. Cada label necesita su propio rectángulo de borrado, ajustado al carril donde vive el texto, y debe redibujarse solo cuando cambia el estado, la validez del sensor o el color/acento.

```cpp
const bool status_dirty = sensor_valid_changed
                       || status_id_changed
                       || accent_changed;

if (status_dirty) {
    clear_status_label_zone();   // no toca icono, jewel ni valor
    draw_status_label();
}
```

En pantallas `Lab` con cards compuestos, los bordes, títulos, iconos y shells de cards son chrome. Aunque el color del dato cambie rápidamente en Demo Mode, el shell no debe repintarse en cada tick; se actualizan solo los campos dinámicos:

- valor numérico y unidad si cambia unidad;
- barra, gauge, sparkline o VU;
- label de estado si cambia su clave;
- footer textual si cambia su estado.

Evitar `fillRoundRect()` de la card completa para actualizar un número. Si el dato dinámico necesita un fondo, usar un clear rect pequeño dentro del interior del card, dejando intacto el borde.

### Estado externo desconectado

Para `SZ_DS18` y `SZ_SOIL`, la ausencia del sensor es un cambio de validez y por tanto ensucia chrome y datos. Usar `external_sensor_state.*` como fuente común:

- DS18B20 ausente: `temp_ds18b20 < -100`, texto `ST_CHECK_DS18` (`IO33`).
- Suelo ausente: `isnan(soil_humidity)`, texto `ST_CHECK_SOIL` (`IO35`).
- Color: mantener identidad del sensor atenuada (`pbit_external_dim_*`), evitando gris plano o rojo dominante.
- Layout estrecho: preferir dos líneas `Sin sensor` + `Revisa IOxx`; si no cabe, mostrar `IO33`/`IO35` compacto antes que solapar valor, icono o gráfica.
- Conexión/desconexión: `sensor_connection_notice.*` usa splash full-screen semafórico de `1500 ms`, no overlay parcial, para evitar solapes con pantallas densas y dar tiempo real de lectura. Fondo verde para `CONECTADO`, fondo rojo para `DESCONECTADO`, tres líneas centradas (`estado`, `Sensor ...`, `IOxx`). Se dispara solo ante transiciones posteriores al baseline inicial; Demo Mode y overlays críticos tienen prioridad.

---

## Nivel 2.6 — Menús con encoder también son pantallas dinámicas

Un menú no es "estático" si el encoder puede cambiar selección o valor varias veces por segundo. Las reglas anti-flicker aplican igual que en sensores:

| Elemento | Capa | Cuándo se redibuja |
|---|---|---|
| Fondo, header, línea, footer/hint | Shell | Solo al entrar al estado o por `screen_changed` |
| Grid 2×3 de settings | Chrome interactivo | Al entrar se dibujan todos los tiles; al girar normal solo tile anterior + tile nuevo; si el encoder salta >1 posición, todos los tiles se redibujan individualmente sin `clearMenuBands()` |
| Card de valor editable | Shell + data | `drawCenteredMenuValueScreen(..., state_changed)`: título/footer/card al entrar; al girar solo interior del valor y borde/acento si cambia |
| Confirmación de Reset | Shell danger + botones | Fondo rojo/panel/texto al entrar; al girar solo botones `NO`/`SI` |

Reglas estrictas:

- `fillScreen()` queda prohibido dentro de un cambio de índice/valor causado por encoder. Solo se permite al entrar/cambiar de estado de menú, cambio real de pantalla, cambio de idioma completo o `force_full`.
- `drawHeader()` y `drawFooterHint()` son shell; no se redibujan en cada tick si el texto no cambia.
- `clearMenuBands(kMenuBand_All)` y `clearMenuBands(kMenuBand_Title | kMenuBand_Body)` son aceptables para cambio de estado, no para navegación interna.
- Todo menú con encoder mantiene cache de lo visible: `last_drawn_state`, `last_menu_index`, `last_edit_value`, `last_toggle_value`, `last_reset_choice`, `last_saved_kind` o equivalente.
- El `last_menu_index` de grids debe guardar el índice local que se acaba de dibujar, no releer el global mutable después de llamar al helper. Si el encoder cambia durante el frame, releer el global puede dejar seleccionado un tile fantasma.
- Si una función helper común no ofrece modo incremental, no usarla en hot path de encoder; separar `draw_*Shell()` de `update_*Data()` / `update_*Buttons()`.

### Contrato encoder-grid para `drawSettingsGridMenu()`

Cuando un menú raíz usa `drawSettingsGridMenu(primary_items, primary_count, ...)`, los índices son contrato funcional, no solo layout:

```text
0..primary_count-1  -> opciones primarias
primary_count       -> Reset
primary_count + 1   -> Salir
```

Reglas obligatorias:

- `get_*_encoder_max()` del estado menú devuelve `primary_count + 1`.
- `handle_*_button()` mapea `Reset` en `primary_count` y `Salir` en `primary_count + 1`.
- `set_*_input_value()` limita el menú con `get_*_encoder_min()/max()`, no con números mágicos (`0..4`, `0..5`, etc.).
- Definir constantes locales (`*_PRIMARY_COUNT`, `*_RESET_INDEX`, `*_EXIT_INDEX`) para que render, encoder y acción compartan una sola fuente de verdad.
- Si el menú raíz se navega con encoder, el estado debe ser circular en `rotary.cpp`, salvo decisión explícita documentada.

Antes de cerrar cualquier cambio que añada, quite o reordene opciones de menú, comprobar: visual seleccionado == índice encoder == acción del botón.

### Plantilla para confirmaciones Reset

```cpp
if (state_changed) {
    drawResetChoicePromptShell(title, line1, line2, footer);
}
if (state_changed || last_reset_choice != choice) {
    updateResetChoiceButtons(L(MENU_NO), L(MENU_YES), choice);
    last_reset_choice = choice;
}
```

Esto mantiene estable el fondo `danger` y elimina el flash rojo/negro durante alternancia `NO`/`SI`.

---

## Nivel 3 — Orden de capas: chrome se redibuja ÚLTIMO

### El problema: font glyph overhang

Los glyphs de `FONT_SMALL` dibujados con `TL_DATUM` en y=33 se extienden visualmente hasta y≈45 (ascent + descent del bitmap). Si el clear del área de datos empieza en y=43 (inmediatamente debajo del anchor), recorta los últimos 2-4 px del glyph.

**Ejemplo en `ui_lab_sensor_cards.cpp`:**
- Device label dibujado en `kDevLabelY=33` → glyph llega hasta y≈45
- Ícono dibujado en `kIconCy=40` (s=1) → bitmap llega hasta y≈47
- `kValueTopY=43` → el `fillRect` de datos arranca y recorta ambos

### Solución: redibujar chrome DESPUÉS de todos los clears de datos

```cpp
static void draw_card_content(const LabSensorCardSpec& spec, ...) {
    // 1. Primero todos los clears de datos
    tft.fillRect(kCardX + 1, kValueTopY, kCardW - 2, kVizLabelY - kValueTopY, kCardBg);
    tft.fillRect(kCardX + 1, kVizLabelY, kCardW - 2, kVizBottomY - kVizLabelY + 4, kCardBg);

    // 2. Dibujar elementos de datos
    draw_value_compact(...);
    spec.draw_viz(...);

    // 3. ÚLTIMO: restaurar chrome que pisa la zona de datos
    draw_header_strip(spec.device_label, spec.secondary);   // label en y=33
    spec.icon_fn(kIconCx, kIconCy, state.accent);           // ícono en y=40
}
```

**Regla general**: cualquier elemento chrome cuyo bounding box se solape con la zona de datos adyacente debe redibujarse AL FINAL, después de todos los `fillRect` de limpieza.

### Caso especial: `sz_set_active` evita double-header en SENSOR_ZONE_SCREEN

`SENSOR_ZONE_SCREEN` es un *contenedor* que delega a un sub-renderer (`ui_lab_focus`, `ui_lab_widget_showcase`, `ui_graph`, `ui_lab_sensor_cards`). El contenedor ya dibuja el header — el sub-renderer no debe dibujarlo otra vez (causa flicker en y=0..19 y double-print que se ve un frame).

**Patrón canónico** (`src/sensor_zone.cpp:188`):

```cpp
// En tft_display.cpp:906 — ANTES de entrar al sub-renderer
sz_set_active(true);
sz_render_active_sensor(screen_changed, data_changed);
sz_set_active(false);   // línea 928

// En cada sub-renderer (ui_lab_focus.cpp:631, ui_graph.cpp:405, etc.):
if (!sz_is_active()) drawHeader(L(TIT_LAB_FOCUS));
```

> **Regla**: si añades un nuevo sub-renderer para `SENSOR_ZONE_SCREEN`, su primera línea de `drawHeader(...)` **debe** estar guardada por `if (!sz_is_active())`. Si haces `drawHeader` incondicional, vas a ver parpadeo en la franja superior cada vez que cambia un valor.

---

## Nivel 4 — Sprites para zonas con >20 SPI calls/frame

### Política

Si una zona de >40×40 px requiere más de 20 SPI calls por frame → usar `TFT_eSprite`.

El sprite renderiza en RAM y se transfiere al panel como un único burst DMA, eliminando el scan-line visible.

### Zonas identificadas con sprite

| Zona | Sprite | Tamaño | Heap |
|---|---|---|---|
| VU stack + wave (sonido) | `g_vu_spr` | variable | ~10 KB |
| Gauge ring (54 seg × 6 líneas) | `g_ring_spr` | 64×64 | 8 KB |
| Sparkline + grid (≈70 calls) | `g_sparkline_spr` | 56×31 | 3.5 KB |

### Patrón de sprite lazy-init

```cpp
static TFT_eSprite g_ring_spr(&tft);
static bool g_ring_spr_ready = false;

void draw_gauge_ring_sprite(...) {
    if (!g_ring_spr_ready) {
        g_ring_spr.setColorDepth(16);
        g_ring_spr.createSprite(kRingSprSize, kRingSprSize);
        g_ring_spr_ready = true;
    }
    g_ring_spr.fillSprite(kBg);
    // ... draw into sprite ...
    g_ring_spr.pushSprite(cx - kRingSprSize / 2, cy - kRingSprSize / 2);
}
```

### Trampa: sprite que pisa chrome adyacente

El ring sprite (64×64, top en y=44) pisa el label de unidad (dibujado en y=34..47).
Solución: redibujar el label de unidad **después** del `pushSprite()`:

```cpp
g_ring_spr.pushSprite(kGaugeCx - 32, kGaugeCy - 32);
// Restaurar unit label que quedó pisado por el sprite
tft.fillRect(148 - 40, 32, 44, 18, kBg);
tft.drawString(unit_str, 148, 34);
```

---

## Nivel 5 — Demo Mode: cadencia y aislamiento

Demo Mode usa un refresco propio de `220 ms` (ver `demo_mode_value_refresh_ms()` en `src/demo_mode.cpp`). Toda pantalla que reciba `sensor_data_changed=true` durante demo debe responder en ≤ ese tick sin redibujar chrome.

### Reglas obligatorias durante Demo Mode

1. **No cambiar la cadencia base** sin recalibrar todas las cards: `220 ms` está elegido para que las curvas suaves se vean fluidas sin disparar flicker — duplicarlo o reducirlo rompe ese equilibrio.
2. **Demo solo escribe `g_ui_readings_snapshot`**, nunca `global_readings`, NVS ni BLE. Cualquier pantalla que mire otra fuente que `g_ui_readings_snapshot` se queda congelada en demo.
3. **Setters runtime de Sensor Zone** (`sz_set_sensor_runtime`, `sz_set_viz_runtime`) cambian sensor/modo solo en RAM y piden full redraw. Si una pantalla cachea sensor/modo, debe comparar contra estos setters y invalidar cache.
4. **`sensor_connection_notice.*` no dispara durante demo** — la rama de baseline filtra eventos durante `demo_mode_is_active()`.
5. **Si una pantalla nueva no tiene path de Demo Mode**, registra una escena en `src/demo_mode.cpp` con `dwell` apropiado (rango actual `6..10 s`) o documenta por qué se omite.

> **Regla de regresión**: tras cualquier cambio en render dinámico, entrar a Demo Mode (encoder presionado en boot, o long-press en `LAB_HOME_CARDS`) y observar 30 segundos. Si el flicker aparece solo en demo, casi siempre es un clear acotado a 1 cifra que no soporta dos cifras, o un sprite que se inicializa cada frame.

---

## Tabla de estado — todas las pantallas (mayo 2026)

| Archivo | Nivel aplicado | Notas |
|---|---|---|
| `ui_sound.cpp` | L1 chrome/value | fillRect font-height, jewel cache |
| `ui_light.cpp` | L1 chrome/value | idem |
| `ui_temp.cpp` | L1 meta_dirty | info_card + jewel cache |
| `ui_humidity.cpp` | L1 meta_dirty | card_title + jewel cache |
| `ui_ds18.cpp` | L1 meta_dirty | jewel cache |
| `ui_soil.cpp` | L1 limpio | value/category/alert_changed |
| `ui_lab_sound_vu.cpp` | L1+L4 | sprite stack/wave; `kBadgeClearW=92` |
| `ui_lab_focus.cpp` | L1+L2 | fillRect eliminado antes de sprite push |
| `ui_graph.cpp` | L1+L2 | `draw_graph_band(full_clear)` param |
| `ui_lab_home_cards.cpp` | L1+L2 | `draw_card_chrome` + `draw_card_value_and_tank` |
| `ui_lab_sensor_cards.cpp` | L1+L2+L3 | `chrome_dirty()` + chrome-last rule |
| `ui_lab_widget_showcase.cpp` | L1+L2+L3+L4 | DIAL: ring sprite; Valor: sparkline sprite + `chrome_drawn` |
| `ui_lab_dash.cpp` | L1 | per-row dirty flags |
| `ui_lab_dual.cpp` | L1 | shell/content separation |
| `ui_lab_linear_dash.cpp` | L1 | RowCache por fila |

**Nota de aceptación:** "Nivel aplicado" significa que el patrón existe en código. Ghosting/flicker está cerrado por ahora; repetir checks de regresión si se cambia render, Demo Mode o cadencia de datos.

## Vigilancia de regresión y pendientes visuales

- `LAB_SOUND_VU_STACK_SCREEN` y `LAB_SOUND_VU_WAVE_SCREEN`: vigilar que scroll continuo e idle pulse no recuperen congelación periódica.
- `SZ_VIZ_GAUGE`: vigilar ring sprite sin vibración y label de unidad restaurado después del `pushSprite()`.
- `SZ_VIZ_CARD`: vigilar header, valor, visualización y footer sin recortes por clears dinámicos.
- `SZ_VIZ_VALOR`: vigilar sparkline y barra segmentada sin flash negro y con contraste suficiente.
- `SZ_VIZ_GRAPH`: validar etiquetas min/max y línea principal legibles en ES/CAT/EN.
- `LIGHT`: verificar regresión de coherencia visible `Lux / FC / Raw ADC`; mantener RGB apagado en cualquier vista de solo Luz para no contaminar el LDR.
- `DEMO`: validar que el refresco smooth de 220 ms y las gráficas sintéticas no reintroducen flicker por frecuencia excesiva.

---

## Verificación pre-claim — proof obligatorio antes de decir "listo"

Esta sección existe porque siempre caemos en los mismos errores TFT pese a tener reglas: las reglas describen QUÉ hacer pero no obligan a PROBAR que se hizo. Ahora sí.

### Checklist de proof (cada item debe pasar antes de cerrar una tarea de render)

```
□ proof 1 — La función pública tiene firma (bool screen_changed, bool sensor_data_changed)
            y EARLY-RETURN cuando ambos son false y no hay data_dirty.

□ proof 2 — Existe un struct *Cache propio en el archivo .cpp (no header).
            Todos sus campos se actualizan al final de cada redraw exitoso.

□ proof 3 — `fillScreen(...)` aparece en el archivo **0 veces**, o solo dentro de
            `if (screen_changed)`. Verificable con:
              rg -n 'fillScreen' src/ui_my_screen.cpp

□ proof 4 — Cada `fillRect(...)` dinámico tiene ancho ≤ ancho del campo (no full-screen).
            Cada `fillRoundRect(...)` está dentro de `if (chrome_dirty)` o equivalente.

□ proof 5 — El ícono y el header/label adyacentes se redibujan AL FINAL
            si su bounding box solapa con la zona de datos (Nivel 3).

□ proof 6 — Si la pantalla vive dentro de SENSOR_ZONE_SCREEN, su `drawHeader(...)`
            está guardado por `if (!sz_is_active())`.

□ proof 7 — Si se añade sprite: lazy-init con flag `*_ready`, fillSprite UNA vez por
            frame, pushSprite UNA vez, y restore de labels pisados después del push.

□ proof 8 — Compilación local pasa: `py -m platformio run -e esp32dev` con SUCCESS.
            Reportar RAM/Flash y diff vs baseline si cambian > ±200 bytes.

□ proof 9 — Demo Mode activado y observado ≥30 s: ningún campo dinámico parpadea,
            ningún chrome se redibuja en cada tick, transiciones entre escenas limpias.

□ proof 10 — Hardware real validado (no solo simulador) para cambios en:
             clears, sprites, fuentes, idiomas largos (CAT/EN), pantallas LAB densas.
```

### Regla de cierre

Si al menos uno de los proofs no se ejecutó, **decir "queda pendiente proof X"** en el reporte final. Nunca cerrar tarea diciendo "listo, compila" si no se corrió Demo Mode o no se vio en hardware. El compilador no detecta flicker.

### Por qué este nivel es nuevo (lección 2026-06)

La auditoría identificó que las reglas Nivel 1-5 describen patterns correctamente pero no exigen evidencia. Resultado: tareas marcadas como "completadas" con bugs visibles en hardware. Esta sección cambia el contrato: sin proofs, no hay close.

---

## Trampas frecuentes (checklist de code review)

- [ ] `fillRoundRect` de toda la card en update de valor → separar en chrome + data
- [ ] `fillRect(0, y, 160, h)` para limpiar valor → usar solo ancho del área del valor
- [ ] `fillRect` antes de `pushSprite()` → eliminar (el sprite limpia su propia memoria)
- [ ] `kBadgeClearW` < ancho del string más largo → añadir margen ≥14 px o medir con `textWidth()`
- [ ] Chrome dibujado antes de los clears de datos → redibujar chrome al final si hay solapamiento de bounding box
- [ ] Ring sprite pusheado sobre label de unidad → redibujar label de unidad post-push
- [ ] Función de ring original sin llamadas → `-Wunused-function` (limpiar o comentar)

---

## Banderas rojas grep-ables (revisión por terminal)

Estas son las queries que cualquier code review de render debe correr antes de aprobar un cambio. Cada match positivo es "explica o arregla", no un fail automático — pero deja la decisión consciente.

> **Nota PowerShell/cmd:** `src/ui_*.cpp` NO se expande como glob en PowerShell ni en cmd. Usar siempre la forma `src -g 'ui_*.cpp'` para que `ripgrep` (`rg`) resuelva el patrón internamente. En bash/zsh ambas funcionan.

### `fillScreen` fuera de `screen_changed`

```powershell
rg -n 'tft\.fillScreen\(' src -g 'ui_*.cpp' -g 'sensor_zone.cpp' -g 'tft_display.cpp'
```

Cada hit debe estar dentro de `if (screen_changed)`, `if (force_full_redraw)` o función de boot/transición explícita. Cualquier otro contexto es candidato a flicker.

### `fillRoundRect` en función dinámica

```powershell
rg -n 'fillRoundRect' src -g 'ui_*.cpp'
```

Si el match está en una función llamada en cada `sensor_data_changed=true`, está rompiendo Nivel 2 — separar en `draw_*_chrome` / `draw_*_data`.

### `fillRect` con ancho de pantalla en función dinámica

```powershell
rg -n 'fillRect\(\s*0\s*,' src -g 'ui_*.cpp'
rg -n 'fillRect\([^,]+,\s*[^,]+,\s*(tft\.width\(\)|160|TFT_WIDTH)' src -g 'ui_*.cpp'
```

Clear full-width borra elementos vecinos. Si no es bloque `screen_changed`, acota al ancho del campo.

### `TFT_DARKGREY` para estado "sin sensor" (regla obsoleta)

```powershell
rg -n 'TFT_DARKGREY' src -g 'ui_*.cpp'
```

La regla actual es `pbit_external_dim_*` (identidad atenuada del sensor), no gris plano. Cada hit debería migrarse a la nueva paleta.

### `drawHeader` sin guard `sz_is_active`

```powershell
rg -n -B1 -A1 'drawHeader\(' src -g 'ui_lab_*.cpp' -g 'ui_graph.cpp'
```

Cada `drawHeader(...)` en un sub-renderer de `SENSOR_ZONE_SCREEN` debe ir precedido por `if (!sz_is_active())`.

### Cache no declarada o no actualizada

```powershell
rg -n 'struct.*Cache' src -g 'ui_*.cpp'
```

Cada pantalla con datos dinámicos debe tener un struct cache **o** un bloque `meta_dirty` propio (ver Nivel 0). Si no aparece ninguno de los dos, falta Nivel 0.

### Sprite sin lazy-init flag

```powershell
rg -n 'createSprite\(' src -g 'ui_*.cpp'
```

Cada `createSprite` debe estar guardada por un flag `g_*_ready` o equivalente. Crear sprite cada frame fragmenta heap y causa stutter.

### Funciones unused (candidatas a eliminar)

```powershell
# compilar con flag de warnings y filtrar
py -m platformio run -e esp32dev 2>&1 | rg 'Wunused-function|Wunused-variable'
```

Cualquier `[-Wunused-function]` en `ui_*.cpp` o `sensor_zone.cpp` es código muerto candidato a borrar en cleanup.

> **Cadencia recomendada**: correr este bloque de greps al cierre de cada PR que toque `src/ui_*` o `src/tft_display.cpp`. Anotar en el reporte cuántos hits aparecieron y cuáles se decidió tolerar.
