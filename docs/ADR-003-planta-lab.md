# ADR-003 — Planta Lab: mascota multisensor

Fecha original: 2026-06-15
Revisión vigente: 2026-10-11
Estado: Aceptado; mascota implementada, ajuste visual en revisión y validación física completa pendiente.

## Decisión vigente y alcance

Planta Lab representa una planta educativa animada con rostro discreto, tallo y hojas expresivos. Suelo, temperatura, humedad del aire y luz producen reacciones distinguibles. La planta conserva su identidad verde; postura, expresión y movimiento comunican la necesidad, reforzados por acentos de color.

El usuario aprobó esta primera versión con reacciones a sensores y recuperación gradual, sin crecimiento acumulado ni progreso guardado entre reinicios. No hay nueva configuración ni gestos del encoder. El perfil sigue siendo fijo e independiente de los límites editables de cada sensor.

La mascota interpreta condiciones ambientales; no mide el crecimiento ni diagnostica la salud biológica de una planta real. Sus gestos son una metáfora educativa.

## Contexto e historial

La V1 de junio integró terrario, cuatro filas de rango y los estados `BIEN`, `SEDIENTA`, `AHOGADA` y `ESTRÉS`. El dibujo usaba un tallo recto, hojas elípticas, cambios completos de color y un balanceo similar entre condiciones. Su diagnóstico elegía el primer sensor fuera de rango por prioridad fija y trataba lecturas inválidas como necesidades negativas.

El primer ajuste del 2026-10-10 alineó Luz con el techo de lectura de 8000 lux y Suelo con la zona central inclusiva 30..70 %. Esta revisión conserva esos rangos y sustituye el contrato de dibujo, diagnóstico y animación de V1. Las coordenadas y navegación se mantienen; las antiguas reglas «tallo siempre recto», «todo en color de estado» y «sin transición ni buffer» son antecedentes, no instrucciones vigentes.

## Perfil y clasificación compartida

| Canal | Rango normal inclusivo | Escala de la fila | Fuera de rango |
|---|---|---|---|
| Suelo | 30..70 % | 0..100 % | Sed / exceso de agua |
| Temperatura | 15..30 °C | 0..40 °C | Frío / calor con intensidad gradual |
| Humedad del aire | 40..70 % | 0..100 % | Aire seco / aire húmedo |
| Luz | 200..8000 lux | Logarítmica 0..8000 lux | Poca luz |

Estos límites pertenecen al modelo de Planta Lab. La banda de Suelo estabilizada se comparte entre diagnóstico, resaltado y aparición de grietas/agua. El color café y la intensidad de los efectos se ajustan continuamente a la lectura actual dentro de la banda confirmada; no simulan secado por tiempo. Barras y valores también muestran la lectura actual. La postura y el cambio de banda esperan confirmación temporal.

30 y 70 % pertenecen a la zona normal de Suelo; 15/30 °C, 40/70 % y 200/8000 lux son inclusivos en el perfil inicial y las barras. Tras el arranque, Temperatura aplica histéresis de ±0.5 °C: desde Normal entra en frío <14.5 o calor >30.5; desde Low regresa a Normal ≥15.5; desde High, ≤29.5. Mantiene confirmación temporal de bandas. Una barra puede indicar lectura fuera/dentro del rango antes de que cambie la clasificación; no implica incoherencia ni un nuevo límite biológico. No hay exceso de luz observable por encima de 8000; una lectura fuera de 0..8000 es desconocida.

### Origen y significado de los umbrales

Suelo, Aire y Luz conservan intervalos heredados de la UI base `3744f04`. Temperatura cambia de 18..26 a 15..30 °C para una expresión más flexible, tras el reporte del usuario de cambios por un grado. Es una decisión provisional de producto apoyada en referencias de tolerancia amplia de plantas de interior, no un óptimo universal ni un umbral de daño. Fuentes y alcance en [PLANTA_LAB_RESEARCH](PLANTA_LAB_RESEARCH.md).

