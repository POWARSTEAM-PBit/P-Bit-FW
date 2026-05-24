# P-Bit: Funcionamiento Actual del Firmware

Actualizado: 2026-05-24

Este documento explica qué hace hoy el P-Bit con el código actual, cómo se usa y qué posibilidades educativas ofrece en contextos STEAM ambientales.

## Estado de revisión de producción/i18n

- Build OK `esp32dev` con PlatformIO.
- Flags de producción revisadas: `PBIT_ENABLE_GRAPH_LAB=1`, `PBIT_ENABLE_SERIAL_PLOTTER=0` y `FIRMWARE_DEBUG` comentado.
- i18n centralizado para `ES/CAT/EN` mediante el diccionario de `languages.h` y `lang_select.cpp`.
- Cambio de idioma completo desde el selector inicial y desde `Sistema > Idioma`.
- LDR corregido para la polaridad de la placa actual y validado en firmware al rango `0..20000 lux`.
- BLE factory-off: apagado por defecto y oculto en carrusel; la activación queda documentada solo en material técnico interno.
- Sensor Zone activo para los 6 sensores con modos `Focus`, `Valor`, `Gráfica`, `Dial` y `Card`.
- `Sistema` separa `Bip` (sonidos de UI) y `Alarmas` (alertas/timer audibles), con persistencia independiente.
- Refresco de UI acotado y orientado a anti-flicker: shell estático, campos dinámicos y cadencias específicas por pantalla.

La build y la validación de rango del LDR constan a nivel de firmware. Este documento no afirma una calibración física certificada ni una validación con luxómetro de cada unidad.

## 1. Qué es el P-Bit

El P-Bit es un dispositivo educativo ambiental basado en ESP32. Su objetivo es ayudar a niñas y niños a observar, medir e interpretar condiciones del entorno de forma visual, sonora e interactiva.

Con el firmware actual, el P-Bit funciona como:

- estación ambiental portátil
- interfaz de exploración con pantalla y encoder
- herramienta para observar variables de una planta o de un espacio
- dispositivo de aprendizaje STEAM con menús configurables y alertas

## 2. Qué mide el P-Bit hoy

El firmware actual ya integra estas lecturas:

- Temperatura ambiente con `DHT11`
- Humedad relativa del aire con `DHT11`
- Luz con `LDR`, corregido en firmware y limitado a `0..20000 lux`
- Sonido ambiental con micrófono analógico
- Humedad del suelo con sensor capacitivo externo en `J6`
- Temperatura externa con sonda `DS18B20`

Importante a nivel de hardware:

- La placa V3.1 sí expone un bus I2C físico en `GPIO26` (`SDA`) y `GPIO27` (`SCL`).
- Ese bus no choca con la TFT, porque el display usa `GPIO21` (`RST`) y `GPIO22` (`DC`).
- Aun así, el firmware actual no usa `Wire` ni sensores I2C; hoy todo el producto funciona con `DHT11`, `ADC` y `DS18B20` por `1-Wire`.

Además, el sistema también muestra:

- estado BLE, solo si Bluetooth está activado
- idioma activo
- tiempo de encendido (`Tiempo` / `Uptime`)
- cronómetro independiente

## 3. Cómo se usa

### Arranque

- En encendido en frío, primero aparece el selector de idioma.
- El usuario puede elegir `Español`, `Catalán` o `English`.
- La selección de idioma usa el mismo diccionario central que el resto de la interfaz, de modo que títulos, menús, estados e instrucciones cambian como conjunto.
- El código conserva soporte para wake desde `deep sleep`, pero el reposo automático actual del producto se queda en un modo visible con `ZZZ` y no usa deep sleep automático.

### Navegación principal

La navegación entre pantallas se hace con el encoder rotatorio.

Orden actual de pantallas (carrusel circular de 12 posiciones con `PBIT_ENABLE_GRAPH_LAB=1`):

