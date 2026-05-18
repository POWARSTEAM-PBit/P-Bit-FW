# Manual Técnico del P-Bit

Actualizado: 2026-05-17

Este documento describe el estado técnico actual del P-Bit a partir del firmware y la configuración presentes en este repositorio. Está pensado como base de entrenamiento para desarrollo, integración, soporte, mantenimiento y despliegue educativo.

## 1. Resumen técnico

El P-Bit es un dispositivo educativo ambiental basado en ESP32 que integra sensores de temperatura, humedad, luz, sonido, humedad de suelo y temperatura externa. El sistema combina:

- adquisición de datos en tiempo real
- interfaz gráfica en pantalla TFT
- navegación con encoder rotatorio
- alertas visuales, sonoras y RGB
- persistencia de configuración en NVS
- conectividad BLE
- modos de ahorro de energía con reposo visible con `ZZZ`

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
- BLE para visualización/lectura remota

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
  - emitir datos por BLE
  - enviar línea CSV por Serial

#### Loop principal

- Corre en el contexto principal
- Rol:
  - `rotaryEncoder.loop()`
  - `poll_rotary_aux()`
  - `loop_buzzer()`
  - lógica de inactividad
  - transición entre `ACTIVE`, `IDLE` y `DEEP SLEEP`

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
- saturación tratada a partir de `ADC >= 4050`
- filtrado software por EMA

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

Orden real del carrusel de producción (12 posiciones, circular):

`HOME -> CLIMA -> MULTI -> SONIDO VU -> TEMPERATURA -> HUMEDAD -> LUZ -> SONIDO -> SUELO -> DS18B20 -> TIMER -> SISTEMA`

Las primeras 4 posiciones (`Home`, `Clima`, `Multi`, `Sonido VU`) son pantallas de solo lectura. Las 6 posiciones de sensor individual usan `SENSOR_ZONE_SCREEN` con cambio de modo de visualización vía pulsación corta y menú de configuración vía pulsación larga. `GRAPH_SCREEN` existe en el firmware pero no está en el carrusel de producción actual.

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

- pulsación corta: alterna modo de visualización
- la unidad `C/F` es global y compartida con `DS18B20`
- menú `Límites / Unidad / Alertas / Reset / Salir`

#### Humedad del aire

- pulsación corta: alterna modo de visualización
- menú `Límites / Alertas / Reset / Salir`
- usa dos umbrales: `Seco` y `Muy húmedo`

#### Luz

- pulsación corta: alterna modo de visualización
- menú `Calibración / Modo display / Alertas / Reset / Salir`
- modos de vista: `Lux`, `% log`, `Raw ADC`

#### Sonido

- pulsación corta: alterna modo de visualización
- menú `Calibración / Alertas / Reset / Salir`
- calibración interpretativa por umbrales, no física

#### Suelo

- pulsación corta: alterna modo de visualización
- menú `Calibrar sensor / Editar umbrales / Alertas / Reset / Salir`
- clasificación actual: `Seco`, `Óptimo`, `Húmedo`, `Muy húmedo`

#### DS18B20

- pulsación corta: alterna modo de visualización
- la unidad `C/F` es global y compartida con `Temperatura DHT`
- menú `Calibración / Unidad / Alertas / Reset / Salir`

#### Sistema

- menú `Sonido / Reposo / Idioma / Reset / Salir`

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
- al terminar una cuenta regresiva, la UI pasa a rojo y dispara una alarma intermitente corta si el sonido global está activo

#### Gráfica (`GRAPH_SCREEN`)

`GRAPH_SCREEN` no forma parte del carrusel de producción actual. La infraestructura de buffers circulares (160 muestras a 1 s, `g_graph_mux` para acceso cross-core) sigue activa en `graph_buffer.cpp` y el sensor task sigue llenando los 6 buffers. La integración del histórico como modo de visualización adicional dentro de las pantallas de sensor individuales es el paso siguiente natural.

Archivos clave:

- `include/graph_buffer.h` / `src/graph_buffer.cpp` — buffer circular y acceso thread-safe
- `include/ui_graph.h` / `src/ui_graph.cpp` — render de pantalla (conservado pero no en carrusel activo)
- `src/io.cpp` — push a buffers dentro del bloque de sensores lentos (cada 1 s)

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
- `sys_sound`

#### BLE

- `ble_en` (bool, default `false`) — controla si el BLE se inicializa en el arranque

Esta clave se borra junto con el resto del namespace `pbit` en cada nuevo flash (ver build-hash reset a continuación).

#### Idioma

- gestionado desde el módulo de idioma con persistencia propia

## 10. BLE

### Feature gate — BLE desactivado por defecto

El BLE del P-Bit sale desactivado de fábrica.

El flag de control es la clave NVS `ble_en` (bool, namespace `pbit`, default `false`).

Comportamiento:

- En cada nuevo flash, la clave se borra porque el build-hash FNV-1a detecta el nuevo binario y llama a `clear_all_settings_store()`, que limpia el namespace `pbit` entero. `ble_en` vuelve a `false` (su default implícito al no existir).
- En reinicios normales entre flashes, el valor persistido se respeta: si el usuario activó el BLE, sigue activo.
- `init_ble()` solo se llama si `load_ble_enabled_store()` devuelve `true`. Si devuelve `false`, el stack NimBLE nunca se inicia y el dispositivo no emite señal BLE.

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

Estado actual:
- la localización base existe y funciona
- todavía pueden quedar algunas cadenas duras residuales en ciertas pantallas

## 13. Alertas y feedback

Canales de feedback actualmente presentes:

- color en pantalla
- LED RGB
- buzzer

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

- la localización aún no está cerrada al 100%
- el timer ya tiene edición directa `HH:MM:SS` y cuenta regresiva, pero todavía no tiene automatizaciones de experimento
- el módulo de sonido trabaja por umbrales interpretativos, no por calibración acústica absoluta
- la UX visual de alertas y algunos detalles de layout todavía pueden seguir afinándose
- no existe control de brillo del display por hardware

## 16. Recomendaciones para entrenamiento y mantenimiento

- validar siempre la serigrafía real de conectores externos frente a la revisión de placa
- recalibrar suelo si cambia el sensor o la fuente de alimentación
- no interpretar sonido como medición en dB certificada
- en pruebas de luz, recordar que el RGB se apaga a propósito en esa pantalla
- si en una iteración futura se reactiva deep sleep, habrá que revalidar la TFT y el wake por encoder en hardware
- mantener sincronizados `ROADMAP_PBIT.md`, `Menues.MD` y los manuales cuando cambie el firmware

## 17. Documentos relacionados

- `PBIT_FUNCIONAMIENTO_ACTUAL.md`
- `Menues.MD`
- `ROADMAP_PBIT.md`
- `platformio.ini`
- `lib/TFT_eSPI/User_Setup.h`
