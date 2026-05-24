# Manual de Usuario del P-Bit

Actualizado: 2026-05-24

Este manual explica qué es el P-Bit, cómo usarlo y cómo aprovecharlo en actividades de observación ambiental, aula, laboratorio escolar y proyectos STEAM.

Estado de esta revisión:

- build OK `esp32dev`
- interfaz disponible en Español, Catalán e Inglés
- cambio de idioma centralizado para toda la interfaz
- lectura de luz corregida y validada en firmware al rango `0..20000 lux`
- BLE no forma parte del flujo normal de usuario
- nuevas vistas Lab y Sensor Zone activas
- `Sistema` separa `Bip` y `Alarmas`
- refresco de pantalla acotado para reducir parpadeos
- Timer y reposo visible con `ZZZ` incluidos en el flujo normal

## 1. ¿Qué es el P-Bit?

El P-Bit es un dispositivo educativo ambiental que ayuda a observar el entorno de forma simple, visual e interactiva. Permite medir distintas variables, configurar alertas y explorar cómo cambian las condiciones de un espacio, una planta, un terrario o un experimento.

Está pensado para aprender haciendo:

- medir
- comparar
- registrar
- interpretar
- tomar decisiones a partir de datos

## 2. ¿Qué puede medir?

Con el código actual, el P-Bit puede medir:

- temperatura ambiente
- humedad del aire
- luz ambiental en rango de firmware `0..20000 lux`
- sonido ambiental
- humedad del suelo
- temperatura externa con sonda Termómetro

Además, muestra información general del sistema:

- nombre del dispositivo
- tiempo encendido
- idioma activo
- estado de `Bip`
- estado de `Alarmas`
- cronómetro

## 3. Partes principales del equipo

### Entradas

- sensor de temperatura y humedad ambiente
- sensor de luz
- sensor de sonido
- puerto para sensor de humedad de suelo
- puerto para sonda Termómetro (`DS18B20` como identificador técnico)
- encoder rotatorio con botón

### Salidas

- pantalla
- LED RGB
- buzzer

La conectividad BLE queda fuera del flujo normal de uso del aula y de este manual.

## 4. Primer uso

### Encendido

Cuando enciendes el P-Bit desde cero:

1. aparece el selector de idioma
2. gira el encoder para elegir idioma
3. pulsa para confirmar

Idiomas disponibles:

- Español
- Catalán
- English

El idioma se aplica de forma centralizada: los títulos, menús, estados, instrucciones y pantallas principales usan el mismo diccionario de textos. También puedes cambiarlo más tarde desde `Sistema > Idioma`.

El reposo automático actual del equipo mantiene una pantalla visible con `ZZZ`. El código conserva soporte técnico para deep sleep, pero en esta revisión el flujo normal de uso es ese reposo visible, no el apagado profundo automático.

## 5. Cómo se navega

### Girar el encoder

Sirve para:

- cambiar de pantalla
- moverse por menús
- cambiar valores

### Pulsación corta

Sirve para:

- confirmar opciones en menús
- activar algunas funciones rápidas según la pantalla
- en las pantallas de sensor, cambiar el modo visual del sensor

### Pulsación larga

Sirve para:

- abrir el menú de configuración de una pantalla
- en `Timer`, abrir el editor `HH:MM:SS` si está en reposo o resetear si ya estaba corriendo/pausado

## 6. Orden de pantallas

El firmware actual está compilado con `PBIT_ENABLE_GRAPH_LAB=1`. Con ese flag, el carrusel visible para uso normal es:

1. `Home`
2. `Clima`
3. `Multi`
4. `Sonido VU`
5. `Temperatura`
6. `Humedad`
7. `Luz`
8. `Sonido`
9. `Suelo`
10. `Termómetro`
11. `Timer`
12. `Sistema`

Las seis posiciones de sensor usan una zona común llamada `SENSOR_ZONE_SCREEN`. Giras el encoder para pasar de un sensor a otro; dentro de cada sensor, la pulsación corta cambia el modo visual:

- `Focus`
- `Valor`
- `Gráfica`
- `Dial`
- `Card`

La gráfica no aparece como una pantalla separada del carrusel actual: es uno de los modos de cada sensor.