- `Home` — vista Lab de visión global de sensores en cards
- `Clima` — vista Lab de temperatura y humedad del aire en card combinado
- `Multi` — vista Lab de múltiples sensores con widgets
- `Sonido VU` — vista Lab de nivel de sonido ambiental en barras apiladas
- `Temperatura` — zona de sensor con modos visuales y menú de límites/alertas
- `Humedad` — zona de sensor con modos visuales y menú de límites/alertas
- `Luz` — zona de sensor con modos visuales y menú de calibración/alertas
- `Sonido` — zona de sensor con modos visuales y menú de umbrales/alertas
- `Suelo` — zona de sensor con modos visuales, calibración y alertas
- `Termómetro` — zona de sensor para sonda externa `DS18B20`, con offset y alertas
- `Timer` — cronómetro y cuenta regresiva
- `Sistema` — ajustes globales del dispositivo

### Botón del encoder

El encoder tiene dos tipos de acción:

- pulsación corta
- pulsación larga

Acciones rápidas actuales:

- `Home`, `Clima`, `Multi`, `Sonido VU`: sin acciones de pulsación — son pantallas de solo lectura
- `Temperatura`, `Humedad`, `Luz`, `Sonido`, `Suelo`, `Termómetro`: pulsación corta alterna el modo de visualización del sensor; pulsación larga (~1.2 s) abre el menú de configuración del sensor
- `Timer`: pulsación corta inicia/pausa; pulsación larga abre el editor de duración `HH:MM:SS` si está idle o resetea si ya estaba corriendo o pausado
- `Sistema`: pulsación corta alterna `Bip ON/OFF`

Notas:

- La unidad `C/F` es global y compartida entre `Temperatura` y `Termómetro`; se cambia desde el menú de cualquiera de los dos.
- Si `PBIT_ENABLE_GRAPH_LAB` se compila a `0`, el carrusel vuelve a las pantallas clásicas y `GRAPH_SCREEN` queda como pantalla independiente al final.
- `LAB_LINEAR_DASH_SCREEN` sigue compilado como renderer Lab disponible para iteración visual, pero no aparece como posición independiente en el carrusel de usuario actual.

## 4. Qué hace cada pantalla

### Home

Muestra una visión global de todos los sensores en formato de cards. Es la primera pantalla que aparece al encender el dispositivo (tras el selector de idioma en frío).

### Clima

Muestra temperatura ambiente y humedad del aire en un card combinado. Permite leer de un vistazo las dos variables principales del entorno.

### Multi

Vista de múltiples sensores con widgets. Combina varias lecturas en una sola pantalla para una lectura rápida del estado ambiental completo.

### Sonido VU

Muestra el nivel de sonido ambiental en barras apiladas estilo VU meter. Útil para detectar de forma visual e inmediata el nivel sonoro del entorno.

### Pantallas Lab compiladas

La revisión mantiene una familia de pantallas Lab para probar y estabilizar formatos visuales:

- `Home Cards`
- `Dual TH` / `Clima`
- `Widget Mix` / `Multi`
- `Linear Dash`
- `Sound VU`
- `Sensor Focus`, `Valor`, `Gauge/Dial`, `Sensor Card` y `Graph`, usados desde `SENSOR_ZONE_SCREEN`

No todas son posiciones independientes del carrusel de uso normal. Las expuestas al usuario son `Home`, `Clima`, `Multi`, `Sonido VU` y los modos de Sensor Zone.

### Zona de sensores

Las seis posiciones de sensor (`Temperatura`, `Humedad`, `Luz`, `Sonido`, `Suelo`, `Termómetro`) comparten `SENSOR_ZONE_SCREEN`.

Cada sensor recuerda su propio modo visual:

- `Focus`: lectura protagonista
- `Valor`: valor grande con contexto visual
- `Gráfica`: histórico reciente del sensor
- `Dial`: lectura tipo gauge
- `Card`: tarjeta compacta

La pulsación corta avanza entre esos modos. La pulsación larga abre el menú de configuración real del sensor activo.

### Temperatura

Muestra la temperatura ambiente con un tanque visual y color por rango.

Funciones actuales:

- cambio rápido `C/F`
- menú de límites de alerta
- menú de unidad
- menú de alertas `ON/OFF`
- menú `Reset`
- guardado persistente
- alerta visual, LED y sonido al salir del rango
- la unidad visible es compartida con `Termómetro`

### Humedad del aire

Muestra la humedad relativa con una visualización vertical.

Funciones actuales:

- menú de límites `Seco` y `Muy húmedo`
- rango intermedio interpretado como `Óptimo`
- menú de alertas `ON/OFF`
- menú `Reset`
- guardado persistente
- alerta visual, LED y sonido cuando el ambiente está demasiado seco o demasiado húmedo