El porcentaje de Suelo es relativo a los dos puntos de calibración del sensor (referencia seca y húmeda). No equivale al porcentaje volumétrico de agua del sustrato; el método de calibración, sensor y material condicionan su interpretación. `AHOGADA` representa el tramo alto de esa lectura relativa, no una medición de oxígeno ni un diagnóstico real de encharcamiento.

En Luz, 8000 lux es el techo de lectura configurado tras la calibración empírica del LDR, no un nivel óptimo para todas las plantas. El firmware recorta ahí la estimación y no permite conocer cuánta luz hay por encima. El disparador bajo de 200 lux tampoco tiene una justificación por especie documentada. Ver `docs/TECHNICAL.md`, calibración LDR v1 y lectura de Suelo.

La [investigación preliminar del 2026-10-11](PLANTA_LAB_RESEARCH.md) propone revisar esa interpretación: oscuridad momentánea no demuestra falta de luz, y calor/luz/agua requieren contexto, duración y especie. La lógica actual todavía penaliza Luz por debajo de 200 lux; reposo neutral, historial y combinaciones no están implementados. Los tiempos de confirmación/recuperación siguientes son de interfaz, no evidencia fisiológica.

## Comportamiento temporal

El modelo puro está separado del renderer: `include/plant_pet.h` y `src/plant_pet.cpp`. Recibe lecturas y tiempo mediante `PlantPetModel::update()`; `snapshot()` consulta la pose interpolada sin añadir evidencia. El punto de alimentación es la copia del snapshot de UI, incluso mientras otra pantalla esté visible.

- Al arrancar, adopta la clasificación disponible de las lecturas actuales.
- Un cambio posterior de banda requiere al menos dos lecturas nuevas y 1200 ms de evidencia continua. Si entre lecturas hay más de 3000 ms, reinicia la evidencia pendiente.
- Una lectura inválida pasa inmediatamente a `Unknown`. No castiga a la mascota ni mantiene una necesidad inventada.
- Se conservan todas las necesidades conocidas en `affected_mask`; las filas afectadas tienen borde de alerta y la causa dominante un doble borde.
- La causa dominante se elige por desviación relativa respecto a la zona normal, acotada a 0..1. Luz usa `log1p`, acorde con su escala. No se elige siempre Suelo por una prioridad fija.
- Una causa que sigue activa se retiene durante 3 s. Después, otra la reemplaza si supera su gravedad en más de 0.12. Si la causa deja de ser adversa, termina esa retención. Los empates conservan la causa vigente; el orden de canales solo desempata al iniciar.
- La transición entre poses dura 1.5 s; la recuperación de una necesidad a bienestar dura 3 s. La interpolación es visual y no representa crecimiento.

Las instancias de datos reales y de Demo son independientes. Los valores sintéticos no alteran la recuperación de la mascota real, NVS ni las lecturas físicas. Planta Lab sigue fuera del recorrido automático `kDemoScenes`; separar instancias no significa añadir una escena nueva.

### Intensidad térmica independiente del efecto ambiental

- Temperatura Normal permite Content si los demás canales son normales y conocidos. Calor suave puede mostrar ondas manteniendo sonrisa. Suelo/Aire/Luz conservan sus reglas; no garantiza BIEN si otro canal necesita atención.
- `thermal_intensity` escala la pose de Content a Hot de 30 a 36 °C, o a Cold de 15 a 8 °C; saturación 0..255 y rampa de 1500 ms al cambiar objetivo. Repetir una lectura no reinicia la rampa, y `snapshot()` no cuenta como evidencia nueva. Inválidas anulan la intensidad actual.
- `previous_thermal_intensity` conserva el nivel al cambiar de mood para interpolar la recuperación/pose anterior, sin volver bruscamente a una reacción completa. Sonrisa bajo 85/255, cara tranquila neutral hasta 170/255 y gesto térmico completo desde ahí; pasos faciales ilustrativos, no estadios de estrés biológico.
- Ondas magenta 1/2/3/4 desde 27/30/33/36 °C, independientes del mood; la intensidad de postura y la cantidad de ondas expresan cosas distintas. El color de hojas/barras aprobado se conserva. Las ondas se reparten entre ocho posiciones laterales del terrario, suben hasta 3 px, se pausan y reaparecen de forma escalonada; variación pseudoaleatoria determinista por ciclo de 12 ticks (2.64 s), sin saltos aleatorios en cada frame ni estado RNG. Lecturas y rostro quedan despejados.
- `millis()` permite duración dentro de la sesión sin hora del día. El modelo solo usa esos tiempos para estabilizar y animar; historial de luz, persistencia de humedad, combinaciones de calor/luz/agua y compensación por calor propio no están implementados.

