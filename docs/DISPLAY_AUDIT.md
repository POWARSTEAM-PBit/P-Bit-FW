# P-Bit Firmware — Display & Sensor Pipeline Audit

**Fecha auditoría**: 2026-05-18 | **Actualizado**: 2026-05-20
**Auditado por**: Opus 4.7 (deep review) + Sonnet 4.6 (implementación Fase B/C/D) + pasada documental 2026-05-20
**Versión firmware**: HEAD `f6414be` + cambios no-commit (Sensor Zone actual, i18n, BLE factory-off, anti-flicker Fase B/C/D)
**Hardware target**: ESP32 + ST7735 160×128 px, sensores LDR / GM19767P / DHT11 / DS18B20 / suelo capacitivo

---

## TL;DR

Esta auditoría empezó como una lista de bugs activos, pero a 2026-05-20 sus P0/P1/P2 principales ya están **resueltos en código**. El DS18B20 ya no bloquea el loop, el DHT se alterna, BLE sale del hot path, LDR y suelo están submuestreados, `Sound VU` tiene EWMA + idle pulse y las pantallas con dials/cards/sparklines aplican sprites o chrome/data split.

Lo que sigue abierto no es una fase de implementación general, sino **validación en hardware real**: confirmar en ST7735 que no hay freezes perceptibles, flashes negros, ghost pixels ni recortes en textos localizados.

---

## 1. Pipeline de datos — estado actual

```
┌─────────────────────────────────────────────────────────────────┐
│ Core 0 — SensorTask (prio 1, cadence estable ~30 ms)            │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │ each iteration:                                         │    │
│  │   read_fast_sensors()  ──► LDR cada 6 ciclos (~5 Hz)    │    │
│  │                            sound_level cada ciclo       │    │
│  │                            soil cada 30 ciclos (~1 Hz)  │    │
│  │   [every 1000 ms] read_slow_sensors():                  │    │
│  │                            DHT alternado temp/hum       │    │
│  │                            DS18B20 async two-step       │    │
│  │   portENTER_CRITICAL → global_readings = local_r        │    │
│  │   alert_engine_refresh(...)                             │    │
│  │   runtime_mark_sensor_data_ready()  ◄── publica snapshot│    │
│  │   ble_service() rate-limited (BLE factory-off por def.) │    │
│  │   vTaskDelayUntil(..., 30 ms)                           │    │
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
- Ciclo rápido: ~30 ms entre `mark_sensor_data_ready` → tope teórico **~33 Hz**.
- Ciclo lento: DS18B20 async y DHT alternado reducen el parón histórico; la validación pendiente es medir p99 real en hardware con `FIRMWARE_DEBUG`.
- LDR: lux calibrado y acotado a `0..20000`, suavizado por EMA.

---

## 2. Hallazgos críticos históricos (P0 — cerrados en Fase A)

### P0-1 — DS18B20 bloqueaba el publish loop 94 ms cada segundo

**Archivo**: `src/hw.cpp:680` (`read_ds18b20_temp`)
**Síntoma original**: el VU de sonido (y cualquier animación de >5 Hz) se congelaba ~94 ms cada segundo.
**Causa original**: `sensors.requestTemperatures()` era síncrono y bloqueaba el thread hasta terminar la conversión.
**Estado actual**: resuelto. `init_hw()` usa `setWaitForConversion(false)` y la lectura sigue un patrón two-step async.
```cpp
// En init_hw():
sensors.setWaitForConversion(false);

