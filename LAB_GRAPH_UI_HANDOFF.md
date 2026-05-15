# Handoff: Pantallas Lab / Graph

Fecha de actualización: 2026-05-16

Este documento resume el estado actual de las pantallas temporales de laboratorio y de la pantalla de gráfica. Incluye el alcance, los archivos clave, el estado actual de cada pantalla y el último build verificado.

## Estado de la pasada actual

La pasada de unificación visual iniciada el 2026-05-15 está aplicada y compila, pero no está cerrada visualmente hasta nueva validación en hardware.

Checkpoint compacto antes de galerías/templates:

- Aprobadas: `HOME`, `CLIMA LAB`, `GRÁFICA`.
- No tocar shared geometry global salvo necesidad extrema: `include/layout.h` queda como regla madre vigente.
- Próxima ejecución: microajustes locales en `SOUND LAB`, `TEMP LAB`, `GAUGE LAB`, `TEMP CARD`/`PROBE CARD`.
- Nueva arquitectura prevista: `TEMP CARD` y `PROBE CARD` se unen en una galería/template de sensor cards con cambio por pulsación corta; `GAUGE LAB` pasa a galería/template de gauges por sensor.
- Archivos esperados: `src/ui_lab_sound_vu.cpp`, `src/ui_lab_widget_showcase.cpp`, `src/ui_lab_sensor_cards.cpp`, `include/ui_lab_sensor_cards.h`, `include/ui_lab_widget_showcase.h`, `src/tft_display.cpp`, `include/tft_display.h`, `src/rotary.cpp`.
- Validación obligatoria: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` y nota de hardware pendiente.

Pantallas aprobadas como referencia estable:

- `LAB_HOME_CARDS_SCREEN` (`HOME`)
- `LAB_DUAL_TH_SCREEN` (`CLIMA LAB`)
- `GRAPH_SCREEN` (`GRÁFICA`)

Cambios ya aplicados:

- Se creó una regla común de header en `include/layout.h`: `L_HEADER_Y=18`, `L_HEADER_LINE=23`, `L_CONTENT_TOP=27`.
- `drawHeader()` ahora usa `C_BASELINE`, no `TC_DATUM`, para que todos los títulos se midan como baseline real.
- `drawMasterCardHeader()` quedó alineado con la misma regla de header.
- Se añadieron constantes `LC_MASTER_*` para la geometría validada de cards 2x2.
- `HOME`, `ESTADO LAB` y `PLANT LAB` usan la geometría `LC_MASTER_*`.
- Se quitaron footers `Vista experimental` de pantallas donde no aportaban acción.
- Se corrigieron clears y solapes detectados en `CLIMA LAB`, `SENSOR LAB`, `VALOR LAB`, `SYSTEM`, `SOIL`, `LIGHT` y `TIMER`.
- `TEMP CARD` y `PROBE CARD` recibieron un primer rescate de layout: card más alta, rail separado, etiqueta de dispositivo sin pisar el rail y sin footer experimental.
- `SOUND LAB` separa mejor la etiqueta `STACK/WAVE` del valor porcentual.

Ronda de ajuste posterior a capturas del 2026-05-15:

- Se aumentó en `1 px` el aire entre título y línea: `L_HEADER_LINE=23`.
- La primera línea/borde de contenido queda en `y=27`, dejando `3 px` limpios entre línea y card.
- Regla general de cards externas: `LC_SCREEN_X=2`, `LC_SCREEN_W=156`, `LC_SCREEN_BOTTOM=126`, `LC_CARD_RADIUS=4`.
- `HOME`: `LC_MASTER_CARD_H` sube a `48`, con `x=2/82`, `y=27/79`, `w=76`, gap `4`, bottom común `126`.
- `CLIMA LAB`: paneles en `x=2/82`, `y=27`, `w=76`, `h=73`, gap central `4`; footer de estado en `x=2`, `y=104`, `w=156`, `h=23`, bottom `126`; barras internas en `y=89` y texto `Óptimo` centrado en `y=115`.
- `TEMP LAB`: dos cards superiores en `x=2/82`, `y=27`, `w=76`, `h=50`; card inferior de delta en `x=2`, `y=81`, `w=156`, `h=46`, bottom `126`; unidad `C/F` junto a `TEMP/SONDA`.
- `PLANT LAB`: filas en `y=27/47/67/87/107`, alto `16`, gap visual `4`, para usar más el espacio inferior.
- `SENSOR LAB`: summary en `x=2`, `y=27`, `w=156`, `h=42`; gráfico en `x=2`, `y=73`, `w=156`, `h=42`.
- `GRÁFICA`: marco exterior en `x=2`, `y=46`, `w=156`, `h=66`, radio `4`; banda sensor/valor arranca en `L_CONTENT_TOP`.
- `VALOR LAB`: card ampliada a `x=2`, `y=27`, `w=156`, `h=100`, bottom `126`; la unidad ya se dibuja junto al valor y no debajo de la sparkline; sparkline y barra crecen.
- `GAUGE LAB`: icono de sensor XL, gauge `r=32`, unidad arriba a la derecha, min/max alineados con el inicio y fin del arco.
- `TEMP CARD` / `PROBE CARD`: card `x=2`, `y=27`, `w=156`, `h=100`, bottom `126`; rail en `x=140`, `y=52`, `w=12`, `h=62`; línea inferior con fallback de fuente para textos largos.
- Build verificado: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`.

