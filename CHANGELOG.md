# Changelog

## 2026-06-03

### Documentación — Reglas TFT y firmware reforzadas

- **Reforzado:** `docs/TFT_RENDER_RULES.md` (258 → 490 líneas) con Nivel 0 (cache canónica), sub-sección `sz_set_active`, Nivel 5 (Demo Mode), "Verificación pre-claim" con 10 proofs y "Banderas rojas grep-ables".
- **Reforzado:** `docs/TECHNICAL.md` (987 → 1097 líneas) con Apéndice A: stack real (Arduino+PlatformIO vs ESP-IDF), 9 banderas rojas grep firmware, 8 proofs pre-claim, GPIO traps.

### Mantenimiento — Limpieza de fonts y archivos basura

- **Limpiado:** `include/fonts.h` reducido a las 6 fuentes que realmente se compilan (`Roboto_Regular7pt8b`, `Roboto_Light6pt8b`, `Roboto_Medium10pt8b`, `IBMPlexSans_Regular9pt8b`, `IBMPlexMono_Regular12pt8b`, `IBMPlexMono_Regular24pt8b`) + los alias `FONT_*`. Las ~30 declaraciones `extern` muertas se eliminaron; los archivos `.h` quedan en `include/` como librería disponible para reactivar.
- **Eliminado del tracking:** `listado_completo.txt`, `platformio.zip`, `logs/xvba_debug.log`, `preview_probe_icon.html` (ya estaban en .gitignore).
- **Eliminado del repo:** `wokwi.toml`, `diagram.json`, carpeta `archive/`.
- **Movido:** `tools/layout_validation_snippet.cpp` → `docs/`.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes), Flash 72.1% (`945429` bytes).

## 2026-05-29

### Firmware — Estados externos desconectados + temp mono

- **Añadido:** `external_sensor_state.*` centraliza el estado runtime de ausencia para sensores externos: DS18B20/Termómetro falta con `temp_ds18b20 < -100` y Suelo falta con `soil_humidity = NaN`.
- **Ajustado:** textos visibles de desconexión pasan a referencias de PCB visibles: `Revisa IO33` / `Comprova IO33` / `Check IO33` para Termómetro y `Revisa IO35` / `Comprova IO35` / `Check IO35` para Suelo.
- **Ajustado:** pantallas clásicas, Sensor Zone, Graph y vistas Lab usan paleta del sensor atenuada para DS18/Suelo desconectados, evitando gris plano o rojo dominante y manteniendo retorno runtime al reconectar.
- **Refinado:** el estado desconectado queda menos desaturado para mejorar legibilidad; los textos accionables `Sin sensor` / `Revisa IO33/IO35` ganan prioridad visual y se ajustan en Y en Focus y Temp Lab.
- **Añadido:** splash semafórico de conexión/desconexión para sensores externos: al conectar muestra fondo verde con `CONECTADO / Sensor Suelo|Sensor DS18B20 / IO35|IO33`; al desconectar muestra fondo rojo con `DESCONECTADO / Sensor Suelo|Sensor DS18B20 / IO35|IO33`. Dura `1500 ms`, no se dispara al boot ni durante Demo Mode.
- **Ajustado:** color visual de Suelo ahora sigue umbrales: `0%` y tramo bajo se ven amarillo intenso, `Seco..Húmedo` se mantiene verde y por encima de `Húmedo` vira progresivamente a azul. La rampa se comparte en clásica, Focus, Card, Gauge, Plant Lab, iconos y RGB.
- **Corregido:** en Focus/Sensor Zone, el clear dinámico de la card superior respeta el ancho real del título para que `DS18B20` no quede recortado por el área del valor; el label DS18B20 baja 2 px para mejorar alineación.
- **Ajustado:** icono `temp` small/XL/fallback queda monocromo; solo la versión Dial/XXL con argumento de acento conserva detalle multicolor. `probe`/DS18B20 queda separado y sin aprobación final.
- **Documentado:** `USER_GUIDE`, `TECHNICAL`, `PROJECT`, `DESIGN_SYSTEM`, `TFT_RENDER_RULES`, producción y roadmap reflejan `IO33/IO35`, estado runtime no persistente y criterios de validación.
- Build verificado: `SUCCESS` — RAM 14.9% (`48940` bytes), Flash 72.1% (`945429` bytes).

### Firmware — LDR curva empírica v1

- **Ajustado:** `src/io.cpp` reemplaza el modelo teórico GL5528 por la primera curva empírica tomada con luxómetro: `lux = 10 * ((4095 - raw) / (raw + 150))^2`.
- **Ajustado:** el rango visual/base de Luz pasa de `0..20000 lux` a `0..8000 lux`, más coherente con las muestras reales (`5 raw -> ~7400 lux`) y con el techo de la nueva fórmula.
- **Ajustado:** el menú y la validación NVS de rangos de Luz limitan `Max brillante` al nuevo techo de `8000 lux`.
- **Ajustado:** `FC` sigue derivándose del lux calibrado con `lux / 10.764`; no existe una curva independiente para foot-candle.
- **Ajustado:** Demo Mode reduce la amplitud de luz simulada para no recortar en el nuevo techo y calcula `Raw ADC` con la inversa de la misma curva empírica.
- **Documentado:** `docs/TECHNICAL.md` deja trazable la muestra usada, la corrección del punto invertido `120 - 800` como `800 - 120`, la fórmula de ajuste y la aproximación elegida para firmware.
- Build verificado: `SUCCESS` — RAM 14.9% (`48900` bytes), Flash 71.8% (`941425` bytes).

