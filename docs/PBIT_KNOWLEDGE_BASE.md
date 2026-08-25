# P-Bit — Guía Base Completa

Actualizado: 2026-08-25
Versión firmware: ver `CHANGELOG.md`
Estado: documento maestro de producto, uso y referencia técnica

---

## 1. Propósito de este documento

Esta guía es la base de conocimiento completa del P-Bit. Reúne en un solo lugar la explicación de uso, navegación, pantallas, sensores, configuración, alimentación, conectividad y datos técnicos necesarios para entender el dispositivo.

Debe usarse como fuente principal para crear manuales resumidos, guías de aula, materiales comerciales, documentación técnica, soporte y entrenamiento de agentes. Cuando exista una duda entre documentos, esta guía debe alinearse con el firmware de producción y con `docs/TECHNICAL.md`.

La versión de producción descrita aquí usa el carrusel:

`Inicio -> Clima Lab -> Planta Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`

`Planta Lab` aparece solo cuando el sensor de Suelo entrega una lectura válida. Si el sensor no está conectado o la lectura no es válida, el carrusel salta directamente de `Clima Lab` a `Termo Lab`.

---

## 2. Qué es el P-Bit

El P-Bit es un dispositivo educativo ambiental basado en ESP32. Permite observar el entorno mediante sensores integrados y externos, visualizar datos en una pantalla TFT a color y configurar umbrales, alarmas, modos de visualización, idioma, reposo y preferencias de audio.

Está pensado para aula, experimentación STEAM, demostraciones, observación de plantas, comparación de ambientes y aprendizaje de datos. No sustituye instrumentos certificados de laboratorio, seguridad, salud, meteorología profesional o medición acústica normativa.

El P-Bit mide o representa:

| Variable | Sensor | Tipo |
|---|---|---|
| Temperatura ambiente | DHT11 integrado | Interno |
| Humedad relativa del aire | DHT11 integrado | Interno |
| Luz ambiental | LDR integrado | Interno |
| Nivel de sonido | Micrófono analógico | Interno |
| Humedad de suelo | Sensor capacitivo externo | Externo |
| Temperatura puntual | Sonda DS18B20 externa | Externo |

Además incluye:

- Pantalla TFT ST7735 de 160 x 128 px en orientación horizontal.
- Encoder rotatorio con pulsador como único control de usuario.
- LED RGB para estados, feedback y alertas visuales.
- Buzzer pasivo para beeps de interfaz, alarmas y feedback audible.
- ESP32 con capacidad WiFi y Bluetooth/BLE, aunque el firmware inicial de producción no incluye flujo público de configuración WiFi ni activación Bluetooth para consumidor final.

---

## 3. Alimentación y uso físico

### Alimentación recomendada

El uso recomendado del P-Bit es con **3 baterías AAA** instaladas en su portapilas. Esta es la forma prevista para uso normal en aula, demostraciones y mediciones portátiles.

Usar baterías AAA tiene varias ventajas:

- permite usar el P-Bit sin depender de un ordenador;
- evita tirones del cable USB durante actividades;
- separa el uso diario del puerto de programación;
- reduce riesgos de conectar fuentes USB no adecuadas.

### Puerto USB-C

El puerto USB-C se usa principalmente para:

- programar o actualizar el firmware;
- depurar por puerto serie;
- alimentar temporalmente el P-Bit desde un ordenador;
- alimentar de forma auxiliar con un powerbank USB de 5 V.

La alimentación por USB-C o powerbank puede funcionar, pero **no es la opción recomendada como uso principal**. Para consumidor/aula, la recomendación operativa debe ser baterías AAA. El USB-C debe tratarse como puerto de programación y soporte.

### Precauciones de alimentación

- No conectar fuentes por encima de 5 V al USB-C.
- No modificar la alimentación si no se conoce el circuito.
- No conectar ni desconectar sensores externos con el dispositivo encendido salvo que se esté haciendo una prueba controlada.
- Si el equipo se comporta de forma errática, probar con baterías AAA nuevas antes de asumir fallo de sensor.
- Si se usa USB-C para pruebas largas, asegurar que el cable y el puerto USB entregan corriente estable.

---

## 4. Hardware principal

### Plataforma

| Elemento | Descripción |
|---|---|
| Microcontrolador | ESP32 Dev Module |
| Framework | Arduino sobre PlatformIO |
| Pantalla | TFT ST7735, 160 x 128 px, SPI, landscape |
| Control | Encoder rotatorio con pulsador |
| Salidas | LED RGB y buzzer pasivo |
| Persistencia | NVS interna del ESP32 |
| Idiomas | Español, Catalán e Inglés |

