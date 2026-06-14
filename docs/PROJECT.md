# P-Bit — Descripción del Proyecto

Actualizado: 2026-06-13

---

## 1. Qué es el P-Bit

El P-Bit es una placa educativa ambiental basada en ESP32. Mide variables del entorno, las muestra en una pantalla TFT y permite configurar alertas, explorar datos históricos y gestionar un temporizador, todo con un encoder rotatorio como único control físico.

Está diseñado para actividades de observación, experimentación y aprendizaje en contextos STEAM: educación primaria y secundaria, laboratorio escolar, terrarios, plantas, aulas y espacios exteriores.

No es un instrumento de medición certificado. Es una herramienta pedagógica construida sobre hardware real y firmware abierto.

---

## 2. Propósito

El P-Bit existe para que niños, docentes y educadores puedan:

- **Observar** variables del entorno sin necesidad de ordenadores ni software externo
- **Configurar** alertas visuales, sonoras y de LED para aprender a definir rangos
- **Explorar** la evolución temporal de los datos con una gráfica integrada
- **Actuar** a partir de evidencia: regar una planta, cambiar la posición de una maceta, abrir una ventana
- **Aprender** cómo funcionan los sensores, las interfaces y la toma de decisiones basada en datos

---

## 3. Hardware

### Plataforma

| Componente | Valor |
|---|---|
| MCU | ESP32 (módulo Dev) |
| Framework | Arduino via PlatformIO |
| RAM usada | ~14.9 % (48940 bytes de 327680) |
| Flash usada | 72.1 % (945429 bytes de 1310720) |

### Pantalla

| Parámetro | Valor |
|---|---|
| Controlador | ST7735 |
| Resolución | 160 × 128 px |
| Orientación | Paisaje |
| Interfaz | SPI |
| Control de brillo | No disponible por hardware |

### Sensores integrados

| Sensor | Variable medida | Puerto / Pin |
|---|---|---|
| DHT11 | Temperatura ambiente, humedad relativa | GPIO4 |
| LDR | Luz ambiental (0..8000 lux en firmware; modo visible `Lux`/`FC`/`Raw ADC`) | GPIO39 |
| Micrófono (GM19767P + LM358) | Nivel de sonido (0..100 %) | GPIO36 |
| Sensor capacitivo de suelo | Humedad del suelo (calibrado %) | IO35 / GPIO35 |
| Sonda DS18B20 (externa) | Temperatura puntual | IO33 / GPIO33 |

### Interfaz y salidas

| Componente | Descripción |
|---|---|
| Encoder rotatorio + pulsador | Control principal de navegación y configuración |
| LED RGB | Indicador visual de estado y alertas |
| Buzzer pasivo | Beeps de UI y audio de alertas |
| Pantalla TFT | Interfaz visual principal |
| BLE (opcional) | Transmisión de datos inalámbrica; desactivado de fábrica |

### Pinout completo

**Sensores**

| Función | Pin ESP32 |
|---|---|
| LDR | GPIO39 |
| Micrófono | GPIO36 |
| Humedad de suelo | GPIO35 |
| DS18B20 (sonda) | GPIO33 |
| DHT11 | GPIO4 |

**Interfaz humana**

| Función | Pin ESP32 |
|---|---|
| Encoder A | GPIO14 |
| Encoder B | GPIO12 |
| Pulsador encoder | GPIO13 |

**Salidas**

| Función | Pin ESP32 |
|---|---|
| LED RGB — Rojo | GPIO5 |
| LED RGB — Verde | GPIO17 |
| LED RGB — Azul | GPIO16 |
| Buzzer | GPIO18 |

**TFT SPI**

| Señal | Pin ESP32 |
|---|---|
| MOSI | GPIO19 |
| SCLK | GPIO25 |
| CS | GPIO23 |
| DC | GPIO22 |
| RST | GPIO21 |

**Bus I2C disponible en placa (no usado en firmware actual)**

| Señal | Pin ESP32 |
|---|---|
| SDA | GPIO26 |
| SCL | GPIO27 |

---

## 4. Qué hace el firmware

### Pantallas y modos

Con `PBIT_ENABLE_GRAPH_LAB=1` (configuración de producción), el carrusel tiene 11 posiciones:

| Posición | Pantalla | Tipo |
|---|---|---|
| 1 | Inicio | Visión global de sensores en fichas |
| 2 | Clima Lab | Lab — temperatura + humedad combinados |
| 3 | Termo Lab | Lab — DHT11 + DS18B20 + diferencia térmica |
| 4 | Temperatura | Sensor — zona con modos visuales y menú |
| 5 | Humedad | Sensor — zona con modos visuales y menú |
| 6 | Luz | Sensor — zona con modos visuales y menú |
| 7 | Sonido | Sensor — zona con modos visuales, VU/Onda y menú |
| 8 | Suelo | Sensor — zona con modos visuales, calibración y alertas |
| 9 | Termómetro | Sensor — sonda DS18B20 con límites y alertas |
| 10 | Timer | Cronómetro y cuenta regresiva con editor HH:MM:SS |
| 11 | Sistema | Ajustes globales del dispositivo |