## 2026-05-28

### Sensor Cards — paleta aplicada a TEMP/DS18; fix solape icono-valor

- **Corregido:** `temp_accent()` usa `PB_TEMP_P1` (fucsia) en lugar de `getTempColor()` — la tarjeta TEMP del Sensor Zone ahora refleja la identidad de paleta correcta.
- **Corregido:** `ds18_accent()` usa `PB_DS18_P1` en lugar del gradiente anterior — la tarjeta Termómetro/DS18 usa su color de identidad.
- **Paleta DS18:** `PB_DS18_P1` `0xA01F` (violeta) → `0xFB80` rgb(255, 121, 0) **naranja vivo** — evita la confusión visual con TEMP fucsia en el carrusel. P2 `0x045F` azul láser se mantiene (buen complemento frío/cálido).
- **Fix layout:** `kValueClearY` = `kValueTopY - 1` (y=45) → `kValueTopY + 3` (y=49) — el icono llega hasta cy+7=49; la zona de borrado del valor ya no pisa los últimos 4px del icono.
- **Sin cambio:** colores de alerta (azul/rojo), `temp_bar_color` gradient, demo mode y pantalla `LAB_DUAL_TH`.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 1 fila TERMÓMETRO actualizada.

### Paleta — TEMP y SONIDO: identidades visuales rediseñadas

- **TEMP P1:** Naranja ácido `0xFA80` → **Fucsia eléctrico** `0xF817` rgb(255, 0, 184). Temperatura deja de evocar alarma; pasa a identidad neutra/vibrante estilo GBC.
- **TEMP P2:** Rosa eléctrico `0xF814` → **Verde ácido** `0x07E8` rgb(0, 255, 64). Contraste complementario de alta tensión con el fucsia.
- **SONIDO P1:** Magenta punk `0xF81F` → **Naranja cálido** `0xFD40` rgb(255, 168, 0). Naranja encaja semánticamente con energía/vibración sonora.
- **SONIDO P2:** Verde ácido `0x07E8` → **Violeta-púrpura** `0xC01F` rgb(197, 0, 255). Desplazado hacia el morado para diferenciarse del fucsia de TEMP en pantallas multi-sensor.
- **Sin cambio:** P3/P4 de ambos sensores y resto de paleta. Rojo neón `0xF8A0` SONIDO P3 se mantiene para picos VU/alarmas.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 1 tabla de tokens actualizada.

### Documentación — Estado post-validación visual

- **Cerrado por ahora:** ghosting/flicker en pantallas revisadas queda aprobado tras la última validación; se mantiene como vigilancia de regresión, no como bloqueador general.
- **Cerrado:** icono de temperatura `temp` queda como versión final propagada a tamaños small/XL/XXL; no confundir con el icono técnico `probe`/DS18B20.
- **Actualizado:** LDR `Lux / FC / Raw ADC` queda propagado en firmware: valor/unidad ya usan un helper común en Luz, Sensor Zone, cards, dials, dashboards y gráficas; queda validación visual en hardware.
- **Actualizado:** Modo demo entra desde logos con encoder presionado y desde `Home` con pulsación larga; ahora usa escenas con dwell variable, curvas simuladas suaves, RAW coherente con luz y gráficas sintéticas; queda validación visual en hardware.

### Firmware — LDR coherente y Demo smooth

- **Añadido:** `include/light_display.h` / `src/light_display.cpp` centralizan la presentación de Luz (`Lux`, `FC`, `raw`) y la conversión `lux / 10.764`.
- **Propagado:** Sensor Zone `Card`, `Valor`, `Focus`, `Gráfica`, `Dial`, Home cards, dashboards y pantalla clásica de Luz usan el modo LDR activo para valor/unidad visible.
- **Añadido:** `g_graph_light_raw` guarda histórico RAW ADC para que gráficas y sparklines muestren RAW real cuando el modo `Raw ADC` está activo.
- **Ajustado:** Demo Mode pasa a escenas con duración variable, refresco dedicado de 220 ms, curvas suaves por sensor y valores RAW inversamente correlacionados con la luz simulada.
- **Añadido:** `demo_mode_graph_values(...)` genera datos sintéticos para `Graph` durante demo, evitando que las gráficas dependan del histórico físico mientras se muestra la coreografía.
- Build verificado: `SUCCESS` — RAM 14.9% (`48900` bytes), Flash 71.8% (`941497` bytes).

### Firmware — Calibración LDR: RAW estable y modo FC

- **Ajustado:** `io.cpp` estabiliza `ldr_raw` con media móvil de 10 lecturas ADC y calcula lux desde ese RAW promediado, eliminando el EMA posterior sobre lux.
- **Ajustado:** `Luz > Modo` cambia a `Lux / FC / Raw ADC`; `FC` muestra `lux / 10.764`, usa clave i18n `ST_FC_UNIT` y `Raw ADC` muestra la lectura cruda promediada para calibración con luxómetro.
- **Corregido:** `Sensor Cards` amplía 2 px adicionales hacia abajo el clear del valor para eliminar restos inferiores persistentes.
- **Documentado:** `docs/USER_GUIDE.md`, `docs/TECHNICAL.md` y `docs/PROJECT.md` reflejan los modos `Lux / FC / Raw ADC` y el uso del RAW promediado para calibración.
- Build verificado: `SUCCESS` — RAM 14.7% (`48252` bytes), Flash 71.6% (`938497` bytes).