La revisión también conserva pantallas Lab de exploración visual, como `Home Cards`, `Linear Dash` y `Sound VU`. Para el usuario, las vistas Lab estabilizadas aparecen como `Home`, `Clima`, `Multi` y `Sonido VU`; `Linear Dash` queda como vista Lab disponible en firmware, no como posición independiente del carrusel de uso normal.

Para evitar parpadeos, las pantallas separan elementos fijos y datos cambiantes. El P-Bit solo refresca las zonas necesarias: por ejemplo, el `Timer` actualiza el tiempo con una cadencia rápida, `Sistema` refresca su información periódica y los sensores se actualizan al recibir datos nuevos.

## 7. Reglas generales de los menús

En casi todos los menús ocurre lo mismo:

1. mantén pulsado alrededor de 1.2 segundos para entrar
2. gira para elegir opción
3. pulsa para confirmar
4. si editas varios valores, irás paso a paso
5. al guardar suele aparecer un estado de confirmación
6. una pulsación más te devuelve al menú raíz

Importante:

- las opciones tipo `ON/OFF`, idioma o listas suelen ser circulares
- los valores numéricos tienen límites mínimo y máximo
- mientras un menú está abierto, el equipo no entra en reposo automático

## 8. Pantalla de Temperatura

### Qué muestra

- temperatura ambiente
- representación visual tipo tanque
- color según el estado

### Acción rápida

- pulsación corta: cambia el modo visual del sensor
- la unidad `Celsius/Fahrenheit` se cambia desde el menú `Unidad`
- ese cambio también afecta a `Termómetro`, porque ambas pantallas comparten la misma unidad global

### Menú

Opciones:

- `Límites`
- `Unidad`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Límites

Permite configurar:

- límite bajo
- límite alto

Si la temperatura baja demasiado o sube demasiado, el P-Bit puede avisarte.

#### Unidad

Permite elegir:

- `Celsius`
- `Fahrenheit`

#### Alertas

Permite activar o desactivar:

- `ON`
- `OFF`

### Buenas prácticas

- espera un momento si acabas de mover el equipo de un lugar a otro
- evita tocar el sensor directamente durante la medición
- usa alertas para rangos de confort, no solo para extremos

## 9. Pantalla de Humedad del aire

### Qué muestra

- humedad relativa del aire
- representación vertical
- estado interpretado por rangos

### Menú

Opciones:

- `Límites`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Límites

Configura dos umbrales:

- `Seco`
- `Muy húmedo`

Todo lo que quede entre esos dos valores se interpreta como:

- `Óptimo`

#### Alertas

Activa o desactiva las alertas.

### Buenas prácticas

- no acerques la boca o la mano al sensor si quieres una medición estable
- úsalo para comparar zonas del aula, rincones, terrarios o invernaderos escolares

## 10. Pantalla de Luz

### Qué muestra

- valor principal de luz
- barra visual
- categoría interpretada
- lectura en lux limitada por firmware a `0..20000 lux`

### Menú

Opciones:

- `Calibración`
- `Modo display`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Calibración

Permite ajustar:

- `Max penumbra`
- `Max interior`
- `Max brillante`

Esto cambia cómo el equipo clasifica el ambiente lumínico.

#### Modo display

Permite elegir:

- `Lux`
- `% log`
- `Raw ADC`

### Buenas prácticas

- no tapes el sensor con la mano al medir
- recuerda que el P-Bit apaga el LED RGB en esta pantalla para no afectar la lectura
- usa el modo `Lux` para interpretación más intuitiva y `Raw ADC` para exploración técnica
- usa la lectura como referencia educativa y comparativa; no sustituye a un luxómetro calibrado

Nota de revisión: la conversión del LDR se ha corregido para la placa actual y el firmware recorta la lectura al rango `0..20000 lux`. Esto evita valores fuera de escala en pantalla y gráficas; las unidades físicas concretas siguen necesitando una comprobación de plausibilidad antes de entrega.

## 11. Pantalla de Sonido

### Qué muestra

- nivel de sonido en porcentaje
- barra horizontal
- categoría interpretativa

### Menú

Opciones:

- `Calibración`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Calibración

Permite ajustar los umbrales de interpretación:

- `Max silencio`
- `Max normal`
- `Max alto`

Esto no calibra el micrófono en decibelios reales; ajusta cómo el P-Bit interpreta el ambiente.

#### Alertas

Permite activar o desactivar las alertas visuales y RGB del módulo. En esta pantalla no se usa alerta sonora para no contaminar la propia lectura del micrófono.

