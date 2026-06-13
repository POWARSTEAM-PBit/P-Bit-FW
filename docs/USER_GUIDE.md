# Manual de Usuario — P-Bit

Actualizado: 2026-06-13
Versión firmware: ver `CHANGELOG.md`
Idiomas de interfaz disponibles: Español, Catalán, English

---

## Avisos importantes

> **⚠️ ADVERTENCIA**
> No sumergir el dispositivo en agua ni exponer a lluvia. El P-Bit no es resistente al agua.

> **⚠️ PRECAUCIÓN**
> El P-Bit funciona con alimentación USB (5 V). No conectar a fuentes de tensión superiores. No modificar la alimentación eléctrica sin conocimiento técnico.

> **⚠️ PRECAUCIÓN**
> Algunos sensores externos (sensor de suelo, sonda de temperatura) requieren conexión correcta al conector correspondiente. Conectar o desconectar sensores externos **con el dispositivo apagado** siempre que sea posible.

> **ℹ️ NOTA**
> El P-Bit es una herramienta educativa. Los valores que muestra son orientativos. No sustituye instrumentos de medición certificados ni debe usarse para tomar decisiones de seguridad crítica.

> **ℹ️ NOTA**
> Un flash de firmware nuevo borra toda la configuración guardada y restaura los valores por defecto.

---

## 1. Descripción del producto

El P-Bit es un dispositivo educativo ambiental portátil. Mide temperatura, humedad del aire, luz, sonido, humedad del suelo y temperatura externa, y muestra los resultados en una pantalla TFT a color con menús de configuración y alertas.

El control físico es un único encoder rotatorio con pulsador. Se gira para navegar y se pulsa para confirmar o entrar en menús.

### Especificaciones generales

| Parámetro | Valor |
|---|---|
| Procesador | ESP32 |
| Alimentación | 5 V USB |
| Pantalla | TFT ST7735, 160 × 128 px, color |
| Interfaz de usuario | Encoder rotatorio con pulsador |
| Indicador de estado | LED RGB |
| Audio | Buzzer pasivo |
| Idiomas | Español, Catalán, English |
| Conectividad inalámbrica | BLE (desactivado de fábrica) |
| Almacenamiento de configuración | NVS (memoria no volátil interna) |

### Sensores y rangos de medida

| Sensor | Variable | Rango indicativo |
|---|---|---|
| DHT11 | Temperatura ambiente | 0..50 °C / 32..122 °F |
| DHT11 | Humedad del aire | 0..100 % |
| LDR | Luz ambiental | 0..8000 lux aproximado; modo visible `Lux` / `FC` / `Raw ADC` |
| Micrófono analógico | Nivel de sonido | 0..100 % (relativo) |
| Sensor capacitivo de suelo | Humedad del suelo | 0..100 % (calibrado) |
| Sonda DS18B20 (externa) | Temperatura puntual | −55..+125 °C |

> **ℹ️ NOTA**
> Los rangos de luz y sonido son aproximaciones de firmware, no valores calibrados por laboratorio. El sensor de suelo requiere calibración en campo (ver sección 12).

---

## 2. Partes del equipo

### Entradas

- Sensor de temperatura y humedad del aire (integrado en placa)
- Sensor de luz LDR (integrado en placa)
- Sensor de sonido (integrado en placa)
- Puerto visible de PCB para sensor de humedad de suelo (`IO35`)
- Puerto visible de PCB para sonda de temperatura Termómetro, identificador técnico DS18B20 (`IO33`)
- Encoder rotatorio con pulsador (control principal)

### Salidas

- Pantalla TFT a color
- LED RGB
- Buzzer

---

## 3. Precauciones de uso

