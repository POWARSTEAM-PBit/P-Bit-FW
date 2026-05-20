# Análisis de Pantallas Experimentales del P-Bit

Actualizado: 2026-05-20

## Estado tras revisión de producción/i18n

Este documento es secundario: resume el estado visual actual para no arrastrar decisiones de laboratorio antiguas. No es una release histórica y no debe usarse para justificar el carrusel anterior de 18 pantallas.

La UI actual ya no expone las pantallas individuales como paradas principales. El producto se organiza en cuatro pantallas de contexto y una zona unificada por sensor:

- `HOME` / `INICIO` (`LAB_HOME_CARDS_SCREEN`)
- `CLIMA LAB` (`LAB_DUAL_TH_SCREEN`)
- `MULTI LAB` / `TEMP LAB` (`LAB_WIDGET_MIX_SCREEN`)
- `SONIDO VU` / `RUIDO LAB` (`LAB_SOUND_VU_STACK_SCREEN`)
- `SENSOR_ZONE_SCREEN` para Temp, Hum, Luz, Sonido, Suelo y DS18
- `TIMER_SCREEN`
- `SYSTEM_SCREEN`

`GRAPH_SCREEN`, `LAB_SENSOR_FOCUS_SCREEN`, `LAB_VALUE_MODERN_SCREEN`, `LAB_GAUGE_TEMP_SCREEN` y `LAB_SENSOR_CARD_SCREEN` siguen siendo renderers activos, pero entran como modos de `SENSOR_ZONE_SCREEN`, no como paradas top-level del carrusel.

---

## 1. Carrusel visible actual

Orden real en `src/rotary.cpp` -> `kCarousel[]`:

1. `LAB_HOME_CARDS_SCREEN` - Home 2x2
2. `LAB_DUAL_TH_SCREEN` - Clima
3. `LAB_WIDGET_MIX_SCREEN` - Multi / comparación DHT11 vs DS18B20
4. `LAB_SOUND_VU_STACK_SCREEN` - Sonido VU
5. `SENSOR_ZONE_SCREEN` con `SZ_TEMP`
6. `SENSOR_ZONE_SCREEN` con `SZ_HUM`
7. `SENSOR_ZONE_SCREEN` con `SZ_LIGHT`
8. `SENSOR_ZONE_SCREEN` con `SZ_SOUND`
9. `SENSOR_ZONE_SCREEN` con `SZ_SOIL`
10. `SENSOR_ZONE_SCREEN` con `SZ_DS18`
11. `TIMER_SCREEN`
12. `SYSTEM_SCREEN`

### Interacciones

- Girar fuera de menús cambia entre los 12 slots del carrusel.
- Pulsación corta en un slot de Sensor Zone cambia el modo visual del sensor.
- Pulsación larga en un slot de Sensor Zone abre el menú/configuración del sensor correspondiente.
- Pulsación corta en `SYSTEM_SCREEN` conmuta el sonido global.
- Pulsación larga en `SYSTEM_SCREEN` abre ajustes.
- Mantener pulsado `SYSTEM_SCREEN` 60 s desbloquea la pantalla oculta de BLE.

---

## 2. Sensor Zone

`SENSOR_ZONE_SCREEN` concentra la navegación de detalle por sensor. El sensor activo es:

- `SZ_TEMP`
- `SZ_HUM`
- `SZ_LIGHT`
- `SZ_SOUND`
- `SZ_SOIL`
- `SZ_DS18`

Cada sensor conserva su modo visual en NVS. Modos disponibles:

| Modo | Renderer reutilizado | Rol visual |
| --- | --- | --- |
| `SZ_VIZ_FOCUS` | `draw_lab_focus_screen()` | detalle con valor y sparkline |
| `SZ_VIZ_VALOR` | `draw_lab_value_modern_screen()` | valor grande + barra + sparkline |
| `SZ_VIZ_GRAPH` | `draw_graph_screen()` | gráfica histórica |
| `SZ_VIZ_GAUGE` | `draw_lab_gauge_temp_screen()` | dial/gauge por sensor |
| `SZ_VIZ_CARD` | `draw_lab_sensor_card_screen()` | card compacta con indicador específico |