### Pantalla TFT

La pantalla es el centro de la experiencia. Tiene resolución reducida, por lo que la interfaz usa pantallas especializadas en vez de intentar mostrar todo a la vez.

Datos relevantes:

- Controlador: ST7735.
- Resolución física usada: 160 x 128 px.
- Orientación: horizontal.
- Interfaz: SPI.
- Sin touch.
- Sin control de brillo por software en esta revisión de hardware.

El firmware evita redibujar toda la pantalla en cada actualización. Separa partes estáticas ("shell" o estructura) de datos dinámicos para reducir parpadeo y ghosting.

### Encoder rotatorio

El encoder es el único control físico. Tiene giro izquierda/derecha y pulsación.

| Acción | Uso principal |
|---|---|
| Girar | Cambiar de pantalla, moverse por menús o ajustar valores |
| Pulsación corta | Confirmar, cambiar modo visual o iniciar/pausar Timer |
| Pulsación larga (~1,2 s) | Abrir menú de configuración, editar Timer o activar Demo desde Inicio |

Dentro de menús:

- Girar cambia la opción seleccionada o el valor.
- Pulsar confirma.
- `Salir` abandona el menú sin tocar el resto.
- En confirmaciones de `Reset`, `NO` es la opción inicial.

### LED RGB

El LED RGB acompaña el estado visible:

- colores de alerta o rango según sensor;
- feedback de estado del Timer;
- apagado en vistas de Luz para no contaminar la lectura del LDR.

El LED no es la fuente de verdad: la pantalla siempre explica mejor el estado. El LED es una señal rápida a distancia.

### Buzzer

El buzzer pasivo da feedback audible:

- beeps de navegación y confirmacion si `Bip` está activo;
- alarmas de sensores compatibles si `Alarmas` está activo;
- aviso al finalizar una cuenta regresiva del Timer.

La pantalla de Sonido no usa alarma sonora propia, porque el beep contaminaria la lectura del micrófono.

---

## 5. Pinout y conectores

### Sensores

| Funcion | Pin ESP32 | Observacion |
|---|---:|---|
| LDR | `GPIO39` | ADC, entrada solamente |
| Micrófono | `GPIO36` | ADC, entrada solamente |
| Humedad de suelo | `GPIO35` | ADC, entrada solamente; referencia visible `IO35` |
| DS18B20 | `GPIO33` | Bus 1-Wire; referencia visible `IO33` |
| DHT11 | `GPIO4` | Digital, integrado en placa |

### Interfaz humana

| Funcion | Pin ESP32 | Observacion |
|---|---:|---|
| Encoder A | `GPIO14` | Entrada digital |
| Encoder B | `GPIO12` | Entrada digital |
| Pulsador encoder | `GPIO13` | Activo en `LOW`; usado para wake-up |

### Salidas

| Funcion | Pin ESP32 | Observacion |
|---|---:|---|
| LED RGB rojo | `GPIO5` | PWM |
| LED RGB verde | `GPIO17` | PWM |
| LED RGB azul | `GPIO16` | PWM |
| Buzzer pasivo | `GPIO18` | PWM |

### TFT SPI

| Senal | Pin ESP32 |
|---|---:|
| MOSI | `GPIO19` |
| SCLK | `GPIO25` |
| CS | `GPIO23` |
| DC | `GPIO22` |
| RST | `GPIO21` |

### I2C disponible

| Senal | Pin ESP32 | Estado |
|---|---:|---|
| SDA | `GPIO26` | Disponible físicamente; firmware inicial no lo usa |
| SCL | `GPIO27` | Disponible físicamente; firmware inicial no lo usa |

Notas:

- `GPIO36`, `GPIO39` y `GPIO35` son entradas analogicas solamente.
- El firmware usa atenuación ADC 11 dB para ampliar rango útil analógico.
- Para soporte de usuario, usar `IO33` y `IO35` como nombres visibles de los conectores externos.
- Para documentación técnica, usar `GPIO33` y `GPIO35` como referencia eléctrica firme.

---

## 6. Conectividad WiFi y Bluetooth

El ESP32 del P-Bit tiene capacidad WiFi y Bluetooth/BLE a nivel de hardware.

### WiFi

El firmware inicial de producción **no incluye flujo de configuración WiFi**. No hay pantalla publica para seleccionar red, introducir contraseña, enviar datos por WiFi o conectarse a una nube.

WiFi debe documentarse como capacidad posible del hardware ESP32, no como función disponible para consumidor en el firmware actual.

### Bluetooth / BLE

El firmware contiene soporte BLE interno, pero sale apagado de fábrica:

- clave NVS: `ble_en`;
- valor por defecto: `false`;
- al flashear un nuevo firmware se resetea por sello/hash de build;
- si BLE está apagado, no debe anunciar `PBIT-XXXX`;
- la pantalla de Sistema no muestra BLE como opción normal cuando está apagado.

Importante:

- BLE es una función de fábrica/debug, no una función pública de la guía de consumidor.
- No publicar el gesto de activación BLE en manuales de usuario ni guias de aula.
- Si se documenta internamente, hacerlo solo en `docs/TECHNICAL.md` o material de soporte para equipo técnico.

Cuando BLE está activado para pruebas, el dispositivo anuncia un nombre tipo `PBIT-XXXX` y puede notificar lecturas por un servicio propietario y un servicio legado de Environmental Sensing. Ese flujo requiere configuración adicional y no forma parte del firmware inicial visible para consumidor.

---

## 7. Arranque e idioma

En el primer arranque, o después de limpiar configuración por un firmware nuevo/reset, aparece el selector de idioma:

- `Español`
- `Catalán`
- `English`

El usuario gira el encoder para seleccionar y pulsa para confirmar. El idioma queda guardado en NVS (`lang`). Si el idioma ya está guardado, el P-Bit arranca directamente en `Inicio`.

Un flash de firmware nuevo puede limpiar configuración guardada por el mecanismo de sello/hash de firmware. Esto restaura valores por defecto, incluida la salida BLE apagada.

---

## 8. Navegación principal

El P-Bit usa un carrusel circular de pantallas. Girar el encoder cambia de pantalla.

### Carrusel con Suelo conectado

`Inicio -> Clima Lab -> Planta Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`

### Carrusel sin Suelo valido

`Inicio -> Clima Lab -> Termo Lab -> Temperatura -> Humedad -> Luz -> Sonido -> Suelo -> Termómetro -> Timer -> Sistema`

`Planta Lab` se omite porque su propósito depende de combinar suelo y ambiente. La pantalla `Suelo` sigue existiendo para mostrar `Sin sensor`, `Revisa IO35` o permitir conectar/calibrar cuando corresponda.

### Lógica de uso

- Las pantallas Lab son de lectura y comparación; no tienen menú propio.
- Las pantallas de sensor usan Sensor Zone y tienen varios modos visuales.
- El Timer tiene interacción propia de cronómetro/cuenta regresiva.
- Sistema muestra estado general y permite configuración global.

---

## 9. Tipos de pantalla

### Inicio

Pantalla de resumen rápido. Muestra las cuatro magnitudes internas principales en tarjetas:

- temperatura ambiente;
- humedad del aire;
- luz;
- sonido.

Sirve para responder en pocos segundos: "¿cómo está el entorno ahora mismo?". Los sensores externos tienen pantallas propias y avisos de conexión.

### Clima Lab

Pantalla multisensor de temperatura ambiente y humedad del aire. Sirve para interpretar confort ambiental, no solo valores aislados.

Incluye temperatura, humedad y una lectura conjunta del ambiente. Es útil para comparar zonas de un aula o ver como cambia el confort durante una actividad.

### Planta Lab

Pantalla multisensor de salud de planta. Aparece solo si Suelo entrega lectura válida.

Combina:

- humedad de suelo;
- temperatura ambiente;
- humedad del aire;
- luz.

Su objetivo es explicar que una planta depende de varias condiciones al mismo tiempo. El diagnóstico prioriza Suelo y luego variables ambientales. Los estados visibles son `BIEN`, `SEDIENTA`, `AHOGADA` y `ESTRÉS`.

### Termo Lab

Pantalla multisensor de temperatura. Compara:

- temperatura ambiente del DHT11;
- temperatura puntual de la sonda DS18B20;
- diferencia entre ambas.

Sirve para experimentos de contacto, agua, materiales, superficies, zonas frías/calientes o comparaciónes entre aire y objeto.

### Sensor Zone

Sensor Zone es la capa común para:

- `Temperatura`
- `Humedad`
- `Luz`
- `Sonido`
- `Suelo`
- `Termómetro`

Dentro de una pantalla de sensor, la pulsación corta cambia el modo visual. El giro cambia a otra pantalla del carrusel.

Ciclo común:

`Principal -> Rango -> Ficha -> Dato -> Curva -> Principal`

Ciclo de Sonido:

`Principal -> Sonido VU -> Sonido Onda -> Rango -> Ficha -> Dato -> Curva -> Principal`

Cada sensor recuerda su último modo visual.

### Timer

Timer funciona como cronómetro y cuenta regresiva. Una pulsación corta inicia o pausa. Una pulsación larga abre el editor de duración o resetea según estado.