### Buenas prácticas

- úsalo para comparar ambientes, no como instrumento acústico certificado
- ideal para explorar bienestar acústico en aula, biblioteca, pasillos o patio

## 12. Pantalla de Suelo

### Qué muestra

- porcentaje de humedad del suelo
- categoría del estado del sustrato

### Menú

Opciones:

- `Calibrar sensor`
- `Editar umbrales`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Calibrar sensor

Es el paso más importante para una buena lectura.

Secuencia:

1. referencia `Seco al aire`
2. referencia `En agua`
3. guardado

El sistema usa esas dos referencias para convertir el valor analógico en porcentaje.

#### Editar umbrales

Permite configurar:

- `Seco`
- `Óptimo`
- `Húmedo`
- `Muy húmedo`

Con eso el P-Bit clasifica el suelo como:

- `Seco`
- `Óptimo`
- `Húmedo`
- `Muy húmedo`

#### Alertas

Permite activar o desactivar las alertas del suelo. Cuando el valor sale del rango esperado, el P-Bit puede avisar con color y una melodía breve.

### Buenas prácticas

- calibra cada vez que cambies de sensor o de montaje
- inserta el sensor siempre a profundidad parecida
- no dejes el sensor sumergido permanentemente si no está diseñado para ello
- si aparece `Sin sensor`, revisa el puerto externo y la conexión

## 13. Pantalla de Termómetro

### Qué muestra

- temperatura externa de una sonda
- visualización tipo tanque

Nota: `DS18B20` es el identificador técnico de la sonda; en la interfaz de usuario se presenta como `Termómetro` o `Termo` cuando hace falta abreviar.

### Acción rápida

- pulsación corta: cambia el modo visual del sensor
- la unidad `Celsius/Fahrenheit` se cambia desde el menú `Unidad`
- ese cambio también afecta a `Temperatura`, porque ambas pantallas comparten la misma unidad global

### Menú

Opciones:

- `Calibración`
- `Unidad`
- `Alertas`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Calibración

Permite ajustar:

- `Offset`
- `Límite bajo`
- `Límite alto`

El `Offset` sirve para corregir pequeñas diferencias si comparas la sonda con una referencia conocida.

#### Unidad

Permite elegir:

- `Celsius`
- `Fahrenheit`

#### Alertas

Permite activar o desactivar alertas.

### Buenas prácticas

- conecta la sonda antes de empezar, si es posible
- evita doblar o forzar el cable
- úsala para comparar interior/exterior, sombra/sol, agua/aire o distintos materiales

## 14. Pantalla de Sistema

### Qué muestra

- nombre del dispositivo
- tiempo encendido (`Tiempo`)
- idioma
- estado de `Bip`
- estado de `Alarmas`

### Acción rápida

- pulsación corta: cambia `Bip ON/OFF`

### Menú

Opciones:

- `Bip`
- `Alarmas`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

El menú se muestra como una cuadrícula de opciones para que sea más fácil de recorrer con el encoder.

### Qué hace cada opción

#### Bip

Activa o desactiva los pitidos de interacción de la interfaz: pulsaciones, confirmaciones y navegación.

#### Alarmas

Activa o desactiva el audio de alertas y del `Timer` cuando termina una cuenta regresiva. No cambia las alertas visuales ni el LED RGB.

#### Reposo

Permite elegir:

- `30 seg`
- `1 min`
- `2 min`
- `5 min`
- `10 min`
- `Nunca`

Si eliges `Nunca`, el P-Bit no duerme automáticamente.

#### Idioma

Permite cambiar el idioma del sistema entre Español, Catalán e Inglés. El cambio se guarda y afecta a toda la interfaz traducida, no solo a esta pantalla.

#### Reset

Restaura configuraciones y calibraciones a valores por defecto.

### Buenas prácticas

- si estás haciendo una actividad larga, considera usar `5 min`, `10 min` o `Nunca`
- si el grupo cambia de idioma, ajusta esto desde aquí
- usa `Reset` solo cuando realmente quieras volver a empezar configuraciones

## 15. Pantalla de Timer

### Qué hace

Es un temporizador rápido con dos usos:

- `00:00:00`: cronómetro ascendente
- cualquier tiempo `HH:MM:SS` que elijas con el encoder: cuenta regresiva

### Controles