## Sensor → gesto

| Condición | Expresión y movimiento | Canal de entorno |
|---|---|---|
| Condiciones conocidas normales | Rostro tranquilo, hojas abiertas, balanceo leve y parpadeo | Tierra central e iluminación normal |
| Suelo seco | Hojas y tallo decaídos, expresión de sed | Tierra seca/agrietada |
| Suelo húmedo en exceso | Hojas pesadas y gesto agobiado | Tierra oscura, charco/gotas |
| Frío | Hojas recogidas y ráfagas de temblor | Copos cyan 1..3 alternados, con descenso corto y deriva lateral |
| Calor | Postura agobiada, boca pequeña abierta y hoja que abanica | Ondas magenta, más intensas con calor |
| Aire seco | Hojas estrechas y ojos cansados | Trazos violetas quebrados en 1..2 grupos alternados y con deriva suave |
| Aire húmedo | Hojas a media altura y expresión propia | Niebla cyan suspendida en 1..2 grupos alternados y con deriva suave, sin alcanzar la tierra |
| Poca luz | Tallo estirado, mirada hacia arriba y movimiento mínimo | Luna circular <20 lux; desde 20 lux sol de tamaño/brillo creciente, sin confirmar noche |
| Datos incompletos sin necesidades conocidas | Expresión neutral | Fila desconocida atenuada y valor `--` |

La silueta usa una corona en forma de hoja con ojos y boca, dos hojas grandes y dos pequeñas conectadas a un tallo curvo. La causa dominante elige el gesto principal; las demás necesidades se mantienen visibles en sus filas y aportan efectos ambientales secundarios. La paleta de hojas es verde persistente en todas las poses, incluso Unknown. Suelo café y sol amarillo mantienen su identidad natural; calor, frío y aire usan magenta, azul/cyan y violeta.

## Textos visibles e i18n

| Estado | Español | Català | English |
|---|---|---|---|
| Bienestar | `BIEN` | `BÉ` | `OK` |
| Sed | `SEDIENTA` | `ASSEDEGADA` | `THIRSTY` |
| Exceso de agua | `AHOGADA` | `OFEGADA` | `SOGGY` |
| Frío | `FRÍO` | `FRED` | `COLD` |
| Calor | `CALOR` | `CALOR` | `HOT` |
| Aire seco | `AIRE SECO` | `AIRE SEC` | `DRY AIR` |
| Aire húmedo | `AIRE HÚMEDO` | `AIRE HUMIT` | `HUMID AIR` |
| Poca luz | `POCA LUZ` | `POCA LLUM` | `LOW LIGHT` |
| Lecturas incompletas | `FALTAN DATOS` | `FALTEN DADES` | `CHECK DATA` |
| Recuperación | `MEJORANDO` | `MILLORANT` | `RECOVERING` |

Sin necesidades conocidas, algún canal desconocido impide mostrar `BIEN`: se muestra `FALTAN DATOS`. Si hay una necesidad conocida y otra lectura desconocida, se muestra la necesidad; la otra fila permanece atenuada. Los textos usan `L(KEY)` y no cambian los nombres del carrusel: ES/CAT `PLANTA LAB`, EN `PLANT LAB`.

## Ficha de implementación TFT

### Layout elegido: terrario + cuatro filas