### Sistema

Sistema muestra estado general y permite ajustar:

- `Bip`
- `Alarmas`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

---

## 10. Modos visuales de Sensor Zone

### Principal

Es la vista base del sensor. Muestra la lectura protagonista con identidad visual clara. Sirve para consultar el dato actual sin entrar en detalles.

### Rango

Sitúa el dato dentro de su escala. Normalmente usa un dial, arco, aguja o zonas de color. Sirve para entender si el valor está bajo, normal, alto o fuera de rango.

Las `Marcas` del menú muestran u ocultan referencias de límites en este modo. Por defecto están visibles en Temperatura, Humedad, Suelo y Termómetro, y ocultas en Luz y Sonido.

### Ficha

Resume el sensor en una tarjeta más informativa: valor, unidad, estado, visualización compacta y alertas. Sirve para reconocer rápidamente la situación del sensor.

### Dato

Enfatiza el número exacto y una pequeña señal de tendencia. Sirve cuando se quiere leer el valor con precisión.

### Curva

Muestra histórico o evolución temporal. El firmware mantiene buffers circulares de 160 muestras y alimenta las gráficas con lecturas válidas. La cadencia base de sensores lentos es 1 muestra por segundo; Sonido y Luz tienen muestreo interno más rápido, pero la gráfica usa muestras consolidadas.

La gráfica sirve para responder:

- si el valor sube o baja;
- si hay picos;
- si una acción produjo un cambio;
- si el entorno se estabilizó.

En Luz, la Curva respeta el modo visible `Lux`, `FC` o `Raw ADC`; internamente las categorías y alertas siguen usando lux.

### Sonido VU

Modo exclusivo de Sonido. Muestra barras tipo medidor VU con reacción rápida. Sirve para ver variaciones instantáneas de ruido, palmas, voz, música o ambiente.

### Sonido Onda

Modo exclusivo de Sonido. Muestra una onda animada. Sirve para comprender que el sonido cambia constantemente y no es un valor estático.

---

## 11. Menús y reglas comunes

Los menús principales de configuración usan una cuadrícula 2 x 3:

```text
[ opción 1 ] [ opción 2 ]
[ opción 3 ] [ opción 4 ]
[  Reset   ] [  Salir   ]
```

Reglas:

- Máximo 4 opciones primarias.
- `Reset` está abajo izquierda.
- `Salir` está abajo derecha.
- Los espacios vacíos no se usan.
- Girar mueve la selección.
- Pulsar entra o confirma.
- `NO` es la opción inicial en cualquier reset.

Tipos de opciones:

- `Límites`: cambia umbrales de interpretación o alarma.
- `Marcas`: muestra/oculta referencias visuales en `Rango`.
- `Alertas`: activa/desactiva alertas para ese sensor.
- `Unidad`: cambia Celsius/Fahrenheit para temperatura y termómetro.
- `Modo`: cambia forma de presentar Luz.
- `Calibrar sensor`: solo Suelo usa calibración real.
- `Reset`: restaura los valores de esa pantalla.

---

## 12. Umbrales, límites, marcas y alarmas

### Umbrales y límites

Un umbral es un valor que separa categorías. Ejemplos:

- Temperatura por debajo de `Bajo` o por encima de `Alto`.
- Humedad por debajo de `Seco` o por encima de `Muy húmedo`.
- Luz por debajo de `Max. poca luz` o por encima de `Max. brillante`.
- Sonido por encima de `Max. fuerte`.

Los umbrales no cambian la lectura física del sensor. Cambian cómo se interpreta y cuándo se activa una alerta.

### Marcas

Las marcas son referencias visuales que aparecen en el modo `Rango`. Ayudan a ver dónde están los límites configurados.

Modificar límites de Luz o Sonido activa automáticamente las marcas para que el usuario vea el efecto de lo que acaba de ajustar. Un reset del sensor devuelve las marcas a su valor por defecto.

### Alertas

Las alertas pueden afectar:

- color de pantalla;
- joya/indicador de alerta;
- LED RGB;
- sonido de alarma si `Alarmas` está activo.

Sonido no emite alerta audible propia. Aunque sus alertas estén activas, el audio se mantiene visual/LED para no contaminar el micrófono.

### Bip vs Alarmas

`Bip` y `Alarmas` son controles distintos:

| Control | Afecta | No afecta |
|---|---|---|
| `Bip` | Clicks, beeps de UI, navegación, confirmaciones | Alertas automáticas |
| `Alarmas` | Alertas audibles y final de cuenta regresiva | Beeps de UI |

Ambos salen `OFF` por defecto en producción.

---

