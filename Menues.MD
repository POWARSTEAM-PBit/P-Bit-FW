# Menues

Actualizado: 2026-03-27

Este documento describe la estructura real de navegacion, menus y submenus tal como esta implementada hoy en el codigo.

Reglas de mantenimiento:
- Este archivo debe reflejar el comportamiento real, no intenciones futuras.
- Si cambia un menu, un flujo de botones o una validacion, este documento debe actualizarse.

## Alcance Real

- `BOOT_SCREEN` existe en la enum de pantallas, pero no forma parte de la navegacion con encoder.
- La navegacion entre pantallas usa este orden real: `TEMP -> HUMIDITY -> LIGHT -> SOUND -> SOIL -> DS18B20 -> SYSTEM -> TIMER -> CALIBRATION -> CALIBRATION PRECISION`.
- Cuando hay un menu de configuracion abierto, el encoder deja de cambiar de pantalla y pasa a controlar solo ese flujo.
- Los menus activos bloquean el reposo automatico del sistema.
- Dentro de un menu, el boton confirma o avanza por estado al soltarlo.
- Fuera de menu, la pulsacion corta se resuelve al soltar y la pulsacion larga se dispara durante la pulsacion, sin necesidad de soltar para abrir menus o reiniciar el timer.
- El encoder se reconfigura por estado:
  - Los menus raiz y los selectores binarios o de opcion suelen ser circulares.
  - Las ediciones numericas se limitan por minimo y maximo; no envuelven.

## Arranque

### Selector de idioma

En cold boot, el firmware siempre muestra primero el selector de idioma. El código aún conserva una ruta de wake desde deep sleep, pero el reposo automático actual del producto se queda en `ZZZ` visible y no entra en deep sleep automático.

Opciones reales:
- `Español`
- `Catalán`
- `English`

Comportamiento:
- El selector usa el encoder en rango `0..2` y circular.
- La pulsacion confirma la seleccion.
- Al confirmar, el idioma se guarda y la pantalla se limpia.
- El idioma preseleccionado es el ultimo guardado; si no existe, parte en Español.

## Navegacion Global

### Pantallas principales

- `TEMPERATURA`
- `HUMEDAD`
- `LUZ`
- `SONIDO`
- `SUELO`
- `DS18B20`
- `SISTEMA`
- `TIMER`
- `CALIBRATION`
- `CALIBRATION PRECISION`

### Acciones rapidas fuera de menu

- `TEMP`: pulsacion corta sin menu alterna la unidad global compartida `C/F`.
- `DS18B20`: pulsacion corta sin menu alterna la misma unidad global compartida `C/F`.
- `SISTEMA`: pulsacion corta sin menu alterna `sonido ON/OFF`.
- `TIMER`: pulsacion corta inicia o pausa; pulsacion larga de aprox. 1 s abre el editor `HH:MM:SS` si esta idle y reinicia si ya estaba corriendo o pausado.
- `HUMEDAD`, `LUZ`, `SONIDO` y `SUELO`: no tienen accion corta propia fuera de menu.

### Entrada a menus

- `TEMP`: pulsacion larga de aprox. 1.2 s.
- `HUMEDAD`: pulsacion larga de aprox. 1.2 s.
- `LUZ`: pulsacion larga de aprox. 1.2 s.
- `SONIDO`: pulsacion larga de aprox. 1.2 s.
- `SUELO`: pulsacion larga de aprox. 1.2 s.
- `DS18B20`: pulsacion larga de aprox. 1.2 s.
- `SISTEMA`: pulsacion larga de aprox. 1.2 s.

### Regla comun dentro de menus

- Gira el encoder para cambiar la opcion o el valor.
- Pulsa para confirmar el paso actual.
- En varios menus, el estado `SAVED` es intermedio: una pulsacion adicional vuelve al menu raiz.
- Los menus raiz y los selectores de opcion binaria o discreta envuelven en bucle; las ediciones numericas no.
- Las confirmaciones de `Reset` usan selector binario circular `NO / SI`.

## Temperatura DHT

### Accion rapida