- **No cubrir el sensor de luz** con la mano ni colocar objetos encima del LDR durante la medición.
- **En la pantalla de Luz**, el LED RGB se apaga automáticamente para no interferir con la medición. Este comportamiento es correcto.
- **No colocar el sensor de suelo en agua pura** sin referencia de calibración previa.
- **Evitar condensación** sobre la placa. En entornos muy húmedos, proteger la electrónica expuesta.
- **En caso de lectura `Sin sensor`**, verificar la conexión del sensor externo al conector correspondiente antes de asumir fallo del dispositivo.
- Al conectar o desconectar Suelo o Termómetro después del arranque, el P-Bit muestra brevemente un aviso semafórico: fondo verde con `CONECTADO` o fondo rojo con `DESCONECTADO`, seguido del sensor y el puerto `IO35` o `IO33`.
- **No aplicar fuerza excesiva** al encoder. El giro debe ser suave.

---

## 4. Primeros pasos — guía de inicio rápido

Esta guía lleva a alguien que nunca ha usado el P-Bit hasta ver su primer dato de sensor real y navegar con confianza. Tiempo estimado: 5 minutos.

**Necesitas:**
- El P-Bit con firmware cargado
- Un cable USB
- Un ordenador, cargador USB o powerbank (5 V)

> **⚠️ PRECAUCIÓN**
> Si vas a conectar sensores externos (sonda DS18B20 o sensor de suelo), hazlo con el dispositivo **apagado** antes de enchufar el USB.

---

### Paso 1 — Conectar y encender

1. Conecta el cable USB al P-Bit y a la fuente de alimentación.
2. La pantalla se enciende en unos segundos.

Si la pantalla no se enciende, comprueba el cable y la fuente USB.

---

### Paso 2 — Seleccionar el idioma

En el primer arranque (o tras un flash de firmware nuevo) aparece la pantalla de selección de idioma.

1. Gira el encoder para resaltar tu idioma: `Español`, `Catalán` o `English`.
2. Pulsa para confirmar.

✅ El dispositivo guarda el idioma y pasa directamente a la pantalla `Home`.

> **ℹ️ NOTA**
> En arranques posteriores sin flash nuevo, el P-Bit carga el idioma guardado y arranca directamente sin mostrar el selector. Si la configuración está limpia o el idioma nunca se confirmó, el selector vuelve a aparecer.

---

### Paso 3 — La pantalla Home

La primera pantalla que ves es `Home`. Muestra los valores actuales de todos los sensores en tarjetas pequeñas: temperatura, humedad, luz y sonido.

Los valores se actualizan en tiempo real. Si el sensor de suelo o la sonda externa no están conectados, sus tarjetas muestran `—` o `Sin sensor` — es el comportamiento esperado.

---

### Paso 4 — Moverse por el carrusel

Gira el encoder para pasar de una pantalla a otra.

- Girar a la derecha avanza por el carrusel.
- Girar a la izquierda retrocede.
- El carrusel es circular: después de `Sistema` (la última) vuelve a `Home`.

Prueba a girar lentamente y observa cómo cada pantalla muestra una vista diferente del entorno.

---

### Paso 5 — Ver un sensor de cerca

Navega hasta la pantalla de `Temperatura` (posición 5 del carrusel, girando a la derecha desde `Home`).

Verás la temperatura ambiente con un número grande y un indicador visual.

---

### Paso 6 — Cambiar el modo visual

Mientras estás en una pantalla de sensor, haz una **pulsación corta** (pulsa y suelta rápido).

El modo de visualización cambia:

`Principal` → `Dato` → `Curva` → `Rango` → `Ficha` → `Principal` …

Prueba a pulsar varias veces para ver cómo cambia la representación del mismo dato sin cambiar el sensor.

---

### Paso 7 — Abrir y cerrar un menú

Mantén pulsado el encoder durante ~1,2 segundos (**pulsación larga**).

Se abre el menú de configuración de la pantalla activa. Desde aquí puedes ajustar límites, alertas y más.

Para salir sin cambiar nada: gira hasta `Salir` y pulsa.

---

### ✅ Comprobación — ¿ha ido bien?

Al terminar estos pasos deberías poder:

- Ver `Home` con valores reales de temperatura, humedad, luz y sonido.
- Navegar entre pantallas girando el encoder.
- Cambiar el modo visual de un sensor con pulsación corta.
- Abrir y cerrar un menú con pulsación larga.

Si todo esto funciona, el P-Bit está listo para usar.

---

### Siguientes pasos sugeridos