## 13. Temperatura ambiente

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | DHT11 |
| Ubicación | Integrado |
| Pin | `GPIO4` |
| Rango visual | 0..50 °C / 32..122 °F |
| Unidad configurable | Celsius/Fahrenheit, compartida con Termómetro |
| Defaults | bajo 18 °C, alto 28 °C, alertas OFF, marcas ON |

### Qué muestra

La pantalla `Temperatura` muestra la temperatura del aire medida por el DHT11. Es la temperatura ambiente del dispositivo, no la de una superficie.

### Modos

- `Principal`: lectura actual y estado general.
- `Rango`: posición respecto a bajo/alto.
- `Ficha`: resumen compacto con valor y estado.
- `Dato`: número protagonista y tendencia.
- `Curva`: evolución temporal de temperatura ambiente.

### Menú

| Opción | Función |
|---|---|
| `Unidad` | Cambia Celsius/Fahrenheit global |
| `Límites` | Edita bajo y alto |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta del sensor |
| `Reset` | Restaura defaults de temperatura |
| `Salir` | Cierra el menú |

### Alertas

- Temperatura <= límite bajo: alerta baja.
- Temperatura >= límite alto: alerta alta.
- Si `Alarmas` está ON, emite beeps de alerta.

---

## 14. Humedad del aire

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | DHT11 |
| Ubicación | Integrado |
| Pin | `GPIO4` |
| Rango | 0..100 % HR |
| Defaults | seco 30 %, confort/muy húmedo 70 %, alertas ON, marcas ON |

### Qué muestra

La pantalla `Humedad` muestra humedad relativa del aire. Ayuda a interpretar si el ambiente está seco, confortable o demasiado húmedo.

### Modos

- `Principal`: humedad actual.
- `Rango`: posición frente a `Seco` y `Muy húmedo`.
- `Ficha`: resumen del sensor.
- `Dato`: porcentaje protagonista.
- `Curva`: evolución temporal.

### Menú

| Opción | Función |
|---|---|
| `Límites` | Edita `Seco` y `Muy húmedo` |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta |
| `Reset` | Restaura defaults de humedad |
| `Salir` | Cierra el menú |

### Alertas

- Humedad < `Seco`: alerta baja.
- Humedad > `Muy húmedo`: alerta alta.
- Si `Alarmas` está ON, puede sonar.

---

## 15. Luz

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | LDR |
| Ubicación | Integrado |
| Pin | `GPIO39` |
| Lectura física | ADC 0..4095 |
| Rango usuario | 0..8000 lux aproximado |
| Modos visibles | `Lux`, `FC`, `Raw ADC` |
| Defaults | poca luz 100 lux, interior 500 lux, brillante 2000 lux, alertas ON, marcas OFF |

### Qué muestra

La pantalla `Luz` muestra la iluminación ambiental. El firmware convierte la lectura cruda del LDR a lux aproximado mediante una curva empírica y también permite ver `FC` o `Raw ADC`.

El LED RGB se apaga en vistas de solo Luz para no iluminar el LDR y alterar la lectura.

### Modos de lectura

| Modo | Uso |
|---|---|
| `Lux` | Lectura interpretativa principal para usuario |
| `FC` | Foot-candle, conversión desde lux |
| `Raw ADC` | Diagnóstico/calibración; lectura cruda promediada |

### Categorias

`Oscuro -> Poca luz -> Interior -> Brillante -> Sol`

Los límites editables separan las categorias intermedías:

- `Max. poca luz`
- `Max. interior`
- `Max. brillante`

### Modos visuales

- `Principal`: lectura de luz en el modo seleccionado.
- `Rango`: escala visual con marcas opciónales.
- `Ficha`: resumen con estado.
- `Dato`: valor destacado.
- `Curva`: evolución de luz; respeta `Lux`, `FC` o `Raw ADC`.

### Menú

| Opción | Función |
|---|---|
| `Modo` | Cambia `Lux`, `FC` o `Raw ADC` |
| `Límites` | Edita `Max. poca luz`, `Max. interior`, `Max. brillante` |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta |
| `Reset` | Restaura defaults de Luz |
| `Salir` | Cierra el menú |

### Alertas

- Lux < `Max. poca luz`: alerta baja.
- Lux >= `Max. brillante`: alerta alta.
- Si `Alarmas` está ON, puede sonar.

---

## 16. Sonido

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | Micrófono analógico GM19767P + LM358 |
| Ubicación | Integrado |
| Pin | `GPIO36` |
| Medición | Pico a pico en ventana de 20 ms |
| Rango usuario | 0..100 % relativo |
| Defaults | suave 20 %, normal 60 %, fuerte 85 %, alertas OFF, marcas OFF |