### Luz

Mide luz ambiental y la presenta como valor grande + barra horizontal.

Funciones actuales:

- menú de calibración de umbrales
- menú de modo de display
- menú de alertas `ON/OFF`
- menú `Reset`
- modos visibles: `Lux`, `% log`, `Raw ADC`
- categorización visual: `Oscuro`, `Tenue`, `Interior`, `Brillante`, `Luz solar`
- guardado persistente
- conversión corregida para que más ADC corresponda a más lux en la placa actual
- recorte de seguridad a `0..20000 lux`

Nota:

- en esta pantalla el LED RGB se apaga para no interferir con la medición del LDR
- el rango y la polaridad están validados por firmware/build; la comprobación física de cada unidad debe hacerse como parte de producción

### Sonido

Mide nivel de sonido ambiental en porcentaje y lo representa con barra horizontal.

Funciones actuales:

- menú de calibración de umbrales
- menú de alertas `ON/OFF`
- menú `Reset`
- categorías: `Silencio`, `Suave`, `Normal`, `Fuerte`, `Muy fuerte`
- guardado persistente
- alertas visuales y RGB cuando supera niveles definidos

Nota:

- hoy este menú funciona como ajuste de interpretación y alertas, no como calibración física del micrófono

### Suelo

Mide humedad del suelo con sensor capacitivo externo.

Funciones actuales:

- calibración del sensor con referencia en seco y en agua
- edición de umbrales porcentuales
- menú de alertas `ON/OFF`
- menú `Reset`
- categorías: `Seco`, `Óptimo`, `Húmedo`, `Muy húmedo`
- guardado persistente
- detección de ausencia de sensor

### Termómetro / DS18B20

Mide temperatura externa con sonda.

`DS18B20` es el identificador técnico de la sonda y de parte del firmware; el título visible de producto es `Termómetro` o `Termo` cuando hay que abreviar.

Funciones actuales:

- cambio rápido `C/F`
- ajuste de `offset`
- límites de alerta alto y bajo
- menú de unidad
- menú de alertas `ON/OFF`
- menú `Reset`
- guardado persistente
- alerta visual, LED y sonido
- detección de ausencia de sensor
- comparte la unidad global `C/F` con la pantalla de `Temperatura`

### Sistema

Es el centro de ajustes globales del dispositivo.

La pantalla normal muestra:

- `ID` del dispositivo
- `Tiempo`/`Uptime`
- idioma activo
- estado BLE solo si el BLE está habilitado en NVS
- estado de `Bip`
- estado de `Alarmas`

La pantalla usa cards internas: identificación arriba, `Tiempo` e `Idioma` al centro, y audio abajo. BLE desaparece por completo cuando Bluetooth está desactivado, para no confundir a los usuarios en un dispositivo que sale de fábrica sin BLE.

Menú actual:

- `Bip`
- `Alarmas`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

El menú raíz de Sistema usa grid 2×3 para evitar una lista vertical amontonada en 160×128 px.

Desde aquí se puede:

- encender o apagar `Bip`: beeps de interfaz, navegación y confirmaciones
- encender o apagar `Alarmas`: audio de alertas y final de cuenta regresiva del `Timer`
- elegir tiempo de reposo
- cambiar idioma de forma completa entre `ES`, `CAT` y `EN`
- resetear configuraciones

### BLE oculto

BLE es una función interna de fábrica/debug: no aparece en la navegación normal ni forma parte del flujo de aula. De fábrica siempre sale `OFF`, vuelve a `OFF` tras instalar una build nueva por el reset de build-hash y con `ble_en == false` la pantalla normal de `Sistema` oculta la fila BLE.

### Timer

Es un temporizador rápido de uso directo con modo cronómetro y edición directa de `HH:MM:SS`.

Funciones actuales:

- iniciar
- pausar
- resetear
- abrir un editor `HH:MM:SS` con pulsación larga cuando está idle
- elegir horas, minutos o segundos con el encoder y confirmar cada edición con pulsación corta
- guardar la nueva duración con una pulsación larga
- mostrar la duración activa debajo de la tarjeta principal solo cuando hay cuenta regresiva
- usar formato adaptativo `MM:SS:CC` por debajo de una hora y `HH:MM:SS` a partir de una hora
- avisar visualmente en rojo y con alarma corta al terminar una cuenta regresiva si `Alarmas` está activo

