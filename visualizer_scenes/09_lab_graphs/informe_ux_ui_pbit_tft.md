# Informe completo de UX/UI para dashboards ambientales en pantallas TFT 1.8" 160x128 con ESP32 y TFT_eSPI

## Resumen ejecutivo

Las pantallas TFT de 1.8" con resolución 160x128 o 128x160 píxeles se utilizan habitualmente con ESP32 a través de controladores como ST7735 y librerías optimizadas como TFT_eSPI, lo que permite mostrar información a color con una tasa de refresco suficiente para interfaces sencillas y animadas.[web:5][web:8] La librería TFT_eSPI incluye ejemplos de diales animados, medidores lineales y soporte de sprites, que permiten construir dashboards visuales con agujas, barras de progreso y gráficos sin sacrificar demasiado rendimiento.[web:1][web:20][web:17]

Las buenas prácticas de diseño para pantallas pequeñas recomiendan priorizar el contenido esencial, reducir el ruido visual, usar tipografía legible y jerarquizar claramente la información, lo que se alinea muy bien con la naturaleza educativa y de instrumentación del dispositivo P‑Bit.[web:6][web:9][web:12] Complementar TFT_eSPI con la librería TFT_eWidget permite disponer de componentes listos como botones, gráficas, medidores y sliders, acelerando el diseño de interfaces ricas en tan pocos píxeles.[web:4][web:21][web:24]

## Objetivo del informe

Este documento reúne guías, sugerencias, buenas prácticas, patrones visuales y recomendaciones de implementación para diseñar pantallas UX/UI más claras, modernas y pedagógicas en un dispositivo STEAM ambiental como P‑Bit. El objetivo no es solo “hacer una pantalla bonita”, sino construir una interfaz que ayude a interpretar datos ambientales con rapidez, claridad y atractivo visual.[web:12][web:15]

## Capacidades técnicas del hardware y del stack gráfico

### Pantalla TFT 1.8" 160x128 / 128x160

Los módulos TFT de 1.8" más comunes suelen usar el controlador ST7735 y ofrecen una resolución de 128x160 píxeles con 65k colores, comunicándose por SPI de 4 hilos.[web:5][web:8] Funcionan a 3.3 V y son una combinación habitual con ESP32 en proyectos educativos y makers, donde el color y el coste bajo son más importantes que una resolución elevada.[web:5][web:8]

Esa resolución obliga a diseñar con mucha disciplina: no hay espacio para interfaces saturadas, texto pequeño ni detalles excesivos. Por eso la UI debe tratarse como una interfaz instrumental y pedagógica, no como una mini app móvil.[web:9][web:12]

### TFT_eSPI como base gráfica

TFT_eSPI es una librería optimizada para ESP32 y otros microcontroladores, con soporte para distintos controladores de pantalla, rotación, fuentes, primitivas de dibujo y sprites.[web:8][web:23] Incluye ejemplos de diales analógicos animados, medidores lineales y composiciones visuales útiles para dashboards técnicos.[web:1][web:10]

La clase Sprite es especialmente importante porque permite dibujar primero en RAM y luego enviar el resultado a la pantalla con `pushSprite`, reduciendo parpadeos y mejorando el refresco aparente en pantallas SPI.[web:20][web:17] Para una pantalla de 160x128 esto abre la puerta a interfaces más fluidas, con animaciones pequeñas, agujas, barras móviles y cambios parciales de pantalla.

### TFT_eWidget como capa de widgets

TFT_eWidget amplía TFT_eSPI con widgets de GUI como gauges, gráficas, sliders, botones y barras de progreso.[web:4][web:21][web:24] El cheat sheet de esta librería documenta funciones para crear medidores, gráficas y barras con una sintaxis compacta, lo que reduce el tiempo de desarrollo frente a dibujar cada elemento desde cero.[web:4]

Para el caso de P‑Bit, TFT_eWidget es especialmente útil porque permite combinar lectura numérica, elementos comparativos y pequeñas visualizaciones históricas sin cambiar de ecosistema gráfico.[web:4][web:21]

## Principios de UX/UI para pantallas pequeñas embebidas

### Priorizar lo esencial