### Firmware — Icono termómetro v16: diseño final propagado a todas las escalas

- **Definido como FINAL:** `impl_temp_detail` (s=3, XXL/Dial) es el diseño de referencia aprobado.
- **Propagado a XL:** `pbit_draw_temp_icon_xl` ahora llama `impl_temp_detail(cx, cy, c, c, 3)` — íconos de gauge/Focus visualmente idénticos al Dial.
- **Corregido:** `ui_icons.cpp` declara `impl_temp_detail(...)` antes de usarlo en la API XL, cerrando el fallo de compilación de la propagación v16.
- **Propagado a small (s=1):** `impl_temp` actualizado con canal parcial (4s) + rect de mercurio blanco. Sin círculo interior blanco: a s=1, `s+3 == 3s+1` (cubre todo el bulbo).
- **Todas las escalas** comparten: eje central `cx`, bulbo único centrado, mercurio `TFT_WHITE`, ticks `(4s)/3`.
- **Documentado:** `docs/DESIGN_SYSTEM.md` § 2 — tabla de tamaños corregida (eliminado `_large` s=2 inexistente), fila `temp` marcada ✅ FINAL con geometría por escala.

### Firmware — Icono termómetro v15: alineación central completa y ticks recortados

- **Ajustado (v15):** `impl_temp` e `impl_temp_detail` en `src/ui_icons.cpp` — todos los elementos comparten eje `cx`: bulbo simplificado a un único `fillCircle(cx, cy+4s, 3s+1)` (eliminados los dos círculos asimétricos en cx/cx-1), mercurio rect en `cx-s…cx+s`, círculo blanco interior en `cx`. Ticks recortados de `2s` a `(4s)/3` (−1/3).

### Firmware — Icono termómetro v14: círculo bulbo más grande y centrado

- **Ajustado (v14):** `impl_temp_detail` en `src/ui_icons.cpp` — círculo blanco interior del bulbo cambia de `fillCircle(cx-1, cy+4s, s+1)` a `fillCircle(cx, cy+4s, s+3)`: diámetro +4 px y centrado exacto con el rectángulo de mercurio.

### Firmware — Icono termómetro v9: bulbo reducido, ratio 1:2

- **Ajustado (v13):** `impl_temp_detail` — mercurio conectado visualmente: `fillRect` blanco ahora se dibuja DESPUÉS de los `fillCircle` del bulbo, creando una tira blanca continua que atraviesa el centro del bulbo y conecta con el `fillCircle` blanco interior. Columna de mercurio unificada tubo→bulbo.
- **Ajustado (v12):** `impl_temp_detail` — añadido `fillCircle(cx-1, cy+4s, s+1, TFT_WHITE)` dentro del bulbo: mercurio concentrado visible en el bulbo, conecta visualmente con la columna blanca del tubo.
- **Ajustado (v11):** bulbo asimétrico via dos `fillCircle` superpuestos (cx r=3s+1 + cx-1 r=3s+1) → +2px izquierda / +1px derecha. Mercurio cambiado de acento a `TFT_WHITE`.
- **Redimensionado (v10):** `impl_temp` / `impl_temp_detail` — ícono ahora llena el canvas ±7s igual que todos los demás íconos. Tubo extendido de 9s a 11s (cy-7s→cy+4s), bulbo movido a cy+4s (bottom=cy+7s). Orden de render corregido: tubo→canal→bulbo. En `_detail`: mercurio extendido a 6s (cy-2s→cy+4s), bulbo dibujado al final para quedar limpio.
- **Ajustado:** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — ticks reducidos de `3*s` a `2*s` (de 9 px a 6 px en Dial). Pequeñas marcas de escala.
- **Ajustado:** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — radio del bulbo reducido de **r=5s a r=3s**.
  - Ratio ancho:alto corregido de ~1:1.4 a **1:2** exacto (6s × 12s; a s=3 Dial → 18 px Ø × 36 px alto).
  - Aplica a todas las escalas: s=1 (home), s=3 (XL y Dial).
  - Tubo (4s ancho), canal (2s), ticks y mercurio sin cambios.

## 2026-05-27

### Firmware — Anti-flicker Labs, Sensor Cards y Sonido Lab