### Qué muestra

La pantalla `Sonido` muestra un nivel relativo de actividad acústica. No son decibelios calibrados. Sirve para comparar silencio, voz, ruido, palmas o música de forma visual.

### Categorias

`Suave -> Normal -> Ruidoso -> Muy fuerte`

### Modos visuales

Sonido tiene siete modos:

- `Principal`: lectura actual.
- `Sonido VU`: barras de reacción rápida.
- `Sonido Onda`: onda animada.
- `Rango`: posición frente a umbrales.
- `Ficha`: resumen.
- `Dato`: valor numérico.
- `Curva`: evolución/picos acumulados.

### Menú

| Opción | Función |
|---|---|
| `Límites` | Edita `Max. suave`, `Max. normal`, `Max. fuerte` |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta visual/LED |
| `Reset` | Restaura defaults de Sonido |
| `Salir` | Cierra el menú |

### Alertas

- Sonido >= `Max. normal`: alerta alta.
- Sonido >= `Max. fuerte`: alerta critica.
- Nunca emite beep propio por alerta de sonido.

---

## 17. Suelo

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | Humedad capacitiva de suelo |
| Tipo | Externo |
| Conector visible | `IO35` |
| Pin | `GPIO35` |
| Lectura física | ADC 0..4095 |
| Rango usuario | 0..100 % calibrado |
| Defaults calibración | seco raw 3550, mojado raw 1855 |
| Defaults límites | seco 20 %, óptimo 55 %, húmedo 80 %, alertas ON, marcas ON |

### Qué muestra

La pantalla `Suelo` muestra la humedad calibrada del sustrato. Si el sensor falta, muestra `Sin sensor` y `Revisa IO35`.

El sensor necesita calibración cuando cambia el tipo de suelo, humedad de referencia o unidad física del sensor.

### Categorias

La clasificacion deriva de los límites `Seco` y `Humedo`:

- `Muy seco`
- `Seco`
- `Óptimo`
- `Humedo`
- `Muy húmedo`

### Modos visuales

- `Principal`: humedad calibrada.
- `Rango`: posición frente a seco/húmedo.
- `Ficha`: resumen.
- `Dato`: porcentaje destacado.
- `Curva`: evolución temporal de humedad.

### Menú

| Opción | Función |
|---|---|
| `Calibrar sensor` | Captura seco y mojado reales |
| `Límites` | Edita seco, óptimo y húmedo |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta |
| `Reset` | Restaura calibración, límites, marcas y alertas |
| `Salir` | Cierra el menú |

### Calibración

Flujo recomendado:

1. Conectar el sensor de suelo en `IO35`.
2. Entrar en `Suelo`.
3. Pulsación larga para abrir menú.
4. Elegir `Calibrar sensor`.
5. En `Seco al aire`, dejar el sensor seco o en condicion seca de referencia; esperar estabilidad; elegir `Captura`.
6. En `En agua` o condicion muy humeda, introducir el sensor; esperar estabilidad; elegir `Captura`.
7. Revisar resumen `SECO/MOJADO`.
8. Elegir `Guardar` para escribir NVS o `Salir` para descartar.

La válidación exige que el raw seco sea mayor que el raw mojado y que la diferencia sea al menos 300 cuentas ADC. Si no se cumple, la calibración se rechaza.

Durante la calibración, una pulsación larga cancela el paso actual sin escribir NVS y vuelve al menú de Suelo.

### Alertas

- Muy seco/Seco: alerta baja.
- Óptimo: estado OK.
- Humedo: alerta de humedad.
- Muy húmedo: alerta critica.
- Si `Alarmas` está ON, Suelo puede reproducir melodías cortas distintas por estado.

---

## 18. Termómetro externo

### Ficha técnica

| Campo | Valor |
|---|---|
| Sensor | DS18B20 |
| Tipo | Externo |
| Conector visible | `IO33` |
| Pin | `GPIO33` |
| Bus | 1-Wire |
| Resolucion firmware | 9 bits |
| Rango técnico | -55..+125 °C |
| Defaults | bajo 0 °C, alto 40 °C, alertas OFF, marcas ON |

### Qué muestra

`Termómetro` muestra la temperatura puntual de una sonda externa. Es útil para medir agua, objetos, superficies o puntos concretos que no representan necesariamente el aire alrededor del P-Bit.

Si no hay sonda conectada o la lectura no es válida, muestra `Sin sensor`, `---` y `Revisa IO33`.

### Modos

- `Principal`: lectura actual.
- `Rango`: escala amplia de DS18B20 con referencia de 0 °C.
- `Ficha`: resumen de sonda.
- `Dato`: número protagonista.
- `Curva`: evolución temporal de la sonda.