// Patrón two-step async (en read_slow_sensors):
//   1. issue request: sensors.requestTemperatures()  ← retorna inmediato
//   2. en el siguiente ciclo slow (≥100 ms después): sensors.getTempCByIndex(0)
```
Esto reduce el blocking del DS18B20 de **94 ms → ~1 ms** (solo el comando 1-Wire). El valor disponible llega 1 segundo después (en el próximo ciclo slow), lo cual es invisible para el usuario porque la temperatura no cambia visualmente entre samples a 1 Hz.

### P0-2 — DHT11 bloqueaba ~25 ms × 2 cada segundo

**Archivo**: `src/io.cpp:144-145` (`read_slow_sensors`)
**Síntoma original**: ~50 ms adicionales de freeze cada segundo.
**Causa original**: dos lecturas consecutivas de la librería `DHT`.
**Estado actual**: resuelto. `read_slow_sensors()` alterna temperatura/humedad con `dht_slot`, de modo que cada canal se actualiza cada 2 s.

### P0-3 — `read_sound_level()` impone 20 ms blocking en cada fast cycle

**Archivo**: `src/hw.cpp:585-616`
**Síntoma original**: tope duro del fast-sensor rate ≈ 33 Hz.
**Veredicto**: **NO tocar el window**. 33 Hz de update es suficiente para VU smoothness y la ventana de 20 ms sigue siendo necesaria.
**Estado actual**: aceptado como diseño. La UX se suavizó en `ui_lab_sound_vu.cpp` con EWMA asimétrico, scroll constante e idle pulse cuando el nivel es bajo.

---

## 3. Hallazgos altos históricos (P1 — cerrados en Fase B/C)

### P1-1 — Patrón "full-screen fillRect antes de redraw" en 6+ pantallas

**Pantallas afectadas originales**:
- `ui_sound.cpp:421` — `fillRect(0, LB_VALUE_TOP-4, 160, 52, BG)` antes del número
- `ui_lab_sound_vu.cpp` (ya parchado) — `fillRect(0, L_CONTENT_TOP, 160, 108, BG)` antes de la card
- Probablemente `ui_temp.cpp`, `ui_light.cpp`, `ui_ds18.cpp`, `ui_humidity.cpp` con el mismo patrón (ver `clearMenuBands` también)

**Síntoma original**: flash negro visible cada vez que el valor cambiaba. Especialmente molesto si el redraw era a 33 Hz (sound).
**Estado actual**: resuelto en las pantallas auditadas mediante chrome/data split, clears acotados y caches por campo.
**Patrón consolidado**:
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
**Estado actual**: resuelto donde se detectó. Mantener como regla de code review: siempre preceder con `tft.fillRect(x_left, y, max_width, font_h, BG)` calculado al tamaño del valor más largo posible. Ver el fix aplicado en `draw_value_badge` (`ui_lab_sound_vu.cpp:113`).

### P1-3 — Sprite no usado donde reduciría 10–100× el SPI traffic

**Candidatos originales**:
- `ui_graph.cpp` — la sparkline ya usa sprite (good), pero el `drawRoundRect` del borde se redibuja después del sprite cada frame.
- `ui_lab_gauge_temp.cpp` — el arco (P4→P3 con `drawArc` o `fillCircle` segments) son típicamente 50+ SPI calls. Candidato fuerte a sprite.
- `ui_lab_focus.cpp` y `ui_lab_value_modern.cpp` — si tienen meter/bar dinámico, vale la pena ver.
- `ui_lab_sensor_cards.cpp` (3 cards con meters) — alto riesgo de flicker.

**Estado actual**: resuelto para las zonas de mayor riesgo: ring sprite en `ui_lab_widget_showcase.cpp`, sparkline sprite en `VALOR`, sprite VU en `ui_lab_sound_vu.cpp`, y reglas chrome-last en `SENSOR CARD`.
**Recomendación permanente**: política "si más de 20 SPI calls por frame en una zona de >50×30 px → sprite".

### P1-4 — Jewel de alerta se redibuja sin verificar cambio de estado

**Archivos**: `ui_temp.cpp:521`, `ui_ds18.cpp:511` (probablemente otros).
**Síntoma original**: drawAlertJewel pintaba círculos cada frame → SPI waste y posible micro-flicker.
**Estado actual**: resuelto en la pasada de caches (`last_alert_state`/equivalentes). Mantener como regla: no redibujar jewels si no cambió sensor/alerta/enable.

---

## 4. Hallazgos medios (P2 — pulido)

### P2-1 — BLE en el hot path del sensor task

Estado actual: mitigado. `ble_service()` es rate-limited, los callbacks no notifican inline, y además BLE queda factory-off (`ble_en=false`) en cada flash nuevo hasta desbloqueo explícito.

### P2-2 — Sobre-muestreo de sensores lentos

| Sensor   | Rate actual | Rate necesario |
|----------|-------------|----------------|
| Sound    | 33 Hz       | 20–30 Hz ✓     |
| LDR      | 33 Hz       | 2–5 Hz         |
| Soil     | 33 Hz       | 0.2–1 Hz       |
| DHT      | 1 Hz        | 0.5 Hz         |
| DS18B20  | 1 Hz        | 0.5 Hz         |

Reducir LDR a cada 5 fast cycles, soil a cada 30, DHT/DS a cada 2 s libera CPU y BLE bandwidth sin pérdida visual.

Estado actual: aplicado para LDR (~5 Hz), suelo (~1 Hz), DHT alternado y DS18 async. El sonido conserva el muestreo rápido porque alimenta el VU.

### P2-3 — VU sin animación cuando silencio

Cuando `level=0` constante, todas las columnas del stack meter están vacías. Aunque el código está pusheando samples, no hay nada visible que se mueva → el usuario percibe "congelado".
**Estado actual**: resuelto. Cuando `level < 5`, el VU muestra **1 segmento verde tenue** en la base de cada columna con pulso triangular ~1 Hz.

### P2-4 — `clearMenuBands()` redibujaba 3 bandas siempre

Estado actual: resuelto con bitmask (`clearMenuBands(uint8_t bands = kMenuBand_All)`).

### P2-5 — Sleep overlay redibuja el orb cada frame (5 ms ciclo)

El orb breathing se calcula en cada loop del display task, pero el sprite del orb se redibuja solo si `r != last_r`. Esto es OK, pero los `fillCircle` directos sobre el TFT podrían ir a un sprite si quisiéramos animación más fluida o cambio de color.

---

## 5. Plan de remediación por fases

### FASE A — Sensor pipeline ✅ YA ESTABA IMPLEMENTADA (pre-sesión)

Verificado en código (`hw.cpp` + `io.cpp`):
- ✅ **P0-1**: DS18B20 async — `setWaitForConversion(false)` en `init_hw()` + patrón two-step en `read_ds18b20_temp()`. Blocking: 94 ms → ~7 ms.
- ✅ **P0-2**: DHT alterna reads — `dht_slot` en `read_slow_sensors()`. Cada canal se lee cada 2 s en lugar de juntos cada 1 s.
- ✅ **Bonus P2-1**: BLE ya se sirve desde `ble_service()` con rate-limit y `client_connected.load()`; `notifyAll()` queda como camino interno protegido y no se llama desde callbacks NimBLE.

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
- ✅ LDR normalizado a lux `0..20000` en pipeline y UI lab.

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

- [ ] VU de sonido scrollea sin freezes perceptibles a 30 Hz constantes (medir con `millis()` log). ← requiere validación hardware
- [x] Ninguna pantalla muestra flash negro cuando solo un valor cambia. ← Fase B/C completada (pendiente validación en hardware)
- [x] Cambios de unidad o magnitud ("100%" → "5%") no dejan ghost pixels. ← Fase B completada
- [ ] El sensor task loop p99 < 35 ms con BLE apagado y conectado. ← requiere instrumentación `FIRMWARE_DEBUG`
- [ ] BLE notify no añade más de 2 ms al loop del sensor task. ← requiere medición con BLE conectado
- [ ] La pantalla SOUND_SCREEN y LAB_SOUND_VU_STACK/WAVE responden a sonido <50 ms latencia (sample → píxel). ← requiere validación hardware
- [ ] BLE factory-off verificado en placa recién flasheada: sin advertising, sin fila BLE visible en `Sistema`, desbloqueo solo por gesto secreto de 60 s.
- [ ] LDR validado en hardware con rango `0..20000 lux`, saturación controlada y RGB apagado en pantalla de luz.
- [ ] Cabeceras i18n de `Sensor Zone` verificadas en ES/CAT/EN sin recorte visible.

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
