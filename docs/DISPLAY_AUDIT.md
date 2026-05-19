# P-Bit Firmware — Display & Sensor Pipeline Audit

**Fecha auditoría**: 2026-05-18 | **Actualizado**: 2026-05-19
**Auditado por**: Opus 4.7 (deep review) + Sonnet 4.6 (implementación Fase B/C)
**Versión firmware**: HEAD `f6414be` + cambios no-commit (sleep animation, sound sprites, anti-flicker Fase B/C)
**Hardware target**: ESP32 + ST7735 160×128 px, sensores LDR / GM19767P / DHT11 / DS18B20 / suelo capacitivo

---

## TL;DR

El firmware tiene **tres bugs sistémicos** que degradan la experiencia visual, y **un patrón de flicker** repetido en 6+ pantallas. El más visible para el usuario (lag del VU de sonido) NO es bug de la pantalla — es que el `sensor_reading_task` bloquea **94 ms cada segundo** en la conversión síncrona del DS18B20, durante los cuales no se publica nueva data al display.

**Fix más rentable (P0)**: convertir DS18B20 a modo async (`setWaitForConversion(false)`). Estimado: 1 h de trabajo, elimina el ~9 % de downtime visible cada segundo.

---

## 1. Pipeline de datos — estado actual

```
┌─────────────────────────────────────────────────────────────────┐
│ Core 0 — SensorTask (prio 1, 10 ms delay, ~30 ms loop real)     │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ each iteration:                                         │    │
│  │   read_fast_sensors()  ──► LDR (~0.1ms)                 │    │
│  │                            read_sound_level (20 ms)     │    │
│  │                            read_soil_moisture (~2.4 ms) │    │
│  │   [every 1000 ms] read_slow_sensors():                  │    │
│  │                            DHT humidity (~25 ms)        │    │
│  │                            DHT temperature (~25 ms)     │    │
│  │                            DS18B20  (~94 ms BLOCKING)   │    │
│  │   portENTER_CRITICAL → global_readings = local_r        │    │
│  │   alert_engine_refresh(...)                             │    │
│  │   runtime_mark_sensor_data_ready()  ◄── ★ aquí se       │    │
│  │   if (ble_client) notifyAll() (5-15 ms)     publica     │    │
│  │   vTaskDelay(10 ms)                                     │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼  global_readings (spinlock)
┌─────────────────────────────────────────────────────────────────┐
│ Core 1 — SwitchScreen (prio 1, 5 ms delay = 200 Hz wake rate)   │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ each tick:                                              │    │
│  │   sensor_data_changed = runtime_take_..._ready()        │    │
│  │   if (sensor_data_changed || screen_changed):           │    │
│  │     portENTER_CRITICAL                                  │    │
│  │     g_ui_readings_snapshot = global_readings            │    │
│  │     portEXIT_CRITICAL                                   │    │
│  │     draw_<active_screen>(screen_changed,                │    │
│  │                          sensor_data_changed)           │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

**Frame budget efectivo del display**:
- Fast-only ciclo: ~30 ms entre `mark_sensor_data_ready` → tope teórico **~33 Hz**.
- Slow ciclo (1 vez/s): ~145 ms sin publicar data → **el display queda quieto 14 % del segundo**.

---

## 2. Hallazgos críticos (P0 — bloquean la experiencia)

### P0-1 — DS18B20 bloquea el publish loop 94 ms cada segundo

**Archivo**: `src/hw.cpp:680` (`read_ds18b20_temp`)
**Síntoma**: el VU de sonido (y cualquier animación de >5 Hz) se congela ~94 ms cada segundo. Visible como stutter periódico.
**Causa**: `sensors.requestTemperatures()` es síncrono y bloquea el thread hasta que el DS18B20 termina la conversión (93.75 ms @ 9-bit).
**Fix**:
```cpp
// En init_hw():
sensors.setWaitForConversion(false);