- **Optimizado:** `Clima Lab` mantiene paneles, bordes y footer-card como shell estático; el footer ahora usa cache de estado y solo limpia el texto/dot cuando cambia.
- **Optimizado:** `Temp Lab` separa cards superiores y delta-card en shell + datos; en ticks normales actualiza solo valores, barra delta y labels necesarios.
- **Corregido:** `Sensor Cards` deja de repintar icono, jewel, estado y visualización completa cuando solo cambia el valor; los estados (`Óptimo`, `Muy húmedo`, `Muy fuerte`, etc.) tienen clear dedicado para evitar fantasmas.
- **Corregido:** `Sensor Cards` separa físicamente carril de estado, valor grande y visualización inferior: estado sube 2 px, valor/unidad bajan 3 px y las barras inferiores recuperan 16 px bajando a Y+2 para conservar `Suelo Tarjeta`; el clear de visualización queda acotado a la barra para no borrar números ni esquinas inferiores.
- **Corregido:** `Sensor Cards` guarda el ancho/posición del valor anterior y limpia la unión entre caja anterior y actual, evitando fantasmas laterales cuando el dato baja de dos/tres dígitos a uno.
- **Corregido:** `Sensor Cards` amplía el carril horizontal de limpieza del valor grande para cubrir el inicio real de números anchos y eliminar restos laterales al volver a valores cortos.
- **Corregido:** `Sensor Cards` amplía 3 px hacia abajo el clear del valor grande para borrar residuos en los últimos píxeles inferiores de la fuente.
- **Ajustado:** `Sonido Lab` elimina el badge bajo el valor y muestra un número limpio en la esquina superior derecha con clear rect pequeño y fallback de fuente.
- **Ajustado:** `Sound Lab` usa `FONT_HEADER` como fuente menor del valor con fallback a `FONT_BODY`, sube el dato 1 px, baja el icono `MIC` 3 px y el texto `MIC` 2 px.
- **Ajustado:** `Clima Lab` elimina la línea vertical gris entre los cards de temperatura y humedad.
- **Ajustado:** `Sistema` recorta 1 px el card inferior desde arriba para equilibrar la distancia vertical entre cards.
- **Ajustado:** `Sistema` sube 1 px los labels `Bip` y `Alarmas` del card inferior.
- **Refinado:** `Temp Lab` ajusta clears de valores/delta para no tapar `DS18B20`, la barra inferior ni los labels `+10`.
- Build verificado: `SUCCESS` — RAM 14.7% (`48204` bytes), Flash 71.6% (`938437` bytes).

### Firmware — Icono termómetro v8: geometría unificada 4s/2s para todos los tamaños