| Quiero… | Ir a… |
|---------|-------|
| Configurar alertas de temperatura | Sección 8 → `Temperatura > Alertas` |
| Calibrar el sensor de suelo | Sección 12 → `Suelo > Calibrar sensor` |
| Usar el temporizador | Sección 14 → Timer |
| Cambiar el idioma | Sección 15 → `Sistema > Idioma` |
| Ajustar el tiempo de reposo | Sección 15 → `Sistema > Reposo` |

---

## 5. Control — encoder rotatorio

El encoder es el único control físico del dispositivo.

| Acción | Qué hace |
|---|---|
| **Girar** | Cambiar de pantalla / moverse por menú / cambiar valor |
| **Pulsación corta** | Confirmar / avanzar en menú / cambiar modo visual en pantallas de sensor |
| **Pulsación larga (~1.2 s)** | Abrir menú de configuración de la pantalla activa |

Reglas generales dentro de los menús:

- Los menús principales de configuración se muestran como una cuadrícula 2×3: opciones arriba, `Reset` abajo izquierda y `Salir` abajo derecha.
- Al editar una opción, el valor activo aparece dentro de una tarjeta central. La tarjeta cambia de color según el estado o valor seleccionado.
- Las listas de opciones y selectores `ON/OFF` son **circulares** (envuelven).
- Los valores numéricos tienen **límite mínimo y máximo**; no envuelven.
- Al guardar una configuración aparece un estado de confirmación `SAVED`.
- Una pulsación adicional después de `SAVED` vuelve al menú raíz.
- Mientras hay un menú abierto, el reposo automático se bloquea.

---

## 6. Pantallas y carrusel de navegación

Con la configuración de producción estándar, el carrusel tiene 12 posiciones. Se navega girando el encoder.

| Posición | Pantalla | Descripción |
|---|---|---|
| 1 | **Inicio** | Visión global de todos los sensores en fichas |
| 2 | **Clima Lab** | Temperatura y humedad del aire en vista combinada |
| 3 | **Termo Lab** | Temperatura ambiente, sonda externa y diferencia térmica |
| 4 | **Sonido VU** | Nivel de sonido en barras tipo VU meter |
| 5 | **Temperatura** | Temperatura ambiente con menú de configuración |
| 6 | **Humedad** | Humedad del aire con menú de configuración |
| 7 | **Luz** | Luz ambiental con menú de límites |
| 8 | **Sonido** | Nivel de sonido con menú de límites |
| 9 | **Suelo** | Humedad del suelo con calibración y alertas |
| 10 | **Termómetro** | Temperatura externa con sonda DS18B20 |
| 11 | **Timer** | Cronómetro y cuenta regresiva |
| 12 | **Sistema** | Ajustes globales del dispositivo |

Las pantallas 1–4 (Inicio, Clima Lab, Termo Lab, Sonido VU) son de solo lectura. No tienen menú de configuración. `Lab` se reserva para pantallas multisensor o experimentales; las vistas de un sensor individual usan los modos de abajo.

Las pantallas 5–10 son **zonas de sensor**. Dentro de cada una, la pulsación corta cambia el modo de visualización del sensor activo:

| Modo | Descripción |
|---|---|
| **Principal** | Lectura protagonista del sensor |
| **Dato** | Dato numérico con contexto visual |
| **Curva** | Histórico reciente en línea de tendencia |
| **Rango** | Lectura dentro de zonas de color y límites |
| **Ficha** | Resumen compacto de lectura |

La pulsación larga en cualquier pantalla de sensor abre su menú de configuración.

---

## 7. Reposo automático

El P-Bit puede configurarse para entrar en reposo automático tras un periodo de inactividad.

Durante el reposo:
- La pantalla muestra un overlay `ZZZ`.
- Las mediciones continúan en segundo plano.
- Para despertar el dispositivo, gira o pulsa el encoder.

Comportamiento importante:
- **Los menús abiertos bloquean el reposo.** El dispositivo no entrará en reposo mientras haya un menú activo.
- **El Timer activo bloquea el reposo.** Si hay una cuenta corriendo, el dispositivo permanece activo.

