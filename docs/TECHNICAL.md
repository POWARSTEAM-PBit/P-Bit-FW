# Manual Técnico del P-Bit

Actualizado: 2026-05-24

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

Estado de revisión de producción/i18n:

- build local verificado con `py -m platformio run -e esp32dev`
- resultado PlatformIO: `SUCCESS`
- memoria reportada por build: RAM `14.7%` (`48124` bytes de `327680`) y Flash `71.0%` (`931193` bytes de `1310720`)
- revisión estática de i18n, BLE factory-off, LDR, Sensor Zone y fixes anti-flicker completada
- validaciones de hardware real siguen pendientes de unidad física

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
| Humedad de suelo | `GPIO35` | ADC, entrada solamente, puerto externo `J6` |
| DS18B20 | `GPIO33` | bus 1-Wire, puerto externo |
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
- En el código hay referencias mixtas a `J3` y `J4` para la sonda DS18B20; la referencia eléctrica firme es `GPIO33`. Conviene verificar la serigrafía exacta de la placa física.
- Esta confirmación de pines ya no depende solo del firmware: quedó contrastada también contra los archivos KiCad `P-Bit3.kicad_sch` y `P-Bit3.kicad_pcb`.

## 5. Arquitectura del firmware

### Inicialización general

Orden real de `setup()`:

1. `Serial.begin`
2. `nvs_flash_init` (con erase automático si hay páginas corruptas)
3. cálculo de nombre de dispositivo desde MAC
4. inicialización de TFT
5. inicialización BLE **condicional** — solo si `load_ble_enabled_store()` devuelve `true`; por defecto es `false` de fábrica y se resetea a `false` con cada nuevo flash
6. inicialización de LED RGB y buzzer
7. reset de NVS por build-hash si el firmware es nuevo (ver sección 9)
8. inicialización de hardware y sensores
9. detección de tipo de arranque (wake desde sleep o arranque en frío)
10. carga o selección de idioma
11. inicialización del encoder
12. creación de tareas FreeRTOS

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

## 7. Flujo de adquisición de sensores

### Sensores rápidos

Se actualizan en el lazo rápido de `sensor_reading_task`:

- luz
- sonido
- suelo

#### Luz

Lógica actual:

- divisor resistivo con `R = 10k`
- filtro hardware adicional con `C4 = 1uF`
- conversión aproximada a lux por fórmula logarítmica calibrada
- rango útil normalizado a `0..20000 lux`
- saturación alta tratada como `20000 lux`
- lectura cruda disponible en `ldr_raw` para el modo `Raw ADC`
- filtrado software por EMA

La UI de luz ofrece tres modos de display: `Lux`, `% log` y `Raw ADC`. El modo `Raw ADC` muestra la lectura ADC directa, mientras que las vistas de cards, dials y gráficas usan el rango de lux limitado a `0..20000`.

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

### Fixes anti-flicker

La revisión estática confirma las mitigaciones de parpadeo en:

- dials/gauges: caches por sensor y actualizaciones por `data_dirty`/`chrome_dirty`
- cards: limpieza dirigida del rectángulo de valor y cache de estado
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
  - en `Sistema`: mantener 60 s sin girar activa la pantalla oculta de BLE

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
- menú `Rangos / Modo lectura / Alertas / Reset / Salir`
- modos de vista: `Lux`, `% log`, `Raw ADC`