### Menú

| Opción | Función |
|---|---|
| `Unidad` | Cambia Celsius/Fahrenheit global |
| `Límites` | Edita bajo y alto |
| `Marcas` | Muestra/oculta marcas en Rango |
| `Alertas` | Activa/desactiva alerta |
| `Reset` | Restaura defaults del Termómetro |
| `Salir` | Cierra el menú |

No existe offset/corrección configurable en la versión de producción actual.

---

## 19. Timer

Timer permite medir tiempo o ejecutar una cuenta regresiva.

### Cronometro

Si la duración configurada es `00:00:00`, funciona como cronómetro:

- pulsación corta: iniciar/pausar;
- pulsación larga: resetear cuando corresponde;
- formato adaptativo para mostrar tiempo.

### Cuenta regresiva

Si la duración es mayor que cero, funciona como cuenta regresiva:

- pulsación corta: iniciar/pausar;
- al llegar a cero, la pantalla cambia a rojo;
- si `Alarmas` está ON, suena una alarma intermitente corta.

### Editor

La pulsación larga abre el editor de duración:

- seleccionar campo `HH`, `MM` o `SS`;
- pulsar para editar;
- girar para cambiar valor;
- pulsar para volver a selección;
- mantener pulsado para guardar.

Límites:

- horas: 0..23;
- minutos: 0..59;
- segundos: 0..59.

---

## 20. Sistema y configuración global

### Pantalla Sistema

Muestra información general:

- ID del dispositivo;
- tiempo/uptime;
- idioma;
- estado de `Bip`;
- estado de `Alarmas`;
- indicador BLE compacto solo si BLE fue activado internamente.

### Menú Sistema

| Opción | Función |
|---|---|
| `Bip` | Activa/desactiva beeps de interfaz |
| `Alarmas` | Activa/desactiva audio de alertas y Timer |
| `Reposo` | Ajusta tiempo hasta overlay `ZZZ` |
| `Idioma` | Cambia Español/Catalán/English |
| `Reset` | Borra toda la configuración |
| `Salir` | Cierra menú |

### Reposo

Opciones:

- `30 seg`
- `1 min`
- `2 min`
- `5 min`
- `10 min`
- `Nunca`

El reposo de producto muestra overlay `ZZZ`. El dispositivo despierta con giro o pulsación del encoder.

### Reset global

`Sistema > Reset` limpia NVS y restaura:

- idioma;
- unidad C/F;
- límites;
- marcas;
- alertas;
- calibración de Suelo;
- modos de Luz;
- reposo;
- Bip/Alarmas;
- modos persistidos de Sensor Zone;
- BLE apagado.

Después del reset, el flujo vuelve al estado inicial y debe mostrarse selector de idioma en el siguiente arranque limpio.

---

## 21. Persistencia y valores por defecto

El P-Bit guarda configuración en NVS, namespace `pbit`.

| Área | Claves principales | Default |
|---|---|---|
| Idioma | `lang` | Español |
| Unidad temperatura | `sys_unit_f` | Celsius |
| Bip | `sys_sound` | OFF |
| Alarmas | `sys_alarm` | OFF |
| Reposo | `sys_sleep` | 2 min |
| BLE | `ble_en` | OFF |
| Sensor Zone | `sz_sen`, `sz_v0..sz_v5` | Temperatura / Principal |
| Temperatura | `tmp_low`, `tmp_high`, `tmp_aen`, `tmp_marks` | 18/28 °C, alertas OFF, marcas ON |
| Humedad | `hum_dry_max`, `hum_comf_max`, `hum_alert_en`, `hum_marks` | 30/70 %, alertas ON, marcas ON |
| Luz | `lgt_dim`, `lgt_ind`, `lgt_bri`, `lgt_mode`, `lgt_aen`, `lgt_marks` | 100/500/2000 lux, Lux, alertas ON, marcas OFF |
| Sonido | `snd_quiet`, `snd_norm`, `snd_loud`, `snd_aen`, `snd_marks` | 20/60/85 %, alertas OFF, marcas OFF |
| Suelo | `soil_dry`, `soil_wet`, `soil_thr_dry`, `soil_thr_opt`, `soil_thr_moi`, `soil_aen`, `soil_marks` | 3550/1855 raw, 20/55/80 %, alertas ON, marcas ON |
| Termómetro | `d18_alow`, `d18_ahigh`, `d18_aen`, `d18_marks` | 0/40 °C, alertas OFF, marcas ON |
| Firmware stamp | `fw_stamp` | fuerza reset de NVS en firmware nuevo |

---

## 22. Modo demo