Las buenas prácticas para pantallas pequeñas coinciden en priorizar el contenido esencial y suprimir todo lo accesorio.[web:6][web:9] En una pantalla de 160x128, cada píxel debe justificar su presencia, por lo que conviene limitar la información simultánea y evitar la tentación de “mostrarlo todo”.[web:12][web:15]

Esto significa que la pregunta de diseño principal no es “qué puedo meter”, sino “qué necesita entender el usuario en 1 o 2 segundos”. En un contexto educativo, esta rapidez de interpretación es crucial para que alumnado y docentes conecten sensor, entorno y significado.

### Una tarea o foco por pantalla

Las guías de UX para pantallas pequeñas recomiendan un foco dominante por vista: una lectura principal, una interacción principal o una visualización principal.[web:9][web:12] En dispositivos embebidos esto cobra aún más importancia, porque la interfaz se consulta en contextos breves y funcionales.[web:12][web:15]

Por tanto, un patrón muy recomendable para P‑Bit es separar una pantalla Home de visión general y varias pantallas de detalle, una por sensor o grupo de sensores. Esto mantiene claridad visual y reduce carga cognitiva.

### Legibilidad extrema

Las recomendaciones para small screens insisten en usar tipografía suficientemente grande, alto contraste, espaciado claro y pocas familias tipográficas.[web:6] En hardware embebido de baja resolución esto se traduce en fuentes simples, valores numéricos grandes, etiquetas cortas y unidades visibles.[web:6][web:12]

La regla práctica es clara: el número debe ser lo primero que se lea, el color debe aportar significado, y el texto auxiliar debe ser mínimo. Si un dato requiere mucha lectura para entenderse, probablemente la pantalla está mal resuelta.

### Consistencia visual

Los diseños embebidos eficaces usan patrones consistentes de color, iconografía, estados y distribución.[web:12][web:15] Si el verde siempre significa confort y el rojo alerta, el usuario aprende la interfaz más rápido y necesita menos esfuerzo para interpretar los datos.

En P‑Bit, esta consistencia es aún más valiosa porque el objetivo es pedagógico. Una interfaz coherente enseña visualmente, no solo muestra datos.

## Estructura recomendada del sistema de pantallas

### Arquitectura general

La estructura más sólida para tu caso es una arquitectura en tres niveles:

- Pantalla Home o resumen general.
- Pantallas de detalle por sensor o familia de sensores.
- Pantallas funcionales específicas, como cronómetro, experimento o modo diagnóstico.

Este patrón es coherente con las recomendaciones de UX en sistemas pequeños, donde conviene separar lectura general, exploración de detalle e interacción específica.[web:9][web:12]

### Pantalla Home

La Home debe responder a una sola pregunta: “¿Cómo está el entorno ahora mismo?”.[web:12][web:15] Por eso debe mostrar pocas variables, bien jerarquizadas y con codificación visual inmediata.

La mejor solución para 160x128 suele ser una de estas dos:

| Patrón | Uso recomendado | Ventaja principal |
|---|---|---|
| 2x2 cards | 4 métricas clave | Lectura rápida y clara |
| Lista de medidores lineales | 4 a 6 sensores | Muy eficiente en vertical |

En ambos casos, cada módulo debería incluir:

- Icono simple.
- Valor numérico grande.
- Unidad.
- Estado por color o mini barra.

### Pantallas de detalle

Cada pantalla de detalle debe profundizar en una sola magnitud o en una familia semántica de magnitudes.[web:12][web:15] Esto permite dedicar más espacio a visualizaciones expresivas como gauges, barras amplias o gráficas de tendencia.

Patrón sugerido por detalle:

- Sensor principal arriba con número grande.
- Widget protagonista en el centro (gauge, barra, gráfica o icono dinámico).
- Información secundaria abajo: mínimo, máximo, tendencia o estado.

### Agrupación semántica

Una interfaz educativa funciona mejor cuando agrupa sensores por significado y no solo por disponibilidad técnica.[web:12][web:15] En vez de mostrar “sensor A, B, C, D”, conviene pensar en bloques conceptuales como confort térmico, calidad acústica, iluminación y observación experimental.

Propuesta de agrupación para P‑Bit:

- **Confort térmico:** temperatura ambiente, DS18B20, humedad.
- **Suelo y cultivo:** humedad del suelo, temperatura asociada si procede.
- **Ambiente acústico:** nivel de sonido.
- **Luz y entorno:** nivel de luz.
- **Experimento:** cronómetro y lectura contextual.

## Patrones visuales más recomendables para tu pantalla

### Cards o tarjetas compactas

Las cards son un patrón muy útil para organizar información en pequeños bloques visuales y facilitar el escaneo rápido.[web:9][web:15] En una pantalla de 160x128 pueden usarse cuatro tarjetas con bordes suaves, icono, valor grande y color de estado.

Ventajas:

- Separan claramente la información.
- Permiten una jerarquía visual limpia.
- Funcionan muy bien en la Home.

Limitaciones:

- Si metes mucho texto, se rompen enseguida.
- No sirven bien para histórico largo ni para widgets complejos.

### Medidores lineales

Los medidores lineales son probablemente el patrón más rentable para este tamaño de pantalla.[web:1] Requieren poco espacio, permiten comparar varios sensores a la vez y comunican muy bien magnitudes relativas.

Se adaptan muy bien a:

- Humedad.
- Luz.
- Sonido.
- Índices compuestos.

Además, encajan con los widgets de barras de progreso de TFT_eWidget y con el ejemplo de linear analogue meter de TFT_eSPI.[web:1][web:4][web:24]

### Gauges circulares o diales

Los diales analógicos con aguja son muy atractivos visualmente y funcionan bien para magnitudes físicas como temperatura o ruido.[web:1][web:10] Son muy buenos como pantalla de detalle, especialmente cuando se acompañan de zonas de color y una lectura numérica grande.

No son la mejor opción para mostrar muchos sensores a la vez, porque consumen demasiado espacio. Por eso conviene reservarlos a pantallas dedicadas y no a la Home.

### Gráficas pequeñas o sparklines

Las pequeñas gráficas de línea son muy útiles para enseñar evolución y tendencia, algo muy valioso en un proyecto STEAM.[web:7] En pantallas pequeñas no deben intentar mostrar escalas complejas, sino una señal breve de si el valor sube, baja o se mantiene.

Lo ideal es usar una ventana corta, por ejemplo 16 a 24 muestras, y reescalar verticalmente con criterio para que el cambio se perciba.[web:7][web:4] Esto convierte la pantalla en una herramienta de observación científica, no solo de visualización instantánea.

### Iconos que se llenan o cambian

Las guías de small screens recomiendan iconos simples y semánticamente claros.[web:6][web:12] En tu caso, hay una gran oportunidad de diseño pedagógico usando iconos que se llenan, cambian de color o se activan por niveles.

Ejemplos útiles:

- Gota que se llena para humedad.
- Bombilla o sol para luz.
- Altavoz con barras para sonido.
- Hoja o maceta para suelo/cultivo.
- Termómetro con columna coloreada para temperatura.

Este tipo de recurso visual comunica muy rápido y da identidad al dispositivo.

## Recomendaciones específicas por sensor

### Temperatura ambiente y DS18B20

La temperatura es ideal para representarse como valor grande + gauge circular o valor grande + sparkline histórica.[web:1][web:7][web:10] También es muy útil codificar rangos con colores, por ejemplo azul para frío, verde para confort y rojo para calor.[web:25][web:1]

Recomendación concreta:

- Home: valor grande con icono y pequeño indicador de estado.
- Detalle: gauge de media pantalla y sparkline pequeña debajo.
- Texto breve de rango si quieres enfoque educativo: “frío”, “ok”, “calor”.

### Humedad del suelo

La humedad del suelo se beneficia mucho de metáforas gráficas de llenado o saturación.[web:6][web:12] Una gota rellena, una maceta o una barra segmentada seco→húmedo suele ser más intuitiva que un simple número.

Recomendación concreta:

- Home: porcentaje + gota rellena.
- Detalle: barra horizontal segmentada con etiquetas “seco / óptimo / húmedo”.
- Si quieres reforzar el aprendizaje, añade un color neutro cuando el valor esté dentro de rango y colores de alerta fuera de rango.

### Humedad ambiental

