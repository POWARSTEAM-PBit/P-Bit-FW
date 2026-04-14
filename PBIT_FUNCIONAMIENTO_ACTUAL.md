# P-Bit: Funcionamiento Actual del Firmware

Actualizado: 2026-04-10

Este documento explica qué hace hoy el P-Bit con el código actual, cómo se usa y qué posibilidades educativas ofrece en contextos STEAM ambientales.

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
- Luz con `LDR`
- Sonido ambiental con micrófono analógico
- Humedad del suelo con sensor capacitivo externo en `J6`
- Temperatura externa con sonda `DS18B20`

Importante a nivel de hardware:

- La placa V3.1 sí expone un bus I2C físico en `GPIO26` (`SDA`) y `GPIO27` (`SCL`).
- Ese bus no choca con la TFT, porque el display usa `GPIO21` (`RST`) y `GPIO22` (`DC`).
- Aun así, el firmware actual no usa `Wire` ni sensores I2C; hoy todo el producto funciona con `DHT11`, `ADC` y `DS18B20` por `1-Wire`.

Además, el sistema también muestra:

- estado BLE
- idioma activo
- tiempo de encendido (`UP`)
- cronómetro independiente

## 3. Cómo se usa

### Arranque

- En encendido en frío, primero aparece el selector de idioma.
- El usuario puede elegir `Español`, `Catalán` o `English`.
- El código conserva soporte para wake desde `deep sleep`, pero el reposo automático actual del producto se queda en un modo visible con `ZZZ` y no usa deep sleep automático.

### Navegación principal

La navegación entre pantallas se hace con el encoder rotatorio.

Orden actual de pantallas:

- `Temperatura`
- `Humedad`
- `Luz`
- `Sonido`
- `Suelo`
- `DS18B20`
- `Sistema`
- `Timer`
- `Gráfica`

### Botón del encoder

El encoder tiene dos tipos de acción:

- pulsación corta
- pulsación larga

Acciones rápidas actuales:

- `Temperatura`: alterna la unidad global compartida `C/F`
- `DS18B20`: alterna la misma unidad global compartida `C/F`
- `Sistema`: alterna `Sonido ON/OFF`
- `Timer`: con pulsación corta inicia/pausa; con pulsación larga abre el selector de minutos si está idle y resetea si ya estaba corriendo o pausado
- `Gráfica`: pulsación corta cambia el sensor mostrado (`Temperatura` ↔ `Humedad`)

En la mayoría de pantallas de sensores, la pulsación larga (~1.2 s) abre el menú propio de esa pantalla.

## 4. Qué hace cada pantalla

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
- la unidad visible es compartida con `DS18B20`

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

Nota:

- en esta pantalla el LED RGB se apaga para no interferir con la medición del LDR

### Sonido

Mide nivel de sonido ambiental en porcentaje y lo representa con barra horizontal.

Funciones actuales:

- menú de calibración de umbrales
- menú de alertas `ON/OFF`
- menú `Reset`
- categorías: `Silencio`, `Tranquilo`, `Normal`, `Ruidoso`, `Muy ruidoso`
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

### DS18B20

Mide temperatura externa con sonda.

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

- nombre del dispositivo
- uptime
- estado BLE
- idioma activo
- estado de sonido

Menú actual:

- `Sonido`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

Desde aquí se puede:

- encender o apagar sonido global
- elegir tiempo de reposo
- cambiar idioma
- resetear configuraciones

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
- avisar visualmente en rojo y con beep corto al terminar una cuenta regresiva si el sonido global está activo

El valor `00:00:00` funciona como cronómetro ascendente. Cualquier otro valor funciona como cuenta regresiva.

### Gráfica

Muestra la evolución temporal de un sensor como gráfica de línea en pantalla completa.

Funciones actuales:

- historial de las últimas ~2 min 40 s (160 muestras a 1 muestra/s)
- auto-escalado del eje Y con rango mínimo y margen de respiración
- cambio de sensor (Temperatura ↔ Humedad) con pulsación corta del encoder
- etiquetas dimmed de valor mínimo y máximo en las esquinas de la gráfica
- render sin parpadeo mediante sprite de hardware
- sensores disponibles actualmente: `Temperatura` y `Humedad`

Nota:

- la pantalla no tiene menú de configuración; en futuras versiones podría añadirse un rango fijo o más sensores

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

## 6. Gestión de energía

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

## 7. Persistencia

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
- estado global de sonido

Esto significa que el equipo recuerda la configuración entre reinicios.

## 8. Qué valor educativo tiene hoy

Con el firmware actual, el P-Bit ya sirve para actividades STEAM ambientales reales.

### Ciencias

Permite observar fenómenos del entorno:

- cómo cambia la temperatura durante el día
- qué ocurre con la humedad del suelo después del riego
- cómo influye la luz en una planta
- qué zonas del aula son más ruidosas

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

## 9. Ejemplos de uso STEAM ambiental

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

### Ejemplo 4: Mapa de ruido del colegio

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

- usar el `DS18B20` como instrumento externo de exploración

Actividad:

- medir temperatura en distintos recipientes o materiales
- comparar sombra y sol
- comparar interior y exterior

Aprendizajes:

- diferencias entre temperatura ambiente y temperatura puntual
- uso de sonda externa
- precisión y calibración

## 10. Estado actual y límites

Aunque el firmware ya es funcional y útil, todavía hay aspectos en evolución:

- la UX visual todavía puede refinarse más
- el timer ya permite editar `HH:MM:SS`, pero aún no tiene funciones de experimento más avanzadas
- la pantalla de sonido sigue siendo interpretación por umbrales, no calibración física
- la pantalla de gráfica es un prototipo funcional validado en hardware; quedan ajustes visuales finos y la ampliación a más sensores

Esto no impide su uso educativo actual, pero sí marca oportunidades claras para las siguientes iteraciones.

## 11. Resumen breve

Hoy el P-Bit ya es una plataforma educativa ambiental funcional que:

- mide variables relevantes del entorno
- permite navegar y configurar cada sensor
- guarda ajustes
- usa alertas visuales, LED y sonido
- gestiona reposo automático
- muestra la evolución temporal de sensores como gráfica de línea interactiva
- sirve para actividades STEAM reales con plantas, clima, luz, ruido y análisis del entorno

En su estado actual, ya puede usarse como herramienta de observación, experimentación y aprendizaje en educación ambiental.