- **Rediseñado (v8):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp` — fórmula única para s=1, s=2 y s=3:
  - Tubo: **4s de ancho** → paredes **1s** cada lado, canal interior **2s**. Altura: **9s** (igual que v6)
  - Bulbo: **r=5s**, centro cy+2s → base en cy+7s (borde exacto del canvas ±7s)
  - `impl_temp_detail` (Dial): misma base + `fillRect(cx-s, cy-2s, 2s, 4s, accent)` para mercurio
  - A **s=3 (Dial):** paredes 3 px | canal/mercurio **6 px** | paredes 3 px = 12 px total
  - A **s=1 (home):** 1 px | 2 px | 1 px = 4 px total (1 px menos que v6 — imperceptible)
- **Eliminado:** branch `s==1` especial de v7 — una sola fórmula para toda la familia de tamaños

## 2026-05-26

### Firmware — Defaults silenciosos, BLE 30 s y Demo simulado

- **Ajustado:** gesto secreto BLE baja de 60 s a 30 s.
- **Ajustado:** `Bip` y `Alarmas` vuelven apagados por defecto tras build/reset; BLE sigue factory-off.
- **Añadido:** Demo Mode aplica una capa visual de valores simulados sobre `g_ui_readings_snapshot` sin modificar sensores reales, NVS ni BLE.
- **Añadido:** fuente común de color en `sensor_visuals.*` para que gauges/diales y LED RGB usen la misma lógica cromática por sensor/estado.
- **Añadido:** Luz y Sonido incorporan opción persistente `Ver límites` en sus menús; se guarda en NVS y se restaura con reset del sensor.
- **Ajustado:** diales muestran marcas de rango según criterio de confianza: Temp. DHT, Humedad y Suelo visibles por defecto; Luz y Sonido solo si `Ver límites` está activo; Termómetro/DS18B20 muestra solo la marca fija de `0 °C`.
- **Ajustado:** el dial de Termómetro/DS18B20 usa escala completa `-55..+125 °C` (`-67..+257 °F`) y rampa térmica amplia: blanco hielo/cian/azul → amarillo/naranja/rojo.
- **Ajustado:** LED RGB sincronizado con la visualización activa de sensor/timer: el color físico sigue el color semántico mostrado en pantalla; cualquier vista de solo Luz mantiene RGB apagado para no contaminar el LDR.
- **Ajustado:** `Sonido Lab` baja/recorta 1 px el borde superior de la card y Sensor Cards sube 2 px el grupo numérico para evitar cortes con las gráficas inferiores.
- **Refinado:** textos abreviados de temperatura usan punto (`TEMP.`) y los mensajes `Guardado` de menús pasan a magenta, 3 px más arriba, para no confundirse con valores verdes.
- Build verificado: `SUCCESS` — RAM 14.7% (`48156` bytes), Flash 71.5% (`936653` bytes).

### Firmware — Icono termómetro v6: proporciones reales, tubo 5s paredes 1px

- **Rediseñado (v6):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo: vuelve a **5s** de ancho (proporciones de termómetro real, no fat)
  - Paredes: **1s** cada lado → interior **3s** (vs 1s en v4, vs 5s en v5 que era demasiado)
  - Bulbo: **r=5s** — equator 11px vs tubo 5px = ratio 1:2.2; claramente más ancho que el tubo
  - Altura: **9s** (cy-7s a cy+7s = canvas completo)
  - Air: `fillRect(cx-s, cy-6s, 3s, 4s, 0x1082)` — 3px wide × 4 rows
  - Mercury (Dial): `fillRect(cx-s, cy-2s, 3s, 4s, a)` — 3px wide × 4 rows en acento temperatura
  - Ticks: desde cx+3s (1px tras la pared), 3s de largo
- **Razonamiento:** v5 (7s ancho) se veía "gordo"; v6 recupera la silueta estilizada con interior 3× más visible que v4
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.3% (`934929` bytes).

### Firmware — Icono termómetro v5: tubo grande, paredes 1px, interior amplio

- **Rediseñado (v5):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo: **5s → 7s** de ancho (usa el canvas completo en horizontal)
  - Altura: 8s → **9s** (cy-7s..cy+2s, toca el borde superior del canvas)
  - Paredes: **2s → 1s** cada lado — el espacio ganado pasa al interior
  - Canal de aire: **1s×3s → 5s×4s** — 20× más área visible
  - Mercurio (Dial): **1s×4s → 5s×4s** — mismo salto
  - Bulbo: r=4s → **r=5s** — proporcional al tubo más ancho, toca cy+7s (canvas edge)
  - Ticks: desde cx+4s (1s tras la pared), longitud 3s
- **Resultado a s=1:** `[pared 1px][aire/merc 5px][pared 1px]` — todo visible sin lupa
- **Resultado a s=3 (Dial):** aire 15×12px + mercurio 15×12px dentro de paredes 3px — efecto muy dramático
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.3% (`934913` bytes).

### Firmware — Iconos: unificación de familia (estructura interior en todos)

- **Rediseñado `impl_humidity`**: añade `fillCircle(cx-2s, cy-s, s, 0x1082)` — punto de brillo reflectante en cuadrante superior-izquierdo de la gota. Convierte la masa sólida en "vidrio de agua" con estructura interior, igual que el termómetro.
- **Rediseñado `impl_sound`**: añade `drawFastHLine(cx-3s, cy-3s, 6s, 0x1082)` — franja de membrana en el tercio superior de la cápsula. Convierte el bloque sólido en "cápsula con diafragma visible".
- **Principio aplicado** (pixel-art-sprites skill): todos los iconos ahora tienen UN elemento de estructura interior en color bg (`0x1082`). Ningún icono es masa pura. Unifica el set como familia visual.

| Icono | Estructura interior |
|-------|-------------------|
| Humidity | Punto de brillo (gloss) |
| Sound | Línea de membrana |
| Light | Rayos crean espacios negativos |
| Plant | Tallo vs hojas (implícito) |
| Probe | Gap collar/cuerpo + seam (detail) |
| Temp | Canal de aire + mercurio |

### Firmware — Icono termómetro v4: paredes 2px, familia unificada

- **Rediseñado (v4 final):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo pasa de **3s a 5s** de ancho → paredes de **2px a s=1** (igual que cable sonda, tallo planta, rayos luz)
  - Cap integrado via `fillRoundRect` con `r=s` — sin círculo separado
  - Bulbo pasa de **r=3s a r=4s** — proporcional al tubo más ancho, mantiene el saliente visible
  - Ticks pasan de `cx+2s` a **`cx+3s`** (borde exterior del tubo 5s)
  - Canal de aire 1s centrado en cx — conservado: paredes 2s|aire 1s|paredes 2s
  - `impl_temp_detail` (Dial s=3): tubo 15px, paredes 6px, canal 3px, mercurio 3px×12px
- **Intención:** grosor visual equiparado con resto de iconos; espacio negativo del vidrio conservado
- Build verificado: `SUCCESS` — RAM 14.7% (`48132` bytes), Flash 71.2% (`933781` bytes).

## 2026-05-25

### Firmware — Modo demo runtime y ajustes finos Sistema/reposo

- **Añadido:** `include/demo_mode.h` / `src/demo_mode.cpp` con Modo demo runtime activable al encender con el encoder presionado. Recorre una banda de pantallas, bloquea reposo mientras está activo y sale con cualquier interacción posterior del encoder.
- **Protegido:** Demo Mode usa setters runtime de Sensor Zone (`sz_set_sensor_runtime`, `sz_set_viz_runtime`) para no persistir sensor/modo en NVS.
- **Ajustado:** reposo visible — las `Z` suben 4 px y el clear rect acompaña la nueva posición.
- **Ajustado:** pantalla `Sistema` — ID/valor y cards medias quedan 1 px más bajos tras la última validación visual.
- **Ajustado:** pantalla `Sistema` mantiene estables las cards medias e inferiores cuando BLE está habilitado; BLE pasa a badge compacto en la card superior de ID y el bloque de audio conserva su ancho completo.
- **Mejorado:** activación de Modo demo — `run_boot_sequence(true)` muestrea el encoder durante la animación de arranque; una pulsación larga desde `Home` activa el demo manualmente; al entrar se muestra un splash breve `MODO DEMO`.
- **Ajustado:** Modo demo rota todas sus escenas cada 6 segundos.
- **Corregido:** icono de humedad en Home usa el icono común actualizado y `Clima Lab` eleva la gota 2 px para evitar el recorte inferior.
- **Corregido:** Sensor Cards amplía la limpieza del indicador superior (`Óptimo`, `Seco`, `Húmedo`, etc.) y desplaza el valor para evitar ghosting/solapes.
- **Refinado:** encabezados compuestos de Sensor Zone usan abreviaturas con punto (`HUM. TARJETA`, `TERMO. LAB`) para evitar recortes.
- **Revisado:** menús de configuración con multiagentes; los helpers comunes ahora reducen fuente si el valor no cabe, separan mejor los summaries guardados del footer y `Luz > Modo lectura` pasa a `Luz > Modo`.
- **Documentado:** manual de usuario, manual técnico, checklist/release, proyecto y roadmap reflejan activación, salida y restricciones del Modo demo.
- Build verificado: `SUCCESS` — RAM 14.7% (`48148` bytes), Flash 71.2% (`932717` bytes).

### Firmware — Icono termómetro v3: canal de aire + mercurio cromático en Dial

- **Rediseñado (v3 final):** `impl_temp` / `impl_temp_detail` en `src/ui_icons.cpp`:
  - Tubo estrecho 3s + cap redondeado (`fillCircle`) + bulbo r=3s + ticks en `cx+2s`
  - **`impl_temp` (card, monocromo):** canal de aire en `0x1082` — mercurio implícito en `c`
  - **`impl_temp_detail` (Dial):** canal de aire + columna de mercurio explícita en **color acento `a`** coordinado con temperatura: `mix3_565(PB_TEMP_P4→azul, kNeonYellow, TFT_RED→rojo, amount)`
  - Intención: icono monocromo a resolución pequeña (card); mercurio cromático en el Dial
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.2%.

### Firmware — Icono DS18B20 v6: aprobado + Dial más grande

- **Diseño aprobado.** Icono DS18B20 marcado como final en `docs/ROADMAP.md` (eliminado de pendientes).
- **Rediseñado (v6):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp`:
  - Cable: 1px → 2px (añadido `drawFastVLine(cx+1, ...)`) en ambas funciones
  - Collar: 7s → 5s de ancho (proporcional al cuerpo de 3s)
  - Cuerpo: 5s×5s → 3s×6s (más estrecho y alargado verticalmente)
  - Punta: `fillCircle(cx, cy+5s, s)` — r=s, ajustada al cuerpo de 3s
  - `impl_probe_detail`: V-highlight ampliado de 3s → 4s alto