Comparte lógica visual con la humedad del suelo, pero conviene diferenciarla por icono y color secundario para no confundir ambas magnitudes. Una buena decisión UX es que “humedad del aire” y “humedad del suelo” nunca tengan exactamente la misma representación.

Recomendación concreta:

- Humedad aire: nube/gota.
- Humedad suelo: maceta/gota/tierra.
- Mismo patrón de lectura, iconografía distinta.

### Luz

La luz funciona muy bien con iconos de brillo progresivo y barras crecientes.[web:4][web:20] También puede representarse como “modo ambiente” con cambios de fondo sutiles, siempre que no afecten a la legibilidad.

Recomendación concreta:

- Home: icono de bombilla o sol con intensidad visual.
- Detalle: barra o gauge semicircular simple.
- Si el sensor lo permite, una mini gráfica es muy útil para ver cambios por nubes, sombras o encendido de luces.

### Sonido

El sonido pide una visualización más dinámica, cercana a un VU meter.[web:12][web:15] Una barra rápida vertical u horizontal con zonas verde‑amarillo‑rojo suele ser la mejor opción para transmitir variabilidad y umbral.

Recomendación concreta:

- Home: icono de altavoz con 3 barras simples.
- Detalle: VU bar más grande, actualizada rápido, con un valor medio o pico.
- Si quieres un enfoque educativo, puedes etiquetar niveles como “silencio”, “actividad”, “ruido alto”.

### Cronómetro

El cronómetro debe ser extremadamente claro y de alta legibilidad, porque su propósito es funcional.[web:6][web:9] Debe parecer una herramienta de experimento más que un widget secundario.

Recomendación concreta:

- Dígitos muy grandes.
- Pocos adornos.
- Opción de mostrar una variable asociada debajo, como la última temperatura leída.

## Diseño visual: estilo, color y composición

### Paleta cromática

Para un proyecto de educación ambiental tiene sentido una paleta inspirada en naturaleza: verdes, azules, turquesas y tonos tierra, reservando naranja y rojo para alertas.[web:1][web:25] Esta elección refuerza la identidad narrativa del producto y su coherencia temática.

Recomendación de sistema cromático:

- Fondo oscuro o muy neutro para maximizar contraste.
- Verde para estado correcto o saludable.
- Amarillo/naranja para atención.
- Rojo para alarma.
- Azul para frío, agua o humedad.

### Contraste y fondos

En pantallas pequeñas el contraste manda sobre la estética.[web:6][web:12] Los fondos recargados, degradados complejos o imágenes muy texturadas suelen empeorar la lectura.

La mejor práctica es usar:

- Fondo uniforme o casi uniforme.
- Contenedores o cards con contraste suficiente.
- Colores de estado concentrados en barras, iconos o bordes.

### Bordes, radios y separación

En interfaces modernas, pequeñas tarjetas con bordes suaves, espacios consistentes y divisiones limpias generan sensación de orden aunque la pantalla sea muy pequeña.[web:15] En TFT_eSPI esto se puede aproximar con rectángulos redondeados, líneas finas y cajas bien proporcionadas.

Aunque la resolución sea baja, la interfaz puede sentirse “actual” si respeta espaciados, jerarquía y consistencia visual. La modernidad aquí no depende de ornamentos, sino de estructura limpia.

## Animación y dinamismo

### Animación útil, no decorativa

La UX en dispositivos embebidos recomienda animaciones discretas y funcionales: deben indicar cambio, actividad, transición o alerta, no distraer.[web:12][web:15] Esto es especialmente importante en entornos educativos, donde demasiada animación puede competir con la interpretación del dato.

Buenas animaciones para P‑Bit:

- Aguja de gauge moviéndose suavemente.
- Barra de sonido oscilando en tiempo real.
- Icono de sensor con pequeño pulso al actualizar.
- Gráfica que desplaza su historial al entrar nueva muestra.

### Ritmo de actualización

No todos los sensores necesitan refrescar igual. Una buena UI embebida diferencia entre variables rápidas y lentas.

Propuesta orientativa:

- Sonido: refresco rápido.
- Cronómetro: por segundo o décimas si hace falta.
- Temperatura y humedad: refresco medio.
- Luz: refresco medio o según cambio.
- Histórico: refresco más lento que el valor instantáneo.