- Pulsacion corta sin menu: alterna `C/F`.
- La unidad es compartida con `DS18B20`, asi que el cambio afecta a ambas pantallas.

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Límites`
- `Unidad`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de limites

Secuencia real:
- Editar `Limite bajo`
- Editar `Limite alto`
- Guardar
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Limite alto` debe ser mayor que `Limite bajo`.
- La edicion usa la unidad visible.
- Los valores se guardan internamente en Celsius.
- El rango visible depende de la unidad, pero sigue el mismo tope fisico `0..50 C` o equivalente en Fahrenheit.

### Flujo de unidad

Secuencia:
- Elegir `Celsius` o `Fahrenheit`
- Guardar
- Pulsacion adicional para volver al menu raiz
- El valor guardado actualiza tambien la pantalla `DS18B20`, porque ambas comparten la misma unidad global.

### Flujo de alertas

Secuencia:
- Elegir `OFF` o `ON`
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar limites, alertas y unidad visible a `Celsius`
- Mostrar `SAVED`
- Pulsacion adicional para volver al menu raiz

### Runtime activo

- La pantalla detecta estado sin sensor y muestra `Sin sensor`.
- Alerta baja: color azul, LED azul y beep al entrar en el estado.
- Alerta alta: color rojo, LED rojo y beep al entrar en el estado.
- Sin alerta: vuelve al LED naranja normal de la pantalla.
- Los umbrales son inclusivos en los bordes: `<= limite bajo` y `>= limite alto`.
- El menu y la pantalla normal limpian bandas concretas antes de redibujar para evitar ghosting.
- El indicador minimo de alertas se dibuja abajo a la izquierda para no competir con el valor ni con el titulo.

### Nota de estado

- La pantalla de guardado tiene rama de error en UI, pero el flujo normal de encoder hace que sea dificil llegar a un guardado invalido.

## Humedad del aire

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Límites`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de limites

Secuencia real:
- Editar `Seco`
- Editar `Muy humedo`
- Guardar
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Muy humedo` debe ser mayor que `Seco`.
- Todo lo que quede entre ambos valores se considera `Optimo`.

### Flujo de alertas

Secuencia:
- Elegir `OFF` o `ON`
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar `Seco`, `Muy humedo` y el estado de alertas
- Mostrar `SAVED`
- Pulsacion adicional para volver al menu raiz

### Runtime activo

- Si las alertas estan activas:
  - por debajo de `Seco`: valor naranja, LED naranja y beep agudo
  - por encima de `Muy humedo`: valor rojo, LED rojo y beep grave
- Si no hay alerta: vuelve el LED azul normal.
- El indicador minimo de alertas se dibuja abajo a la izquierda y ya no compite con el texto principal.
- Los limites son `dry < comfort`; los bordes se tratan como zona no alerta.

## Luz

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Calibración`
- `Modo display`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de calibracion

Secuencia real:
- `Max penumbra`
- `Max interior`
- `Max brillante`
- `Guardado`
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Interior` debe ser mayor que `Penumbra`.
- `Brillante` debe ser mayor que `Interior`.
- El rango editable va de `10` a `10000`.

### Flujo de modo display

Opciones reales:
- `Lux`
- `% log`
- `Raw ADC`

Secuencia:
- Elegir modo
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de alertas

Opciones reales:
- `OFF`
- `ON`

Secuencia:
- Elegir estado
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar umbrales de luz y `Modo display`
- Mostrar `SAVED`
- Pulsacion adicional para volver al menu raiz

### Runtime activo

- `Raw ADC` cambia solo el valor grande y la unidad mostrada; la categoria y la barra siguen calculandose con lux.
- Las categorias reales son `Oscuro`, `Tenue`, `Interior`, `Brillante` y `Luz solar`.
- Hay alerta visual minima en runtime:
  - por debajo de `Max penumbra`: indicador en cian y beep corto al entrar al estado
  - desde `Max brillante` en adelante: indicador en naranja y beep corto al entrar al estado
  - entre ambos rangos: indicador verde
- Si las alertas estan en `OFF`, el indicador pasa a estado apagado y no suenan avisos en runtime.
- Al entrar en la pantalla de luz, el LED se fuerza a apagado para no interferir con la medida del LDR.

