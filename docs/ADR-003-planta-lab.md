# ADR-003 — Planta Lab: mascota multisensor

Fecha original: 2026-06-15
Revisión vigente: 2026-10-12
Estado: diseño conservado; comportamiento y configuración General/Personalizado V1 implementados en software. Validación física específica del nuevo candidato pendiente.

## Decisión vigente y alcance

Planta Lab representa una planta educativa animada con rostro discreto, tallo y hojas expresivos. El diseño aprobado se conserva: planta verde, barras por sensor, tierra café progresiva y efectos ambientales distribuidos. La mascota describe las condiciones medidas; no diagnostica salud, crecimiento ni daño biológico.

La V1 de comportamiento/configuración autorizada el 2026-10-12 incorpora Luz como contexto de reposo, nueva evidencia por adquisición y caducidad por canal, intensidad gradual de Suelo/Temperatura/Aire y continuidad al interrumpir recuperaciones. General/Personalizado permite editar seis referencias educativas con guardado confirmado y Reset exclusivo. No añade crecimiento persistente, historial diario, RTC, presets por especie ni consejos por exposición sostenida.


## Contexto e historial

La V1 de junio integró terrario, cuatro filas de rango y los estados `BIEN`, `SEDIENTA`, `AHOGADA` y `ESTRÉS`. El dibujo usaba un tallo recto, hojas elípticas, cambios completos de color y un balanceo similar entre condiciones. Su diagnóstico elegía el primer sensor fuera de rango por prioridad fija y trataba lecturas inválidas como necesidades negativas.

El primer ajuste del 2026-10-10 alineó Luz con el techo de lectura de 8000 lux y Suelo con la zona central inclusiva 30..70 %. Esta revisión conserva esos rangos y sustituye el contrato de dibujo, diagnóstico y animación de V1. Las coordenadas y navegación se mantienen; las antiguas reglas «tallo siempre recto», «todo en color de estado» y «sin transición ni buffer» son antecedentes, no instrucciones vigentes.

## Perfil y clasificación compartida

| Canal | Referencia General inclusiva | Escala de la fila | Interpretación |
|---|---|---|---|
| Suelo | 30..70 % | 0..100 % | SUELO SECO / SUELO HÚMEDO; gesto gradual |
| Temperatura | 15..30 °C | 0..40 °C en General | FRÍO / CALOR; gesto gradual |
| Aire | 40..70 % | 0..100 % | AIRE SECO / AIRE HÚMEDO; gesto gradual |
| Luz | 200..8000 lux contextual | Logarítmica 0..8000 lux | Bajo referencia permite REPOSO; sin atención adversa |

`PlantPetProfile` versión 1 contiene únicamente referencias baja/alta de Suelo, Temperatura y Aire. Personalizado sustituye esas referencias; escala gráfica, dominio de validez instrumental y anclajes ambientales siguen separados. La barra de temperatura usa 0..40 °C en General y extiende su mínimo/máximo gráfico si el personalizado necesita mostrar referencias fuera de ese tramo; no recorta la medición ni cambia amplitudes de gesto. Barras y modelo usan la misma revisión activa. Validación: finitos, dominio 0..100 % en Suelo/Aire o -20..80 °C en temperatura, separación mínima 5 puntos porcentuales / 2 °C. Guardar °C incluso si se edita en °F. Estas restricciones son de software, no recomendaciones de cultivo.

Los extremos de General y Personalizado son inclusivos. Tras el arranque, temperatura conserva histéresis ±0.5 °C alrededor de sus referencias activas; confirmar una banda requiere nueva evidencia. La barra muestra el valor actual, mientras la clasificación se estabiliza. Suelo comparte banda/referencia entre gesto, resaltado y aparición progresiva de grietas/agua; el tono café sigue el porcentaje relativo actual. No simula secado autónomo.

Luz bajo 200 lux se conserva en `reference_mask` y `low_light`, pero nunca en `attention_mask/affected_mask` ni como causa dominante. Con datos conocidos y sin otra atención se muestra REPOSO; una necesidad de Suelo/Temperatura/Aire conserva su gesto aunque esté oscuro. El disco de luna <20 lux es ilustrativo, sin detectar noche. Fuera de 0..8000, la lectura de Luz es desconocida.

### Origen y significado de las referencias