El valor `00:00:00` funciona como cronómetro ascendente. Cualquier otro valor funciona como cuenta regresiva.

### Modo Gráfica

Con `PBIT_ENABLE_GRAPH_LAB=1`, la gráfica ya no se presenta como pantalla separada del carrusel: aparece como modo `Gráfica` dentro de cada sensor de `SENSOR_ZONE_SCREEN`.

Los 6 sensores tienen buffer circular propio, con 160 muestras a 1 muestra/s. La vista de gráfica selecciona el sensor activo, autoescala el eje Y y usa paleta propia por sensor.

`GRAPH_SCREEN` sigue existiendo en el firmware como renderer reutilizable y como pantalla independiente cuando se compila sin el carrusel lab/sensor-zone.

## 5. Alertas y feedback

El P-Bit ya usa feedback multimodal:

- color en pantalla
- LED RGB
- sonido

Esto permite que la información sea visible incluso si el usuario no está leyendo el número exacto.

En el estado actual:

- `Sound` se mantiene sin alerta sonora local para no contaminar la lectura del micrófono
- la lógica global de alertas ya puede disparar audio selectivo y RGB en segundo plano, aunque la capa visual global en pantalla quedó retirada temporalmente

Ejemplos:

- temperatura alta: rojo
- temperatura baja: azul
- sonido alto: naranja o rojo
- humedad del suelo óptima: verde

## 6. Refresco de pantalla y anti-flicker

La UI actual está organizada para reducir parpadeos en la TFT:

- cada pantalla recibe `screen_changed` y `sensor_data_changed`
- los elementos fijos se dibujan al entrar en pantalla
- los valores cambiantes limpian y redibujan solo zonas acotadas
- Sensor Zone sincroniza el renderer solo cuando cambia el sensor o el modo visual
- el LDR se muestrea a unos 5 Hz, el suelo a alrededor de 1 Hz y el sonido mantiene la cadencia rápida necesaria para VU
- `Timer` refresca a 40 ms cuando muestra centésimas y a 100 ms en el resto de formatos
- `Sistema` refresca su información periódica sin redibujar toda la pantalla cada ciclo

Esto mejora la lectura en aula y evita que números, barras o gráficas produzcan parpadeo innecesario.

## 7. Gestión de energía

El firmware actual ya implementa reposo automático visible.

Comportamiento actual:

- si el usuario selecciona `Nunca`, no hay reposo automático
- con cualquier timeout activo, el sistema entra en `IDLE` visible
- durante ese estado aparece un overlay de reposo con `ZZZ`
- si el usuario interactúa, el equipo vuelve a `ACTIVE`
- el deep sleep automático está desactivado en la práctica porque en esta revisión de hardware la TFT queda blanca al dormir

Además:

- si hay un menú abierto, el reposo automático se bloquea
- si el timer está corriendo, el reposo automático también se bloquea

## 8. Persistencia

El P-Bit guarda configuraciones importantes en memoria no volátil (`NVS`).

Esto incluye:

- idioma
- límites de temperatura
- alertas de temperatura
- umbrales y alertas de humedad
- configuración de luz
- configuración de sonido
- calibración y umbrales de suelo
- offset y alertas de DS18B20
- tiempo de reposo
- estado global de `Bip` (`sys_sound`)
- estado global de `Alarmas` (`sys_alarm`)
- estado BLE (`ble_en`)
- sensor activo y modo visual por sensor (`sz_sen`, `sz_v0..sz_v5`)

Esto significa que el equipo recuerda la configuración entre reinicios.

## 9. Qué valor educativo tiene hoy

Con el firmware actual, el P-Bit ya sirve para actividades STEAM ambientales reales.

### Ciencias

Permite observar fenómenos del entorno:

- cómo cambia la temperatura durante el día
- qué ocurre con la humedad del suelo después del riego
- cómo influye la luz en una planta
- qué zonas del aula tienen más sonido ambiental

### Tecnología

Permite comprender:

- cómo diferentes sensores producen datos
- cómo una interfaz ayuda a interpretar mediciones
- cómo una configuración cambia el comportamiento del sistema