- **Dial más grande:** `pbit_draw_probe_icon_xxl` pasa de s=3 a s=4 — el icono en el centro del Dial ocupa más área dentro del clear rect disponible (60×62px). Resto de sensores sin cambio.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.1%.

### Firmware — Icono DS18B20 v5: gap collar/cuerpo + punta alineada al borde

- **Rediseñado:** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp`:
  - Cable: 4s → 3s (hace hueco al gap visual)
  - Collar: desplazado a `cy-4s` — deja 1s de gap físico entre collar y cuerpo
  - Cuerpo: 6s → 5s alto, sigue siendo 5s de ancho — proporciones más limpias
  - Punta: `fillCircle(cx, cy+4s, 2s)` — centro en borde inferior del cuerpo; diámetro ecuatorial = ancho del cuerpo
  - `impl_probe_detail`: groove eliminado (reemplazado por gap físico), V-highlight ajustado a 3s alto
- **Skills activos:** `pbit-tft-screen`, `pixel-art-sprites`, `8-bit-pixel-art-patterns`, `icon-design`, `retro`
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

### Firmware — Icono DS18B20 v4: simetría vertical garantizada

- **Corregido:** `impl_probe` en `src/ui_icons.cpp` — collar ampliado de 5s a 7s de ancho (igual que `impl_probe_detail`): `fillRect(cx-(7s)/2, cy-3s, 7s, 2s)`. El collar sobresale 1s a cada lado del housing (5s) en todas las escalas.
- **Resultado:** simetría axial vertical perfecta — silueta idéntica a izquierda y derecha del eje cx. El V-highlight izquierdo (color/brillo) se mantiene asimétrico por diseño; solo la forma es simétrica.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

### Firmware — Icono DS18B20 v3: cable vertical centrado

- **Rediseñado (v3):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp` — tercera iteración: el cable pasa de diagonal (corner superior-izquierdo) a **vertical centrado** (sale del centro del collar hacia arriba). La silueta es ahora: cable recto ↑ + collar horizontal + cuerpo cilíndrico + punta redondeada ↓. Versión detallada (XXL) usa dos `drawFastVLine` adyacentes en color acento para cable 2px de grosor.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.0%.

## 2026-05-24

### Firmware — Rediseño iconos sensor

- **Rediseñado (v2):** `impl_probe` / `impl_probe_detail` en `src/ui_icons.cpp` — segunda iteración del icono DS18B20: ahora representa la probeta waterproof real (cápsula metálica con punta redondeada). Estructura: cable diagonal al corner superior-izquierdo → collar horizontal (anillo de conexión) → cuerpo cilíndrico recto → punta redondeada inferior. El cable en diagonal rompe toda simetría vertical. Versión XXL añade cable de 2px (acento), collar con groove, highlights metálicos blancos en housing y punta.
- **Ajustado:** `impl_sound` / `impl_sound_detail` — cápsula micrófono de 10s×10s (casi circular, r=4s) a 8s×10s (r=3s): más oval, reconocible como cápsula a tamaños pequeños.
- Build verificado: `SUCCESS` — RAM 14.7%, Flash 71.1%.