Este control del ritmo mejora percepción de rendimiento y reduce ruido visual.[web:20][web:13]

## Rendimiento y viabilidad técnica con TFT_eSPI

### Uso de sprites

Los sprites son una técnica clave para que una interfaz parezca moderna en pantallas SPI.[web:20] Dibujar todo directamente sobre el TFT puede provocar parpadeo y sensación de lentitud, mientras que componer primero en RAM y luego empujar el sprite mejora mucho el resultado visual.[web:17][web:20]

Estrategias útiles:

- Sprite completo para Home si la RAM lo permite.
- Sprite parcial para widgets animados.
- Redibujado solo de zonas cambiantes.

### Separar fondo estático y capa dinámica

Una práctica muy eficaz consiste en distinguir elementos estáticos y dinámicos.[web:13][web:20] El fondo, las etiquetas y las cajas pueden dibujarse una sola vez, mientras que agujas, barras y valores se actualizan aparte.

Esto mejora rendimiento y simplifica el código. Además, hace que la UI se vea más estable.

### Evitar solapamientos innecesarios

Las recomendaciones de Bodmer para diales indican que el barrido de la aguja no debería cruzar otras zonas con contenido que haya que restaurar constantemente.[web:13] Si una aguja pisa texto o gráficos, la lógica de redibujado se complica y el refresco empeora.

Regla práctica: cada widget animado debe tener su propio espacio limpio.

### ¿Cuándo quedarse en TFT_eSPI y cuándo mirar LVGL?

LVGL permite interfaces más complejas con temas, animaciones y widgets avanzados, y existen ejemplos con gráficas en ESP32 TFT.[web:7] Sin embargo, para pantallas pequeñas con flujo simple, muchos desarrolladores consideran que una solución ligera con TFT_eSPI resulta más controlable y eficiente.[web:3]

Para P‑Bit, salvo que quieras una interfaz muy compleja o táctil con múltiples capas, TFT_eSPI + TFT_eWidget parece la combinación más adecuada por simplicidad, rendimiento y control fino.[web:4][web:21]

## Sistema de diseño recomendado para P‑Bit

### Reglas visuales base

Propuesta de design system mínimo para el proyecto:

- 1 fondo principal.
- 1 estilo de card o contenedor.
- 1 tipografía base para etiquetas.
- 1 tipografía grande para valores.
- 1 color fijo por familia de sensor.
- 3 colores de estado: ok, atención, alerta.
- 1 estilo de icono simple y consistente.

Este enfoque hace que la interfaz sea escalable y evita que cada pantalla parezca de un proyecto distinto.[web:12][web:15]

### Convenciones sugeridas por familia de sensor

| Familia | Color principal | Icono sugerido | Widget recomendado |
|---|---|---|---|
| Temperatura | Azul / rojo / verde por rango | Termómetro | Gauge o valor + sparkline |
| Humedad aire | Azul | Nube + gota | Barra o card con llenado |
| Humedad suelo | Verde / azul tierra | Gota / maceta | Barra segmentada o icono lleno |
| Luz | Amarillo / blanco cálido | Sol / bombilla | Barra o semicírculo |
| Sonido | Morado / naranja / rojo por umbral | Altavoz | VU bar |
| Cronómetro | Blanco / cian | Tiempo / reloj | Dígitos grandes |

### Lenguaje visual pedagógico

La interfaz debería ayudar a responder preguntas educativas, no solo mostrar valores. Por ejemplo:

- “¿El aula está en confort térmico?”
- “¿La planta necesita agua?”
- “¿Hay demasiado ruido?”
- “¿Cómo cambia la luz durante el día?”

Esto implica que la UI debe favorecer lectura interpretativa y comparativa, no solo numérica.[web:12][web:15]

## Propuestas de layouts concretos

### Layout A: Home con 4 cards

Distribución:

- Arriba izquierda: temperatura.
- Arriba derecha: humedad.
- Abajo izquierda: luz.
- Abajo derecha: sonido.

Cada card incluye:

- Icono arriba izquierda.
- Valor grande en el centro.
- Unidad pequeña.
- Banda o borde de color según estado.