### Ingeniería

Permite experimentar con:

- calibración de sensores
- definición de límites
- diseño de alertas útiles
- ahorro de energía con modos de reposo

### Matemáticas

Permite trabajar con:

- porcentajes
- comparación de valores
- rangos
- tiempo
- registro y análisis de datos

## 10. Ejemplos de uso STEAM ambiental

### Ejemplo 1: Diario de una planta

Objetivo:

- observar qué necesita una planta para mantenerse en estado óptimo

Actividad:

- medir suelo, luz, temperatura y humedad cada día
- anotar cuándo la planta está en condiciones buenas o malas
- comparar días con riego y sin riego

Aprendizajes:

- interpretación de variables ambientales
- relación entre datos y salud de la planta
- formulación de hipótesis

### Ejemplo 2: Comparación de microclimas en el aula

Objetivo:

- descubrir si todas las zonas del aula tienen las mismas condiciones

Actividad:

- medir cerca de una ventana, una puerta, una pared interior y una estantería
- comparar luz, temperatura y sonido

Aprendizajes:

- toma de datos comparativa
- visualización de diferencias
- análisis espacial del entorno

### Ejemplo 3: Riego inteligente sin automatización

Objetivo:

- aprender a decidir cuándo regar

Actividad:

- calibrar el sensor de suelo
- definir umbrales
- observar durante varios días cómo baja la humedad
- decidir el momento de riego en función de evidencia

Aprendizajes:

- uso de umbrales
- criterio basado en datos
- pensamiento de ingeniería

### Ejemplo 4: Mapa de sonido del colegio

Objetivo:

- entender cómo cambia el sonido según el lugar y el momento

Actividad:

- medir sonido en aula, pasillo, patio y biblioteca
- registrar picos y promedios
- comparar recreo vs clase

Aprendizajes:

- recolección de datos
- representación comparativa
- conversación sobre bienestar acústico

### Ejemplo 5: Experimento de luz y crecimiento

Objetivo:

- estudiar si una planta recibe suficiente luz en distintos lugares

Actividad:

- colocar la planta en varias posiciones
- registrar nivel de luz y evolución visible
- usar el modo `Lux` o `% log`

Aprendizajes:

- relación entre luz y crecimiento
- medición repetida
- interpretación de categorías

### Ejemplo 6: Sonda externa y exploración térmica

Objetivo:

- usar el Termómetro (`DS18B20`) como instrumento externo de exploración

Actividad:

- medir temperatura en distintos recipientes o materiales
- comparar sombra y sol
- comparar interior y exterior

Aprendizajes:

- diferencias entre temperatura ambiente y temperatura puntual
- uso de sonda externa
- precisión y calibración

## 11. Estado actual y límites

Aunque el firmware ya es funcional y útil, todavía hay aspectos en evolución:

- la UX visual todavía puede refinarse más
- el timer ya permite editar `HH:MM:SS`, pero aún no tiene funciones de experimento más avanzadas
- la pantalla de sonido sigue siendo interpretación por umbrales, no calibración física
- la pantalla de gráfica ya cubre los 6 sensores; quedan ajustes visuales finos opcionales
- el LDR está corregido y acotado en firmware, pero no se documenta aquí una calibración física certificada por luxómetro
- BLE debe comprobarse apagado con escaneo externo antes de entregar unidades, aunque el firmware lo fuerce `OFF` de fábrica

Esto no impide su uso educativo actual, pero sí marca oportunidades claras para las siguientes iteraciones.

## 12. Resumen breve

Hoy el P-Bit ya es una plataforma educativa ambiental funcional que:

- mide variables relevantes del entorno
- permite navegar y configurar cada sensor
- ofrece interfaz completa en Español, Catalán e Inglés
- guarda ajustes
- usa alertas visuales, LED y sonido
- gestiona reposo automático
- muestra la evolución temporal de los 6 sensores como gráfica de línea interactiva con paleta por sensor
- mantiene BLE oculto y apagado de fábrica
- compila correctamente para `esp32dev`
- sirve para actividades STEAM reales con plantas, clima, luz, sonido y análisis del entorno

En su estado actual, ya puede usarse como herramienta de observación, experimentación y aprendizaje en educación ambiental.