El header lo dibuja Sensor Zone con `sz_header_name()`. Los renderers internos deben respetar `sz_is_active()` para no duplicar cabeceras.

---

## 3. Pantallas de producción visual

### Home

`LAB_HOME_CARDS_SCREEN` es la portada actual. Muestra cuatro cards:

- Temp
- Hum
- Luz
- Sonido

La luz usa escala `0..20000 lux`. El valor ADC `0..4095` queda como dato técnico/calibración, no como magnitud principal de Home.

### Clima

`LAB_DUAL_TH_SCREEN` agrupa temperatura y humedad en lectura de confort ambiental. Es la pantalla semántica para "cómo se siente el entorno".

### Multi

`LAB_WIDGET_MIX_SCREEN` compara DHT11 y DS18B20 con cards y barra diferencial. En idioma puede seguir viéndose como `TEMP LAB`; documentar como Multi/Temp para evitar confundirlo con la antigua pantalla individual `TEMP_SCREEN`.

### Sonido VU

`LAB_SOUND_VU_STACK_SCREEN` es la parada visible de sonido dinámico. La sub-vista `LAB_SOUND_VU_WAVE_SCREEN` sigue compilada y se alterna por pulsación corta desde Sonido VU.

El VU usa historial suavizado, sprite y badge de porcentaje con limpieza acotada para evitar congelaciones y ghost digits.

---

## 4. Pantallas compiladas pero no top-level

Estas pantallas existen porque se reutilizan como renderers, menús o tooling, pero no deben documentarse como paradas visibles directas:

- `TEMP_SCREEN`, `HUMIDITY_SCREEN`, `LIGHT_SCREEN`, `SOUND_SCREEN`, `SOIL_SCREEN`, `DS18B20_SCREEN`: pantallas/menús de configuración abiertos desde Sensor Zone.
- `GRAPH_SCREEN`: renderer del modo `SZ_VIZ_GRAPH`.
- `LAB_SENSOR_FOCUS_SCREEN`: renderer del modo `SZ_VIZ_FOCUS`.
- `LAB_VALUE_MODERN_SCREEN`: renderer del modo `SZ_VIZ_VALOR`.
- `LAB_GAUGE_TEMP_SCREEN`: renderer del modo `SZ_VIZ_GAUGE`.
- `LAB_SENSOR_CARD_SCREEN`: renderer del modo `SZ_VIZ_CARD`.
- `LAB_DASH_OVERVIEW_SCREEN`, `LAB_LINEAR_DASH_SCREEN`, galerías de iconos y tamaños: tooling interno/legacy.
- `BLE_TOGGLE_SCREEN`: oculta; solo accesible con gesto secreto de 60 s en Sistema.

---

## 5. i18n vigente

La fuente de verdad de idiomas está centralizada:

- `include/languages.h` define `Language` y `LangKey`.
- `src/lang_select.cpp` contiene las traducciones ES/CAT/EN.
- `L(key)` se usa en runtime para el idioma activo.
- `LIn(language, key)` se usa cuando hace falta leer un idioma concreto.

Idiomas soportados:

- `LANG_ES`
- `LANG_CAT`
- `LANG_EN`

Regla para nuevas pantallas/snippets: no introducir textos de UI hardcodeados si ya existe o debe existir una `LangKey`. Se aceptan etiquetas técnicas cortas como `DHT11`, `DS18B20`, `LDR`, `MIC` cuando son identificadores de hardware.

El selector de idioma se muestra en primer arranque tras borrado de NVS por cambio de build. El idioma también se ajusta desde Sistema.

---

## 6. BLE y estado de fábrica

BLE está factory-off:

- `ble_en` se borra con el reset por build hash al flashear firmware nuevo.
- `init_ble()` solo se llama si `load_ble_enabled_store()` devuelve true.
- `BLE_TOGGLE_SCREEN` no está en el carrusel visible.
- La pantalla BLE se desbloquea solo manteniendo pulsado `SYSTEM_SCREEN` 60 s.
- No restaurar ni documentar BLE como flujo visible normal de usuario.

En visualizer, las escenas `system_runtime_ble_connected/disconnected` deben tratarse como legacy/debug o estado oculto, no como pantalla visible de producto.

---

## 7. LDR y luz

La lectura actual de luz es lux calibrado y acotado:

- Entrada ADC: `0..4095` (`ldr_raw`)
- Magnitud de UI: `0..20000 lux`
- Saturación alta: `20000 lux`
- Porcentaje logarítmico: `log10(lux) / log10(20000) * 100`

Documentación y visualizer deben priorizar `lux` como magnitud de usuario. El modo raw ADC existe para calibración/depuración.

---

## 8. Reglas anti-flicker vigentes

Patrón común:

- `screen_changed`: dibuja shell completo, header, cards, bordes, labels fijos e iconos.
- `sensor_data_changed`: redibuja solo valor, barra, dial, sparkline, VU o estados que cambian.
- Caches por pantalla deciden si se redibuja chrome o solo data.
- Clears acotados antes de valores variables para evitar ghosting.
- Sprites para gráfica, sparkline, dial/ring y VU cuando el widget produce muchos draw calls.

Piezas relevantes:

- Home: cards 2x2 con chrome estable y tanque/valor dinámico.
- Sensor Card: cache por sensor, limpieza de unión de rects para valores.
- Valor Lab: badges/chrome separados de data; sparkline en sprite.
- Gauge/Dial: ring en sprite, escala y unidad como chrome.
- Sonido VU: stack/wave en sprite con suavizado asimétrico.
- Graph: sprite para el área de curva y banda superior con clears parciales.

---

## 9. Visualizer: estado esperado

`visualizer_scenes/` es auxiliar y no forma parte del build. Tras la revisión de producción/i18n, sus README deben reflejar:

- UI visible: Home, Clima, Multi, Sonido VU, Sensor Zone, Timer y Sistema.
- Sensor Zone como contenedor de detalle por sensor.
- LDR mostrado en `0..20000 lux`.
- Textos sincronizados con `LangKey` ES/CAT/EN.
- BLE como factory-off oculto.
- Anti-flicker mediante shell/data, caches y sprites.

Escenas individuales antiguas siguen siendo útiles para menús de configuración, pero ya no representan paradas directas del carrusel.

---

## 10. Qué queda pendiente para visualizer

Prioridad alta:

- Añadir escenas de Home 2x2 actual.
- Añadir Clima actual si el snippet no cubre la geometría final.
- Añadir Multi/Temp Lab actual.
- Añadir Sonido VU stack y wave.
- Añadir ejemplos de Sensor Zone: focus, valor, graph, gauge y card.
- Revisar escenas de luz para que `lux` use escala `0..20000`.
- Marcar escenas BLE como ocultas/debug en Sistema.

Prioridad baja:

- Regenerar galerías legacy si se decide conservarlas como tooling.
- Archivar o comentar snippets que dependan del carrusel de 18 pantallas.

---

## 11. Reglas para siguientes sesiones

- No reintroducir el carrusel antiguo como estado actual.
- No convertir pantallas legacy en visibles salvo decisión explícita de producto.
- No hardcodear textos traducibles fuera de `languages.h` / `lang_select.cpp`.
- Mantener LDR como lux `0..20000` en documentación de usuario.
- Mantener BLE fuera del flujo normal.
- Cualquier nueva escena visualizer debe declarar si es `production-current`, `sensor-zone-renderer`, `config-menu`, `hidden-debug` o `legacy-lab`.
