# TFT Render Rules — P-Bit Firmware

**Protocolo anti-flicker para ST7735 160×128 px via SPI (TFT_eSPI)**

Fecha: 2026-05-20 | Versión consolidada post-Fase B/C/D

---

## Estado actual

Las reglas de este documento ya están aplicadas en las pantallas de mayor riesgo del firmware actual: dials/gauges, cards, `Sound VU`, `Graph`, `Valor`, `Home` y pantallas de sensor clásicas. La deuda activa no es repetir la implementación anti-flicker, sino **validar en hardware real** que el ST7735 no muestra scan-line, flash negro, ghost pixels ni recortes en textos localizados.

Cambios cerrados desde la auditoría original:
- `Sound VU`: sprite, EWMA asimétrico, scroll continuo e idle pulse.
- `DIAL/GAUGE`: ring sprite 64×64 y chrome/data split.
- `VALOR`: sparkline sprite y clears acotados.
- `SENSOR CARD`: chrome-last rule para evitar recorte por overhang de glyphs.
- `HOME` y cards lab: `draw_card_chrome` separado de `draw_card_value_and_tank`.
- `LDR`: las vistas usan rango de producto `0..20000 lux`; el ADC crudo `0..4095` queda solo para calibración/debug.

## Por qué importa

El ST7735 no tiene doble buffer. Cada `fillRect` o `drawString` sobre el TFT es una secuencia de comandos SPI síncronos que el panel pinta fila a fila mientras el CPU sigue ejecutando el loop. Si en el mismo frame borramos una zona y luego la redibujamos, el ojo humano ve el negro intermedio como "flash" o "flicker", especialmente en sensores que actualizan a >10 Hz (sonido, luz).

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

**Nota de aceptación:** "Nivel aplicado" significa que el patrón existe en código. La aceptación final para producción requiere probar esas pantallas en el P-Bit físico y firmar que no hay flicker perceptible.

## Validaciones hardware pendientes

- `LAB_SOUND_VU_STACK_SCREEN` y `LAB_SOUND_VU_WAVE_SCREEN`: scroll continuo, idle pulse visible en silencio, sin congelación periódica.
- `SZ_VIZ_GAUGE`: ring sprite sin vibración, label de unidad restaurado después del `pushSprite()`.
- `SZ_VIZ_CARD`: header, valor, visualización y footer sin recortes por clears dinámicos.
- `SZ_VIZ_VALOR`: sparkline y barra segmentada sin flash negro y con contraste suficiente.
- `SZ_VIZ_GRAPH`: etiquetas min/max y línea principal legibles en ES/CAT/EN.
- `LIGHT`: rango `0..20000 lux` y RGB apagado para no contaminar el LDR.

---

## Trampas frecuentes (checklist de code review)

- [ ] `fillRoundRect` de toda la card en update de valor → separar en chrome + data
- [ ] `fillRect(0, y, 160, h)` para limpiar valor → usar solo ancho del área del valor
- [ ] `fillRect` antes de `pushSprite()` → eliminar (el sprite limpia su propia memoria)
- [ ] `kBadgeClearW` < ancho del string más largo → añadir margen ≥14 px o medir con `textWidth()`
- [ ] Chrome dibujado antes de los clears de datos → redibujar chrome al final si hay solapamiento de bounding box
- [ ] Ring sprite pusheado sobre label de unidad → redibujar label de unidad post-push
- [ ] Función de ring original sin llamadas → `-Wunused-function` (limpiar o comentar)