El tiempo de reposo se configura en `Sistema > Reposo`. Opciones: `30 seg`, `1 min`, `2 min`, `5 min`, `10 min`, `Nunca`.

---

## 7.1. Modo demo

El Modo demo permite dejar el P-Bit encendido mostrando una rotación automática de pantallas y sensores.

Para activarlo:

1. Mantén presionado el encoder mientras conectas la alimentación USB y durante el logo de arranque.
2. Suelta el encoder cuando el dispositivo termine de arrancar.

También puedes activarlo desde `Home` con una pulsación larga del encoder.

Durante el Modo demo, el P-Bit recorre pantallas representativas del carrusel con ritmo variable, anima valores de ejemplo con transiciones suaves y bloquea el reposo automático. Para salir, gira o pulsa el encoder una vez. La entrada desde logos y desde `Home`, la señal visual, la salida y la coreografía smooth están implementadas en firmware; la validación visual final debe hacerse en hardware. El Modo demo no cambia la configuración guardada del usuario.

---

## 8. Pantalla de Temperatura

### Acción rápida (sin menú)

Pulsación corta: cambia el modo visual del sensor (Principal / Dato / Curva / Rango / Ficha).

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Unidad** | Elige Celsius o Fahrenheit (global, afecta también a Termómetro) |
| **Límites** | Define temperatura mínima y máxima para activar alerta |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura unidad, límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

### Alertas de temperatura

| Estado | Color pantalla | LED | Sonido |
|---|---|---|---|
| Temperatura por debajo del límite bajo | Azul | Azul | Beep al entrar |
| Temperatura por encima del límite alto | Rojo | Rojo | Beep al entrar |
| Dentro del rango | Normal | Naranja | — |
| Sin sensor | Mensaje `Sin sensor` | — | — |

> **ℹ️ NOTA**
> La unidad Celsius/Fahrenheit es compartida entre `Temperatura` y `Termómetro`. Cambiarla en una pantalla la actualiza en ambas y queda guardada al reiniciar.

---

## 9. Pantalla de Humedad del aire

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Límites** | Define umbrales `Seco` y `Muy húmedo` |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

El rango entre `Seco` y `Muy húmedo` se interpreta automáticamente como `Óptimo`.

### Alertas de humedad

| Estado | Color pantalla | LED | Sonido |
|---|---|---|---|
| Por debajo de `Seco` | Naranja | Naranja | Beep agudo |
| Por encima de `Muy húmedo` | Rojo | Rojo | Beep grave |
| `Óptimo` | Normal | Azul | — |

---

## 10. Pantalla de Luz

> **ℹ️ NOTA**
> El LED RGB se apaga automáticamente al entrar en esta pantalla para no interferir con la medición del sensor LDR. Es comportamiento esperado.

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Modo** | Selecciona la unidad visible entre `Lux`, `FC` o `Raw ADC` |
| **Límites** | Define umbrales `Max penumbra`, `Max interior`, `Max brillante` |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura modo, límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

### Categorías de luz

`Oscuro` → `Tenue` → `Interior` → `Brillante` → `Luz solar`

> **ℹ️ NOTA**
> El modo de lectura de Luz se propaga a la pantalla clásica, Sensor Zone, cards, dials, dashboards y gráficas. Las categorías y alertas siguen usando lux interno; `Raw ADC` sirve para calibración y diagnóstico.
> `Lux` usa la curva empírica v1 tomada con luxómetro y `FC` se calcula desde ese mismo lux con `lux / 10.764`.

### Alertas de luz

| Estado | Indicador | Sonido |
|---|---|---|
| Por debajo de `Max penumbra` | Cian | Beep corto |
| Por encima de `Max brillante` | Naranja | Beep corto |
| Entre ambos rangos | Verde | — |

---

## 11. Pantalla de Sonido

> **ℹ️ NOTA**
> La pantalla de Sonido no tiene alerta sonora propia: emitir un beep durante la medición contaminaria la propia lectura del micrófono. Las alertas de sonido son solo visuales y de LED.

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Límites** | Define umbrales `Max silencio`, `Max normal`, `Max alto` |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

### Categorías de sonido