#### Sonido

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_SOUND`
- pulsación corta: alterna modo de visualización
- menú `Niveles / Alertas / Reset / Salir`
- niveles interpretativos por umbrales, no calibración acústica física

#### Suelo

- en carrusel actual: slot de `SENSOR_ZONE_SCREEN` con sensor `SZ_SOIL`
- pulsación corta: alterna modo de visualización
- menú `Calibrar sensor / Rangos / Alertas / Reset / Salir`
- clasificación actual derivada desde dos umbrales editables: `Muy seco`, `Seco`, `Óptimo`, `Húmedo`, `Muy húmedo`

#### Termómetro / DS18B20

- en carrusel actual: slot visible `Termómetro` de `SENSOR_ZONE_SCREEN` con sensor técnico `SZ_DS18`
- pulsación corta: alterna modo de visualización
- la unidad `C/F` es global, persistente y compartida con `Temperatura DHT`
- menú `Corrección / Límites / Unidad / Alertas / Reset / Salir`

#### Sistema

- menú `Bip / Alarmas / Reposo / Idioma / Reset / Salir`
- pantalla principal con card general y panels internos: `ID`, `Tiempo`, `Idioma`, audio y BLE solo si `ble_en == true`
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

#### Sistema

- `sys_sleep`
- `sys_sound` (bool, default `true`) — `Bip`: beeps de interfaz, navegación y confirmaciones
- `sys_alarm` (bool, default `true`) — `Alarmas`: audio de alertas de sensores y final del `Timer`
- `sys_unit_f` (bool, default `false`) — unidad global de temperatura: `false=Celsius`, `true=Fahrenheit`

#### Sensor zone

- `sz_sen`
- `sz_v0` .. `sz_v5`

#### BLE

- `ble_en` (bool, default `false`) — controla si el BLE se inicializa en el arranque

Esta clave se borra junto con el resto del namespace `pbit` en cada nuevo flash (ver build-hash reset a continuación).

#### Idioma

- gestionado desde el módulo de idioma con persistencia propia
- el idioma activo se normaliza con `normalizeLanguage(...)`
- `LANG_COUNT` define el límite de idiomas soportados
- `LIn(language, key)` permite renderizar textos en un idioma explícito
- `L(key)` traduce usando el idioma activo

## 10. BLE

### Feature gate — BLE desactivado por defecto

El BLE del P-Bit sale desactivado de fábrica.

El flag de control es la clave NVS `ble_en` (bool, namespace `pbit`, default `false`).

Comportamiento:

- En cada nuevo flash, la clave se borra porque el build-hash FNV-1a detecta el nuevo binario y llama a `clear_all_settings_store()`, que limpia el namespace `pbit` entero. `ble_en` vuelve a `false` (su default implícito al no existir).
- En reinicios normales entre flashes, el valor persistido se respeta: si el usuario activó el BLE, sigue activo.
- `init_ble()` solo se llama si `load_ble_enabled_store()` devuelve `true`. Si devuelve `false`, el stack NimBLE nunca se inicia y el dispositivo no emite señal BLE.
- La fila BLE de `Sistema` solo se dibuja si `ble_en == true`; en estado de fábrica queda oculta.

Nota de producción: si se flashea sobre una unidad de desarrollo que tenía BLE activado, validar antes de entregar que el dispositivo no anuncia BLE. El reset por build-hash se ejecuta antes de cargar BLE/settings, así que una build nueva debe volver a `OFF` desde el primer arranque.

### Pantalla BLE Toggle (gesto secreto de producción)

La pantalla `BLE_TOGGLE_SCREEN` es una función de fábrica. No aparece en el carrusel de navegación normal.

#### Cómo activarla

Desde la pantalla `Sistema`, mantener el encoder presionado durante **60 segundos** sin girarlo.

Flujo interno:

1. A los ~1.2 s el menú de Sistema se abre normalmente (longpress estándar).
2. El conteo de tiempo continúa en el fondo; el flag `g_ble_secret_eligible` se mantiene desde el momento en que empezó la pulsación.
3. Al cumplirse 60 s desde el inicio de la pulsación, `poll_rotary_aux()` detecta la condición (`g_ble_secret_eligible && !g_ble_secret_fired && hold >= 60000 ms`), navega a `BLE_TOGGLE_SCREEN` y suprime el callback de release.
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
- `src/rotary.cpp` — detección del gesto (variable `g_ble_secret_eligible`, constante `BLE_SECRET_PRESS_MS = 60000`)
- `include/settings_store.h` / `src/settings_store.cpp` — `load_ble_enabled_store()` / `save_ble_enabled_store()`
- `src/main.cpp` — `if (load_ble_enabled_store()) { init_ble(); }`

### Modo BLE activo

Cuando `ble_en == true`, el dispositivo inicia publicidad BLE con nombre derivado de la MAC:

- formato: `PBIT-XXXX`

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

En cold boot: selector visible en `Español` / `Catalán` / `English`. Encoder en rango `0..2` circular. Al confirmar, idioma se guarda y pantalla se limpia. Idioma preseleccionado = último guardado; default = Español.

### Acciones rápidas fuera de menú

| Pantalla | Pulsación corta | Pulsación larga |
|---|---|---|
| TEMP / TERMÓMETRO | Alterna unidad global `C/F` | Abre menú (~1.2 s) |
| HUMEDAD / LUZ / SONIDO / SUELO | Cambia modo visual del sensor | Abre menú (~1.2 s) |
| SISTEMA | Alterna `Bip ON/OFF` | Abre menú (~1.2 s) |
| TIMER | Inicia o pausa | Abre editor HH:MM:SS si idle; resetea si corriendo/pausado (~1.0 s) |

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

Opciones raíz: `Rangos / Modo lectura / Alertas / Reset / Salir`

- **Rangos**: `Max penumbra` → `Max interior` → `Max brillante` → Guardar. Rango editable: 10..10 000. Validación: brillante > interior > penumbra.
- **Modo lectura**: `Lux` / `% log` / `Raw ADC`.
- `Raw ADC` cambia solo el valor grande; la barra y la categoría siguen usando lux.

#### Sonido

Opciones raíz: `Niveles / Alertas / Reset / Salir`

- **Niveles**: `Max silencio` → `Max normal` → `Max alto` → Guardar. Validación: alto > normal > silencio.
- Sin alerta sonora propia (no contaminar lectura del micrófono).

#### Suelo

Opciones raíz: `Calibrar sensor / Rangos / Alertas / Reset / Salir`

- **Calibrar sensor**: `Seco al aire` → `En agua` → Guardar o Error. Validación: seco_raw > húmedo_raw, diferencia ≥ 300.
- **Rangos**: `Seco` → `Húmedo` → Guardar. Validación: seco < húmedo, todos en 0..100.
- Clasificación derivada: `0..Seco/2` = `Muy seco`; `Seco/2..Seco` = `Seco`; `Seco..Húmedo` = `Óptimo`; `Húmedo..(Húmedo+100)/2` = `Húmedo`; tramo final = `Muy húmedo`.
- Sin sensor: muestra `Sin sensor` y `Check J6 (GPIO35)`.

#### Termómetro / DS18B20

Opciones raíz: `Corrección / Límites / Unidad / Alertas / Reset / Salir`

- **Corrección**: `Offset` (décimas, aprox. −5.0..+5.0) → Guardar.
- **Límites**: `Límite bajo` → `Límite alto` → Guardar. Valores internos en Celsius.
- **Unidad**: `Celsius` / `Fahrenheit`. Compartida con Temperatura y persistida en `sys_unit_f`.
- Sin sensor: muestra `Sin sensor` y `Check J4`.

#### Sistema

Opciones raíz en grid 2×3: `Bip / Alarmas / Reposo / Idioma / Reset / Salir`

- **Reposo**: `30 seg / 1 min / 2 min / 5 min / 10 min / Nunca`.
- **Idioma**: `Español / Catalán / English`. Solicita full redraw con `runtime_request_ui_full_redraw()`.
- **Reset**: borra todo el namespace `pbit`. Incluye calibraciones, umbrales, idioma, unidad, Bip, Alarmas.

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
- Confirmaciones de `Reset` usan `NO / SI` (circular).

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