### Nota de estado

- El guardado de calibracion no muestra error en UI; el encoder ya limita el rango a combinaciones validas.

## Sonido

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Calibración`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de calibracion

Secuencia real:
- `Max silencio`
- `Max normal`
- `Max alto`
- `Guardado`
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Normal` debe ser mayor que `Silencio`.
- `Alto` debe ser mayor que `Normal`.

### Flujo de alertas

Opciones reales:
- `OFF`
- `ON`

Secuencia:
- Elegir estado
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar umbrales y alertas del menu de sonido
- Mostrar `SAVED`
- Pulsacion adicional para volver al menu raiz

### Runtime activo

- Si las alertas estan activas:
  - entre `normal_max` y antes de `loud_max`: LED naranja e indicador visual
  - desde `loud_max` en adelante: LED rojo e indicador visual
- Si no hay alerta: vuelve el LED magenta normal de la pantalla.
- No hay alerta sonora en runtime de sonido para no contaminar la propia lectura del microfono.
- Las categorias visibles son `Silencio`, `Tranquilo`, `Normal`, `Ruidoso` y `Muy ruidoso`.
- El indicador minimo de alertas se dibuja abajo a la izquierda y no invade el valor grande.

### Nota de estado

- El guardado de calibracion y de alertas tiene su propio estado `SAVED` antes de volver al menu raiz.

## Suelo

### Menu raiz de calibracion

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Calibrar sensor`
- `Editar umbrales`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de calibracion del sensor

Secuencia real:
- `Seco al aire`
- `En agua`
- `Guardado` o `Error`
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Seco al aire` debe ser mayor que `En agua`.
- La diferencia entre ambos debe ser al menos `300` cuentas ADC.
- Si falla la validacion o el guardado, la UI entra en `Error`.

### Flujo de umbrales

Secuencia real:
- `Seco`
- `Optimo`
- `Humedo`
- `Guardado` o `Error`
- Pulsacion adicional para volver al menu raiz

### Flujo de alertas

Secuencia real:
- Entrar en `Alertas`
- Elegir `OFF` o `ON`
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia real:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar calibracion del sensor y umbrales porcentuales
- Mostrar `Reset aplicado`
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Seco < Optimo < Humedo`
- Cada valor debe quedar entre `0` y `100`

### Runtime activo

- Las categorias reales son `Seco`, `Optimo`, `Humedo` y `Muy humedo`.
- Si no hay sensor, la pantalla muestra `Sin sensor` y el aviso `Check J6 (GPIO35)`.
- En el modo normal, el color del LED sigue la categoria del suelo:
  - seco: rojo
  - optimo: verde
  - humedo o saturado: azul oscuro
- El indicador minimo de runtime usa esta lectura:
  - `Seco`: naranja
  - `Optimo`: verde
  - `Humedo`: cian
  - `Muy humedo`: azul
- Al cambiar a `Seco`, suena un arpegio corto descendente si el sonido global esta activo.
- Al cambiar a `Optimo`, suena un arpegio corto ascendente si el sonido global esta activo.
- Al cambiar a `Muy humedo`, suena un arpegio corto descendente mas grave si el sonido global esta activo.
- Si las alertas estan en `OFF`, no suenan melodias y el indicador queda en estado apagado.
- Al entrar en calibracion, el LED pasa a naranja; al salir vuelve al color normal de suelo.

## DS18B20

### Accion rapida

- Pulsacion corta sin menu: alterna `C/F`.
- La unidad es compartida con `TEMP`, asi que el cambio afecta a ambas pantallas.

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Calibración`
- `Unidad`
- `Alertas`
- `Reset`
- `Salir`

### Flujo de calibracion

Secuencia real:
- Editar `Offset`
- Editar `Limite bajo`
- Editar `Limite alto`
- Guardar
- Pulsacion adicional para volver al menu raiz

Validaciones reales:
- `Limite alto` debe ser mayor que `Limite bajo`.
- Los limites se editan en la unidad visible.
- Los valores se convierten a Celsius al guardar.
- El `Offset` se edita en decimas, con rango visible aproximado de `-5.0` a `+5.0`.

### Flujo de unidad

