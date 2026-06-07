# Manual Técnico del P-Bit

Actualizado: 2026-06-03

Este documento describe el estado técnico actual del P-Bit a partir del firmware y la configuración presentes en este repositorio. Está pensado como base de entrenamiento para desarrollo, integración, soporte, mantenimiento y despliegue educativo.

---

## Índice

1. [Resumen técnico](#1-resumen-técnico)
2. [Plataforma base](#2-plataforma-base)
3. [Componentes electrónicos integrados](#3-componentes-electrónicos-integrados)
4. [Pinout actual del firmware y buses confirmados por hardware](#4-pinout-actual-del-firmware-y-buses-confirmados-por-hardware)
5. [Arquitectura del firmware](#5-arquitectura-del-firmware)
6. [Modelo de datos](#6-modelo-de-datos)
7. [Flujo de adquisición de sensores](#7-flujo-de-adquisición-de-sensores)
8. [Interfaz de usuario](#8-interfaz-de-usuario)
9. [Persistencia en NVS](#9-persistencia-en-nvs)
10. [BLE](#10-ble)
11. [Gestión de energía](#11-gestión-de-energía)
12. [Idiomas](#12-idiomas)
13. [Alertas y feedback](#13-alertas-y-feedback)
14. [Serial y modo laboratorio](#14-serial-y-modo-laboratorio)
15. [Limitaciones actuales](#15-limitaciones-actuales)
16. [Recomendaciones para entrenamiento y mantenimiento](#16-recomendaciones-para-entrenamiento-y-mantenimiento)
17. [Navegación y menús — referencia completa](#17-navegación-y-menús--referencia-completa)
18. [Documentos relacionados](#18-documentos-relacionados)

---

## 1. Resumen técnico

El P-Bit es un dispositivo educativo ambiental basado en ESP32 que integra sensores de temperatura, humedad, luz, sonido, humedad de suelo y temperatura externa. El sistema combina:

- adquisición de datos en tiempo real
- interfaz gráfica en pantalla TFT
- navegación con encoder rotatorio
- alertas visuales, sonoras y RGB
- persistencia de configuración en NVS
- conectividad BLE opcional, oculta y desactivada por defecto
- modos de ahorro de energía con reposo visible con `ZZZ`
- control global separado de `Bip` y `Alarmas`
- Modo demo runtime activable al arrancar con el encoder presionado

Estado de revisión de producción/i18n:

- build local verificado con `py -m platformio run -e esp32dev`
- resultado PlatformIO: `SUCCESS`
- memoria reportada por build: RAM `14.9%` (`48940` bytes de `327680`) y Flash `72.1%` (`945429` bytes de `1310720`)
- revisión estática de i18n, BLE factory-off, Sensor Zone y fixes anti-flicker completada
- ghosting/flicker validado como resuelto por ahora; mantener checks de regresión en ST7735 real
- Modo demo validado en entrada desde logos, entrada desde `Home`, splash y salida con interacción; coreografía smooth implementada en firmware y pendiente de validación visual final en hardware
- LDR `Lux` / `FC` / `Raw ADC` propagado en firmware a `LIGHT_SCREEN`, Sensor Zone, cards, dashboards, dials y gráficas mediante helper común de presentación

## 2. Plataforma base

### Microcontrolador

- MCU: `ESP32 Dev Module`
- Framework: `Arduino`
- Toolchain/proyecto: `PlatformIO`
- Archivo de entorno: `platformio.ini`

### Librerías principales

- `TFT_eSPI`
- `NimBLE-Arduino`
- `DHT sensor library`
- `DallasTemperature`
- `OneWire`
- `ESP32RotaryEncoder`
- `Preferences`

### Display

- Controlador TFT: `ST7735`
- Resolución: `128x160`
- Orientación actual: paisaje (`rotation = 1`)
- Interfaz: `SPI`
- Pin de backlight por software: no disponible
- Touch: no disponible

Nota importante:
- Esta placa no controla el brillo del display por pin.
- En esta revisión de hardware el deep sleep automático deja la TFT en blanco, así que el comportamiento activo del producto mantiene un reposo visible con `ZZZ`.

## 3. Componentes electrónicos integrados

### Sensores

- `DHT11` para temperatura ambiente y humedad relativa
- `LDR` para luz ambiental
- micrófono analógico con acondicionamiento por `LM358` y cápsula tipo `GM19767P`
- sensor capacitivo de humedad de suelo externo
- sonda `DS18B20` externa

### Interfaz y salidas

- encoder rotatorio con pulsador
- LED RGB
- buzzer pasivo
- pantalla TFT ST7735
- BLE opcional para visualización/lectura remota, oculto y `OFF` por defecto

## 4. Pinout actual del firmware y buses confirmados por hardware

### Sensores

| Función | Pin ESP32 | Observación |
|---|---:|---|
| LDR | `GPIO39` | ADC, entrada solamente |
| Micrófono | `GPIO36` | ADC, entrada solamente |
| Humedad de suelo | `GPIO35` | ADC, entrada solamente; referencia visible PCB `IO35` |
| DS18B20 | `GPIO33` | bus 1-Wire; referencia visible PCB `IO33` |
| DHT11 | `GPIO4` | lectura digital |

### Interfaz humana

| Función | Pin ESP32 | Observación |
|---|---:|---|
| Encoder A | `GPIO14` | entrada digital |
| Encoder B | `GPIO12` | entrada digital |
| Pulsador encoder | `GPIO13` | wake-up de deep sleep por `EXT0`, activo en `LOW` |

### Salidas

| Función | Pin ESP32 | Observación |
|---|---:|---|
| RGB Rojo | `GPIO5` | PWM, LED común cátodo |
| RGB Verde | `GPIO17` | PWM |
| RGB Azul | `GPIO16` | PWM |
| Buzzer pasivo | `GPIO18` | PWM |

### TFT SPI

| Señal | Pin ESP32 |
|---|---:|
| MOSI | `GPIO19` |
| SCLK | `GPIO25` |
| CS | `GPIO23` |
| DC | `GPIO22` |
| RST | `GPIO21` |

### I2C disponible en la placa

| Señal | Pin ESP32 | Observación |
|---|---:|---|
| SDA | `GPIO26` | net `SDA`, expuesta en conectores externos de 4 pines |
| SCL | `GPIO27` | net `SCL`, expuesta en conectores externos de 4 pines |

Notas de hardware:
- `GPIO36`, `GPIO39` y `GPIO35` son entradas analógicas únicamente.
- El código configura atenuación `ADC_11db` para ampliar el rango útil de lectura analógica.
- El DS18B20 usa `INPUT_PULLUP` antes del escaneo del bus.
- La TFT no comparte estos pines I2C: `DC` y `RST` del display van a `GPIO22` y `GPIO21`, respectivamente.
- La placa V3.1 sí deja un bus I2C físico disponible en `GPIO26/GPIO27`, pero el firmware actual todavía no llama a `Wire.begin(...)` ni usa sensores I2C.
- Si en una iteración futura se añade un sensor I2C como `SCD41`, la inicialización correcta debe ser explícita sobre esos pines: `Wire.begin(26, 27)`.
- Para soporte de usuario, la referencia visible de PCB es `IO33` para la sonda DS18B20 y `IO35` para Suelo; la referencia eléctrica firme del firmware sigue siendo `GPIO33`/`GPIO35`.
- Esta confirmación de pines ya no depende solo del firmware: quedó contrastada también contra los archivos KiCad `P-Bit3.kicad_sch` y `P-Bit3.kicad_pcb`.

## 5. Arquitectura del firmware

### Inicialización general

Orden real de `setup()`:

1. `Serial.begin`
2. `nvs_flash_init` (con erase automático si hay páginas corruptas)
3. incremento de contador RTC de arranque
4. reset de NVS por stamp derivado del ELF SHA256 si el firmware es nuevo (ver sección 9)
5. cálculo de nombre de dispositivo desde MAC
6. inicialización de TFT
7. inicialización de LED RGB y buzzer
8. reset del motor de alertas
9. inicialización de hardware y sensores
10. lectura del botón del encoder para detectar Modo demo en arranque en frío
11. inicialización BLE **condicional** — solo si `load_ble_enabled_store()` devuelve `true`; por defecto es `false` de fábrica y se resetea a `false` con cada nuevo flash
12. detección de tipo de arranque (wake desde sleep o arranque en frío)
13. carga o selección de idioma; si se pidió Modo demo, carga idioma sin mostrar selector
14. inicialización de Sensor Zone
15. inicialización del encoder
16. activación runtime de Modo demo si corresponde
17. creación de tareas FreeRTOS

### Tareas y responsabilidades

#### UI Task

- Función: `switch_screen`
- Núcleo: `Core 1`
- Stack asignado: `4096`
- Rol:
  - router visual
  - snapshots seguros de lectura
  - overlays de energía
  - refresco selectivo por pantalla

#### Sensor Task

- Función: `sensor_reading_task`
- Núcleo: `Core 0`
- Stack asignado: `4096`
- Rol:
  - leer sensores rápidos y lentos
  - actualizar `global_readings`
  - marcar `g_sensor_data_ready`
  - emitir datos por BLE si `ble_en == true`
  - enviar línea CSV por Serial solo si `PBIT_ENABLE_SERIAL_PLOTTER=1`

#### Loop principal

- Corre en el contexto principal
- Rol:
  - `rotaryEncoder.loop()`
  - `poll_rotary_aux()`
  - `loop_buzzer()`
  - `demo_mode_service()`
  - lógica de inactividad
  - transición entre `ACTIVE` e `IDLE` visible

## 6. Modelo de datos

La estructura central de medición es `Reading`:

- `humidity`
- `temperature`
- `ldr`
- `ldr_raw`
- `mic`
- `soil_humidity`
- `temp_ds18b20`

Sincronización:

- `global_readings` se protege con `portMUX_TYPE readings_mux`
- UI y BLE consumen snapshots bajo sección crítica

Convenciones de datos:

- `NAN` para algunas lecturas inválidas del DHT o suelo
- `-999.0f` como sentinel de ausencia en `DS18B20`

Rangos canónicos usados por UI, gauges, cards, gráficas y RGB:

| Sensor | Lectura firmware | Rango visual canónico | Nota |
|---|---:|---:|---|
| Temperatura DHT11 | °C | `0..50 °C` / `32..122 °F` | Rango útil del DHT11 |
| Humedad aire DHT11 | `%` | `0..100 %` | Porcentaje relativo |
| Luz LDR | lux aproximados / `FC` / `Raw ADC` | `0..8000 lux`, `0..4095 raw` | Curva empírica v1; categorías/alertas usan lux interno |
| Sonido MIC | `%` relativo | `0..100 %` | No dB SPL |
| Suelo capacitivo | `%` calibrado | `0..100 %` | Mapeado desde seco/húmedo calibrados |
| Termómetro DS18B20 | °C | `-55..+125 °C` / `-67..+257 °F` | Rango técnico de la sonda; lecturas fuera de rango se tratan como inválidas |

Marcas de diales:

- Temperatura DHT11, Humedad aire y Suelo muestran marcas de rangos por defecto.
- Luz y Sonido guardan `Ver límites` en NVS y solo dibujan marcas si esa opción está activa o si el usuario guarda sus rangos/niveles.
- Termómetro/DS18B20 no dibuja límites de alarma por defecto; solo muestra una marca fija de referencia en `0 °C`.
- En Luz, el dial usa una progresión visual logarítmica para que los primeros `1000 lux` tengan cambios más visibles.

## 7. Flujo de adquisición de sensores

### Sensores rápidos

Se actualizan en el lazo rápido de `sensor_reading_task`:

- luz
- sonido
- suelo

#### Luz

Lógica actual:

- divisor LDR actual: a mayor luz, menor RAW ADC; a mayor oscuridad, mayor RAW ADC
- filtro hardware adicional con `C4 = 1uF`
- lectura cruda disponible en `ldr_raw` para el modo `Raw ADC`; se estabiliza con media móvil de 10 lecturas ADC
- cálculo de lux aproximado a partir del RAW promediado mediante curva empírica v1
- rango útil normalizado a `0..8000 lux`
- `FC` calculado desde el lux calibrado con `lux / 10.764`

##### Calibración empírica LDR v1 — 2026-05-29

Esta es la primera curva de LDR derivada de una comparación manual entre el RAW ADC del P-Bit y un luxómetro externo. El modelo anterior era teórico tipo GL5528 (`R10`, `gamma`, resistencia de referencia) y saturaba el tramo alto a `20000 lux`; en esta placa sobreestimaba con fuerza la luz alta, por ejemplo RAW `64` quedaba cerca de `19600 lux` cuando la muestra manual marcaba unos `3650 lux`.

Durante el análisis se trató el punto `120 - 800` como dato invertido y se corrigió a `800 - 120`, porque así mantiene la relación esperada del divisor: RAW más bajo significa más luz, RAW más alto significa menos luz.

Muestra base usada para la curva:

| RAW ADC | Luxómetro | Firmware v1 |
|---:|---:|---:|
| 5 | 7400 | 6963 |
| 43 | 4280 | 4408 |
| 64 | 3650 | 3548 |
| 102 | 2490 | 2511 |
| 105 | 2200 | 2448 |
| 122 | 2020 | 2134 |
| 494 | 380 | 313 |
| 750 | 138 | 138 |
| 800 | 120 | 120 |
| 816 | 118 | 115 |
| 845 | 89 | 107 |
| 1007 | 64 | 71 |
| 1111 | 59 | 56 |
| 1220 | 45 | 44 |
| 1270 | 42 | 40 |
| 1700 | 17 | 17 |

El ajuste de potencia sobre la muestra queda cercano a:

`lux ≈ 10.22 * ((4095 - raw) / (raw + 141))^1.962`

En firmware se usa una aproximación equivalente y barata para ESP32:

`x = (4095 - raw) / (raw + 150)`

`lux = clamp(10 * x * x, 0, 8000)`

La aproximación evita `powf` en el lazo rápido, mantiene el error medio de la muestra alrededor de `6%` y deja el máximo práctico cerca del punto más luminoso medido. Si cambia el LDR, el divisor, la carcasa o la geometría de medición, esta tabla debe repetirse y versionarse como una nueva curva.

La UI de luz usa un helper común de presentación (`include/light_display.h` / `src/light_display.cpp`) para tres modos: `Lux`, `FC` y `Raw ADC`. `FC` es foot-candle calculado desde lux (`lux / 10.764`). `Raw ADC` usa la lectura ADC cruda promediada y se muestra como `raw` en campos compactos. La propagación visual está implementada en firmware para `LIGHT_SCREEN`, Sensor Zone (`Card`, `Valor`, `Focus`, `Gráfica`, `Dial`), Home cards, dashboards y gráficas. Categorías, alertas y RGB siguen usando lux interno; en vistas de solo Luz el RGB permanece apagado para no contaminar el LDR. Las gráficas RAW usan `g_graph_light_raw`.

#### Sonido

Lógica actual:

- ventana de captura de `50 ms`
- medición de amplitud pico a pico
- mapeo a `0..100`
- suavizado con EMA

Importante:
- no es un sonómetro calibrado en dB SPL
- hoy representa intensidad relativa útil para educación y alertas

#### Suelo

Lógica actual:

- lectura promedio de ADC
- detección heurística de sensor desconectado
- mapeo a porcentaje usando dos puntos de calibración:
  - referencia seca
  - referencia húmeda
- suavizado por EMA

### Sensores lentos

Se actualizan cada `1000 ms`:

- `DHT11` temperatura
- `DHT11` humedad
- `DS18B20`

Protecciones:

- el DHT invalida lectura tras fallos repetidos
- DS18B20 reintenta escaneo del bus si no detecta dispositivos

## 8. Interfaz de usuario

### Navegación principal

Orden real del carrusel actual con `PBIT_ENABLE_GRAPH_LAB=1` (12 posiciones, circular):

`HOME -> CLIMA -> MULTI -> SONIDO VU -> TEMPERATURA -> HUMEDAD -> LUZ -> SONIDO -> SUELO -> TERMÓMETRO -> TIMER -> SISTEMA`

Nota de nomenclatura: `TERMÓMETRO` es el título visible de la sonda externa; `DS18B20`, `DS18B20_SCREEN` y `SZ_DS18` se conservan como identificadores técnicos de firmware/hardware.

Implementación:

- Las primeras 4 posiciones son pantallas lab/producto de solo lectura: `LAB_HOME_CARDS_SCREEN`, `LAB_DUAL_TH_SCREEN`, `LAB_WIDGET_MIX_SCREEN`, `LAB_SOUND_VU_STACK_SCREEN`.
- Las 6 posiciones de sensor reutilizan `SENSOR_ZONE_SCREEN`; al entrar en cada slot, `rotary.cpp` llama a `sz_set_sensor(...)`.
- `SENSOR_ZONE_SCREEN` conserva un modo visual por sensor: `Focus`, `Valor`, `Gráfica`, `Dial`, `Card`.
- La pulsación corta en una posición de sensor ejecuta `sz_next_viz()`.
- La pulsación larga abre el menú clásico del sensor activo (`TEMP_SCREEN`, `HUMIDITY_SCREEN`, `LIGHT_SCREEN`, `SOUND_SCREEN`, `SOIL_SCREEN` o `DS18B20_SCREEN`).
- `GRAPH_SCREEN` existe en el firmware; con el flag actual se usa como renderer del modo `Gráfica` dentro de `SENSOR_ZONE_SCREEN`.

Si `PBIT_ENABLE_GRAPH_LAB` se compila a `0`, el carrusel cae al rango clásico `TEMP_SCREEN -> GRAPH_SCREEN`.

### Sensor Zone

`SENSOR_ZONE_SCREEN` es la capa común para los seis sensores principales:

- Temperatura DHT (`SZ_TEMP`)
- Humedad ambiente (`SZ_HUM`)
- Luz (`SZ_LIGHT`)
- Sonido (`SZ_SOUND`)
- Suelo (`SZ_SOIL`)
- Termómetro / DS18B20 (`SZ_DS18`)

Cada sensor conserva su modo visual persistido (`sz_v0` .. `sz_v5`) y `sz_sync_renderer(...)` sincroniza el sub-renderer activo solo cuando cambia pantalla, sensor o modo. Esta arquitectura evita invalidar caches en cada tick de lectura y reduce flicker en dials, cards, gráficas y vistas de valor.

Los sensores externos conectables por el usuario comparten estado runtime de ausencia en `external_sensor_state.*`: `SZ_DS18` falta si `temp_ds18b20 < -100` y `SZ_SOIL` falta si `soil_humidity` es `NaN`. El helper también centraliza `ST_CHECK_DS18`/`ST_CHECK_SOIL` y colores atenuados basados en la paleta del sensor. No persiste nada en NVS; al reconectar, la siguiente lectura válida restaura colores y datos normales.

`sensor_connection_notice.*` observa únicamente muestras físicas reales desde `io.cpp`, no el snapshot visual ni Demo Mode. Tras la primera muestra de baseline, si `SZ_DS18` o `SZ_SOIL` pasan de desconectados a conectados o de conectados a desconectados, encola un splash semafórico de `1500 ms`: fondo verde para `CONECTADO` y fondo rojo para `DESCONECTADO`, con tres líneas (`CONECTADO`/`DESCONECTADO`, `Sensor Suelo`/`Sensor DS18B20`, `IO35`/`IO33`). No dispara avisos por el estado inicial al boot, no usa NVS, no añade sonido/LED especial y no interrumpe Demo Mode.

Para flujos transitorios como Modo demo existen `sz_set_sensor_runtime(...)` y `sz_set_viz_runtime(...)`. Cambian sensor/modo solo en RAM, piden full redraw y no escriben NVS.

### Modo demo runtime

Activación:

- En arranque en frío, `setup()` lee `DI_ENCODER_SW` después de `init_hw()`.
- `run_boot_sequence(true)` sigue muestreando el botón durante logos, gaps y retención final para tolerar mejor el gesto de arranque.
- Si el encoder se detecta presionado (`LOW`) en cualquiera de esos puntos, se omite el selector de idioma y se activa `demo_mode_start()` después de `init_rotary()`.
- El primer release del botón de arranque se consume con `demo_mode_consume_boot_release()` para no salir del demo accidentalmente.
- Una pulsación larga en `LAB_HOME_CARDS_SCREEN` también activa el Modo demo sin consumir un release de arranque.

Comportamiento:

- `src/demo_mode.cpp` define una banda runtime de escenas con duración variable (`6..10 s`) para dar ritmo visual a la demo.
- Al activarse, `tft_display.cpp` muestra una señal visual breve `MODO DEMO / Iniciando demo` antes de entrar a la primera escena.
- Con `PBIT_ENABLE_GRAPH_LAB=1`, recorre Home, Clima, Multi, Sonido VU, seis escenas de Sensor Zone y Timer.
- Las escenas de Sensor Zone usan setters runtime, por lo que no modifican el sensor ni el modo visual guardados.
- Si `kDemoSimulatedReadings == true`, `demo_mode_apply_simulated_readings(...)` modifica solo `g_ui_readings_snapshot` para animar valores visuales; no toca `global_readings`, NVS, BLE ni lecturas físicas.
- `demo_mode_value_refresh_ms()` fija un refresco demo dedicado de `220 ms`, separado de la cadencia normal de sensores/gráficas.
- `demo_mode_graph_values(...)` genera histórico sintético para `Graph` durante demo, de modo que las gráficas muestren intención visual sin depender de buffers físicos previos.
- Las curvas simuladas usan suavizado por sensor; el RAW del LDR se calcula con la inversa de la curva empírica v1 para que `Raw ADC` sea coherente con el lux simulado en calibración/demostración.
- Cualquier giro o pulsación posterior del encoder ejecuta `demo_mode_stop()`, reconfigura límites del encoder y devuelve control al usuario.
- Mientras `demo_mode_is_active()` es `true`, el reposo automático queda bloqueado.

Estado actual:

- entrada desde logos con encoder presionado, entrada desde `Home` con pulsación larga, señal visual y salida con interacción confirmadas en hardware.
- coreografía smooth implementada en firmware; queda validación visual final en hardware para ajustar dwell/refresco si hiciera falta.

### Fixes anti-flicker

Ghosting/flicker queda cerrado por ahora tras la revisión y validación práctica actual. Mantener checks de regresión cuando se cambien limpiezas, sprites, textos localizados o ritmos de Demo Mode. Las mitigaciones aplicadas cubren:

- dials/gauges: caches por sensor y actualizaciones por `data_dirty`/`chrome_dirty`; los diales dibujan ticks de umbral/rango sobre el arco usando los rangos guardados
- cards: limpieza dirigida del rectángulo de valor, banda superior ampliada para estados (`Óptimo`, `Seco`, etc.) y cache de estado
- menús y footers: clears por bandas en vez de `fillScreen` constante
- `Sound VU`: sprites/cache para medidor y badge, con push de zona dinámica sin redibujar chrome completo
- cambio de pantalla/idioma: `runtime_request_ui_full_redraw()` fuerza un frame limpio cuando corresponde

### Gestos del encoder

- giro: cambia de pantalla o modifica opciones/valores
- pulsación corta:
  - confirma pasos dentro de menú
  - en pantallas de sensor individual: alterna el modo de visualización del sensor
- pulsación larga:
  - en pantallas de sensor individual: abre el menú de configuración del sensor
  - en `Timer`: abre el editor de duración `HH:MM:SS` si está idle o resetea si ya estaba corriendo/pausado
  - en `Sistema`: mantener 30 s sin girar activa la pantalla oculta de BLE

Tiempos actuales:

- apertura de menú: aprox. `1.2 s`
- reset de timer: aprox. `1.0 s`

### Pantallas y menús disponibles

#### Home

Vista global de todos los sensores en cards. Solo lectura; no tiene menú.

#### Clima

Temperatura ambiente y humedad del aire en card combinado. Solo lectura; no tiene menú.

#### Multi

Múltiples sensores con widgets en una sola pantalla. Solo lectura; no tiene menú.

#### Sonido VU

Nivel de sonido ambiental en barras apiladas. Solo lectura; no tiene menú.

#### Temperatura DHT

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_TEMP`
- pulsación corta: alterna modo de visualización
- la unidad `C/F` es global, persistente y compartida con `Termómetro` (`DS18B20` técnico)
- menú `Límites / Unidad / Alertas / Reset / Salir`

#### Humedad del aire

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_HUM`
- pulsación corta: alterna modo de visualización
- menú `Rangos / Alertas / Reset / Salir`
- usa dos umbrales: `Seco` y `Muy húmedo`

#### Luz

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_LIGHT`
- pulsación corta: alterna modo de visualización
- menú `Rangos / Modo / Alertas / Reset / Salir`
- modos de vista: `Lux`, `FC`, `Raw ADC`
- propagación implementada: valor/unidad visible siguen el modo activo en pantalla clásica, Sensor Zone, cards, dashboards, dials y gráficas

#### Sonido

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_SOUND`
- pulsación corta: alterna modo de visualización
- menú `Niveles / Alertas / Reset / Salir`
- niveles interpretativos por umbrales, no calibración acústica física

#### Suelo

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_SOIL`
- pulsación corta: alterna modo de visualización
- menú `Calibrar sensor / Rangos / Alertas / Reset / Salir`
- dentro de la calibración, pulsación larga cancela el subpaso sin escribir NVS; en el menú raíz sale de configuración
- clasificación actual derivada desde dos umbrales editables: `Muy seco`, `Seco`, `Óptimo`, `Húmedo`, `Muy húmedo`

#### Termómetro / DS18B20

- en carrusel actual: slot visible `Termómetro` de `SENSOR_ZONE_SCREEN` con sensor técnico `SZ_DS18`
- pulsación corta: alterna modo de visualización
- la unidad `C/F` es global, persistente y compartida con `Temperatura DHT`
- menú `Corrección / Límites / Unidad / Alertas / Reset / Salir`

#### Sistema

- menú `Bip / Alarmas / Reposo / Idioma / Reset / Salir`
- pantalla principal con card general y panels internos: `ID` con badge BLE condicional, `Tiempo`, `Idioma` y audio. El bloque inferior de audio no cambia de layout si BLE está habilitado.
- menú raíz renderizado como grid 2×3 para evitar solapes en ST7735 160×128
- `Bip` controla `g_sound_enabled` / NVS `sys_sound`: beeps de UI, confirmaciones y navegación.
- `Alarmas` controla `g_alarm_sound_enabled` / NVS `sys_alarm`: audio de alertas y final de cuenta regresiva del `Timer`.

#### Timer

- no tiene submenú dedicado
- corto: iniciar/pausar
- largo en idle: abrir editor de duración `HH:MM:SS`
- largo en running/paused: resetear
- `00:00:00` funciona como cronómetro ascendente
- cualquier valor mayor que `00:00:00` funciona como cuenta regresiva
- el render usa formato adaptativo: `MM:SS:CC` por debajo de una hora y `HH:MM:SS` desde una hora
- el editor usa dos capas:
  - selección de campo `HH / MM / SS`
  - edición del valor del campo seleccionado
- short press alterna entre seleccionar campo y editar valor
- long press en modo selección confirma y guarda la nueva duración
- en runtime, la duración objetivo solo se dibuja en la banda inferior cuando hay cuenta regresiva
- al terminar una cuenta regresiva, la UI pasa a rojo y dispara una alarma intermitente corta si `Alarmas` está activo

#### Modo Gráfica (`SZ_VIZ_GRAPH` / `GRAPH_SCREEN`)

Con `PBIT_ENABLE_GRAPH_LAB=1`, `GRAPH_SCREEN` se usa como sub-renderer del modo `SZ_VIZ_GRAPH` dentro de `SENSOR_ZONE_SCREEN`.

La infraestructura de buffers circulares está activa para los 6 sensores (160 muestras a 1 muestra/s, `g_graph_mux` para acceso cross-core). `sz_sync_renderer(...)` sincroniza el sensor activo con `graph_set_sensor(...)` antes de dibujar.

Cuando el flag se compila a `0`, `GRAPH_SCREEN` queda disponible como pantalla independiente del carrusel clásico.

Archivos clave:

- `include/graph_buffer.h` / `src/graph_buffer.cpp` — buffer circular y acceso thread-safe
- `include/ui_graph.h` / `src/ui_graph.cpp` — render de gráfica reutilizado por `SENSOR_ZONE_SCREEN`
- `src/io.cpp` — push a buffers dentro del bloque de sensores lentos (cada 1 s)
- `include/sensor_zone.h` / `src/sensor_zone.cpp` — selección de sensor y modo visual persistente

## 9. Persistencia en NVS

Namespace utilizado:

- `pbit`

### Claves actuales de configuración

#### Suelo

- `soil_dry`
- `soil_wet`
- `soil_thr_dry`
- `soil_thr_opt`
- `soil_thr_moi`
- `soil_aen`

#### Humedad ambiente

- `hum_dry_max`
- `hum_comf_max`
- `hum_alert_en`

#### DS18B20

- `d18_off`
- `d18_alow`
- `d18_ahigh`
- `d18_aen`

#### Sonido

- `snd_quiet`
- `snd_norm`
- `snd_loud`
- `snd_aen`
- `snd_marks`

#### Temperatura DHT

- `tmp_low`
- `tmp_high`
- `tmp_aen`

#### Luz

- `lgt_dim`
- `lgt_ind`
- `lgt_bri`
- `lgt_mode`
- `lgt_aen`
- `lgt_marks`

#### Sistema

- `sys_sleep`
- `sys_sound` (bool, default `false`) — `Bip`: beeps de interfaz, navegación y confirmaciones
- `sys_alarm` (bool, default `false`) — `Alarmas`: audio de alertas de sensores y final del `Timer`
- `sys_unit_f` (bool, default `false`) — unidad global de temperatura: `false=Celsius`, `true=Fahrenheit`

#### Sensor zone

- `sz_sen`
- `sz_v0` .. `sz_v5`

#### BLE

- `ble_en` (bool, default `false`) — controla si el BLE se inicializa en el arranque

Esta clave se borra junto con el resto del namespace `pbit` cuando cambia la huella del firmware (ver firmware-stamp reset a continuación).

#### Idioma

- `lang` (`uint8_t`, default `0 = LANG_ES`) — idioma activo de la UI
- acceso encapsulado: `has_language_store()` / `load_language_store()` / `save_language_store(uint8_t)` en `include/settings_store.h` y `src/settings_store.cpp`
- `src/lang_select.cpp` delega en esas funciones (no abre `Preferences` directamente)
- en cold boot se muestra el selector hasta que exista la clave `lang`; esto cubre NVS limpia, reset general y reinicios ocurridos antes de confirmar idioma
- el valor leído se normaliza con `normalizeLanguage(...)`; `LANG_COUNT` define el límite de idiomas soportados
- `LIn(language, key)` permite renderizar textos en un idioma explícito
- `L(key)` traduce usando el idioma activo

## 10. BLE

### Feature gate — BLE desactivado por defecto

El BLE del P-Bit sale desactivado de fábrica.

El flag de control es la clave NVS `ble_en` (bool, namespace `pbit`, default `false`).

Comportamiento:

- En cada nuevo binario, la clave se borra porque `src/main.cpp` calcula un stamp compacto a partir del ELF SHA256 (`esp_ota_get_app_elf_sha256`) y llama a `clear_all_settings_store()` si cambia frente a `fw_stamp`. `ble_en` vuelve a `false` (su default implícito al no existir).
- En reinicios normales entre flashes, el valor persistido se respeta: si el usuario activó el BLE, sigue activo.
- `init_ble()` solo se llama si `load_ble_enabled_store()` devuelve `true`. Si devuelve `false`, el stack NimBLE nunca se inicia y el dispositivo no emite señal BLE.
- El indicador BLE de `Sistema` solo se dibuja si `ble_en == true`; aparece como badge compacto dentro de la card superior de ID, no como panel propio. En estado de fábrica queda oculto.

Nota de producción: si se flashea sobre una unidad de desarrollo que tenía BLE activado, validar antes de entregar que el dispositivo no anuncia BLE. El reset por firmware-stamp se ejecuta antes de cargar BLE/settings, así que un binario nuevo debe volver a `OFF` desde el primer arranque.

### Pantalla BLE Toggle (gesto secreto de producción)

La pantalla `BLE_TOGGLE_SCREEN` es una función de fábrica. No aparece en el carrusel de navegación normal.

#### Cómo activarla

Desde la pantalla `Sistema`, mantener el encoder presionado durante **30 segundos** sin girarlo.

Flujo interno:

1. A los ~1.2 s el menú de Sistema se abre normalmente (longpress estándar).
2. El conteo de tiempo continúa en el fondo; el flag `g_ble_secret_eligible` se mantiene desde el momento en que empezó la pulsación.
3. Al cumplirse 30 s desde el inicio de la pulsación, `poll_rotary_aux()` detecta la condición (`g_ble_secret_eligible && !g_ble_secret_fired && hold >= 30000 ms`), navega a `BLE_TOGGLE_SCREEN` y suprime el callback de release.
4. El sistema emite un tono de 1800 Hz por 150 ms como confirmación audible.

#### Cómo funciona la pantalla

- Fondo azul puro full-screen (sin header estándar).
- Ícono Bluetooth blanco XL centrado en la parte superior.
- Label `BLUETOOTH` centrado debajo del ícono.
- Dos botones: `OFF` (izquierda) y `ON` (derecha). El seleccionado se resalta en blanco con texto azul; el otro queda con borde dimmed.
- El encoder gira entre `OFF` (valor 0) y `ON` (valor 1).
- La selección inicial carga el valor actual de `ble_en` desde NVS.
- Una pulsación corta confirma: llama a `save_ble_enabled_store(true/false)`, muestra una pantalla de confirmación azul con "REINICIANDO..." y llama a `esp_restart()` tras 700 ms.
- No hay forma de salir sin confirmar excepto apagando el dispositivo.

#### Archivos clave

- `include/ui_ble_toggle.h` / `src/ui_ble_toggle.cpp` — pantalla y estado
- `src/rotary.cpp` — detección del gesto (variable `g_ble_secret_eligible`, constante `BLE_SECRET_PRESS_MS = 30000`)
- `include/settings_store.h` / `src/settings_store.cpp` — `load_ble_enabled_store()` / `save_ble_enabled_store()`
- `src/main.cpp` — `if (load_ble_enabled_store()) { init_ble(); }`

### Modo BLE activo

Cuando `ble_en == true`, el dispositivo inicia publicidad BLE con nombre derivado de la MAC:

- formato: `PBIT-XXXX`
- nombre publicado en `scan response` (`adv->setScanResponse(true)` + `scanData.setName(dev_name)` en `src/ble.cpp`), de modo que escaneos BLE externos con filtro `namePrefix=PBIT-` resuelven el dispositivo correctamente.

### Servicios y características

#### Servicio principal propietario

- UUID servicio: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- UUID característica: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- propiedades:
  - `NOTIFY`
  - `WRITE`

Uso:
- si el cliente escribe un byte inicial `0x01`, el equipo envía un paquete inmediato

#### Servicio legado

- servicio: `0x181A` (`Environmental Sensing`)
- característica: `0x2A6E`
- propiedad: `NOTIFY`

### Formato del paquete binario

Longitud total:

- `20 bytes`

Cabecera:

- byte 0: `0x02`
- byte 1: `0x00`

Campos posteriores en grupos de 3 bytes:

- ID `1`: temperatura DHT en `int16 x10`
- ID `2`: humedad en `uint16 x10`
- ID `3`: luz
- ID `4`: sonido
- ID `5`: suelo
- ID `6`: DS18B20 en `int16 x10`

Endianess:

- little-endian

### Salida JSON legado

También se genera un JSON compacto con claves:

- `temp`
- `hum`
- `ldr`
- `mic`
- `soil`
- `ds18`

## 11. Gestión de energía

### Modos

- `POWER_ACTIVE`
- `POWER_IDLE`
- `DEEP SLEEP`

### Lógica actual

- si el timeout de reposo es `0`, el reposo automático se desactiva
- con timeout activo:
  - entra en `IDLE`
  - muestra aviso `ZZZ`
  - permanece en reposo visible hasta que el usuario interactúa

Bloqueos de reposo:

- menús abiertos
- timer corriendo
- Modo demo activo
- cliente BLE conectado en el momento de evaluar reposo

### Wake-up

- fuente: `EXT0`
- pin: `GPIO13`
- nivel activo: `LOW`

Detalle importante:
- como no hay control de backlight por hardware, el flujo automático actual no apaga la TFT ni entra en deep sleep automático; el equipo queda en reposo visible mostrando `ZZZ` hasta que el usuario lo despierte.

## 12. Idiomas

Idiomas soportados:

- Español
- Catalán
- English

Comportamiento:

- en encendido en frío se muestra selector de idioma
- el código conserva soporte de wake desde deep sleep, pero el flujo normal del producto usa reposo visible con `ZZZ`
- el idioma puede cambiarse desde `Sistema`
- al guardar idioma se solicita full redraw con `runtime_request_ui_full_redraw()`
- el loop de UI consume el evento con `runtime_take_ui_full_redraw()` y reinicia caches visuales para evitar mezcla de idiomas

Estado actual tras auditoría estática:

- los textos visibles de UI están migrados al diccionario mediante `L(...)` o `LIn(...)`
- las unidades visibles de menú, incluido `FC` para foot-candle, usan clave del diccionario (`ST_FC_UNIT`)
- las cadenas directas restantes localizadas por búsqueda son símbolos/no lingüísticas (`>`, `---`, separadores, ticks numéricos o `ZZZ`)
- `LANG_COUNT`, `LIn(...)` y `normalizeLanguage(...)` están presentes y se usan en selector, menú de sistema y renderizado del idioma activo

## 13. Alertas y feedback

Canales de feedback actualmente presentes:

- color en pantalla
- LED RGB
- buzzer

El buzzer tiene dos permisos globales independientes:

- `Bip` (`g_sound_enabled`, NVS `sys_sound`) habilita sonidos de interacción: clicks de encoder, confirmaciones y beeps de menú.
- `Alarmas` (`g_alarm_sound_enabled`, NVS `sys_alarm`) habilita sonidos automáticos de alerta y el aviso audible del `Timer`.

`AlertEngine` recibe el estado de `Alarmas` desde `sensor_reading_task`, por lo que desactivar `Bip` no silencia alertas ni el final del temporizador. Desactivar `Alarmas` mantiene colores, LED RGB y UI, pero evita audio automático.

El LED RGB sigue la visualización activa en pantallas de sensores y Timer: su color deriva de `sensor_visuals.*`, la misma fuente cromática usada por los gauges/diales. Así, frío/caliente, seco/húmedo, sonido bajo/alto o estados del Timer se comunican igual en pantalla y en LED. Excepción deliberada: en cualquier vista de solo Luz el RGB permanece apagado para no contaminar la lectura del LDR; en pantallas multisensor puede permanecer activo.

Ejemplos:

- temperatura baja: azul
- temperatura alta: rojo
- humedad muy baja: naranja
- humedad muy alta: rojo
- suelo óptimo: verde

Nota técnica:
- la UX visual de alertas sigue siendo un área activa de refinamiento; el sistema ya es funcional, pero puede seguir iterando visualmente.

## 14. Serial y modo laboratorio

El firmware incluye una salida CSV-like por Serial para uso con el Arduino Serial Plotter y actividades de captura de datos. Esta salida está desactivada por defecto en producción y se controla con el flag de compilación `PBIT_ENABLE_SERIAL_PLOTTER` en `include/config.h`.

Para activarla: cambiar `#define PBIT_ENABLE_SERIAL_PLOTTER 0` a `1` y recompilar.

Cuando está activa, emite por Serial cada ciclo del sensor task:

`Temp:..., Hum:..., Luz:..., Sonido:..., Suelo:..., DS18:...`

Uso previsto:

- inspección rápida de lecturas durante desarrollo
- captura de datos en modo laboratorio
- uso con Arduino Serial Plotter
- actividades STEAM de registro y comparación con el IDE conectado

## 15. Limitaciones actuales

- el timer ya tiene edición directa `HH:MM:SS` y cuenta regresiva, pero todavía no tiene automatizaciones de experimento
- el módulo de sonido trabaja por umbrales interpretativos, no por calibración acústica absoluta
- la UX visual de alertas, gráfica y algunos detalles de layout todavía pueden seguir afinándose
- no existe control de brillo del display por hardware
- las pruebas de hardware real post-build siguen pendientes: escaneo BLE externo, sensores físicos, feedback RGB/buzzer y reposo en unidad flasheada

## 16. Recomendaciones para entrenamiento y mantenimiento

- validar siempre la serigrafía real de conectores externos frente a la revisión de placa
- recalibrar suelo si cambia el sensor o la fuente de alimentación
- no interpretar sonido como medición en dB certificada
- en pruebas de luz, recordar que el RGB se apaga a propósito en esa pantalla
- si en una iteración futura se reactiva deep sleep, habrá que revalidar la TFT y el wake por encoder en hardware
- mantener sincronizados `docs/ROADMAP.md` y `docs/TECHNICAL.md` (sección 17) cuando cambie el firmware o la navegación
- seguir `docs/PRODUCTION_CHECKLIST.md` antes de entregar builds o unidades

## 17. Navegación y menús — referencia completa

La estructura de menús y flujos de encoder está documentada aquí directamente para que este documento sea la fuente única de referencia técnica.

### Alcance real del carrusel

Con `PBIT_ENABLE_GRAPH_LAB=1`:

`HOME → CLIMA → MULTI → SONIDO VU → TEMPERATURA → HUMEDAD → LUZ → SONIDO → SUELO → TERMÓMETRO → TIMER → SISTEMA`

- `BOOT_SCREEN` existe en la enum pero no forma parte de la navegación con encoder.
- Las seis posiciones de sensor son slots de `SENSOR_ZONE_SCREEN`.
- `DS18B20_SCREEN` sigue existiendo como menú/configuración técnica de la sonda Termómetro (`SZ_DS18`).
- Los menús activos bloquean el reposo automático.
- El encoder se reconfigura por estado: menús raíz y selectores binarios son circulares; ediciones numéricas no envuelven.

### Arranque — Selector de idioma

En cold boot: selector visible en `Español` / `Catalán` / `English` cuando `lang` no existe en NVS. Encoder en rango `0..2` circular. Al confirmar, idioma se guarda y pantalla se limpia. Idioma preseleccionado = último guardado si existe; default = Español.

### Acciones rápidas fuera de menú

| Pantalla | Pulsación corta | Pulsación larga |
|---|---|---|
| TEMP / TERMÓMETRO | Alterna unidad global `C/F` | Abre menú (~1.2 s) |
| HUMEDAD / LUZ / SONIDO / SUELO | Cambia modo visual del sensor | Abre menú (~1.2 s) |
| SISTEMA | Alterna `Bip ON/OFF` | Abre menú (~1.2 s) |
| TIMER | Inicia o pausa | Abre editor HH:MM:SS si idle; resetea si corriendo/pausado (~1.0 s) |

En Modo demo, cualquier giro o pulsación corta/larga posterior al release inicial sale del demo antes de ejecutar la acción normal. Desde `Home`, una pulsación larga activa el Modo demo.

### Menús por sensor

#### Temperatura DHT

Opciones raíz: `Límites / Unidad / Alertas / Reset / Salir`

- **Límites**: edita `Límite bajo` → `Límite alto` → Guardar. Validación: alto > bajo. Rango: 0..50 °C interno (en la unidad visible).
- **Unidad**: `Celsius` / `Fahrenheit`. Compartida con Termómetro y persistida en `sys_unit_f`.
- **Alertas**: `OFF` / `ON`.
- **Reset**: `NO` / `SI`. Restaura límites, alertas y unidad a Celsius.

#### Humedad del aire

Opciones raíz: `Rangos / Alertas / Reset / Salir`

- **Rangos**: edita `Seco` → `Muy húmedo` → Guardar. Validación: húmedo > seco.

#### Luz

Opciones raíz: `Rangos / Modo / Alertas / Ver límites / Reset / Salir`

- **Rangos**: `Max penumbra` → `Max interior` → `Max brillante` → Guardar. Rango editable: 10..8000. Validación: brillante > interior > penumbra.
- **Modo**: `Lux` / `FC` / `Raw ADC`.
- **Ver límites**: muestra u oculta las marcas de rango en el dial. Se guarda en NVS.
- Esta opción afecta el valor/unidad visible de Luz en pantalla clásica, Sensor Zone (`Card`, `Valor`, `Focus`, `Gráfica`, `Dial`), Home cards, dashboards y gráficas. `FC` convierte el lux mostrado a foot-candle y `Raw ADC` muestra la lectura cruda promediada. Barras, categorías y alertas conservan lux interno cuando representan rangos ambientales.

#### Sonido

Opciones raíz: `Niveles / Alertas / Ver límites / Reset / Salir`

- **Niveles**: `Max silencio` → `Max normal` → `Max alto` → Guardar. Validación: alto > normal > silencio.
- **Ver límites**: muestra u oculta las marcas de nivel en el dial. Se guarda en NVS.
- Sin alerta sonora propia (no contaminar lectura del micrófono).

#### Suelo

Opciones raíz: `Calibrar sensor / Rangos / Alertas / Reset / Salir`

- **Calibrar sensor**: `Seco al aire` → `En agua` → Guardar o Error. Validación: seco_raw > húmedo_raw, diferencia ≥ 300.
- **Cancelar calibración**: una pulsación larga durante `Seco al aire`, `En agua`, `Rangos`, `Alertas` o confirmación de `Reset` vuelve al menú de Suelo sin escribir NVS. Desde el menú raíz, la pulsación larga sale de la configuración.
- **RAW live**: durante `Seco al aire` / `En agua`, la pantalla muestrea cada 250 ms con deadband de 3 cuentas ADC y redibuja solo la card del valor después del shell inicial para evitar flicker.
- **Rangos**: `Seco` → `Húmedo` → Guardar. Validación: seco < húmedo, todos en 0..100.
- Clasificación derivada: `0..Seco/2` = `Muy seco`; `Seco/2..Seco` = `Seco`; `Seco..Húmedo` = `Óptimo`; `Húmedo..(Húmedo+100)/2` = `Húmedo`; tramo final = `Muy húmedo`.
- Color visual compartido: `pbit_soil_visual_color()` usa los umbrales configurados; `0%` es amarillo intenso, el tramo bajo interpola amarillo→verde hasta `Seco`, `Seco..Húmedo` permanece verde y por encima de `Húmedo` interpola verde→azul.
- Sin sensor: muestra `Sin sensor`, `---` y `Revisa IO35` / `Revisa IO35` / `Check IO35`, con paleta de Suelo atenuada. Al conectar/desconectar durante el uso, muestra splash semafórico `CONECTADO`/`DESCONECTADO` con `Sensor Suelo / IO35`.

#### Termómetro / DS18B20

Opciones raíz: `Corrección / Límites / Unidad / Alertas / Reset / Salir`

- **Corrección**: `Offset` (décimas, aprox. −5.0..+5.0) → Guardar.
- **Límites**: `Límite bajo` → `Límite alto` → Guardar. Valores internos en Celsius.
- **Unidad**: `Celsius` / `Fahrenheit`. Compartida con Temperatura y persistida en `sys_unit_f`.
- Sin sensor: muestra `Sin sensor`, `---` y `Revisa IO33` / `Revisa IO33` / `Check IO33`, con paleta de Termómetro/DS18 atenuada. Al conectar/desconectar durante el uso, muestra splash semafórico `CONECTADO`/`DESCONECTADO` con `Sensor DS18B20 / IO33`.

#### Sistema

Opciones raíz en grid 2×3: `Bip / Alarmas / Reposo / Idioma / Reset / Salir`

- **Reposo**: `30 seg / 1 min / 2 min / 5 min / 10 min / Nunca`.
- **Idioma**: `Español / Catalán / English`. Solicita full redraw con `runtime_request_ui_full_redraw()`.
- **Reset**: borra todo el namespace `pbit`, muestra overlay de reinicio y llama a `esp_restart()`. Incluye calibraciones, umbrales, idioma, unidad, Bip, Alarmas y `fw_stamp`; el siguiente arranque se comporta como primer boot de firmware limpio y vuelve a mostrar selector de idioma.

#### Timer

Sin submenú raíz de lista.

- Corto: inicia si parado, pausa si corriendo.
- Largo en idle: abre editor HH:MM:SS.
- Largo en running/pausado: resetea.
- Editor: giro sin editar = selección de campo HH/MM/SS; pulsar = entrar/salir de edición del campo; giro en edición = cambiar valor; pulsación larga en selección = guardar y salir.
- `00:00:00` = cronómetro ascendente; cualquier valor > 0 = cuenta regresiva.

### Regla común de todos los menús

- Los menús raíz de settings usan grid 2×3 con `Reset` abajo izquierda y `Salir` abajo derecha.
- Las pantallas de selección/edición muestran el valor activo dentro de una card central con borde semántico.
- Gira para cambiar opción o valor.
- Pulsa para confirmar el paso actual.
- `SAVED` es un estado intermedio: una pulsación adicional vuelve al menú raíz.
- Menús raíz y selectores de opción binaria/discreta envuelven en bucle.
- Ediciones numéricas no envuelven.
- Confirmaciones de `Reset` usan `drawResetChoicePrompt()`: pantalla roja `danger`, header estándar con línea blanca, panel central con descripción de la acción y botones `NO / SI` circulares. `NO` es la selección por defecto; `SI` confirma la acción destructiva.

---

## 18. Documentos relacionados

- `docs/PROJECT.md` — descripción completa del producto
- `docs/USER_GUIDE.md` — manual de usuario
- `docs/ROADMAP.md` — pendientes y mejoras futuras
- `docs/DESIGN_SYSTEM.md` — paleta, iconos, fuentes y reglas visuales
- `docs/TFT_RENDER_RULES.md` — protocolo anti-flicker
- `docs/PRODUCTION_CHECKLIST.md` — checklist de producción
- `platformio.ini` — configuración de build
- `lib/TFT_eSPI/User_Setup.h` — configuración del display

---

## Apéndice A — Banderas rojas y verificación pre-claim de firmware

> Esta sección es paralela a la "Verificación pre-claim" de `docs/TFT_RENDER_RULES.md`, pero para temas de firmware no-render. Si una tarea toca render, **además** se aplica el protocolo del doc TFT.

Para temas estrictamente de pantalla/render, no duplicar reglas aquí — consultar `docs/TFT_RENDER_RULES.md` directamente.

### A.1 Stack real del proyecto (recordatorio para evitar mezclar con ESP-IDF)

| Tema | Realidad P-Bit |
|---|---|
| Framework | **Arduino**, no ESP-IDF (`platformio.ini:framework = arduino`) |
| Build | `py -m platformio run -e esp32dev` |
| Flash | `py -m platformio run -e esp32dev -t upload` (o desde IDE) |
| Monitor serial | `py -m platformio device monitor -b 115200` |
| Config | Flags en `platformio.ini` + `include/config.h`. **No hay** `sdkconfig`, no hay `idf.py`, no hay `menuconfig` |
| Tareas | Arduino `setup()/loop()` + FreeRTOS `xTaskCreatePinnedToCore` para UI/sensors |
| NVS | `Preferences` (wrapper Arduino), no `nvs_flash_set_*` directo |
| BLE | `NimBLE-Arduino`, no `esp_bt_*` directo |
| OTA / Secure Boot / Flash encryption | **No habilitados** — fuera de scope del producto educativo |

Cualquier patrón ESP-IDF puro (idf.py, sdkconfig, esp_ota_ops, partition CSV custom) **no aplica** en este firmware. El skill global `esp32-firmware-engineer` está orientado a ESP-IDF y se debe filtrar contra esta realidad.

### A.2 Banderas rojas grep-ables (firmware no-render)

> **Nota PowerShell/cmd:** `src/ui_*.cpp` NO se expande como glob en PowerShell ni en cmd. Usar siempre `src -g 'ui_*.cpp'` para que `rg` resuelva el patrón internamente. En bash/zsh ambas formas funcionan.

```powershell
# 1. Bloqueo prolongado en tareas FreeRTOS — sospechoso si está en UI/sensor task
rg -n 'delay\(\s*[0-9]{4,}\s*\)' src         # delay >= 1000 ms

# 2. NVS write en ISR o callback síncrono del encoder (rotary callbacks corren en task context, pero verificar)
rg -n -B2 'prefs\.put|nvs_set' src/rotary.cpp src/io.cpp

# 3. String dinámico en hot path (fragmentación de heap)
rg -n 'String\s+[a-z_]+\s*=' src -g 'ui_*.cpp' -g 'io.cpp'

# 4. malloc/new en hot path
rg -n '\b(malloc|new\s+\w)' src -g 'ui_*.cpp' -g 'io.cpp'

# 5. portMUX/portENTER fuera de readings_mux conocido
rg -n 'portENTER_CRITICAL|portEXIT_CRITICAL|taskENTER_CRITICAL' src

# 6. Funciones marcadas IRAM_ATTR (deben ser cortas; bloqueo es disaster)
rg -n 'IRAM_ATTR' src include

# 7. Lecturas ADC sin atenuación 11dB (rango incorrecto)
rg -n 'analogSetPinAttenuation|analogReadResolution|adcAttachPin' src

# 8. esp_sleep_* fuera del path de reposo
rg -n 'esp_sleep_' src

# 9. -D flags activos que no deben llegar a producción
rg -n '^(\s*)-D(FIRMWARE_DEBUG|PBIT_ENABLE_SERIAL_PLOTTER)' platformio.ini
```

Cada hit positivo es "explica o arregla", no fail automático.

### A.3 Verificación pre-claim (firmware)

Antes de cerrar tarea de firmware no-render, los 8 proofs siguientes:

```
□ proof 1 — Build local pasa: `py -m platformio run -e esp32dev` con SUCCESS.

□ proof 2 — RAM/Flash reportados y comparados con baseline. Si delta > ±200 bytes,
            explicar la causa. Si delta > +2 KB Flash o > +1 KB RAM, ya es un
            cambio sustantivo que debe justificarse en el commit/PR.

□ proof 3 — Si tocaste NVS: claves nuevas o cambios de tipo documentados en § 9.
            Build-hash bump si rompe compatibilidad con builds previos.

□ proof 4 — Si tocaste sensores (`src/io.cpp`): verificar que el rango canónico
            (§ 6 tabla) sigue siendo válido. Para LDR, validar contra § 7 muestra
            empírica v1.

□ proof 5 — Si tocaste Demo Mode: validar que cadencia 220 ms sigue siendo el tick
            base (ver `docs/TFT_RENDER_RULES.md` § Nivel 5).

□ proof 6 — Si tocaste BLE: confirmar que sigue siendo factory-off (`ble_en=false`),
            que el reset por firmware-stamp funciona, y que el gesto secreto del
            SYSTEM_SCREEN no se documenta en USER_GUIDE.

□ proof 7 — Si tocaste tareas (UI/sensor/loop): stack usado vs asignado (`uxTaskGetStackHighWaterMark`).
            Si pasa de 75% del stack reservado, ampliar stack o reducir scope.

□ proof 8 — Hardware real validado (no solo build) para cambios en: pinout,
            timing crítico, ISR, sleep/wakeup, paths que afectan al usuario final.

□ Si algún proof no se ejecutó: decir "queda pendiente proof X" en el reporte.
  Nunca cerrar tarea diciendo "listo, compila" si no se corrió en hardware.
```

### A.4 Cómo NO romper el chip por descuido

- **GPIO34, GPIO35, GPIO36, GPIO39** son input-only. Cualquier `pinMode(.., OUTPUT)` en estos pines silenciosamente no funciona; verificar el pin destino.
- **GPIO12** es strapping pin (voltage select de flash). El encoder B vive ahí: ya está fijo y funciona. No reusar para otra cosa sin estudiar el strap.
- **DS18B20** (`GPIO33`) requiere `INPUT_PULLUP` antes del bus scan; si se quita, el OneWire falla silenciosamente.
- **ADC ATT 11dB** está configurado en `src/io.cpp` para todos los ADC del proyecto; cualquier nuevo sensor analógico debe pasar por la misma configuración para tener rango útil completo.
- **I2C en `GPIO26/27`** está físicamente disponible pero el firmware **no llama** `Wire.begin(...)`. Si se añade un sensor I2C, la inicialización debe ser `Wire.begin(26, 27)` explícita.

### A.5 Build de depuración (esp32dev_debug)

Environment separado en `platformio.ini` que activa `-DFIRMWARE_DEBUG` solo para esa build. **No tocar `esp32dev` de producción** ni descomentar `FIRMWARE_DEBUG` en `include/config.h`.

```powershell
py -m platformio run -e esp32dev_debug                # solo compilar
py -m platformio run -e esp32dev_debug -t upload      # compilar + flashear
py -m platformio device monitor --baud 115200         # leer Serial
```

Con `FIRMWARE_DEBUG` activo, ambas tasks emiten periódicamente el high water mark de stack:

```
[Stack] DisplayTask HWM free: 2400 bytes (worst: 1800)
[Stack] SensorTask HWM free: 2880 bytes (worst: 1600)
```

Semántica:
- En ESP32 + `framework-arduinoespressif32`, `uxTaskGetStackHighWaterMark()` ya devuelve **bytes** directamente (NO words; ver comentario `"in bytes not words"` en `task.h` del SDK local). El firmware imprime el valor crudo.
- **HWM free** = el **menor stack libre observado por FreeRTOS** para esa tarea desde el boot hasta esa muestra. No es stack libre instantáneo; es la marca histórica que FreeRTOS mantiene internamente.
- **worst** = peor valor (más bajo) observado por el código de instrumentación mientras esta build ha estado corriendo en esta sesión.
- Muestreo cada `1000 ms`. Log cuando el `worst` empeora o cada `60 s`.

Para volver a build de producción tras la validación: `py -m platformio run -e esp32dev`. El env `esp32dev_debug` está versionado en `platformio.ini`, pero no se invoca en CI ni se entrega como build de producción.

### A.6 Cuándo pedir ayuda al usuario antes de seguir

- Cambios de pinout (PCB y firmware deben moverse juntos).
- Cambios de cadencia de Demo Mode o de tasks (rompen anti-flicker validado).
- Activación de BLE como feature visible (decisión de producto).
- Cualquier cambio que afecte el contrato del menú raíz del Sistema (`Bip` / `Alarmas` / `Reposo` / `Idioma` / `Reset` / `Salir`).

> Esta lista es deliberadamente corta. Para todo lo demás, las reglas del firmware son las que estén explícitas en este documento y en `AGENTS.md`. Cuando un pattern no esté documentado y la decisión sea ambigua, pregunta antes de inventarte una regla.