`Silencio` → `Suave` → `Normal` → `Fuerte` → `Muy fuerte`

---

## 12. Pantalla de Suelo

> **⚠️ PRECAUCIÓN**
> Antes de usar el sensor de suelo, es necesario calibrarlo con el material específico donde se va a medir. Sin calibración, los porcentajes mostrados no son representativos.

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Calibrar sensor** | Define el punto seco y el punto húmedo para el suelo actual |
| **Límites** | Ajusta los porcentajes de `Seco` y `Húmedo` |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura calibración, límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

### Cómo calibrar el sensor de suelo

1. Entra en `Calibrar sensor`.
2. Con el sensor **al aire** (seco), espera a que el RAW se estabilice y elige `Captura`. `Salir` vuelve al menú sin guardar.
3. Introduce el sensor en agua (o en suelo muy húmedo), espera unos segundos y elige `Captura`. `Salir` vuelve al menú sin guardar.
4. Revisa el resumen de valores `SECO` / `MOJADO`.
5. Elige `Guardar` para aplicar la calibración o `Salir` para descartarla.
6. Si aparece `Error`, repite el proceso asegurando que los dos valores estén bien diferenciados (≥ 300 cuentas ADC).

Si el sensor de suelo no está conectado, el P-Bit muestra `Sin sensor`, `Conecta sensor` y `Revisa IO35` en lugar de iniciar la captura.

Durante la calibración, una pulsación larga cancela el paso actual sin guardar cambios y vuelve al menú de Suelo. Desde el menú raíz, la pulsación larga cierra la configuración.

### Alertas del sensor de suelo

Los rangos de suelo se ajustan con dos puntos:

- Por debajo de `Seco`, el dispositivo distingue automáticamente entre `Muy seco` y `Seco`.
- Entre `Seco` y `Húmedo`, el estado se interpreta como `Óptimo`.
- Por encima de `Húmedo`, el dispositivo distingue automáticamente entre `Húmedo` y `Muy húmedo`.

Las alertas usan melodías cortas según el estado:

| Estado | Melodía | LED |
|---|---|---|
| Seco | Arpegio descendente | Amarillo |
| Óptimo | Arpegio ascendente | Verde |
| Muy húmedo | Arpegio grave descendente | Azul |

Si `Alarmas` está en `OFF`, las melodías no suenan, pero los colores de pantalla y LED siguen activos.

Si el sensor no está conectado, la pantalla muestra `Sin sensor`, `---` y la indicación `Revisa IO35` (o su equivalente en el idioma activo).
Cuando el sensor se desconecta o vuelve a conectarse durante el uso, aparece un aviso breve: `DESCONECTADO / Sensor Suelo / IO35` o `CONECTADO / Sensor Suelo / IO35`.

---

## 13. Pantalla de Termómetro (sonda DS18B20)

El Termómetro usa una sonda externa de temperatura que se conecta al puerto visible de PCB `IO33`.

> **⚠️ PRECAUCIÓN**
> Verificar la orientación correcta del conector de la sonda antes de conectar. Una conexión incorrecta puede dañar la sonda o el dispositivo.

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Unidad** | Elige Celsius o Fahrenheit (global, afecta también a Temperatura) |
| **Límites** | Define temperatura mínima y máxima para activar alerta |
| **Marcas** | Muestra u oculta las marcas de límites en el dial |
| **Alertas** | Activa o desactiva el aviso automático |
| **Reset** | Restaura unidad, límites, marcas y alertas a valores por defecto |
| **Salir** | Cierra el menú |

Si la sonda no está conectada, la pantalla muestra `Sin sensor`, `---` y la indicación `Revisa IO33` (o su equivalente en el idioma activo).
Cuando la sonda se desconecta o vuelve a conectarse durante el uso, aparece un aviso breve: `DESCONECTADO / Sensor DS18B20 / IO33` o `CONECTADO / Sensor DS18B20 / IO33`.

---

## 14. Timer

El Timer funciona como cronómetro y como cuenta regresiva.