El Modo demo permite mostrar el P-Bit en rotación automática, con valores simulados y transiciones suaves.

Activación del Modo demo:

- mantener pulsado el encoder durante el logo de arranque;
- o pulsación larga desde `Inicio`.

Comportamiento:

- muestra splash breve de entrada;
- recorre escenas representativas de producción;
- anima valores sin escribir NVS;
- bloquea reposo automático;
- sale con cualquier giro o pulsación posterior.

En la versión actual, Demo recorre Inicio, Clima Lab, Termo Lab, Sensor Zone con varios sensores/modos, Sonido VU/Onda y Timer. `Planta Lab` pertenece al carrusel de producción, pero no está incluida actualmente en las escenas demo.

---

## 23. Solución de problemas

| Problema | Causa probable | Acción |
|---|---|---|
| No enciende | Baterías agotadas o alimentación inestable | Cambiar 3 AAA; probar USB-C solo como diagnóstico |
| Pantalla muestra `Sin sensor` en Suelo | Sensor ausente o lectura inválida | Revisar conexión `IO35` |
| Pantalla muestra `Sin sensor` en Termómetro | DS18B20 ausente o mal conectado | Revisar conexión `IO33` |
| Suelo muestra porcentajes raros | Calibración no corresponde al sustrato | Recalibrar en `Suelo > Calibrar sensor` |
| Luz no cambia como se espera | LDR tapado, RGB externo o modo Raw | Destapar LDR; usar modo `Lux`; evitar iluminarlo directamente con LED |
| Sonido no llega a valores altos | Umbrales/gain del micrófono o entorno silencioso | Probar palmas cerca; interpretar como valor relativo |
| No suenan beeps de menú | `Bip` OFF | Activar `Sistema > Bip` |
| No suenan alertas | `Alarmas` OFF o alerta de Sonido | Activar `Sistema > Alarmas`; recordar que Sonido no emite beep |
| Pantalla entra en `ZZZ` | Reposo automático | Girar/pulsar encoder o cambiar `Sistema > Reposo` |
| WiFi no aparece | Firmware inicial no tiene flujo WiFi | Requiere desarrollo/configuración adicional |
| BLE no aparece en escáner | BLE sale OFF de fábrica | Correcto para producción/consumidor |

---

## 24. Mantenimiento y recomendaciones

- Usar 3 baterías AAA nuevas o en buen estado para actividades.
- Retirar baterías si el dispositivo no se usará durante mucho tiempo.
- Mantener seco el P-Bit; no es resistente al agua.
- Conectar sensores externos con el dispositivo apagado cuando sea posible.
- Recalibrar Suelo al cambiar de sustrato, sensor o condiciones de referencia.
- No usar Sonido como sonómetro legal ni Luz como luxómetro certificado.
- Verificar BLE apagado antes de entregar unidades de producción.
- Mantener limpio el LDR y sin obstrucciones.
- Evitar tirar del cable USB-C durante programación o pruebas.

---

## 25. Usos educativos

### Diario de una planta

Usar `Planta Lab`, `Suelo`, `Luz`, `Temperatura` y `Humedad` durante varios días. Registrar cambios y relacionarlos con riego, luz solar o ubicación.

### Comparación de zonas

Comparar aula, pasillo, exterior y cerca de ventanas. Usar `Inicio`, `Clima Lab`, `Luz` y `Sonido`.

### Experimento de riego

Calibrar Suelo, medir antes/después de regar y observar la `Curva`. Discutir por qué el porcentaje cambia gradualmente.

### Mapa de sonido

Usar `Sonido VU` y `Sonido Onda` para comparar ambientes. Recordar que el valor es relativo, no dB.

### Transferencia de calor

Usar `Termo Lab` y `Termómetro` para comparar agua fría/caliente, materiales o superficies.

### Tiempo y método científico

Usar `Timer` para medir duraciónes iguales durante experimentos y relacionar tiempo con cambio de lectura.

---

## 26. Documentos relacionados

| Documento | Uso |
|---|---|
| `docs/USER_GUIDE.md` | Manual practico derivado para usuario/aula |
| `docs/PROJECT.md` | Descripción resumida del producto y mapa documental |
| `docs/TECHNICAL.md` | Referencia técnica profunda de firmware, NVS, BLE y arquitectura |
| `docs/DESIGN_SYSTEM.md` | Sistema visual, colores, iconos y reglas UI |
| `docs/TFT_RENDER_RULES.md` | Reglas anti-flicker y render TFT |
| `docs/PRODUCTION_CHECKLIST.md` | Checklist de entrega y válidación |
| `CHANGELOG.md` | Historial de cambios |