Pantalla ST7735 160×128, landscape. El header y el carrusel existente se conservan.

| Zona | X | Y | W | H | Capa |
|---|---|---|---|---|---|
| Header/divisor | — | Base 18 / línea 23 | — | — | Shell |
| Terrario exterior | 2 | 27 | 56 | 99 | Shell |
| Buffer interior de mascota y entorno | 3 | 28 | 54 | 95 | Dinámica |
| Fila Suelo | 62 | 27 | 96 | 17 | Shell + campos |
| Fila Temperatura | 62 | 46 | 96 | 17 | Shell + campos |
| Fila Aire | 62 | 65 | 96 | 17 | Shell + campos |
| Fila Luz | 62 | 84 | 96 | 17 | Shell + campos |
| Card de diagnóstico | 62 | 104 | 96 | 22 | Shell + texto/borde |

Las filas usan iconos locales de aproximadamente 9×10 px centrados en x68 dentro de un carril de 11 px, barra x77 de 48×7 px y dato blanco alineado en x156 dentro de un carril de 30 px. Cada sensor conserva su color: Suelo verde, temperatura magenta, aire cyan y Luz violeta; el tramo normal es luminoso y los extremos son versiones oscurecidas del mismo color, con marcador blanco. Una lectura ausente usa estado neutro. Temperatura se presenta sin decimal y con unidad °C/°F explícita; el grado se dibuja como glifo propio 3×3 dentro de una celda de Font 1 para evitar CP437; el diagnóstico y la posición de la barra usan °C aunque el dato visible esté en °F. La barra de Luz es logarítmica y su extremo representa 8000 lux. Las zonas comparten la cuantización de píxel del marcador; los extremos normales están incluidos en el relleno de la zona central.

Luz menor de 1000 lux se muestra completa (`999lx`); desde 1000 se compacta como `1K`, `1.2K` o `8K`, redondeado a una décima. `K` significa mil lux. El formato visible no modifica la lectura, el modelo ni la escala de la barra.

### Paleta e iconos

La [comparativa de color del 2026-10-11](PLANTA_LAB_COLOR_REVIEW.md) conserva las propuestas históricas A/B/C. El usuario eligió una adaptación: barras B con identidad por sensor y extremos oscurecidos, planta verde A, tierra café progresiva y sol amarillo. Esta combinación ya está implementada; la validación física sigue pendiente.

La mascota tiene una paleta de ilustración propia en `src/plant_pet_art.cpp`, distinta de los tokens de identidad de sensores. Verde compartido `PLANT_PET_GREEN=0x374B` en `include/plant_pet_art.h`; sombra verde oscura, luz menta y brillo verde pálido iguales en todos los estados. El gesto y el entorno comunican las condiciones, sin cambiar hojas a amarillo/naranja. Suelo café claro → café medio → café oscuro con la lectura relativa 0..100 %; grietas progresivas por debajo de 30 % y pequeñas manchas de agua que se amplían por encima de 70 %, dentro de las bandas confirmadas. Ausencia/no finito produce tierra café neutra sin grietas/charcos. Esta ilustración no mide volumen de agua ni diagnostica drenaje/oxígeno. Tokens completos en `docs/DESIGN_SYSTEM.md`.

Los cuatro iconos de fila son dibujos locales compactos de Suelo/Temperatura/Humedad/Luz en `src/ui_lab_plant.cpp`, con sus identidades de sensor. La API global de `src/ui_icons.cpp` y los iconos de otras vistas no cambian. La mascota sigue siendo un personaje distinto del icono técnico de Suelo.

El diagnóstico usa `FONT_BODY` (Roboto Regular 7), con fallback a `FONT_SMALL` cuando `textWidth()` supera 88 px (ambas Roboto con tildes), alineación `C_BASELINE` y baseline y120. Los acentos de `FRÍO`/`AIRE HÚMEDO` y los textos CAT no pasan por CP437. Solo se limpia el interior de la card cuando cambia el estado visible.

### Shell y campos dinámicos