### Documentación — Auditoría Diataxis y mejoras por documento

- **Reescrito:** `docs/ROADMAP.md` — reestructurado con framework Now/Next/Later (`product-management:roadmap-update`); sección "Hecho" eliminada (contenido en `CHANGELOG.md`); nueva sección "Estado del firmware" con snapshot de métricas; tablas de validación hardware explícitas por área.
- **Reescrito:** `docs/DESIGN_SYSTEM.md` — reestructurado de formato análisis/propuesta a referencia canónica con `design:design-system`; nueva estructura: Tokens → Iconos → Anatomía → Patrones → Reglas → Estado → Historial; secciones "Problemas detectados" y "Propuestas" comprimidas en "Historial de decisiones".
- **Actualizado:** `docs/TFT_RENDER_RULES.md` — fecha footer actualizada a 2026-05-24.
- **Corregidas** referencias huérfanas en `docs/TECHNICAL.md` (l.728), `docs/ROADMAP.md` (l.35-37, 64) y `docs/DESIGN_SYSTEM.md` (l.381) apuntando a archivos renombrados o archivados.
- **Ampliado:** `docs/USER_GUIDE.md` sección 4 — reemplazada "Primer encendido" (4 bullets) por "Primeros pasos — guía de inicio rápido" completa (`user-guide-writing`): 7 pasos con prerequisitos, success check y tabla de siguientes pasos sugeridos.
- **Añadido:** `docs/TECHNICAL.md` — tabla de contenidos con 18 secciones enlazadas al inicio del documento (`engineering:documentation`).
- **Activados** skills: `embedded-systems`, `esp32-firmware-engineer`, `pbit-tft-screen`, `diataxis`, `user-guide-writing`, `design:design-system`, `product-management:roadmap-update`, `technical-roadmaps`, `ui-design-system`.

### Documentación — Reestructura completa

- Nueva estructura de documentación: raíz limpia con 3 archivos (`README.md`, `CHANGELOG.md`, `AGENTS.md`) y todos los docs en `docs/`.
- **Nuevo:** `docs/PROJECT.md` — biblia del producto: hardware, capacidades, usos, mapa de documentación.
- **Reescrito:** `docs/USER_GUIDE.md` — manual de producto con avisos de seguridad, precauciones, especificaciones, menús completos y solución de problemas.
- **Actualizado:** `docs/TECHNICAL.md` — absorbió contenido de `Menues.MD` (sección 17 con flujos de encoder y menús completos) y actualizó referencias internas.
- **Renombrado:** `PALETTE_AND_ICONS_PROPOSAL.md` → `docs/DESIGN_SYSTEM.md` (ya no es propuesta, es canon implementado).
- **Movidos a `docs/`:** `ROADMAP_PBIT.md` → `docs/ROADMAP.md`, `MANUAL_TECNICO_PBIT.md` → `docs/TECHNICAL.md`, `MANUAL_DE_USUARIO_PBIT.md` → `docs/USER_GUIDE.md`.
- **Archivados en `archive/`:** `LAB_GRAPH_UI_HANDOFF.md`, `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md`, `docs/DISPLAY_AUDIT.md`.
- **Eliminados:** `PBIT_FUNCIONAMIENTO_ACTUAL.md` (contenido distribuido en `PROJECT.md` y `USER_GUIDE.md`), `Menues.MD` (absorbido en `TECHNICAL.md`), `TFT_RENDER_RULES.md` de raíz (duplicado de `docs/`), `docs/REPO_HYGIENE.md` (efímero).
- **Actualizados:** `README.md` (simplificado a landing de 1 página), `AGENTS.md` (mapa de docs corregido).

### Firmware/UI

- Unificados los menús raíz de sensores y Sistema con `drawSettingsGridMenu()` en grid 2×3: opciones primarias arriba, `Reset` siempre abajo izquierda y `Salir` abajo derecha.
- Añadida card central para selectores/edición de menús (`ON/OFF`, `C/F`, límites y modos), con borde semántico por valor.
- Ajustada alineación vertical de valores dentro de cards de menú y diferenciados los resúmenes de texto frente al hint inferior mediante color/separador.
- Refinada pantalla `Sistema`: panels medio/inferior reposicionados, `ID` justificado izquierda/derecha y borde inferior de audio unificado para evitar lectura de doble card.
- Renombradas opciones raíz para evitar falsas calibraciones: `Luz > Rangos`, `Sonido > Niveles`, `Humedad > Rangos` y `Termómetro > Corrección / Límites / Unidad / Alertas`.
- Simplificada la edición de rangos de Suelo a dos umbrales (`Seco` y `Húmedo`); `Muy seco`, `Óptimo` y `Muy húmedo` se derivan automáticamente.
- Añadida persistencia NVS de la unidad global `C/F` mediante `sys_unit_f`; las pulsaciones cortas en Temperatura/Termómetro y los menús de unidad sobreviven reinicios.
- Reorganizada la pantalla `Sistema`: card principal con panels internos para `ID`, `Tiempo`, `Idioma`, audio y BLE condicional; el menú raíz pasa a grid 2×3.
- Ocultación BLE reforzada en `Sistema`: si `ble_en == false`, no se dibuja panel BLE ni estado `OFF`.
- Ajustada la progresión visual del icono de luz en diales con más pasos en el tramo bajo `0..1000 lux`.
- Corregidas zonas de limpieza/capa en cards para evitar que valores dinámicos tapen labels técnicos como `DS18B20`.
- Subidos los indicadores mínimo/máximo de diales y mantenido `DS18B20` como icono no final pendiente de rediseño/validación.