- pulsación corta:
  - iniciar
  - pausar
- pulsación larga:
  - si está en `0:00:00`, abre el selector de tiempo
  - si ya estaba corriendo o pausado, resetea

La pulsación larga de `Timer` es de alrededor de 1 segundo.

### Qué verás en pantalla

- si abres el selector, verás `HH:MM:SS`
- primero eliges si quieres cambiar horas, minutos o segundos
- pulsas para entrar a editar ese campo
- giras para cambiar el valor
- pulsas para salir de la edición
- cuando ya esté listo, mantienes pulsado para guardar
- cuando está en modo cronómetro, el tiempo grande usa `MM:SS:CC`
- si el tiempo visible llega a una hora o más, cambia a `HH:MM:SS`
- si configuraste una cuenta regresiva, la duración elegida aparece pequeña y centrada debajo de la tarjeta
- color azul en reposo
- color verde mientras corre
- color amarillo al pausar
- color rojo si una cuenta regresiva llega a cero

### Usos sugeridos

- medir tiempo de exposición de una planta a la luz
- cronometrar toma de datos
- controlar duración de experimentos o riegos
- trabajar con ventanas de observación ajustadas al experimento, por ejemplo `3`, `12` o `25` minutos

## 16. Alertas del P-Bit

El P-Bit puede comunicar estados por varias vías:

- cambios de color en pantalla
- LED RGB
- pitidos de interfaz si `Bip` está activo
- alarmas audibles si `Alarmas` está activo

Esto ayuda a que la lectura no dependa solo del número mostrado.

Ejemplos:

- azul: condición baja o fría
- rojo: condición alta o crítica
- verde: condición correcta u óptima
- naranja: advertencia o transición

## 17. Reposo automático

El P-Bit puede entrar en reposo si no se usa durante un tiempo.

Comportamiento esperado:

- si el tiempo de reposo está activo, aparece un aviso visual con `ZZZ`
- mientras ese aviso está activo, el equipo queda en reposo visible
- si interactúas, vuelve al uso normal

El reposo automático se bloquea cuando:

- hay un menú abierto
- el timer está corriendo

El reposo visible está pensado para el aula: la pantalla no se apaga de forma profunda automáticamente, sino que muestra el estado de descanso y vuelve al uso normal con una interacción del encoder.

## 18. Consejos de uso en aula y proyectos

### Para explorar plantas

- mide suelo, luz, temperatura y humedad a la misma hora cada día
- compara antes y después del riego
- usa umbrales para decidir cuándo intervenir

### Para explorar espacios

- compara rincones del aula
- mide cerca de la ventana y en zonas interiores
- registra sonido en momentos distintos del día

### Para proyectos STEAM

- plantea hipótesis antes de medir
- registra varias observaciones, no solo una
- compara resultados entre grupos
- usa el timer para ordenar el experimento

## 19. Buenas prácticas generales

- no fuerces conectores ni cables
- evita humedad directa sobre la electrónica principal
- deja estabilizar la lectura tras mover el equipo
- calibra el suelo antes de sacar conclusiones
- no interpretes el módulo de sonido como medición profesional en dB
- revisa el menú `Sistema` antes de una sesión larga

## 20. Solución de problemas básica

### No cambia de pantalla

- gira el encoder con decisión, un paso a la vez
- revisa si no estás dentro de un menú

### No entra a un menú

- mantén el botón presionado un poco más
- en la mayoría de pantallas la entrada es por pulsación larga

### Un sensor no aparece

- revisa la conexión física
- si es suelo o Termómetro, comprueba que esté bien conectado en su puerto

### La lectura de suelo parece rara

- recalibra el sensor
- comprueba que la sonda esté bien insertada

### La luz parece afectada

- evita tapar el sensor
- mide en una posición estable y sin sombras accidentales

## 21. Qué puede aprender un usuario con el P-Bit

El P-Bit ayuda a aprender:

- observación ambiental
- medición y registro
- interpretación de datos
- calibración
- relación entre ambiente y seres vivos
- diseño de experimentos
- toma de decisiones basada en evidencia

## 22. Documentos relacionados

- `PBIT_FUNCIONAMIENTO_ACTUAL.md`
- `Menues.MD`
- `ROADMAP_PBIT.md`
- `MANUAL_TECNICO_PBIT.md`
- `docs/PRODUCTION_CHECKLIST.md`