- **Shell, al entrar:** header, divisor, borde del terrario, fondos/bordes de filas e iconos. Si cambia la validez de una lectura, su icono se refresca localmente con la identidad normal o atenuada.
- **Dinámica:** interior completo del terrario compuesto en RAM, barras/valores, bordes de necesidades y texto/borde de diagnóstico.
- El interior se dibuja en un buffer fijo RGB565 de 54×95: 10 260 bytes de RAM estática, reutilizados. No se reserva heap por frame.
- Cada frame se compone completo en RAM y llega a la TFT mediante un único `pushImage()`, sin borrado previo del panel. Se guarda/restaura el estado de intercambio de bytes.
- El buffer queda dentro del terrario, sin pisar header ni filas. Tras rasterizar y antes del envío, se reponen los seis píxeles de máscara de las esquinas superiores para conservar el borde redondeado r4.
- El renderer mantiene caché de lecturas visibles, clasificación, causa, clave de animación y valores de entorno (`soil_x10/temp_x10/hum_x10/lux`). Cambiar una lectura dentro de la misma banda, del mismo dígito visible o del mismo tick permite actualizar el terrario; los campos derechos solo se actualizan si cambia lo visible. No se borra la pantalla por tick.
- `include/plant_pet_art.h` / `src/plant_pet_art.cpp` componen mediante `plant_pet_render(..., readings)`, con lecturas opcionales para detalle continuo (sin ellas usa valores representativos por banda), sin dependencia de TFT ni heap. Reloj de animación de 220 ms, con pausas por gesto; el parpadeo ocupa 2 de 29 pasos (~440 ms cada 6.38 s). Geometría interpolada y expresión cambiada a media transición.
- El tiempo de la animación y el refresco son compatibles: el paso observado no depende de una fase más rápida que la llamada del router.

Reglas canónicas de render y prueba: `docs/TFT_RENDER_RULES.md`. La composición en RAM reduce comandos y elimina el borrado visible previo; la ausencia de parpadeo necesita comprobación en TFT real.

## Navegación e integración

`Planta Lab` se mantiene entre `Clima Lab` y `Termo Lab`. Solo aparece en el carrusel si Suelo entrega lectura válida; sin Suelo se omite. Es una pantalla de solo lectura, sin menú ni submodos.

`LAB_PLANT_SCREEN` se enruta en `src/tft_display.cpp` a `draw_lab_plant_screen()`. La actualización del modelo ocurre fuera del draw; volver a esta vista no reinicia la recuperación. El renderer no dibuja el badge global `L_ALERT_JEWEL`, que invadiría el terrario.

## Verificación y pruebas pendientes

2026-10-12: el usuario comunica que la versión cargada funciona muy bien y pide conservar el diseño actual. Se da por cerrado provisionalmente el ajuste visual de producto; esta observación general no verifica individualmente todas las poses, idiomas, recuperación, flicker ni Demo, ni cierra la validación completa por unidad/24 h.

Próximo paso recomendado, aún no ejecutado ni aprobado como implementación: revisión acotada de comportamiento y contrato de perfiles antes de diseñar el setup. Alcance: `plant_pet.cpp`, interfaz `plant_pet.h`, integración de snapshot/modelo en Planta Lab y ADR-003/investigación; revisar el renderer solo si aparece un defecto funcional concreto. Definir reposo con poca luz, observaciones frente a diagnóstico, respuesta de Suelo/aire, datos inválidos/techo LDR, duración válida de sesión y prioridades. Salida esperada: hallazgos priorizados y contrato de configuración mínimo; no auditoría global, rediseño ni implementación de historial/presets por inferencia.

El usuario aprobó en P-Bit colores actuales de planta/medidores y valoró favorablemente el suelo. Esta aceptación puntual no cierra poses, idiomas, recuperación, flicker ni Demo. Sol/luna, ondas y flexibilidad térmica ya están implementados y comprobados por software; validar el último ajuste en TFT antes del setup. Humedad ambiental flexible, reposo neutral, historial y combinaciones siguen propuestos.