Secuencia:
- Elegir `Celsius` o `Fahrenheit`
- Guardar
- Pulsacion adicional para volver al menu raiz
- El valor guardado actualiza tambien la pantalla `TEMP`, porque ambas comparten la misma unidad global.

### Flujo de alertas

Secuencia:
- Elegir `OFF` o `ON`
- Guardar
- Pulsacion adicional para volver al menu raiz

### Flujo de reset

Secuencia:
- Entrar en `Reset`
- Elegir `NO` o `SI`
- Si se confirma, restaurar offset, limites, alertas y unidad visible a `Celsius`
- Mostrar `SAVED`
- Pulsacion adicional para volver al menu raiz

### Runtime activo

- La pantalla muestra `Sin sensor` cuando no detecta sonda y añade `Check J4`.
- Alerta baja: color azul, LED azul y beep al entrar.
- Alerta alta: color rojo, LED rojo y beep al entrar.
- Sin alerta: vuelve el LED blanco normal.
- La referencia visual de `0` grados se mantiene en el tanque.
- El indicador minimo de alertas se dibuja abajo a la izquierda para no chocar con el termometro ni con el valor.

### Nota de estado

- Como en Temperatura, el estado `SAVED` es intermedio y una pulsacion adicional devuelve al menu raiz.

## Sistema

### Pantalla base

La pantalla normal de sistema muestra:
- `DEV`
- `UP`
- `BLE`
- `LAN`
- footer con `Sonido ON/OFF (Pulsa)`

### Accion rapida

- Pulsacion corta sin menu: alterna `sonido ON/OFF` y lo guarda.

### Menu raiz

Se abre con pulsacion larga de aprox. 1.2 s.

Opciones reales:
- `Sonido`
- `Reposo`
- `Idioma`
- `Reset`
- `Salir`

### Submenu de sonido

Opciones:
- `OFF`
- `ON`

Comportamiento:
- Selecciona el estado.
- Al confirmar, se guarda, muestra `SAVED` y una pulsacion adicional vuelve al menu raiz.

### Submenu de reposo

Opciones reales:
- `30 seg`
- `1 min`
- `2 min`
- `5 min`
- `10 min`
- `Nunca`

Reglas reales de energia:
- `Nunca` desactiva el reposo automatico.
- Con cualquier timeout activo, la UI entra en `IDLE` visible durante los ultimos `5 seg` del contador de inactividad.
- Durante ese estado se muestra el overlay `ZZZ` y la UI queda congelada hasta que el usuario despierte el equipo con el encoder.
- El firmware conserva codigo de deep sleep, pero el flujo automatico actual no lo usa porque en esta placa la TFT queda blanca al dormir.

### Submenu de idioma

Opciones reales:
- `Español`
- `Catalán`
- `English`

Comportamiento:
- Guarda el idioma en NVS.
- Redibuja el menu completo con el nuevo idioma.
- El propio submenu tambien se muestra en el idioma activo.
- Tras guardar, muestra `SAVED` y una pulsacion adicional vuelve al menu raiz.

### Submenu de reset

Opciones reales:
- `NO`
- `SI`

Si se confirma:
- Borra calibraciones y umbrales de todo el dispositivo.
- Restaura idioma por defecto.
- Restaura la unidad de temperatura visible a `Celsius`.
- Muestra `SAVED` y una pulsacion adicional vuelve al menu raiz.

## Timer

No tiene submenu raiz con lista, pero si tiene un editor propio de duracion.

Comportamiento real:
- Pulsacion corta: inicia si esta parado, pausa si esta corriendo.
- Pulsacion larga mientras esta en `0:00:00`: abre el editor `HH:MM:SS`.
- Pulsacion larga mientras ya corrio o esta pausado: reinicia el temporizador.
- En el editor:
  - se muestra una duracion `HH:MM:SS`
  - al girar sin editar, eliges campo `HH / MM / SS`
  - al pulsar, entras o sales de la edicion del campo seleccionado
  - al girar en edicion, cambias el valor del campo
  - al mantener pulsado en modo seleccion, guardas y sales
