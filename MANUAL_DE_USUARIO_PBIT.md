# Manual de Usuario del P-Bit

Actualizado: 2026-03-27

Este manual explica qué es el P-Bit, cómo usarlo y cómo aprovecharlo en actividades de observación ambiental, aula, laboratorio escolar y proyectos STEAM.

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
- luz ambiental
- sonido ambiental
- humedad del suelo
- temperatura externa con sonda

Además, muestra información general del sistema:

- nombre del dispositivo
- tiempo encendido
- estado BLE
- idioma activo
- estado del sonido
- cronómetro

## 3. Partes principales del equipo

### Entradas

- sensor de temperatura y humedad ambiente
- sensor de luz
- sensor de sonido
- puerto para sensor de humedad de suelo
- puerto para sonda DS18B20
- encoder rotatorio con botón

### Salidas

- pantalla
- LED RGB
- buzzer
- BLE

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

### Pulsación larga

Sirve para:

- abrir el menú de configuración de una pantalla
- en `Timer`, abrir el selector de minutos si está en reposo o resetear si ya estaba corriendo/pausado

## 6. Orden de pantallas

El orden actual de navegación es:

1. `Temperatura`
2. `Humedad`
3. `Luz`
4. `Sonido`
5. `Suelo`
6. `DS18B20`
7. `Sistema`
8. `Timer`

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

- pulsación corta: cambia entre `Celsius` y `Fahrenheit`
- ese cambio también afecta a `DS18B20`, porque ambas pantallas comparten la misma unidad global

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

## 13. Pantalla de DS18B20

### Qué muestra

- temperatura externa de una sonda
- visualización tipo tanque

### Acción rápida

- pulsación corta: cambia entre `Celsius` y `Fahrenheit`
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
- tiempo encendido (`UP`)
- estado BLE
- idioma
- estado del sonido

### Acción rápida

- pulsación corta: cambia `Sonido ON/OFF`

### Menú

Opciones:

- `Sonido`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

### Qué hace cada opción

#### Sonido

Activa o desactiva el sonido global del dispositivo.

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

Permite cambiar el idioma del sistema.

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
- pitidos o tonos

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
- si es suelo o DS18B20, comprueba que esté bien conectado en su puerto

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