Suelo/Aire y la referencia contextual de Luz proceden de la UI base `3744f04`, sin especie documentada. Temperatura 15..30 °C, histéresis y amplitudes visuales son decisiones provisionales de mascota para una expresión flexible. No son óptimos universales, umbrales de daño ni presets validados. Fuentes y límites de interpretación en [PLANTA_LAB_RESEARCH](PLANTA_LAB_RESEARCH.md).

Suelo depende de dos referencias RAW de calibración; no equivale a humedad volumétrica ni mide oxígeno/drenaje. Se sustituyen los textos diagnósticos antiguos por SUELO SECO/HÚMEDO. En un Personalizado, cambiar las referencias RAW solicita revisar Suelo: se conserva su porcentaje blanco, pero la referencia queda neutral y no participa en el gesto hasta reconfirmar el perfil.

8000 lux es el techo de estimación del LDR; no máximo saludable ni prueba de exposición solar directa. Una lectura baja instantánea no determina carencia diaria. El reposo contextual está implementado; historial/cobertura, combinaciones sostenidas y corrección por calor propio permanecen fuera de V1.


## Comportamiento temporal

El modelo portable `plant_pet.h/.cpp` recibe valores, validez, identidad y fecha por canal. `update()` evalúa adquisiciones y frescura fuera del renderer; `snapshot()` consulta animación sin añadir evidencia.

- Arranque adopta datos conocidos/frescos. Una nueva banda posterior exige ≥2 muestras distintas que abarquen 1200 ms; separación entre adquisiciones >3000 ms reinicia candidatos. No basta copiar el mismo valor UI; una adquisición idéntica sí es nueva evidencia.
- `ReadingSampleStamp` avanza secuencia/fecha solo al medir válidamente. DHT retenido ante error/TOO_SOON mantiene ambas; al pasar a NAN su calidad Invalid se atiende inmediatamente.
- Caducidad local: Suelo/Luz >1000 ms, Temperatura/Aire >3000 ms desde último éxito. Desconocidos no generan necesidades ni efectos inventados. No se altera la política global de alertas/lecturas; estos tiempos no demuestran estrés.
- Atención solo para Suelo/Temperatura/Aire. Se conservan sus desviaciones simultáneas; borde por fila y doble borde para dominante. Selección por intensidad de representación 0..1, retención 3 s mientras siga activa y ventaja >0.12 para reemplazarla después. No compara gravedad fisiológica.
- Transición de pose 1.5 s; recuperación a Content o reposo 3 s. Nueve pesos normalizados (suma 65535) describen la mezcla efectiva; una nueva transición congela esa mezcla como origen. Interrumpir la recuperación conserva continuidad.
- Suelo/Temperatura/Aire tienen intensidad relativa 0..255 suavizada 1500 ms. Amplitudes V1 positivas desde referencia baja/alta: Suelo 30/30 puntos, temperatura 7/6 °C y Aire 40/30 puntos. Evitan denominadores nulos/invertidos al editar referencias; son amplitudes de gesto.
- Cambiar perfil borra candidatos/causa pendiente, incrementa revisión e invalida barras; recalcula con lecturas frescas sin fingir MEJORANDO. Personalizado de Suelo pendiente de revisión no participa en la expresión.

Instancias real/Demo separadas. Demo usa General y nunca guarda perfil; valores sintéticos no alteran la mascota real, NVS ni sensores. Planta Lab permanece fuera de `kDemoScenes`.

### Gesto relativo y efectos ambientales

El gesto corresponde a las referencias activas; el decorado describe el ambiente medido con anclajes absolutos conservados. Frío bajo 15 °C y progresión hacia 8; ondas 1/2/3/4 desde 27/30/33/36 °C; Aire seco <40 % o húmedo >70 %. Un personalizado puede mostrar una sonrisa junto a un pictograma ambiental sin contradicción: representan capas diferentes.

Copos/ondas/aire alternan posiciones seguras dentro del terrario, con recorridos suaves y máximo cuatro grupos conjuntos. Calor asciende, frío desciende y aire deriva lateralmente. Ritmos/paleta aprobados se conservan; no hay RNG por frame ni heap. Tierra café mantiene progresión según lectura y referencias activas de Suelo.

`millis()` permite medir duración durante la sesión; la V1 lo usa para frescura, confirmación y animación. No aporta hora del día, cobertura diaria, duración agronómica ni observación durante apagados.


## Sensor → gesto