- `00:00:00` funciona como cronometro ascendente.
- Cualquier valor mayor que `00:00:00` funciona como cuenta regresiva.
- En runtime, la duracion activa se muestra pequena y centrada debajo del card solo cuando hay cuenta regresiva; en modo cronometro ese espacio queda limpio.
- El valor grande usa formato adaptativo: `MM:SS:CC` por debajo de una hora y `HH:MM:SS` cuando el tiempo visible llega a una hora o mas.
- Al terminar una cuenta regresiva, el borde y el valor pasan a rojo y suena una alarma intermitente corta si el sonido global esta en `ON`.
- El valor grande del runtime se compone sobre un sprite transparente y luego se repinta el borde del card, para que la caja siga viendose completa en la TFT real y no quede tapada por el fondo negro del sprite.
- El card runtime quedo unos pixeles mas abajo y el texto de estado acompano ese cambio, mientras que el tiempo grande se mantuvo para ganar aire arriba y abajo dentro del card sin romper la lectura.
- El card runtime tambien se estiro un poco hacia abajo para aprovechar mejor el espacio libre inferior, y el card del editor bajó para que el titulo `DURACIÓN` ya no pelee con el recuadro.
- En el editor del timer, el card de configuracion siguió bajando hasta `Y=64`, pero el valor `HH:MM:SS` se mantuvo arriba para poder separar mejor el recuadro sin arrastrar también el bloque numérico.
- En runtime, el valor del cronometro ya no fuerza un repintado completo del card en cada tick; solo se limpia y actualiza su banda, y el refresco de centesimas es un poco menos agresivo para reducir ghosting.
- Temporalmente, despues de `TIMER` hay dos pantallas internas de calibracion pensadas para alinear el visualizador externo con la TFT real:
  - `CALIBRATION`: comparacion general de datums y posiciones `Y` con las fuentes reales del timer.
  - `CALIBRATION PRECISION`: instrumento fino con reticula `5/10 px`, regla lateral por pixel, pruebas de baseline y un caso de `TL_DATUM`.

## Runtime Compartido

- Los menus de configuracion bloquean el reposo automatico mientras estan abiertos.
- El color del LED puede cambiar al entrar en cada pantalla o al dispararse alertas de sensores.
- Los flujos de guardado con estado `SAVED` usan una pulsacion extra para volver al menu raiz en `TEMP`, `HUMEDAD`, `LUZ`, `SONIDO`, `SUELO`, `DS18B20` y `SISTEMA`.
- `TEMP`, `HUMEDAD`, `LUZ`, `SONIDO`, `SUELO` y `DS18B20` ya reutilizan el marco comun de menús para listas, estados de valor, prompts de reset y pantallas guardadas; los menus de 4 y 5 opciones comparten espaciado y los resúmenes multilinea usan bandas y separaciones comunes.
- `TEMP`, `HUMEDAD`, `LUZ`, `SONIDO`, `SUELO` y `DS18B20` muestran un indicador minimo de alertas abajo a la izquierda en runtime cuando aplica.
- La evaluacion de alertas ya se refresca desde el `sensor task` y las pantallas consumen el estado estable del `AlertEngine`; el render ya no es la fuente de verdad de las alertas runtime.
- El `AlertEngine` tambien mantiene una alerta global principal en segundo plano: la elige por severidad (`critica > alta > baja`) y, si empatan, por orden de sensor `SUELO > TEMP > HUM > DS18 > LUZ > SONIDO`.
- Si hay varias alertas simultaneas, el motor conserva la principal y el conteo adicional `+N`, aunque ese resumen ya no se dibuja en pantalla porque la capa visual global esta desactivada temporalmente.
- El `AlertEngine` global sigue evaluando alertas en background, con prioridad, audio selectivo y RGB global cuando aplica.
- Cuando retomemos esta UX, la señal visual global se volvera a decidir con una zona segura real por familia de pantalla.
- El audio de alerta ya puede dispararse en background segun el estado global, con cooldown para no saturar. `SONIDO` sigue sin alerta sonora para no contaminar la lectura del microfono.
- El RGB tambien puede seguir la alerta global principal; la excepcion es `LUZ`, donde el LED se mantiene apagado para no interferir con el LDR.