### Documentación

- Documentada la separación de `Sistema` entre `Bip` (`sys_sound`, beeps de UI) y `Alarmas` (`sys_alarm`, alertas y timer audibles).
- Actualizados términos visibles: `Sonido` en lugar de `Ruido`, `Gráfica` en lugar de abreviaturas `Graf`, y `Termómetro`/`Termo` para la sonda externa, manteniendo `DS18B20` como identificador técnico.
- Mantenida la activación BLE secreta fuera del manual de usuario; queda tratada solo como documentación técnica/agent handoff.
- Actualizados manuales, roadmap, checklist/release y escenas de Sistema del visualizador. Añadido `AGENTS.md` para futuros agentes.

## 2026-05-20

### Revisión de producción/i18n

- Build local verificado con `py -m platformio run -e esp32dev`: `SUCCESS`.
- Tamaño reportado por PlatformIO: RAM `14.7%` (`48028` bytes de `327680`) y Flash `70.6%` (`925873` bytes de `1310720`).
- Auditoría estática de i18n: `LANG_COUNT`, `LIn(...)`, `normalizeLanguage(...)`, `L(...)` y full redraw al cambiar idioma presentes.
- Textos visibles migrados al diccionario de idiomas; la búsqueda de literales directos solo deja símbolos/no lingüísticos como cursores, separadores, placeholders y ticks numéricos.
- BLE confirmado como factory-off por defecto: `ble_en` carga `false`, `init_ble()` queda condicionado y el reset por build-hash limpia NVS en una build nueva.
- LDR documentado con rango lux `0..20000`, saturación alta a `20000 lux`, `ldr_raw` y modo `Raw ADC`.
- Sensor Zone consolidado como capa común para los seis sensores, con modos `Focus`, `Valor`, `Gráfica`, `Dial` y `Card`.
- Fixes anti-flicker documentados para dials/gauges, cards, menús/footers y `Sound VU`.
- `logs/xvba_debug.log` marcado como log local/no pertinente para este commit salvo decisión explícita del usuario.

### Documentación

- Actualizados `MANUAL_TECNICO_PBIT.md`, `CHANGELOG.md`, `docs/PRODUCTION_CHECKLIST.md`, `docs/PRODUCTION_RELEASE.md` y `docs/REPO_HYGIENE.md` con el estado de build, auditoría estática y pendientes de hardware real.
- Actualizados `README.md`, `MANUAL_DE_USUARIO_PBIT.md` y `PBIT_FUNCIONAMIENTO_ACTUAL.md` para reflejar el carrusel actual, Sensor Zone, BLE oculto, i18n completo, LDR `0..20000 lux`, Timer y reposo visible.
- Actualizados `ROADMAP_PBIT.md`, `docs/DISPLAY_AUDIT.md`, `LAB_GRAPH_UI_HANDOFF.md`, `PALETTE_AND_ICONS_PROPOSAL.md` y reglas TFT para separar trabajo resuelto, deuda de producto y validación pendiente en hardware.
- Actualizada documentación secundaria de visualizer (`ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md` y `visualizer_scenes/**/*.md`) para no arrastrar el carrusel antiguo ni rangos de LDR obsoletos.

## 2026-05-19

### Documentación

- Sincronizado el estado real del carrusel con `PBIT_ENABLE_GRAPH_LAB=1`: pantallas lab/producto iniciales y seis slots de sensor sobre `SENSOR_ZONE_SCREEN`.
- Documentada la gráfica como modo por sensor, no como pantalla separada del carrusel actual.
- Reforzado que BLE está oculto y desactivado por defecto, con acceso interno por gesto de 60 s en `Sistema`.
- Añadido checklist de producción en `docs/PRODUCTION_CHECKLIST.md`.
- Añadidas notas de repo hygiene en `docs/REPO_HYGIENE.md`.

### Firmware

- El arranque limpia NVS por build-hash antes de cargar BLE y settings, evitando que una unidad reflasheada anuncie BLE con estado anterior.
- BLE usa buffers fijos, bitmap de validez y servicio rate-limited desde la tarea de sensores; se evita `String` en el camino caliente y la notificación desde callbacks.
- LDR usa lectura ADC protegida, arranca con valores inválidos explícitos y mantiene el rango de luz documentado para producción.
- Luz y sonido ignoran lecturas inválidas en alertas y UI, evitando falsos estados de sol directo o sonido crítico.
- Lecturas ADC compartidas pasan por un guard común para reducir carreras entre calibración UI y tarea de sensores.
- El buzzer deja de llamar LEDC dentro de secciones críticas; usa mutex de tarea.
- Alert jewels de Temp/Humedad/DS18/Luz/Sonido se repintan al cambiar pantalla y se reducen clears que podían pisarlos.