| Condición | Expresión y movimiento | Canal de entorno |
|---|---|---|
| Condiciones conocidas normales | Rostro tranquilo, hojas abiertas, balanceo leve y parpadeo | Tierra central e iluminación normal |
| Suelo bajo referencia | Hojas y tallo progresivamente decaídos; gesto gradual | Tierra seca/agrietada |
| Suelo sobre referencia | Hojas progresivamente pesadas; gesto gradual | Tierra oscura, charco/gotas |
| Frío | Hojas recogidas y ráfagas de temblor | Copos cyan 1..3 alternados, con descenso corto y deriva lateral |
| Calor | Postura agobiada, boca pequeña abierta y hoja que abanica | Ondas magenta, más intensas con calor |
| Aire seco | Hojas estrechas y ojos cansados | Trazos violetas quebrados en 1..2 grupos alternados y con deriva suave |
| Aire húmedo | Hojas a media altura y expresión propia | Niebla cyan suspendida en 1..2 grupos alternados y con deriva suave, sin alcanzar la tierra |
| Reposo por luz baja | Gesto tranquilo; no anula atención de Suelo/Temperatura/Aire | Luna circular <20 lux; desde 20 lux sol de tamaño/brillo creciente, sin confirmar noche |
| Datos incompletos sin necesidades conocidas | Expresión neutral | Fila desconocida atenuada y valor `--` |

La silueta usa una corona en forma de hoja con ojos y boca, dos hojas grandes y dos pequeñas conectadas a un tallo curvo. La causa dominante elige el gesto principal; las demás necesidades se mantienen visibles en sus filas y aportan efectos ambientales secundarios. La paleta de hojas es verde persistente en todas las poses, incluso Unknown. Suelo café y sol amarillo mantienen su identidad natural; calor, frío y aire usan magenta, azul/cyan y violeta.


## Textos visibles e i18n

| Estado | Español | Català | English |
|---|---|---|---|
| Bienestar | BIEN | BÉ | OK |
| Suelo bajo referencia | SUELO SECO | SÒL SEC | DRY SOIL |
| Suelo alto | SUELO HÚMEDO | SÒL HUMIT | WET SOIL |
| Frío | FRÍO | FRED | COLD |
| Calor | CALOR | CALOR | HOT |
| Aire seco | AIRE SECO | AIRE SEC | DRY AIR |
| Aire húmedo | AIRE HÚMEDO | AIRE HUMIT | HUMID AIR |
| Luz baja contextual | REPOSO | REPÒS | RESTING |
| Datos incompletos | FALTAN DATOS | FALTEN DADES | CHECK DATA |
| Recuperación | MEJORANDO | MILLORANT | RECOVERING |
| Calibración personalizada cambiada | REVISAR SUELO | REVISAR SÒL | CHECK SOIL |

Sin atención conocida, datos desconocidos impiden BIEN/REPOSO. Una atención conocida sigue visible si otro canal es desconocido. La revisión de Suelo mantiene el valor numérico actual pero neutraliza su referencia. Todas las cadenas usan `L(KEY)`; los nombres del carrusel permanecen ES/CAT PLANTA LAB y EN PLANT LAB. Perfil/Ajustar/Cancelar y los valores General/Personalizado también están localizados.


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

La [comparativa de color del 2026-10-11](PLANTA_LAB_COLOR_REVIEW.md) conserva las propuestas históricas A/B/C. El usuario eligió una adaptación: barras B con identidad por sensor y extremos oscurecidos, planta verde A, tierra café progresiva y sol amarillo. Esta combinación fue aceptada provisionalmente por el usuario en P-Bit; la revisión V1 requiere sus propias comprobaciones físicas.

La mascota tiene una paleta de ilustración propia en `src/plant_pet_art.cpp`, distinta de los tokens de identidad de sensores. Verde compartido `PLANT_PET_GREEN=0x374B` en `include/plant_pet_art.h`; sombra verde oscura, luz menta y brillo verde pálido iguales en todos los estados. El gesto y el entorno comunican las condiciones, sin cambiar hojas a amarillo/naranja. Suelo café claro → café medio → café oscuro con la lectura relativa 0..100 %; grietas/agua progresivas respecto a la referencia baja/alta activa, dentro de las bandas confirmadas. Ausencia/no finito produce tierra café neutra sin grietas/charcos. Esta ilustración no mide volumen de agua ni diagnostica drenaje/oxígeno. Tokens completos en `docs/DESIGN_SYSTEM.md`.