| Acción | Cómo hacerlo |
|---|---|
| Iniciar | Pulsación corta cuando está en `00:00:00` |
| Pausar | Pulsación corta mientras está corriendo |
| Resetear | Pulsación larga mientras está corriendo o pausado |
| Editar duración | Pulsación larga cuando está en `00:00:00` (editor HH:MM:SS) |

### Modo cronómetro

Con `00:00:00`, el timer cuenta hacia arriba desde cero.

### Modo cuenta regresiva

Con cualquier valor mayor que `00:00:00`, el timer cuenta hacia abajo.
- Al llegar a cero, el borde y el valor pasan a rojo.
- Si `Alarmas` está activo, suena una alarma intermitente corta.

### Editor de duración

1. Pulsación larga (con timer en reposo) para entrar al editor.
2. Gira para seleccionar el campo: `HH`, `MM` o `SS`.
3. Pulsación corta para entrar a editar el campo seleccionado.
4. Gira para cambiar el valor.
5. Pulsación corta para confirmar el campo.
6. Pulsación larga para guardar la duración completa y salir del editor.

---

## 15. Pantalla de Sistema

Sistema es el centro de ajustes globales del dispositivo.

### Lo que muestra

- ID del dispositivo (`PBIT-XXXX`, derivado de la MAC), en una línea compacta
- Tiempo de encendido (`Uptime`)
- Idioma activo
- Estado de `Bip`
- Estado de `Alarmas`
- Indicador BLE compacto, solo si está activado

### Acción rápida (sin menú)

Pulsación corta: activa o desactiva `Bip` directamente.

### Menú de configuración (pulsación larga ~1.2 s)

| Opción | Qué hace |
|---|---|
| **Bip** | Activa o desactiva beeps de navegación y confirmación |
| **Alarmas** | Activa o desactiva audio de alertas y final del Timer |
| **Reposo** | Configura el tiempo de inactividad para entrar en reposo |
| **Idioma** | Cambia el idioma completo del dispositivo |
| **Reset** | Restaura toda la configuración del dispositivo a valores por defecto |
| **Salir** | Cierra el menú |

### Diferencia entre Bip y Alarmas

| Ajuste | Controla |
|---|---|
| **Bip** | Beeps de interfaz: clicks del encoder, confirmaciones, navegación |
| **Alarmas** | Audio automático: alertas de sensores y final de cuenta regresiva del Timer |

Desactivar `Bip` **no** silencia las alertas automáticas. Desactivar `Alarmas` **no** desactiva los beeps de navegación.

### Reset global

> **⚠️ PRECAUCIÓN**
> El Reset de Sistema borra toda la configuración guardada: calibraciones, rangos, niveles, límites, idioma, unidades y preferencias de audio. Esta acción no se puede deshacer.

Opciones de confirmación: `NO` (por defecto) / `SI`

Al confirmar `SI`, el P-Bit muestra `Reiniciando...`, reinicia y vuelve al flujo inicial. En el siguiente arranque aparece el selector de idioma porque la configuración guardada quedó limpia. Las pantallas de confirmación de Reset usan fondo rojo de alerta.

---

## 16. Audio — Bip y Alarmas

El P-Bit tiene dos canales de audio independientes.

### Bip

- Beeps cortos al girar el encoder en un menú.
- Confirmación al pulsar y guardar.
- Retroalimentación de navegación.
- Se controla con `Sistema > Bip` o con pulsación corta en la pantalla de Sistema.
- En una build nueva o tras `Reset`, queda en `OFF` por defecto.

### Alarmas

- Audio al activarse una alerta de sensor (temperatura, humedad, DS18B20).
- Melodías del sensor de suelo al cambiar de categoría.
- Alarma al terminar una cuenta regresiva del Timer.
- Se controla con `Sistema > Alarmas`.
- En una build nueva o tras `Reset`, queda en `OFF` por defecto.

---

## 17. LED RGB

El LED RGB indica el estado visible del dispositivo. En diales y tarjetas sigue el mismo significado de color que la pantalla; en vistas de solo Luz permanece apagado para no alterar el LDR.