El 2026-10-11 el usuario comunicó que probó la mascota en el P-Bit y le gustó la primera idea. Es una primera impresión física favorable, no el cierre de todas las poses, idiomas, transiciones ni flicker. Primero se cierran los ajustes visuales; después se discutirá configuración/presets.

La aprobación del alcance y la implementación software se distinguen de la aprobación física completa. La evidencia de pruebas nativas, preview y build final se registra en CHANGELOG; no equivale a flasheo ni validación de unidades.

- [x] Alcance aprobado: mascota verde con rostro, reacciones por sensor y recuperación sin persistencia.
- [x] Rangos compartidos y conservación de carrusel/visibilidad condicional.
- [x] Contrato de modelo independiente de render, con desconocidos y estabilización temporal.
- [x] Pruebas nativas: Modelo nativo, incluidos casos térmicos nuevos, 1215 combinaciones de pose/fase/transición, 101 niveles de Suelo y 4 casos ausentes/no finitos, PASS; host `-Wall -Wextra -Werror` limpio. Fuentes reproducibles: `test/plant_pet_native.cpp` y `test/plant_pet_art_native.cpp`.
- [x] Preview software actual: nueve estados, once niveles de suelo, quince temperaturas, diez niveles/límites de símbolos de Luz y GIF de 64 frames desde código real; 138 comparaciones incremental/completo PASS. Artefactos en `.pio/plant_pet_review/effects/`, sin versionar. Incluye 48 frames adicionales de calor, galería solar con pantalla completa y GIF/galería de posiciones; usuario acepta el preview general de sol/luna y calor distribuido y pide extender los efectos a frío/aire, ahora integrados según el preview autorizado. Galerías térmica/solar revisadas por el agente; no equivale a observación TFT.
- [x] Compilación final `esp32dev` SUCCESS: RAM 59 580 bytes (18.2 %), Flash 971 189 bytes (74.1 %). Árbol base `3744f04` + mascota/diseño local, sin commit; evidencia en CHANGELOG/PRODUCTION_RELEASE.
- [ ] **Proof 9:** observar Demo en dispositivo durante al menos 30 s; Planta Lab no se incluye automáticamente.
- [ ] **Proof 10:** observar la mascota en TFT real: poses, ritmo, transiciones, bordes/valores, ausencia de parpadeo y textos ES/CAT/EN.
- [ ] Validar Luz 8000 en extremo de barra y coherencia de tierra/diagnóstico a ambos lados de 30 y 70 %, incluyendo el retardo de confirmación.
- [ ] Validar necesidades simultáneas, lectura desconocida y recuperación al cambiar a otra vista y regresar.

Prueba final de 24 h y validación física completa por unidad: abiertas y aplazadas por decisión del usuario.

## Efectos ambientales distribuidos — integrados 2026-10-11

Estado: implementación autorizada por el usuario después del preview. El usuario acepta el preview general de sol/luna y calor distribuido, y solicita extender distribución/movimiento a frío y posiblemente humedad/sequedad del aire. Esa aceptación de preview no cierra la prueba TFT.