Este layout es ideal si quieres que la pantalla principal sea muy clara y moderna.

### Layout B: Lista de sensores con barras

Distribución vertical de 4 a 5 líneas:

- Nombre corto del sensor.
- Valor pequeño/medio.
- Barra larga de estado.

Ventajas:

- Muestra más sensores a la vez.
- Es muy eficiente en píxeles.
- Muy fácil de leer en contexto técnico.

### Layout C: Pantalla detalle narrativa

Ideal para trabajo en aula o demostración.

Elementos:

- Título del sensor.
- Valor grande.
- Widget protagonista (gauge o gráfico).
- Etiqueta interpretativa breve: “Óptimo”, “Seco”, “Ruido alto”, etc.
- Mini histórico debajo.

Este layout convierte cada sensor en una “historia visual” y encaja muy bien en educación STEAM.

## Buenas prácticas de implementación

### Texto corto siempre

Las etiquetas deben ser muy breves. En vez de “Humedad relativa del aire”, es mejor “Hum. aire” o “H. aire” si la interfaz sigue siendo comprensible. El espacio textual debe reservarse para el valor y el significado.

### Unidades visibles

No ocultes unidades. Mostrar “23.5” sin “°C” o “58” sin “%” reduce comprensión, especialmente en un contexto educativo. La unidad forma parte de la alfabetización científica del dispositivo.

### Diseñar para distancia real

La interfaz no se diseña solo para verse bien de cerca, sino para la distancia real de uso. Conviene probarla físicamente en mano y sobre mesa, con distintas condiciones de luz, porque eso afecta mucho a legibilidad y contraste.[web:6][web:12]

### Prototipar antes de programar

Antes de codificar, conviene hacer wireframes de baja fidelidad en papel o en Figma con la resolución real 160x128. Esto permite validar jerarquía visual, reparto del espacio y densidad de información sin gastar tiempo en firmware.

### Pensar en estados, no solo en pantallas

No basta con diseñar “la pantalla bonita”. También hay que pensar en:

- Estado normal.
- Sensor fuera de rango.
- Sensor desconectado.
- Valor no disponible.
- Carga / inicialización.
- Navegación entre vistas.

Una UI robusta necesita contemplar todos estos estados para no romperse en uso real.[web:12][web:15]

## Checklist de diseño para revisar cada pantalla

Antes de dar por válida una pantalla, conviene revisar:

- ¿Se entiende en menos de 2 segundos?
- ¿Hay un foco visual principal?
- ¿El valor más importante es el más grande?
- ¿Los colores tienen significado claro?
- ¿La unidad está visible?
- ¿El icono realmente ayuda?
- ¿Hay demasiado texto?
- ¿Se ve bien con iluminación distinta?
- ¿Se puede actualizar sin parpadeos?
- ¿El alumnado entendería qué está viendo sin explicación larga?

## Recomendaciones finales accionables

Las mejores decisiones para una pantalla como la tuya no pasan por copiar dashboards de móvil o de web, sino por adaptar principios modernos a un contexto de muy baja resolución.[web:9][web:12][web:15] En tu caso, la combinación más prometedora es:

- Home con cards o barras lineales.
- Pantallas de detalle con gauge o sparkline.
- Uso consistente de iconos simples.
- Código de color semántico.
- Sprites para animaciones y refresco limpio.[web:1][web:4][web:20]

La oportunidad diferencial de P‑Bit está en unir instrumentación, pedagogía y diseño visual. Si la interfaz consigue que el alumnado lea un valor, entienda su significado y vea su evolución con rapidez, el diseño habrá cumplido su función mucho mejor que una pantalla “bonita” pero confusa.[web:12][web:15]

## Próximo paso recomendado

El siguiente paso más útil es convertir este informe en un sistema visual concreto de pantallas. Para ello, conviene definir:

- Una paleta cerrada.
- Una librería de iconos de 1 bit o color plano.
- Un layout Home.
- 4 a 6 layouts de detalle por sensor.
- Un conjunto de reglas para tamaños de texto, márgenes, estados y animaciones.

Después, ese sistema ya se puede traducir a código con TFT_eSPI y TFT_eWidget de forma coherente y escalable.[web:4][web:21][web:24]
