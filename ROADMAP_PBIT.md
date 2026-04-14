# P-Bit Roadmap

Documento de referencia para futuras iteraciones del firmware del P-Bit.

Objetivos:
- dejar registradas las mejoras sugeridas y acordadas
- separar deuda técnica de mejoras de producto
- ordenar futuras iteraciones de menús, sensores y UX
- maximizar el potencial educativo (STEAM) para los niños

## Estado real del código actual

### Hecho
- Snapshot seguro de lecturas entre tareas y pantallas. `io`, `tft_display` y `ble` ya copian `Reading` con `portENTER_CRITICAL` / `portEXIT_CRITICAL` (`src/io.cpp:18-64`, `src/tft_display.cpp:153-162`, `src/ble.cpp:105-107`).
- Menús por pantalla ya implementados con subestados, confirmación con encoder y persistencia en NVS: `Temp`, `Humidity`, `Sound`, `Light`, `DS18`, `System` y `Soil` (`src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_sound.cpp`, `src/ui_light.cpp`, `src/ui_ds18.cpp`, `src/ui_system.cpp`, `src/ui_soil.cpp`).
- `System` ya funciona como hub global con sonido, sleep, idioma y reset (`src/ui_system.cpp`).
- El encoder está centralizado para navegar la app y los submenús, y también para salir de `IDLE` y restaurar contexto (`src/rotary.cpp`).
- El modo de energía ya está resuelto a nivel de producto actual: `IDLE` visible con `ZZZ`, timeout configurable y bloqueo mientras hay menús activos o el timer está corriendo. El deep sleep automático quedó desactivado en la práctica porque esta revisión de hardware deja la TFT en blanco al dormir (`src/main.cpp`, `src/ui_system.cpp`).
- La localización base ya existe: menú de idioma en cold boot, carga silenciosa desde NVS al despertar y cambio dinámico desde `System` (`src/main.cpp`, `src/lang_select.cpp`, `src/ui_system.cpp`).
- DS18B20 ya está integrado de forma completa: lectura, sentinel de ausencia, offset, umbrales y alertas persistentes (`src/hw.cpp`, `src/io.cpp`, `src/ui_ds18.cpp`).
- DHT11 ya está integrado como fuente de temperatura y humedad del aire; las lecturas fallidas pasan a `NAN` tras reintentos (`src/io.cpp`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`).
- Las alertas ya cubren todos los sensores del producto actual: `Temp`, `Humidity`, `DS18`, `Light`, `Sound` y `Soil`. `Light` y `Soil` ya tienen además submenú propio `Alertas ON/OFF`; en `Sound` la alerta quedó intencionalmente solo visual/RGB para no contaminar la lectura del micrófono, mientras que `Soil` usa melodías cortas por transición a `Seco`, `Óptimo` y `Muy húmedo` (`src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_ds18.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_soil.cpp`, `src/main.cpp`, `src/rotary.cpp`).
- La limpieza visual ya mejoró bastante: router de UI, overlays de energía, redraw selectivo y borrado de áreas concretas reducen flicker y repintados completos innecesarios; además `System`, `Timer` y varios menús ya usan bandas de limpieza más explícitas para textos cambiantes, y el overlay de reposo usa las mismas GFX fonts del sistema para asegurar visibilidad real en pantalla (`src/tft_display.cpp`, `src/main.cpp`, `src/ui_*`, `src/ui_widgets.cpp`).
- El indicador mínimo de alertas de runtime ya se movió a una zona segura común abajo a la izquierda, se redujo de tamaño y además se redibuja después de las bandas dinámicas para no quedar tapado por los `clear rects` de `Temp`, `Humidity`, `Sound` y `DS18` (`src/ui_widgets.cpp`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_sound.cpp`, `src/ui_ds18.cpp`, `include/layout.h`).
- Los menús configurables ya tienen `Reset` local a valores por defecto del propio módulo en `Temp`, `Humidity`, `Light`, `Sound`, `Soil` y `DS18`, con persistencia en NVS y confirmación binaria consistente (`src/hw.cpp`, `include/hw.h`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_soil.cpp`, `src/ui_ds18.cpp`).
- La confirmación de reset del menú de `System` ya se compactó para que respire mejor y no pelee con los textos (`src/ui_system.cpp`, `src/ui_widgets.cpp`).
- La persistencia ya no vive mezclada dentro de `hw.cpp`: existe una capa dedicada `settings_store.*` para NVS, mientras `hw.cpp` conserva defaults, validación y caché en memoria (`src/settings_store.cpp`, `include/settings_store.h`, `src/hw.cpp`, `include/hw.h`).
- El runtime ya usa `runtime_events.*` como capa explícita de señalización entre tareas y ahora también centraliza las solicitudes de refresh/redraw desde las UIs con `runtime_request_ui_refresh(...)`, reduciendo la manipulación directa de flags compartidos (`src/runtime_events.cpp`, `include/runtime_events.h`, `src/io.cpp`, `src/main.cpp`, `src/rotary.cpp`, `src/tft_display.cpp`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_ds18.cpp`, `src/ui_soil.cpp`, `src/ui_system.cpp`).
- Ya existe una primera extracción real del framework común de menús: helpers centrados para listas, pantallas de valor, bandas de limpieza, prompts de reset y estados `Saved`, usados ya por `System` y por las pantallas de `Temp`, `Humidity`, `Light`, `Sound`, `Soil` y `DS18` (`src/ui_widgets.cpp`, `include/ui_widgets.h`, `src/ui_system.cpp`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_soil.cpp`, `src/ui_ds18.cpp`).
- La pasada fina de UX ya homogeneizó mejor el marco de menús: los menús de 4 y 5 opciones usan espaciado compartido, los prompts de `Reset` ya muestran el footer/hint con el mismo patrón en todas las pantallas, y los resúmenes multilinea de `Saved` ahora reutilizan helpers comunes en vez de dibujarse cada uno con posiciones mágicas distintas (`include/layout.h`, `include/ui_widgets.h`, `src/ui_widgets.cpp`, `src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_soil.cpp`, `src/ui_ds18.cpp`, `src/ui_system.cpp`).
- Ya existe un `AlertEngine` compartido que centraliza clasificación/transiciones de alertas y emisión de audio, se refresca desde el `sensor task` con getters estables y ahora además resuelve un estado global real: alerta principal por severidad y prioridad de sensor, conteo de alertas adicionales, cooldown de audio y `entry notice` breve para la UI (`src/alert_engine.cpp`, `include/alert_engine.h`, `src/io.cpp`, `src/tft_display.cpp`).
- La base técnica de alertas globales ya está implementada en runtime: el router puede reaccionar al estado global del `AlertEngine`, y el RGB puede seguir la alerta principal sin contaminar la lectura del LDR en la pantalla de `Light`; la capa visual global quedó desactivada temporalmente hasta cerrar una ubicación/layout que no invada las pantallas (`src/tft_display.cpp`, `src/ui_widgets.cpp`, `include/layout.h`, `include/ui_widgets.h`).
- La salida tipo `Serial Plotter` ya está en formato CSV limpio y ahora además queda detrás del flag `PBIT_ENABLE_SERIAL_PLOTTER`, para que el firmware normal no tenga que emitirla siempre; falta solo documentarla mejor si se quiere enseñar como modo laboratorio (`src/io.cpp`, `include/config.h`).
- Ya existe documentación funcional de alto nivel del firmware actual y su potencial educativo en `PBIT_FUNCIONAMIENTO_ACTUAL.md`.
- Los manuales largos ya quedaron resincronizados con el firmware real: tiempos de pulsación, menús actuales, alertas, suelo `Muy húmedo`, unidad global compartida `TEMP/DS18`, `Timer v2` con editor directo `HH:MM:SS` y modelo de reposo visible con `ZZZ` ya están alineados en `MANUAL_TECNICO_PBIT.md`, `MANUAL_DE_USUARIO_PBIT.md` y `PBIT_FUNCIONAMIENTO_ACTUAL.md`.
- Ya quedó contrastado firmware vs hardware con KiCad V3.1: la TFT usa `IO21/IO22` para `RST/DC`, mientras el bus I2C externo de placa vive en `IO26/IO27` (`SDA/SCL`). El firmware actual todavía no inicializa `Wire`, así que un futuro sensor I2C como `SCD41` deberá entrar con `Wire.begin(26, 27)` y no asumir I2C por defecto en `21/22`.
- La localización quedó prácticamente centralizada en `L(KEY)`: se eliminaron el placeholder muerto `ST_SOIL_THRESH_TODO`, los `tr(...)` dispersos y las funciones locales `tr()` de las pantallas de UI. Añadir un cuarto idioma requiere tocar sobre todo `include/languages.h` y `src/lang_select.cpp`; aun así, conviene asumir que pueden quedar cadenas residuales puntuales en módulos nuevos como la gráfica (`src/ui_temp.cpp`, `src/ui_humidity.cpp`, `src/ui_light.cpp`, `src/ui_sound.cpp`, `src/ui_soil.cpp`, `src/ui_ds18.cpp`, `src/ui_system.cpp`, `include/languages.h`, `src/lang_select.cpp`).
- `Timer v2` ya está aterrizado sin romper los gestos existentes: cronómetro en `00:00:00`, editor directo `HH:MM:SS` con encoder, layout mas limpio del card, duracion activa centrada debajo del card solo en cuenta regresiva, formato adaptativo `MM:SS:CC` / `HH:MM:SS`, final en rojo y alarma intermitente al terminar cuenta regresiva (`src/timer.cpp`, `include/timer.h`, `src/ui_timer.cpp`, `src/rotary.cpp`, `src/main.cpp`, `src/tft_display.cpp`).
- El runtime del `Timer` ya recompone mejor el valor grande sobre el card real: la banda del tiempo se limpia primero, el sprite se empuja con negro transparente y el borde se repinta al final para que la caja no quede “cortada” por el fondo del sprite en hardware (`include/layout.h`, `src/ui_timer.cpp`).
- El layout runtime del `Timer` siguió afinándose en hardware: el card bajó unos píxeles más y el estado acompañó ese ajuste, pero el tiempo grande se mantuvo donde mejor respiraba para abrir aire real arriba y abajo dentro del card (`include/layout.h`).
- El `Timer` ganó una iteración más de layout: el card runtime creció un poco hacia abajo para aprovechar mejor el espacio inferior sin pegar el borde a los dígitos, y el card del editor bajó también para separarse mejor del título `DURACIÓN` (`include/layout.h`, `src/ui_timer.cpp`).
- La calibración fina del editor del `Timer` ya se apoya también en el visualizador externo: el card de configuración se bajó otra vez hasta `Y=64`, pero el valor `HH:MM:SS` se mantuvo en su altura anterior para poder separar mejor el recuadro sin mover también el bloque numérico (`src/ui_timer.cpp`).
- Las dos pantallas temporales de calibración tipográfica que se usaron para ajustar el visualizador externo ya cumplieron su función y se retiraron del carrusel del firmware, dejando otra vez el producto centrado solo en las pantallas reales de uso (`include/tft_display.h`, `src/tft_display.cpp`, `src/rotary.cpp`, `src/main.cpp`).
- El selector de idioma también recibió un ajuste visual conservador: la flecha `>` mantiene la fuente interna `2`, pero baja `2 px` para respirar mejor frente al texto del idioma sin cambiar aún de tamaño ni mover su anclaje X (`src/lang_select.cpp`).
- El runtime del `Timer` también redujo trabajo por frame: la banda del tiempo ya no obliga a repintar el card completo en cada tick, y el refresco de centésimas se relajó un poco para reducir ghosting sin perder la lectura fina del cronómetro (`src/ui_timer.cpp`, `src/tft_display.cpp`).
- La pantalla de **Gráfica** ya está integrada en el carrusel como una nueva pantalla de runtime. La v1 actual cubre Temperatura y Humedad, usa buffers circulares para histórico reciente, auto-escala Y con rango mínimo por sensor y render en sprite para reducir parpadeo; hoy la escritura de muestras depende del ciclo lento del `sensor task` (~1 s) y la interacción de usuario se limita a pulsación corta para cambiar de sensor (`include/graph_buffer.h`, `src/graph_buffer.cpp`, `include/ui_graph.h`, `src/ui_graph.cpp`, `src/io.cpp`, `src/tft_display.cpp`, `src/rotary.cpp`, `include/layout.h`, `include/languages.h`, `src/lang_select.cpp`).

### Parcial

- La limpieza visual sigue siendo parcial: hay redraw selectivo, bandas de limpieza más claras, reset prompts homogéneos, resúmenes `Saved` más consistentes y mejor ubicación del indicador mínimo, pero todavía falta una validación final en hardware pixel a pixel.
- El menú de sonido no es una calibración física real del micrófono; hoy edita umbrales y activa/desactiva alertas (`src/ui_sound.cpp`, `src/hw.cpp`).
- El `Timer v2` ya existe con cronómetro y editor `HH:MM:SS`, pero todavía no tiene automatizaciones o flujos más ricos para laboratorio/experimentos (`src/rotary.cpp`, `src/timer.cpp`, `src/ui_timer.cpp`).
- El modelo global de alertas ya existe, pero sigue siendo deliberadamente simple: una alerta principal, contador `+N` y aviso corto. Si el producto crece, todavía podría evolucionar a una cola o política más rica.
- El framework de menús ya cubre listas, pantallas de valor, prompts de reset y buena parte de los resúmenes `Saved` en `System`, `Temp`, `Humidity`, `Light`, `Sound`, `Soil` y `DS18`, pero aún quedan detalles de contenido específicos por sensor que siguen siendo intencionalmente bespoke.
- La pantalla de **Gráfica** ya funciona como v1 de producto, pero todavía no participa en el marco común de menús/subestados y sigue necesitando una pasada fina de UX y localización antes de considerarla completamente asentada.
- Las pantallas temporales `ESTADO LAB`, `SENSOR LAB` y `CLIMA LAB` ya están integradas como banco de pruebas visual con snippets espejo para el visualizador; su estado fino, paletas actuales y pendientes inmediatos se documentan en [LAB_GRAPH_UI_HANDOFF.md](/c:/POWAR-GIT/P-Bit-FW%20-%20edit/LAB_GRAPH_UI_HANDOFF.md).

### Pendiente
- Gamificación de alertas: arcoíris rápido y sonido feliz al pasar a estado óptimo.
- Decidir si el menú de sonido debe seguir como edición de umbrales o si merece una calibración más real del entorno.
- Documentar el modo laboratorio / `Serial Plotter`.
- Diseñar una evolución del timer orientada a laboratorio sin romper los gestos actuales.
- Redefinir la UX visual global de alertas: hoy la lógica global existe, pero la señal visual global se retiró temporalmente porque las posiciones probadas invadían el layout. Hay que decidir una solución realmente segura o dejar las alertas globales solo en RGB/audio.
- Seguir reduciendo bloques bespoke del contenido de menús solo cuando valga la pena, sin forzar una abstracción peor que el layout específico del sensor.
- Ejecutar una auditoría visual pixel a pixel de todas las pantallas reales y sus escenas de visualizador para detectar solapes, textos que compiten, zonas de limpieza mal dimensionadas, ghosting y refrescos innecesarios antes de seguir ampliando layouts.

#### Pendientes de refinado de la pantalla Gráfica (v1)
- Ajuste visual fino de la gráfica en hardware: el usuario confirmó que funciona bien pero quedan pequeños retoques pendientes por definir (posición de etiquetas min/max, grosor de línea, color de grid, contraste general).
- Añadir Luz y Sonido al buffer circular y hacerlos seleccionables desde la pantalla Gráfica (actualmente solo Temperatura y Humedad).
- Añadir Suelo y DS18B20 al buffer si se añaden como sensores lentos con la misma cadencia de 1 s.
- Evaluar si el auto-escalado Y necesita un modo `fijo` (rango absoluto del sensor) además del modo `adaptativo` actual.
- Considerar mostrar el número exacto de muestras disponibles o el tiempo cubierto como texto de apoyo en la UI.
- Escena del visualizador externo para la pantalla Gráfica: actualmente no existe ninguna escena canon.

## Roadmap Paralelo: Visualizador / TFT Workstation

Este bloque no forma parte del firmware embebido, pero ya es una herramienta estratégica para iterar layouts, validar snippets y documentar pantallas del P-Bit antes de tocar hardware real.

### Estado actual consolidado

#### Hecho
- Ya existe una biblioteca canon de escenas del P-Bit, con escenas base y variantes de runtime / edición para varios módulos.
- El visualizador ya soporta escenas con lógica de layout bastante cercana al firmware real: variables simples, expresiones aritméticas, `tft.textWidth(...)`, mutación de variables (`+=`, `-=`, etc.) y preservación correcta de strings numéricas con ceros a la izquierda.
- El flujo de inspección visual ya es potente: selección en canvas, inspector editable, drag-and-drop, nudges finos, checkpoints, reset de escena, compare mode, medición en píxeles, snapping, undo/redo y exportación de capturas.
- El editor ya puede usarse para calibrar tipografía y revisar layouts complejos como el `Timer v2`, incluidos los estados del editor `HH:MM:SS`.
- La carpeta auxiliar `visualizer_scenes/` ya permite mantener snippets independientes del firmware para revisión manual y carga directa en la workstation.

#### Parcial
- El visualizador ya entiende bastante lógica real, pero aún no interpreta directamente helpers propios del firmware como `drawHeader(...)`, `drawCard(...)`, `drawCenteredMenuFrame(...)`, `drawSplitDecimalValue(...)` o `L(KEY)`.
- El motor ya es suficientemente útil para layouts complejos, pero todavía conviene evitar tratarlo como compilador C++ general.
- Las escenas canon ya existen para varias familias, pero falta una validación final más sistemática por lotes y una forma formal de marcar una escena como `golden` o `aprobada`.

### Mejoras sugeridas de mayor valor

1. Soporte de helpers propios del firmware.
   - Objetivo: poder pegar código más cercano al firmware real y no solo escenas aplanadas.
   - Prioridad alta para: `drawHeader(...)`, `drawCard(...)`, `drawCenteredMenuFrame(...)`, `drawSplitDecimalValue(...)`, `drawTankRight(...)`, `L(KEY)`.

2. Pipeline de iconos / activos gráficos.
   - Objetivo: facilitar símbolos reutilizables sin depender de emoji Unicode real.
   - Prioridad alta para: `drawBitmap(...)`, `drawXBitmap(...)`, `pushImage(...)`, importación de PNG/SVG pequeño y exportación a array C/C++.

3. Variables de escena editables.
   - Objetivo: cambiar desde UI valores de runtime sin duplicar escenas.
   - Ejemplos: temperatura, humedad, idioma, estado de alerta, valor de luz, estado del timer.

4. Diff visual / golden screenshots.
   - Objetivo: comparar render actual vs captura aprobada o vs versión anterior y detectar desviaciones visuales.

5. Funciones TFT_eSPI extra solo si aportan valor real.
   - Candidatas: `setViewport(...)`, `resetViewport(...)`, `setTextPadding(...)`, `setCursor()` + `print()/printf()`.
   - Regla: solo implementarlas si el firmware o los snippets reales empiezan a necesitarlas.

### Propuesta de suite de calibración visual

Objetivo:
- crear un conjunto estable de escenas de prueba que permitan verificar que el visualizador interpreta correctamente tipos, métricas, datums, colores, primitivas, sprites y helpers.
- permitir un flujo de ida y vuelta con AI Studio basado en capturas aprobadas, sin depender siempre del P-Bit físico.

Bloques sugeridos de calibración:

1. Tipografía y datum.
   - Validar: `TL/TC/TR`, `ML/MC/MR`, baseline, offsets verticales y horizontales, diferencias entre fuentes GFX y fuentes internas.

2. Strings y métricas.
   - Validar: `textWidth(...)`, `fontHeight()`, strings con ceros a la izquierda, layout secuencial `HH:MM:SS`, variables intermedias y `x += ...`.

3. Primitivas y geometría.
   - Validar: `fillRect`, `drawRect`, `drawRoundRect`, `drawFastHLine`, `drawFastVLine`, `drawCircle`, `fillCircle`, alineación, tamaño y grosor aparente.

4. Capas y limpieza.
   - Validar: orden de dibujo, `fillRect` de limpieza, overlays, hints, bandas negras y elementos que se pisan.

5. Sprites y transparencia.
   - Validar: `createSprite`, `fillSprite`, `drawString` en sprite, `pushSprite(...)`, transparencia, recorte y orden final sobre la TFT.

6. Color y contraste.
   - Validar: colores TFT frecuentes, grises técnicos, negros de fondo, legibilidad de amarillo/cian/verde/rojo y comportamiento del realce en tonos oscuros.

7. Widgets compuestos.
   - Validar: tanques verticales, barras horizontales, alert jewel, valores partidos entero/decimal, `%` / `lux` / unidad, categorías.

8. Activos gráficos futuros.
   - Validar: iconos bitmap o `pushImage(...)`, escalado, transparencia, alineación y tamaño visual.

### Flujo propuesto para calibración por capturas

1. Preparar una escena de calibración concreta.
2. Renderizarla en el visualizador.
3. Exportar la captura.
4. Devolver esa captura al chat de AI Studio junto con el código fuente de la escena.
5. Pedir análisis de:
   - si la escena se ve como se espera
   - qué parte parece correcta
   - qué parte revela un bug del motor
   - si hay que ajustar el visualizador o el snippet
6. Guardar la captura aprobada como referencia `golden`.
7. Repetir por bloques hasta cubrir tipografía, sprites, primitivas y widgets.

### Criterio de uso

- Si una desviación aparece en hardware real y en visualizador, revisar primero el firmware o el layout base.
- Si una desviación aparece solo en el visualizador, priorizar motor / parser / renderizador antes de tocar el snippet.
- Evitar calibrar a ojo escenas complejas completas cuando aún no existe una prueba mínima que aísle el fallo.

### Siguiente fase recomendada: auditoría visual cruzada

Objetivo:
- separar con rigor qué problemas pertenecen al firmware real, cuáles al visualizador y cuáles a escenas de calibración débiles o ambiguas.

Estrategia recomendada:
- validar primero el visualizador como instrumento con escenas mínimas de calibración
- auditar después las pantallas reales por lotes visuales, no por archivo aislado
- comparar siempre captura real del P-Bit, captura del visualizador y snippet/código fuente

Lotes iniciales sugeridos:
- `Temp`, `DS18`, `Timer` y selector de idioma como lote crítico de arranque
- `Humidity`, `Light`, `Sound` y `Soil` como lote de sensores secundarios
- `System`, menús, prompts de reset y pantallas `Saved` como lote de estados UI
- `Graph` como lote separado cuando la v1 de la pantalla ya esté más asentada

Criterio de clasificación:
- `Firmware`: el mismo fallo aparece en hardware real y en visualizador
- `Visualizador`: el fallo solo aparece en el visualizador
- `Escena`: la prueba no aísla bien el problema
- `Mixto`: hay debilidad en la escena y además un fallo real de motor o firmware

## Prioridades Reales

1. Mantener estables los menús ya existentes antes de añadir más complejidad.
2. Resolver solo una iteración nueva por pantalla cuando haya una necesidad clara.
3. Revisar primero si el sensor admite calibración real o solo umbrales de interpretación.
4. Consolidar visualmente la pantalla Gráfica actual antes de ampliar sensores o modos.
5. No volver a planificar como futuro lo que ya está implementado en código.

## Observaciones

- Si una pantalla ya tiene subestados, persistencia en NVS y feedback consistente, se considera base estable para seguir iterando.
- Antes de añadir una mejora nueva, comprobar si afecta al encoder, al sleep o a la legibilidad visual del conjunto.