- Frío: 1..3 copos pequeños cyan/azul claro según la intensidad térmica ya suavizada (1 + intensidad/86, máximo 3) y la banda fría confirmada, independientemente de la necesidad dominante. Alternar posiciones laterales y alturas, descenso corto y deriva lateral de 1 px; ciclo de 18 ticks (3.96 s), pausa de 4 ticks; aumentar cantidad con intensidad, sin tormenta. Copos como pictograma de frío, no afirmación de nieve/helada.
- Aire húmedo: 1..2 grupos de trazos/motas cyan suspendidos, con deriva lateral lenta y apariciones alternadas a ambos lados. Mantenerlos separados del suelo; no gotas de lluvia ni charcos ambientales. Detalle crece dentro de la banda alta confirmada.
- Aire seco: conservar trazos cortos/discontinuos violetas, añadir desplazamiento leve y pausas largas en posiciones alternadas. 1..2 grupos, menos densos que los copos. Evitar polvo/hojas cayendo y grietas fuera del suelo. El movimiento no representa una medición de viento.
- Composición: máximo cuatro grupos ambientales visibles a la vez entre temperatura y aire; reservar uno para aire cuando ambos requieren señal. Posiciones seguras dentro del terrario, sin rostro, sol/luna, suelo, marcador de desconocidos ni lecturas. Variación determinista entre apariciones, sin random por frame, manteniendo la cadencia de 220 ms y el buffer fijo. Ritmos distintos: calor asciende, frío desciende, aire deriva lateralmente.
- Mantener paleta, terreno, expresiones y rangos actuales. Son decisiones visuales de producto, no nuevos umbrales agronómicos. Preview de tres intensidades de frío y aire, más combinaciones frío+aire húmedo y calor+aire seco autorizado e integrado. Usuario comunica funcionamiento general favorable en TFT el 2026-10-12; comprobaciones exhaustivas siguen abiertas.

Preview preparado desde una copia aislada de `plant_pet_art.cpp` en `.pio/plant_pet_review/ambient_proposal/`: nueve escenas de intensidad y dos combinaciones, con 48 frames por escena. Galerías PNG y tres GIF desde el layout/fuentes reales 160×128. Host C++11 `-Wall -Wextra -Werror` limpio; 528 comparaciones incremental/completo PASS. Ese preview precedió la autorización e integración en `src/plant_pet_art.cpp`. Ahora: 528 comparaciones host PASS y 528 frames idénticos al preview; raster 1215 casos +101 niveles de Suelo y 4 ausentes/no finitos PASS. Build ESP32 SUCCESS (20.103 s), RAM 59580, Flash 971189. Carga posterior autorizada en COM8 SUCCESS (29.848 s), hashes verificados y reinicio RTS enviado; observación general favorable del usuario el 2026-10-12; comprobaciones visuales específicas pendientes.

## Futuro, fuera de esta revisión

### Configuración de Planta Lab — propuesta posterior al cierre visual

Esta propuesta no está implementada ni añade configuración en esta revisión. La [investigación para perfiles de aula](PLANTA_LAB_RESEARCH.md) recomienda comenzar a validar judía enana, rábano y albahaca en sustrato. Antes del setup se deben acordar observaciones, reposo y avisos, además de las reglas de duración/cobertura. Un preset no se reduce necesariamente a cuatro intervalos independientes.

- Menú raíz con grid 2×3: `Preset` y `Personalizar` como opciones primarias; `Reset` abajo izquierda y `Salir` abajo derecha. Mantener hasta cuatro primarias y el contrato encoder-grid.
- `Personalizar` ofrecería los cuatro sensores; el flujo de mínimo/máximo es un borrador que deberá revisarse con la semántica de luz, historial y combinaciones. Los ajustes pertenecerían al perfil de Planta Lab, distinguiéndolos de los límites de alertas de los sensores y de la calibración RAW de Suelo.
- Los presets deberán declarar alcance, fuentes y método de calibración compatible. No traducir un porcentaje agronómico de humedad directamente a la escala relativa del P-Bit ni proponer rangos de Luz que el techo de 8000 impida observar.
- Persistencia del perfil en NVS, significado de `Reset`, fuentes por especie y validaciones de límites se diseñarán después. Guardar un perfil es independiente del crecimiento persistente, que continúa fuera de alcance.
- Personajes por especie y fichas/presets versionados en la web son ideas futuras; no hay catálogo ni importación implementados.



- Crecimiento acumulado, brotes y etapas guardadas entre reinicios.
- Perfiles de planta y límites de Planta Lab configurables.
- Misiones de cuidado, historial de condiciones y comparación entre experimentos.
- Inclusión de Planta Lab en una coreografía Demo temática.
- Integración con aplicaciones.

Cualquier progreso persistente deberá tener reglas propias de tiempo, reinicio y recuperación; no se infiere de las animaciones de esta versión.