Ronda general de unidad visual:

- `CLIMA LAB`: paneles superiores alineados con HOME (`x=2/82`, `w=76`, `y=27`); footer inferior alineado con los mismos límites (`x=2`, `w=156`, bottom `126`); texto del footer subido `1 px`; barras internas subidas `3 px`.
- `TEMP LAB`: cards superiores alineadas con HOME (`x=2/82`, `w=76`, `y=27`); card inferior crece hasta bottom `126`; unidad `C/F` se dibuja junto al título `TEMP/SONDA`; valores principales bajan dentro de cada card para ganar aire.
- `VALOR LAB`, `GAUGE LAB`, `SENSOR LAB`, `TEMP CARD`, `PROBE CARD`, `SOUND LAB` y `GRAPH` adoptan los límites externos comunes donde tienen card/panel principal.
- `GRAPH` conserva su banda de sensor/valor, pero el marco de gráfica ahora usa límite exterior `x=2,w=156` y radio `4`.
- Build verificado tras esta ronda: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`.

Ronda puntual posterior de microdiagramación:

- `HOME`: icono, nombre de sensor y dato de cada card se movieron `x+2,y+2` sin mover el tanque lateral ni la geometría externa.
- `CLIMA LAB`: iconos internos `x+2`, unidad `C/F/%` `x-1`, barras internas subidas `4 px`, barras más altas (`h=9`) y hint de estado `y-1`.
- `TEMP LAB`: cards superiores nombran `DHT11` y `DS18`; nombres centrados a la misma altura, unidad `C/F` queda en la esquina superior derecha y valores `y+1`.
- `TEMP LAB`: la card inferior dejó de ser una curva histórica y ahora es una barra comparativa de diferencia centrada; `DHT11 - DS18` llena hacia la derecha si DHT11 está más caliente y hacia la izquierda si DS18 está más caliente; la etiqueta usa la clave `LAB_TEMP_DIFF`.
- `GRÁFICA`: etiqueta de sensor y dato de la banda superior suben `1 px` (`LG_SENSOR_Y=28`).
- `SENSOR LAB`: summary y gráfica bajan `1 px` de altura cada una, mantienen gap y top común, y dejan más aire inferior.
- Build verificado tras esta ronda: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`.

Ronda amplia posterior con múltiples agentes:

- Corrección posterior: `LAB_SENSOR_FOCUS_SCREEN` se mantiene visible en el carrusel y restaurable desde sueño; esta es la pantalla/carretel de sensores aprobada como referencia visual.
- La pantalla gris inicial de validación de layout en `setup()` quedó apagada con `kShowStartupLayoutValidation = false`; esa era la prueba de 4 cards antes de los logos, no el carretel de sensores.
- Build verificado tras la corrección: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`, Flash usada `914233 bytes`.
- Menú de idiomas: las tres opciones suben `2 px`.
- `HOME`: icono, etiqueta y valor bajan `2 px`; etiqueta/valor se mueven `x+1`; el tanque lateral duplica ancho y crece hacia la izquierda.
- `CLIMA LAB`: unidades `C/F/%` bajan `1 px`.
- `PLANT LAB`: nombres de sensores se mueven `x+1`.
- `GRÁFICA`: banda superior sube a `LG_SENSOR_Y=27` y usa nombres largos por sensor mediante claves `GRAPH_LABEL_*`.
- `TEMPERATURA`: card principal se alinea con el límite común `LC_SCREEN_X/LC_CARD_TOP` y gana altura para seguir la regla de cards.
- `GAUGE LAB`: icono de temperatura más grande y centrado verticalmente en el espacio útil; etiqueta `TEMP` centrada con el icono; valor dentro del gauge usa fuente más grande.
- `VALOR LAB`: dato principal más grande, barra inferior con doble altura y sparkline con fondo sutil para distinguirse.
- `TEMP LAB`: cards superiores con fondos diferenciados, nombre y unidad en colores distintos; `DS18B20` completo; card de diferencia con escala base `0..10C` / `0..18F` hacia ambos lados y marcas de escala en vez de nombres de sensores.
- `SOUND LAB`: se quitaron `STACK/WAVE` y el hint de cambio de vista; el footer muestra el estado (`Normal`, etc.); chip `MIC`, valor y gráfica se redistribuyen para aprovechar más alto.
- `TEMP CARD` / `PROBE CARD`: se quita `Pulsa F/C`; etiqueta de dispositivo pasa a esquina superior izquierda; dato sube `10 px` y `x-6`; rail derecho triplica ancho y sube; unidad baja `2 px`; joya de alerta baja `6 px`.
- Build verificado tras esta ronda: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.0%`, Flash `69.8%`, Flash usada `914793 bytes`.

Ronda puntual posterior:

- `HOME`, `CLIMA LAB` y `GRÁFICA` quedan aprobadas como patrón visual de referencia.
- `TEMP LAB`: se eliminó el texto `DIF TEMP`; la barra diferencial sube `3 px`; `0` y escala `+10C/+18F` quedan arriba; el valor diferencial queda centrado abajo; si `DHT11` está más caliente llena hacia la izquierda y si `DS18B20` está más caliente llena hacia la derecha.
- `SOUND LAB`: se quitó el chip/card de `MIC`; `MIC` queda como etiqueta suelta, el valor sube `3 px` y el indicador de límites pasa a la esquina inferior izquierda.
- `GRÁFICA`: en humedad, las líneas horizontales dejan de llegar a blanco y pasan a una progresión azul más suave.
- `GAUGE LAB`: icono baja `3 px`, pasa a blanco y la unidad `C/F` usa naranja.
- `VALOR LAB`: barra inferior baja `1 px`, `DHT11` sube `1 px`, gráfica crece `1 px` hacia abajo y valor/unidad bajan `1 px`.
- `TEMP CARD` / `PROBE CARD`: indicador de límites se mueve `x-3,y+4`; nombre del sensor sube y pasa a blanco para evitar corte por clear area.
- Build verificado tras esta ronda: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"` -> `SUCCESS`, RAM `14.1%`, Flash `69.7%`, Flash usada `914169 bytes`.

Corrección de código 2026-05-16:

- `GAUGE LAB` (`src/ui_lab_widget_showcase.cpp`): icono pasa a `TFT_WHITE` cuando el sensor es válido (era `primary`). Motivo: los colores de icono y unidad del GAUGE LAB no se habían aplicado en la ronda anterior aunque el handoff los documentaba como hechos.
- `GAUGE LAB` (`src/ui_lab_widget_showcase.cpp`): unidad (`C/F`, `%`, etc.) pasa a `kWarmOrange` cuando el sensor es válido (era `primary`).
- `src/ui_lab_widget_showcase.cpp`: eliminadas 6 funciones `draw_icon_*_big` que eran dead code y generaban errores de compilación por forward-reference (`draw_solid_drop_large`, `draw_solid_light_large`, `draw_solid_mic_large` usadas antes de definirse).
- Build verificado: `SUCCESS`, RAM `14.1%`, Flash `69.8%`, Flash `915057 bytes`.

Pendiente antes de aprobar:

- Flashear y validar en hardware esta ronda de reglas generales, especialmente la nueva distancia línea-card de `3 px` y el bottom común `126`.
- Validar visualmente la nueva barra diferencial de `TEMP LAB`: dirección izquierda/derecha, lectura de `DHT11`/`DS18` y colores vivos sin exceso de saturación.
- Confirmar que títulos largos como `TEMPORIZADOR`, `TERMÓMETRO` y `TEMPERATURA` siguen sin salirse ni quedar recortados con el header aprobado.
- Revisar si `TEMP LAB` necesita reducir fuente en idiomas donde `Sonda`/`Sense sensor` no quepan en la card superior.
- Decidir si `TEMP CARD`, `PROBE CARD` y `TEMP LAB` se mantienen visibles o se ocultan del carrusel.

Regla de documentación:

- Cada ajuste visual nuevo debe dejar una nota breve en este documento o en `ANALISIS_PANTALLAS_EXPERIMENTALES_PBIT.md`.
- La nota debe indicar archivo tocado, motivo del cambio, estado de build y qué queda pendiente de validar en pantalla real.

## Alcance

Estas iteraciones afectan solo a pantallas temporales o experimentales:

- `GRAPH_SCREEN`
- `LAB_DASH_OVERVIEW_SCREEN` (`ESTADO LAB`)
- `LAB_SENSOR_FOCUS_SCREEN` (`SENSOR LAB`)
- `LAB_DUAL_TH_SCREEN` (`CLIMA LAB`)
- `LAB_ICON_SET_A/B/C_SCREEN` (`OUTLINE / SOLID / PIXEL`)
- `LAB_GAUGE_TEMP_SCREEN` (`GAUGE LAB`)
- `LAB_VALUE_MODERN_SCREEN` (`VALOR LAB`)
- `LAB_WIDGET_MIX_SCREEN` (`WIDGET LAB`)

Esta tanda no está destinada a cerrar un producto final. Son pantallas de prueba visual que se usan como banco de experimentación para definir luego qué se integra y qué se descarta.

## Archivos principales

- `src/ui_lab_dash.cpp`
- `src/ui_lab_focus.cpp`
- `src/ui_lab_dual.cpp`
- `src/ui_lab_widget_showcase.cpp`
- `src/ui_lab_sound_vu.cpp`
- `src/ui_lab_home_cards.cpp`
- `src/ui_lab_sensor_cards.cpp`
- `src/ui_graph.cpp`
- `include/layout.h`
- `include/languages.h`
- `src/tft_display.cpp`
- `src/lang_select.cpp`

Estos son los archivos donde están los ajustes más recientes y el comportamiento visual actual.

## Estado actual general

### 1. `ESTADO LAB` (`LAB_DASH_OVERVIEW_SCREEN`)

- Ya hay un layout de 4 filas totalmente funcional.
- El sistema redibuja solo los datos que cambian, no toda la fila.
- Se mejoró el espaciado para evitar que el contenido llegue demasiado al borde inferior.
- La paleta actual es uniforme: texto blanco + acento cromático por fila.

Pendiente:

- Validar en hardware si el espaciado de las 4 filas resulta homogéneo.
- Confirmar ausencia de parpadeo en cambios rápidos de sensor.

### 2. `SENSOR LAB` (`LAB_SENSOR_FOCUS_SCREEN`)

- La geometría ya es coherente entre los sensores.
- El redibujo se separa en shell estático y contenido dinámico.
- Todos los valores usan misma tipografía y alineación vertical consistente.
- Las transiciones de estado y color están listas para verificación.

Pendiente:

- Comprobar en pantalla real si el valor de `SOUND` se lee con la misma facilidad que `TEMP`.
- Ajustar contraste de unidades para `LIGHT` y `DS18` si hace falta.

### 3. `CLIMA LAB` (`LAB_DUAL_TH_SCREEN`)

- El layout actual ya está en la rama principal.
- Se redujeron las cards de temperatura/humedad para dejar espacio al estado verde de clima óptimo.
- Se mantiene shell/contenido separado para minimizar redraws.

Pendiente:

- Verificar en hardware si la nueva proporción de cards resulta equilibrada.
- Confirmar que la card de estado óptimo ya cabe sin recortes ni solapamientos.