// Patrón two-step async (en read_slow_sensors):
//   1. issue request: sensors.requestTemperatures()  ← retorna inmediato
//   2. en el siguiente ciclo slow (≥100 ms después): sensors.getTempCByIndex(0)
```
Esto reduce el blocking del DS18B20 de **94 ms → ~1 ms** (solo el comando 1-Wire). El valor disponible llega 1 segundo después (en el próximo ciclo slow), lo cual es invisible para el usuario porque la temperatura no cambia visualmente entre samples a 1 Hz.

### P0-2 — DHT11 bloquea ~25 ms × 2 cada segundo

**Archivo**: `src/io.cpp:144-145` (`read_slow_sensors`)
**Síntoma**: ~50 ms adicionales de freeze cada segundo (encima del DS18B20).
**Causa**: La librería `DHT` usa bit-banging sin yields, ~22-25 ms por read. Se hacen DOS reads consecutivos (humidity + temperature).
**Fix opcional** (después de P0-1):
- Alternar reads: leer humidity en un ciclo slow, temperature en el siguiente (cada uno cada 2 s en lugar de cada 1 s). DHT11 cachea de todos modos, no se gana resolución leyendo cada 1 s.
- O mover lectura DHT a su propio task de baja prioridad en core 0.

### P0-3 — `read_sound_level()` impone 20 ms blocking en cada fast cycle

**Archivo**: `src/hw.cpp:585-616`
**Síntoma**: tope dura del fast-sensor rate ≈ 33 Hz (cap teórico para VU smoothness).
**Causa**: para medir peak-to-peak hay que samplear suficientes ciclos del audio. 20 ms cubre 1 ciclo de 50 Hz (zumbido eléctrico mínimo), 4 ciclos de voz (200 Hz).
**Veredicto**: **NO tocar el window**. 33 Hz de update es suficiente para VU smoothness. Lo que duele es que cuando este sample cae dentro del slow cycle (que ya está bloqueando 145 ms), la latencia percibida es mucho mayor.
**Mejora opcional**: añadir EWMA decay en la UI (push_history con interpolación) para que cuando un sample llega tarde, el meter no salte sino que "alcance" suavemente. Mejor todavía: hacer la animación del meter independiente del rate de sampling (animar el scroll a 30 FPS aunque no haya sample nuevo).

---

## 3. Hallazgos altos (P1 — flicker y waste sistemáticos)

### P1-1 — Patrón "full-screen fillRect antes de redraw" en 6+ pantallas

**Pantallas afectadas confirmadas**:
- `ui_sound.cpp:421` — `fillRect(0, LB_VALUE_TOP-4, 160, 52, BG)` antes del número
- `ui_lab_sound_vu.cpp` (ya parchado) — `fillRect(0, L_CONTENT_TOP, 160, 108, BG)` antes de la card
- Probablemente `ui_temp.cpp`, `ui_light.cpp`, `ui_ds18.cpp`, `ui_humidity.cpp` con el mismo patrón (ver `clearMenuBands` también)

**Síntoma**: flash negro visible cada vez que el valor cambia. Especialmente molesto si el redraw es a 33 Hz (sound).
**Fix sistemático**: aplicar el patrón ya validado en `ui_lab_sound_vu.cpp`:
- Separar `draw_<screen>_chrome()` (estático, solo en `screen_changed` o `meta_dirty`) de `draw_<screen>_value()` (dinámico, con clear localizado de su propia área).
- El clear del valor debe ser **del tamaño exacto del texto previo + margen**, no full-width.
- Para variables de ancho variable (`"100%"` vs `"5%"`) calcular `tft.textWidth()` del peor caso y limpiar siempre ese ancho con BG.

### P1-2 — Strings dinámicos sin clear localizado

**Patrón problemático**:
```cpp
snprintf(buf, n, "%.0f", value);
tft.setTextColor(color, BG);   // bg-erase solo cubre el bounding box exacto
tft.drawString(buf, x, y);     // ⚠ valor anterior más largo deja ghost characters
```
**Síntoma**: cuando "100%" → "5%", quedan los píxeles de "10" colgando.
**Fix**: siempre precede con `tft.fillRect(x_left, y, max_width, font_h, BG)` calculado al tamaño del valor más largo posible. Ver el fix aplicado en `draw_value_badge` (`ui_lab_sound_vu.cpp:113`).

### P1-3 — Sprite no usado donde reduciría 10–100× el SPI traffic

**Candidatos identificados**:
- `ui_graph.cpp` — la sparkline ya usa sprite (good), pero el `drawRoundRect` del borde se redibuja después del sprite cada frame.
- `ui_lab_gauge_temp.cpp` — el arco (P4→P3 con `drawArc` o `fillCircle` segments) son típicamente 50+ SPI calls. Candidato fuerte a sprite.
- `ui_lab_focus.cpp` y `ui_lab_value_modern.cpp` — si tienen meter/bar dinámico, vale la pena ver.
- `ui_lab_sensor_cards.cpp` (3 cards con meters) — alto riesgo de flicker.

**Recomendación**: política "si más de 20 SPI calls por frame en una zona de >50×30 px → sprite".

### P1-4 — Jewel de alerta se redibuja sin verificar cambio de estado

**Archivos**: `ui_temp.cpp:521`, `ui_ds18.cpp:511` (probablemente otros).
**Síntoma**: drawAlertJewel pinta antialiased circles cada frame → SPI waste y posible micro-flicker.
**Fix**: añadir a la cache un campo `last_alert_state` y skip si no cambió.

---

## 4. Hallazgos medios (P2 — pulido)

### P2-1 — Tasks con BLE y sensor en mismo core

`notifyAll()` se ejecuta inline en el sensor task con cliente conectado. Asignación de packet + 2 notifies → 5-15 ms adicionales al ciclo. Mover a un timer task que tome snapshot cada 500 ms.

### P2-2 — Sobre-muestreo de sensores lentos

| Sensor   | Rate actual | Rate necesario |
|----------|-------------|----------------|
| Sound    | 33 Hz       | 20–30 Hz ✓     |
| LDR      | 33 Hz       | 2–5 Hz         |
| Soil     | 33 Hz       | 0.2–1 Hz       |
| DHT      | 1 Hz        | 0.5 Hz         |
| DS18B20  | 1 Hz        | 0.5 Hz         |

Reducir LDR a cada 5 fast cycles, soil a cada 30, DHT/DS a cada 2 s libera CPU y BLE bandwidth sin pérdida visual.

### P2-3 — VU sin animación cuando silencio

Cuando `level=0` constante, todas las columnas del stack meter están vacías. Aunque el código está pusheando samples, no hay nada visible que se mueva → el usuario percibe "congelado".
**Fix UX**: cuando `level < 5`, mostrar siempre **1 segmento verde tenue** en la base de cada columna (idle pulse). Da feedback de "estoy vivo y escuchando".

### P2-4 — `clearMenuBands()` redibuja 3 bandas siempre

Pasar bitmask para limpiar solo la banda afectada. Menor flicker en menús.

### P2-5 — Sleep overlay redibuja el orb cada frame (5 ms ciclo)

El orb breathing se calcula en cada loop del display task, pero el sprite del orb se redibuja solo si `r != last_r`. Esto es OK, pero los `fillCircle` directos sobre el TFT podrían ir a un sprite si quisiéramos animación más fluida o cambio de color.

---

## 5. Plan de remediación por fases

### FASE A — Sensor pipeline ✅ YA ESTABA IMPLEMENTADA (pre-sesión)

Verificado en código (`hw.cpp` + `io.cpp`):
- ✅ **P0-1**: DS18B20 async — `setWaitForConversion(false)` en `init_hw()` + patrón two-step en `read_ds18b20_temp()`. Blocking: 94 ms → ~7 ms.
- ✅ **P0-2**: DHT alterna reads — `dht_slot` en `read_slow_sensors()`. Cada canal se lee cada 2 s en lugar de juntos cada 1 s.
- ✅ **Bonus P2-1**: `notifyAll()` ya está guardado por `client_connected.load()` — no se ejecuta si no hay cliente BLE conectado.

### FASE B — Patrón anti-flicker sistemático ✅ COMPLETADA (2026-05-19)

Implementado chrome/value separation en todas las pantallas afectadas:

| Pantalla | Fix aplicado |
|---|---|
| `ui_sound.cpp`, `ui_light.cpp` | chrome/value con fillRect font-height |
| `ui_temp.cpp`, `ui_humidity.cpp`, `ui_ds18.cpp` | meta_dirty + jewel cache |
| `ui_soil.cpp` | value_changed / category_changed / alert_changed |
| `ui_lab_sound_vu.cpp` | `kBadgeClearW=92` (ghost digit fix) |
| `ui_lab_focus.cpp` | fillRect eliminado antes de sprite push |
| `ui_graph.cpp` | `draw_graph_band(full_clear)` — right-half clear en updates de valor |
| `ui_lab_home_cards.cpp` | `draw_card_chrome` + `draw_card_value_and_tank` |
| `ui_lab_sensor_cards.cpp` | `chrome_dirty()` + chrome-last rule (clipping fix) |

**Regla nueva descubierta en Fase B**: elementos chrome cuyo bounding box (glyph overhang) se solapa con la zona de datos adyacente deben redibujarse AL FINAL de `draw_data()`. Ver `docs/TFT_RENDER_RULES.md` § Nivel 3.

### FASE C — Sprites + sobre-muestreo ✅ COMPLETADA (2026-05-19)

- ✅ Ring sprite 64×64 para gauge DIAL en `ui_lab_widget_showcase.cpp` (54 seg × 6 líneas → 1 DMA burst)
- ✅ Sparkline sprite para Valor Lab en `ui_lab_widget_showcase.cpp` (≈70 SPI calls → 1 DMA burst)
- ✅ `draw_lab_gauge_chrome/data` split + `chrome_drawn` flag para Valor Lab
- ✅ LDR submuestreado a ~5 Hz (cada 6 ciclos fast ≈ 180 ms) — `ldr_cycle` en `read_fast_sensors()`
- ✅ Suelo submuestreado a ~1 Hz (cada 30 ciclos fast ≈ 900 ms) — `soil_cycle` en `read_fast_sensors()`
- ✅ `draw_gauge_ring()` eliminado de `ui_lab_widget_showcase.cpp` (era función sin llamadas)

**Ahorro estimado por el submuestreo** (fast loop a 30 Hz):
| Sensor | Antes | Después | CPU liberado |
|---|---|---|---|
| LDR `analogRead` + cálculo | 30 Hz | ~5.5 Hz | ~83% de las llamadas |
| Suelo 12× `analogRead` (~2.4 ms) | 30 Hz | ~1.1 Hz | ~70 ms/s de blocking |

### FASE D — UX polish ✅ COMPLETADA (2026-05-19)

- ✅ **Idle pulse VU** — cuando silencio total, cada columna del stack meter muestra 1 segmento verde tenue pulsante (~1 Hz, onda triangular 32 pasos). Feedback visual de "estoy escuchando".
- ✅ **EWMA asimétrico VU** — subida instantánea, caída suave (~250 ms stack / ~200 ms wave). Comportamiento "balístico" clásico de VU meter. El badge numérico sigue mostrando el valor real del sensor, no el suavizado.
- ✅ **`clearMenuBands` bitmask** — nueva firma `clearMenuBands(uint8_t bands = kMenuBand_All)`. `drawCenteredMenuList` ahora solo limpia `kMenuBand_Title | kMenuBand_Body`, dejando el footer intacto en cada tick del encoder.

---

## 6. Paralelización propuesta para la ejecución

| Tarea  | Modelo recomendado | Notas |
|--------|--------------------|-------|
| FASE A | **Opus** (yo)      | Cambios delicados a librería externa, riesgo alto si se equivoca |
| FASE B | **Sonnet** (delegar agente por archivo) | Patrón ya validado, replicar mecánicamente |
| FASE C | **Sonnet** | Sprite porting es directo |
| FASE D | **Sonnet o Opus** | UX puede requerir iteración visual |

Cada agente de FASE B puede recibir el mismo prompt-plantilla con el archivo objetivo. Se pueden lanzar 4 en paralelo (uno por pantalla).

---

## 7. Criterios de éxito (acceptance)

- [ ] VU de sonido scrollea sin freezes perceptibles a 30 Hz constantes (medir con `millis()` log). ← requiere Fase A
- [x] Ninguna pantalla muestra flash negro cuando solo un valor cambia. ← Fase B/C completada (pendiente validación en hardware)
- [x] Cambios de unidad o magnitud ("100%" → "5%") no dejan ghost pixels. ← Fase B completada
- [ ] El sensor task loop p99 < 35 ms (vs ~145 ms actual). ← requiere Fase A
- [ ] BLE notify no añade más de 2 ms al loop del sensor task. ← requiere Fase A bonus
- [ ] La pantalla SOUND_SCREEN y LAB_SOUND_VU_STACK/WAVE responden a sonido <50 ms latencia (sample → píxel). ← requiere Fase A + validación hardware

---

## 8. Métricas a instrumentar para validar

Sugiero añadir (gated por `FIRMWARE_DEBUG`):

```cpp
// En sensor_reading_task:
uint32_t loop_start = micros();
// ... read sensors ...
uint32_t loop_dur = micros() - loop_start;
static uint32_t loop_max = 0, loop_count = 0, loop_sum = 0;
if (loop_dur > loop_max) loop_max = loop_dur;
loop_sum += loop_dur; loop_count++;
if (loop_count >= 100) {
    DPRINT("[Sensor] loop avg=%lu max=%lu us\n", loop_sum/100, loop_max);
    loop_count = 0; loop_sum = 0; loop_max = 0;
}
```

Repetir en `switch_screen` para validar que el display task efectivamente está a 5 ms.