| Pantalla | Color LED | Significado |
|---|---|---|
| Temperatura | Naranja | Normal |
| Temperatura | Azul | Alerta: temperatura baja |
| Temperatura | Rojo | Alerta: temperatura alta |
| Humedad | Azul | Normal / óptimo |
| Humedad | Naranja | Alerta: muy seco |
| Humedad | Rojo | Alerta: muy húmedo |
| Luz | **Apagado** | Apagado para no interferir con el LDR |
| Sonido | Magenta | Normal |
| Sonido | Naranja | Sonido fuerte |
| Sonido | Rojo | Sonido muy fuerte |
| Suelo | Verde | Óptimo |
| Suelo | Amarillo | Seco / muy seco |
| Suelo | Azul | Húmedo / muy húmedo |
| Termómetro | Blanco | Normal |
| Termómetro | Azul | Alerta: temperatura baja |
| Termómetro | Rojo | Alerta: temperatura alta |

---

## 18. Solución de problemas frecuentes

| Problema | Causa probable | Solución |
|---|---|---|
| La pantalla muestra `Sin sensor` en Termómetro | Sonda DS18B20 no conectada o mal conectada | Verificar conexión en `IO33` |
| La pantalla muestra `Sin sensor` en Suelo | Sensor de suelo no conectado | Verificar conexión en `IO35` |
| El sensor de Suelo muestra valores incorrectos | Sin calibración o calibración en condiciones diferentes | Recalibrar con `Calibrar sensor` |
| El LED se apaga al entrar en Luz | Comportamiento esperado | Normal — el LED se apaga para no interferir con la medición |
| Las alertas no suenan | `Alarmas` está en OFF | Activar desde `Sistema > Alarmas` |
| El dispositivo no guarda los ajustes tras reiniciar | Se flasheó firmware nuevo | Un flash nuevo borra la configuración. Volver a configurar |
| El valor de temperatura parece incorrecto | DHT11 sin estabilizar o con ruido térmico | Esperar 1–2 minutos desde el encendido |
| La pantalla vuelve a mostrar parpadeo o ghosting | Regresión visual puntual tras cambios de pantalla o demo | Anotar pantalla/modo y revisar las reglas anti-flicker; el problema queda resuelto por ahora |
| No hay respuesta al girar el encoder | El dispositivo está en un menú con edición de valor activa | Pulsar para salir de la edición antes de girar |

---

## 19. Mantenimiento y recomendaciones

- **Limpiar los sensores externos** con un paño seco. No usar líquidos ni solventes sobre la placa.
- **Recalibrar el sensor de suelo** si se cambia el tipo de sustrato o la fuente de alimentación.
- **Verificar la build activa** antes de entregar el dispositivo. Seguir `docs/PRODUCTION_CHECKLIST.md`.
- **Confirmar BLE desactivado** con escaneo externo antes de entregar unidades. El firmware lo fuerza `OFF` en cada flash nuevo, pero conviene verificarlo.
- No usar la pantalla de Sonido como instrumento de medición acústica. Sus valores son relativos e interpretativos.
- Los valores de temperatura del DHT11 pueden tardar 1–2 minutos en estabilizarse tras el encendido.

---

## 20. Apéndice — Usos educativos sugeridos

### Diario de una planta

Medir suelo, luz y temperatura diariamente. Registrar cuándo la planta está en condiciones óptimas y comparar días con riego y sin riego. Aprendizaje: interpretación de variables ambientales, relación entre datos y salud de la planta.

### Comparación de zonas del aula

Medir cerca de una ventana, una puerta y una pared interior. Comparar luz, temperatura y sonido. Aprendizaje: toma de datos comparativa, visualización de diferencias, análisis espacial.

### Experimento de riego

Calibrar el sensor de suelo. Definir los rangos `Seco` y `Húmedo`. Observar durante varios días cómo baja la humedad y decidir el momento de riego con base en los datos.

### Mapa de sonido

Medir sonido en aula, pasillo, patio y biblioteca. Registrar picos. Comparar recreo vs clase. Aprendizaje: recolección de datos, representación comparativa, bienestar acústico.

### Exploración con sonda externa

Medir temperatura en distintos recipientes, materiales o posiciones (sombra/sol, interior/exterior). Comparar con la temperatura ambiente del DHT11.