Los cuatro iconos de fila son dibujos locales compactos de Suelo/Temperatura/Humedad/Luz en `src/ui_lab_plant.cpp`, con sus identidades de sensor. La API global de `src/ui_icons.cpp` y los iconos de otras vistas no cambian. La mascota sigue siendo un personaje distinto del icono técnico de Suelo.

El diagnóstico usa `FONT_BODY` (Roboto Regular 7), con fallback a `FONT_SMALL` cuando `textWidth()` supera 88 px (ambas Roboto con tildes), alineación `C_BASELINE` y baseline y120. Los acentos de `FRÍO`/`AIRE HÚMEDO` y los textos CAT no pasan por CP437. Solo se limpia el interior de la card cuando cambia el estado visible.


### Shell y campos dinámicos

- **Shell, al entrar:** header, divisor, borde del terrario, fondos/bordes de filas e iconos. Si cambia la validez de una lectura, su icono se refresca localmente con la identidad normal o atenuada.
- **Dinámica:** interior completo del terrario compuesto en RAM, barras/valores, bordes de necesidades y texto/borde de diagnóstico.
- El interior se dibuja en un buffer fijo RGB565 de 54×95: 10 260 bytes de RAM estática, reutilizados. No se reserva heap por frame.
- Cada frame se compone completo en RAM y llega a la TFT mediante un único `pushImage()`, sin borrado previo del panel. Se guarda/restaura el estado de intercambio de bytes.
- El buffer queda dentro del terrario, sin pisar header ni filas. Tras rasterizar y antes del envío, se reponen los seis píxeles de máscara de las esquinas superiores para conservar el borde redondeado r4.
- El renderer mantiene caché de lecturas visibles, clasificación, causa, clave de animación, revisión de perfil/calibración y valores de entorno (`soil_x10/temp_x10/hum_x10/lux`). Cambiar una lectura dentro de la misma banda, del mismo dígito visible o del mismo tick permite actualizar el terrario; los campos derechos solo se actualizan si cambia lo visible. No se borra la pantalla por tick.
- `include/plant_pet_art.h` / `src/plant_pet_art.cpp` componen mediante `plant_pet_render(..., readings)`, con lecturas opcionales para detalle continuo (sin ellas usa valores representativos por banda), sin dependencia de TFT ni heap. Reloj de animación de 220 ms, con pausas por gesto; el parpadeo ocupa 2 de 29 pasos (~440 ms cada 6.38 s). Geometría compuesta con pesos efectivos de pose e intensidad gradual, conservando continuidad de recuperaciones interrumpidas.
- El tiempo de la animación y el refresco son compatibles: el paso observado no depende de una fase más rápida que la llamada del router.

Reglas canónicas de render y prueba: `docs/TFT_RENDER_RULES.md`. La composición en RAM reduce comandos y elimina el borrado visible previo; la ausencia de parpadeo necesita comprobación en TFT real.


## Navegación e integración

Planta Lab permanece entre Clima Lab y Termo Lab y se omite si Suelo no entrega lectura válida. `LAB_PLANT_SCREEN` enruta vista y configuración propia; el modelo se alimenta fuera del dibujo también en otras vistas. Volver no reinicia su recuperación. No se dibuja el badge global que invade el terrario.

Pulsación larga de 1200 ms abre configuración; dentro cancela borrador y vuelve a la mascota. Giro navega/edita; corta confirma. La pantalla de lecturas conserva su layout y no añade submodos de sensor.

### Configuración V1

Grid raíz 2×3 con Perfil / Ajustar, fila central vacía y Reset / Salir abajo. `PRIMARY_COUNT=2`, Reset=2, Salir=3, rango encoder 0..3. El grid es incremental: shell al entrar, tiles anterior/nuevo al girar.

Perfil selecciona General/Personalizado en borrador y ofrece resumen Cancelar/Guardar, inicialmente Cancelar. Ajustar recorre Suelo baja/alta, temperatura baja/alta y Aire baja/alta; guardar activa Personalizado. Pasos de 1 punto %, 1 °C o 1 °F según unidad global al entrar. Conversión/base de almacenamiento °C y límites/separaciones validados. Cancelar no cambia perfil ni NVS; larga cancela todo. Error de escritura conserva perfil activo anterior.

General conserva las referencias personalizadas para reutilizarlas. Reset pide NO/SÍ (NO inicial), elimina únicamente la configuración de Planta Lab y restaura General; no cambia alertas, calibración o unidad global.