### 4. `GAUGE LAB` y `VALOR LAB` (`LAB_GAUGE_TEMP_SCREEN`, `LAB_VALUE_MODERN_SCREEN`)

- Se avanzó en propuestas visuales basadas en iconos sólidos.
- `GAUGE LAB` muestra un anillo grande con valor centrado.
- `VALOR LAB` ahora usa cards con valor grande y barra segmentada.

Pendiente:

- Validar si la legibilidad del valor en el anillo es adecuada en la pantalla real.
- Decidir si estas versiones se mantienen como experimentales o se convierten en patrones de diseño.

### 5. `WIDGET LAB` (`LAB_WIDGET_MIX_SCREEN`)

- Se implementó el widget mixto de temperatura/humedad/delta en una sola pantalla.
- Se añadió un gráfico de delta con cero en el centro y color según signo.

Pendiente:

- Verificar si el gráfico de delta es suficientemente claro y si la línea cero se percibe bien.
- Determinar si este estilo merece una pantalla propia o debe simplificarse.

### 6. `SOUND LAB` (`LAB_VU_STACK` / `LAB_VU_WAVE`)

- Se ajustó la altura del valor, de las barras y del texto de estado para que queden más altos y legibles.
- Se desplazó el panel interior `Stack/Wave` hacia arriba y se mejoró el espaciado del badge.
- El estado del texto (`Normal`, `Loud`, etc.) también se subió unos píxeles.

Pendiente:

- Confirmar comportamiento visual en hardware, especialmente la posición y claridad del texto de estado.
- Verificar si la barra interior necesita más desplazamiento en la pantalla real.

## Verificación de documentación técnica

- El documento ahora refleja el estado actual del código y no el estado previo de abril.
- Se incluye el build más reciente y la recomendación de pasos siguientes.
- Se especifica claramente qué se ha hecho y qué está pendiente.

## Build verificada

Última compilación correcta en esta rama:

- Herramienta: `py -3 -m platformio run --project-dir "c:/POWAR-GIT/P-Bit-FW - edit"`
- Resultado: `SUCCESS`
- RAM usada: `14.1%`
- Flash usada: `69.8%`
- Flash bytes: `915057`
- Fecha: `2026-05-16`

## Problema persistente en HOME

- El card `MIC` en `LAB_HOME_CARDS_SCREEN` sigue mostrando un artefacto inicial en la esquina superior derecha al arrancar desde el selector de idioma.
- Se aplicaron las siguientes correcciones sin éxito definitivo:
  - limpieza total de pantalla tras salir de `showLanguageMenu()`
  - doble `tft.fillScreen(TFT_BLACK)` con retardo al iniciar el primer redraw de HOME
  - limpieza explícita del área de cards en `draw_shell()` de `src/ui_lab_home_cards.cpp`
- El bug parece ser un artefacto residual de transición de pantalla, no un error directo de geometría del card MIC.
- Si persiste en hardware, conviene evaluar otro enfoque o modelo visual para el HOME en lugar de insistir en el mismo layout.

## Siguiente paso recomendado

1. Validar en hardware `ESTADO LAB`.
2. Validar en hardware `SENSOR LAB / HUM`.
3. Validar en hardware `SENSOR LAB / LIGHT`.
4. Revisar `CLIMA LAB` para asegurarse de que la card verde entra bien.
5. Validar `SENSOR LAB / SOUND`.
6. Validar `SENSOR LAB / SOIL`.
7. Validar `SENSOR LAB / DS18`.
8. Validar `GRAPH TEMP`.
9. Validar `GRAPH HUM`.

> Nota: esta tanda es de experimentación visual. No se propone integrar todo tal cual; el objetivo actual es concretar cuáles de estas pantallas deben seguir y cuáles deben simplificarse o eliminarse.

Feedback corto esperado:

- `Estado Lab fila 3 bajar 1`
- `Hum unit x -1`
- `Sound sigue vibrando`
- `Clima % ok`
- `Graph temp horizontales aún muy visibles`

## Nota importante para otro agente

No rehacer estas pantallas desde cero.

La base visual ya está bastante conseguida. Lo correcto ahora es:

- retocar microgeometría en hardware
- validar paletas
- y cerrar flicker residual

No abrir cambios grandes de arquitectura salvo que el usuario lo pida explícitamente.