Cada pantalla de sensor soporta cinco modos visuales: Principal, Rango, Ficha, Dato y Curva. `Sonido` añade dos modos exclusivos justo después de Principal: Sonido VU y Sonido Onda. Internamente el firmware conserva nombres históricos (`Focus`, `Valor`, `Graph`, `Gauge`, `Card`) para evitar un refactor sin valor de producto.

### Alertas y feedback

El P-Bit usa tres canales simultáneos:

- **Color en pantalla** — el valor cambia de color según el rango
- **LED RGB** — color semántico por sensor y estado
- **Buzzer** — beeps de confirmación (`Bip`) y alertas automáticas (`Alarmas`)

Los dos canales de audio son independientes y configurables por separado desde `Sistema`.

### Persistencia

El dispositivo guarda en memoria no volátil (NVS):

- Idioma
- Límites y alertas de cada sensor
- Calibración del sensor de suelo
- Modo de visualización activo por sensor
- Tiempo de reposo
- Estado de Bip y Alarmas

La configuración se mantiene entre reinicios. Un flash nuevo resetea todos los ajustes a valores por defecto.

### Idiomas

- Español
- Catalán
- English

El idioma se selecciona en el primer encendido y puede cambiarse desde `Sistema > Idioma`.

---

## 5. Usos típicos

| Contexto | Ejemplo |
|---|---|
| Planta o terrario | Medir suelo, luz y temperatura; calibrar umbrales de riego |
| Aula | Comparar condiciones en diferentes zonas del espacio |
| Laboratorio escolar | Registrar y comparar variables ambientales en experimentos |
| Exploración exterior | Medir temperatura y luz en distintos puntos de un patio o jardín |
| Actividad de datos | Usar la gráfica integrada para observar la evolución de una variable |
| Experimento de sonido | Detectar qué zonas del colegio tienen más ruido ambiental |

---

## 6. Limitaciones conocidas

- El LDR no es un luxómetro calibrado. El rango 0..8000 lux usa la curva empírica v1 tomada con luxómetro y acotada por firmware; la coherencia visual `Lux`/`FC`/`Raw ADC` ya está implementada en firmware para las vistas de Luz, Sensor Zone, cards, dials, dashboards y gráficas. Queda validación en hardware real.
- Los sensores externos conectables por el usuario usan estado runtime común de ausencia: Suelo muestra `Revisa IO35` y Termómetro/DS18B20 muestra `Revisa IO33`, con la paleta del sensor atenuada en lugar de un gris genérico. Este estado no se guarda en NVS.
- El micrófono mide intensidad relativa (0..100 %), no decibelios SPL absolutos.
- El deep sleep automático está desactivado porque en esta revisión de hardware la TFT queda en blanco al dormir. El reposo actual muestra una pantalla `ZZZ`.
- El Modo demo es runtime: se activa encendiendo con el encoder presionado durante el logo o con pulsación larga desde `Home`, muestra una señal visual breve, rota pantallas del carrusel con dwell variable, anima valores y gráficas con curvas suaves, sale con giro/pulsación y no modifica preferencias guardadas. Queda validación visual final en hardware.
- BLE sale apagado de fábrica y no forma parte del flujo normal de aula.
- El ghosting/flicker de pantallas queda resuelto por ahora en hardware real; mantener vigilancia de regresión visual junto con la calibración RGB y la confirmación BLE off.

---

## 7. Arquitectura resumida

```
setup()
  └── init NVS, sensores, TFT, BLE condicional, idioma, Sensor Zone, encoder, demo runtime, FreeRTOS

FreeRTOS
  ├── Core 0: sensor_reading_task  →  leer sensores → global_readings → BLE/Serial
  ├── Core 1: switch_screen        →  router visual, overlays, refresco selectivo
  └── Loop:   encoder + buzzer + demo runtime + lógica de inactividad
```

La comunicación entre tareas usa secciones críticas (`portMUX`) y flags de runtime (`runtime_events`).

---

## 8. Documentación disponible

| Documento | Qué contiene | Para quién |
|---|---|---|
| `README.md` | Build y flash rápido | Cualquiera |
| `docs/PROJECT.md` | Este documento — descripción completa | Cualquiera |
| `docs/TECHNICAL.md` | Arquitectura, pinout detallado, código, BLE, NVS | Desarrollador / ingeniero / agente IA |
| `docs/USER_GUIDE.md` | Manual de producto: uso, menús, seguridad, configuración | Educador / usuario final |
| `docs/DESIGN_SYSTEM.md` | Paleta, iconos, fuentes, reglas de UI para TFT | Diseñador / desarrollador de pantallas |
| `docs/TFT_RENDER_RULES.md` | Protocolo anti-flicker, sprites, chrome/data | Desarrollador de pantallas |
| `docs/ROADMAP.md` | Estado actual, pendientes, mejoras futuras | Desarrollador / product owner |
| `CHANGELOG.md` | Historial de cambios | Todos |
| `docs/PRODUCTION_CHECKLIST.md` | Lista de verificación antes de entregar build | Producción |
| `AGENTS.md` | Guía de entrada para agentes IA | Agentes IA |