El personalizado guarda las referencias RAW de calibración Suelo. Si cambian, REVISAR SUELO mantiene el porcentaje blanco pero neutraliza referencia/gesto. Recorrer Ajustar, revisar los seis valores y Guardar vincula nuevamente. Elegir Personalizado en el selector no elimina una revisión pendiente.

NVS `pbit/plant_cfg`: blob único de 36 bytes, schema/perfil versión 1, modo, seis floats, dos RAW `uint16_t` y checksum FNV-1a32. Se valida longitud, versiones, checksum y referencias al cargar; ausente/corrupto/incompatible vuelve a General. Solo se escribe al confirmar, nunca por tick ni desde Demo. Detalle en TECHNICAL §9/§17 y USER_GUIDE §6.


## Verificación y pruebas pendientes

2026-10-12: el usuario comunica que la versión cargada funciona muy bien y pide conservar el diseño actual. Se da por cerrado provisionalmente el ajuste visual de producto; esta observación general no verifica individualmente todas las poses, idiomas, recuperación, flicker ni Demo, ni cierra la validación completa por unidad/24 h.

Revisión acotada completada el 2026-10-12, con cuatro trazas nativas reproducidas; [hallazgos y contrato V1 propuesto](PLANTA_LAB_BEHAVIOR_REVIEW.md). El usuario autorizó implementar el alcance completo; la V1 incorpora luz contextual, evidencia/frescura, gesto gradual, continuidad y configuración General/Personalizado sin reabrir el diseño. La validación física del nuevo candidato continúa pendiente.

Antecedente visual: el usuario aprobó en P-Bit colores actuales de planta/medidores y valoró favorablemente el suelo. Esta aceptación puntual no cierra poses, idiomas, recuperación, flicker ni Demo. Sol/luna, ondas y flexibilidad térmica ya están implementados y comprobados por software; validar el último ajuste en TFT antes del setup. Reposo contextual e intensidad de Suelo/Aire están implementados en V1; historial/combinaciones y presets siguen pendientes.

El 2026-10-11 el usuario comunicó que probó la mascota en el P-Bit y le gustó la primera idea. Es una primera impresión física favorable, no el cierre de todas las poses, idiomas, transiciones ni flicker. Primero se cierran los ajustes visuales; después se discutirá configuración/presets.

La aprobación del alcance y la implementación software se distinguen de la aprobación física completa. La evidencia de pruebas nativas, preview y build final se registra en CHANGELOG; no equivale a flasheo ni validación de unidades.

- [x] Alcance aprobado: mascota verde con rostro, reacciones por sensor y recuperación sin persistencia.
- [x] Rangos compartidos y conservación de carrusel/visibilidad condicional.
- [x] Contrato de modelo independiente de render, con desconocidos y estabilización temporal.
- [x] Pruebas nativas: Modelo nativo, incluidos casos térmicos nuevos, 1215 combinaciones de pose/fase/transición, 101 niveles de Suelo y 4 casos ausentes/no finitos, PASS; host `-Wall -Wextra -Werror` limpio. Fuentes reproducibles: `test/plant_pet_native.cpp` y `test/plant_pet_art_native.cpp`.
- [x] Preview software actual: nueve estados, once niveles de suelo, quince temperaturas, diez niveles/límites de símbolos de Luz y GIF de 64 frames desde código real; 138 comparaciones incremental/completo PASS. Artefactos en `.pio/plant_pet_review/effects/`, sin versionar. Incluye 48 frames adicionales de calor, galería solar con pantalla completa y GIF/galería de posiciones; usuario acepta el preview general de sol/luna y calor distribuido y pide extender los efectos a frío/aire, ahora integrados según el preview autorizado. Galerías térmica/solar revisadas por el agente; no equivale a observación TFT.
- [x] Evidencia histórica del candidato visual 2026-10-11, compilación `esp32dev` SUCCESS: RAM 59 580 bytes (18.2 %), Flash 971 189 bytes (74.1 %). Árbol base `3744f04` + mascota/diseño local, con cambios locales de esa revisión; evidencia en CHANGELOG/PRODUCTION_RELEASE. La build vigente de V1 se registra allí por separado.
- [ ] **Proof 9:** observar Demo en dispositivo durante al menos 30 s; Planta Lab no se incluye automáticamente.
- [ ] **Proof 10:** observar la mascota en TFT real: poses, ritmo, transiciones, bordes/valores, ausencia de parpadeo y textos ES/CAT/EN.
- [ ] Validar Luz 8000 en extremo de barra y coherencia de tierra/estado en las referencias General y Personalizado, incluyendo el retardo de confirmación.
- [ ] Validar necesidades simultáneas, lectura desconocida y recuperación al cambiar a otra vista y regresar.

Prueba final de 24 h y validación física completa por unidad: abiertas y aplazadas por decisión del usuario.

## Efectos ambientales distribuidos — integrados 2026-10-11

Antecedente de la distribución aprobada; V1 conserva arte/ritmos y separa sus anclajes absolutos del perfil editable. Estado original: implementación autorizada por el usuario después del preview. El usuario acepta el preview general de sol/luna y calor distribuido, y solicita extender distribución/movimiento a frío y posiblemente humedad/sequedad del aire. Esa aceptación de preview no cierra la prueba TFT.

- Frío: 1..3 copos pequeños cyan/azul claro según la intensidad térmica del contrato anterior (1 + intensidad/86, máximo 3) y la banda fría confirmada, independientemente de la necesidad dominante. Alternar posiciones laterales y alturas, descenso corto y deriva lateral de 1 px; ciclo de 18 ticks (3.96 s), pausa de 4 ticks; aumentar cantidad con intensidad, sin tormenta. Copos como pictograma de frío, no afirmación de nieve/helada.
- Aire húmedo: 1..2 grupos de trazos/motas cyan suspendidos, con deriva lateral lenta y apariciones alternadas a ambos lados. Mantenerlos separados del suelo; no gotas de lluvia ni charcos ambientales. Detalle crece dentro de la banda alta confirmada.
- Aire seco: conservar trazos cortos/discontinuos violetas, añadir desplazamiento leve y pausas largas en posiciones alternadas. 1..2 grupos, menos densos que los copos. Evitar polvo/hojas cayendo y grietas fuera del suelo. El movimiento no representa una medición de viento.
- Composición: máximo cuatro grupos ambientales visibles a la vez entre temperatura y aire; reservar uno para aire cuando ambos requieren señal. Posiciones seguras dentro del terrario, sin rostro, sol/luna, suelo, marcador de desconocidos ni lecturas. Variación determinista entre apariciones, sin random por frame, manteniendo la cadencia de 220 ms y el buffer fijo. Ritmos distintos: calor asciende, frío desciende, aire deriva lateralmente.
- En la integración original se mantuvieron paleta, terreno, expresiones y rangos actuales. Son decisiones visuales de producto, no nuevos umbrales agronómicos. Preview de tres intensidades de frío y aire, más combinaciones frío+aire húmedo y calor+aire seco autorizado e integrado. Usuario comunica funcionamiento general favorable en TFT el 2026-10-12; comprobaciones exhaustivas siguen abiertas.

Preview preparado desde una copia aislada de `plant_pet_art.cpp` en `.pio/plant_pet_review/ambient_proposal/`: nueve escenas de intensidad y dos combinaciones, con 48 frames por escena. Galerías PNG y tres GIF desde el layout/fuentes reales 160×128. Host C++11 `-Wall -Wextra -Werror` limpio; 528 comparaciones incremental/completo PASS. Ese preview precedió la autorización e integración en `src/plant_pet_art.cpp`. Ahora: 528 comparaciones host PASS y 528 frames idénticos al preview; raster 1215 casos +101 niveles de Suelo y 4 ausentes/no finitos PASS. Build ESP32 SUCCESS (20.103 s), RAM 59580, Flash 971189. Carga posterior autorizada en COM8 SUCCESS (29.848 s), hashes verificados y reinicio RTS enviado; observación general favorable del usuario el 2026-10-12; comprobaciones visuales específicas pendientes.


## Futuro, fuera de V1

- Presets documentados por especie/variedad, etapa, sustrato/maceta y calibración. Judía, rábano y albahaca son candidatas de investigación, no perfiles listos ni validados.
- Historial con cobertura, duración observada y reglas para apagados antes de ofrecer consejos por exposición sostenida o combinaciones.
- Personajes por especie, crecimiento/brotes guardados, misiones e integración con aplicaciones.
- Fichas y catálogo web versionados; importación de presets no implementada.
- Inclusión de Planta Lab en una coreografía Demo temática.

General/Personalizado, referencias editables, guardado y Reset ya pertenecen a la implementación V1. El progreso persistente futuro necesitará reglas propias; no se infiere de estas animaciones.
