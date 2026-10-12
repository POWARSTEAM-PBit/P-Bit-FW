# Planta Lab — investigación para comportamiento y perfiles de aula

Fecha de investigación: 2026-10-11. Revisión de alcance: 2026-10-12. Estado: investigación preliminar; comportamiento contextual y General/Personalizado V1 implementados como reglas educativas. Historial, combinaciones sostenidas y presets por especie sin implementar/validar.

Este documento fundamenta la revisión de comportamiento. El contrato implementado está en [ADR-003](ADR-003-planta-lab.md) y los pendientes en [ROADMAP](ROADMAP.md). Distinguir las decisiones provisionales de UI ya implementadas de las recomendaciones biológicas y perfiles aún pendientes.

## Qué está implementado en V1

General conserva Suelo 30..70 %, temperatura 15..30 °C y Aire 40..70 %. Personalizado permite seis referencias educativas, guardadas solo al confirmar. Temperatura mantiene histéresis ±0.5 °C; Suelo/Temperatura/Aire tienen gesto gradual relativo al perfil, sin ampliar por arrastre Aire a 30..75 %.

Luz bajo referencia de 200 lux permite REPOSO sin competir como atención adversa; Suelo, temperatura o Aire conservan sus gestos si requieren atención. No confirma noche ni suficiencia diaria. Sol de cuatro tamaños y luna <20 lux se conservan. Efectos ambientales mantienen anclajes absolutos ilustrativos (frío <15 °C hacia 8, calor desde 27/30/33/36, Aire <40 o >70 %), separados del gesto relativo al perfil.

Cada canal distingue adquisición nueva, dato retenido y edad: solo nuevas muestras confirman cambios; caducidad local Suelo/Luz >1000 ms y DHT >3000 ms. Intensidades/transiciones de 1.5/3 s son de interfaz. La mezcla efectiva de poses mantiene continuidad al interrumpir recuperaciones. Estos tiempos no demuestran estrés ni recuperación biológicos.

Personalizado de Suelo guarda su vínculo con calibración RAW; una calibración distinta neutraliza esa referencia hasta revisar/confirmar. Lectura real sigue visible. Perfil y Reset propios no cambian alarmas ni calibración. No hay historial diario, crecimiento persistente, RTC ni presets validados. Contrato y evidencia actual en [ADR-003](ADR-003-planta-lab.md) y [PRODUCTION_RELEASE](PRODUCTION_RELEASE.md).


## Hallazgos y límites de interpretación

1. **Oscuridad momentánea no significa daño.** Los ciclos de luz/oscuridad intervienen en el desarrollo; la oscuridad tiene funciones normales, incluida la regulación de floración de ciertas especies. No corresponde penalizar automáticamente una noche. [Oregon State University](https://extension.oregonstate.edu/es/node/115936/printable/print).
2. **Importan intensidad y duración.** Una muestra no representa la luz recibida durante un día. Lux mide iluminación según la sensibilidad visual humana; PPFD y DLI describen luz fotosintética instantánea y acumulada. La conversión depende de la fuente luminosa. [Purdue, HO-238](https://ag.purdue.edu/department/hla/extension/extension-publications-library/ext-pubs/ho-238-w.html).
3. **Calor, luz y agua interactúan.** La transpiración ayuda a enfriar las hojas; calor y aire seco pueden aumentar la pérdida de agua. La respuesta depende de la especie y de la disponibilidad de agua. Mucha luz y calor no prueban daño por sí solos. [Oregon State University](https://extension.oregonstate.edu/es/node/115936/printable/print), [Purdue](https://ag.purdue.edu/department/hla/extension/extension-publications-library/ext-pubs/ho-238-w.html).
4. **Temperatura y humedad del aire necesitan contexto común.** El gradiente de presión de vapor entre hoja y aire influye en la pérdida de agua. [UF/IFAS y universidades colaboradoras, VPD](https://propg.ifas.ufl.edu/02-environment/02-water/09-water-vpd.html).
5. **Agua medida y agua disponible no son equivalentes.** La interpretación depende del sustrato, sus propiedades, la ubicación del sensor y la calibración. [University of Minnesota, sensores de humedad](https://extension.umn.edu/natural-resources/conservation/agricultural-soil-and-water/irrigation/soil-moisture-sensors-for-irrigation-scheduling).
6. **Los grupos genéricos no bastan.** Por ejemplo, los cactus navideños proceden de selvas y requieren luz filtrada y condiciones de humedad distintas de muchos otros cactus. [University of Minnesota, cactus navideños](https://extension.umn.edu/garden-and-home/yard-and-garden/gardening-in-minnesota/holiday-cacti).

Consecuencias para el P-Bit — inferencias de ingeniería, no afirmaciones de las fuentes sobre este dispositivo:

- El LDR por sí solo no distingue noche, sombra, aula cerrada o sensor tapado. Un gesto de reposo es una metáfora; identificar «noche» requiere contexto temporal adicional.
- La estimación de Luz se recorta a 8000 lux. En ese techo se desconoce cuánto lo supera la iluminación; no demuestra un máximo biológico ni permite cuantificar exposición intensa. Un futuro índice de exposición en lux tendría límites propios y no se presentaría como DLI medido.
- El DHT11 mide aire, no temperatura de hojas o raíces. Una aproximación ambiental de VPD no sería VPD foliar exacto ni un diagnóstico.
- Suelo muestra un porcentaje relativo entre referencias de calibración. No es contenido volumétrico ni agua utilizable por raíces. No copiar porcentajes agronómicos al firmware. Una lectura alta no mide oxígeno, drenaje, pudrición o «ahogamiento».
- Nutrición, plagas, enfermedades, estado de raíces y crecimiento no se observan con estas cuatro lecturas. Estar dentro del perfil solo describe las condiciones medidas.

## Comportamiento: observación V1 y consejos sostenidos futuros

V1 separa observación inmediata, contexto de luz y atención del perfil. Reposo, textos observacionales y recuperación visual están implementados. Los consejos sostenidos de esta tabla siguen siendo propuestas y necesitan evidencia compatible con especie, duración y cobertura. La intensidad normalizada de representación no es gravedad fisiológica.

| Evidencia | Respuesta / estado | Condición necesaria |
|---|---|---|
| Poca luz ahora | REPOSO contextual implementado; sin penalización por luz | No afirmar que es de noche; conservar avisos térmicos o de agua si los hay |
| Exposición insuficiente repetida | Sugerir revisar la ubicación/luz | Perfil y registro con cobertura suficiente; no completar huecos con oscuridad inventada |
| Calor, aire seco y secado del sustrato | Gesto de calor y posible aviso para revisar agua | Considerar duración, tendencia y necesidades de la especie; no afirmar quemadura |
| Suelo húmedo tras riego | SUELO HÚMEDO y gesto gradual implementados; sin diagnóstico automático | No inferir «ahogada» por una lectura alta aislada |
| Humedad persistente incompatible con el perfil | Sugerir revisar sustrato/drenaje | El drenaje se comprueba por observación; el P-Bit no lo mide |
| Falta de datos o de historial | Mantener incertidumbre | Separar dato inválido, falta de cobertura y techo de medición |
| Mejoran las condiciones | Recuperación visual continua implementada | La animación no demuestra recuperación de la planta real |

Las referencias educativas General/Personalizado ya tienen contrato V1. Los límites y tiempos de futuros consejos agronómicos siguen abiertos; cuando haya fundamento podrán añadirse avisos por extremos; el calor extremo también puede ser relevante sin suelo seco. Germinación, plántula y planta establecida requieren contextos distintos. La etapa se seleccionaría explícitamente; estos sensores no permiten deducirla con fiabilidad.

El historial debe registrar tiempo observado, huecos y reinicios. Un periodo con el equipo apagado no cuenta como luz cero. Antes de hablar de un día completo hay que definir ventana temporal, cobertura mínima y comportamiento tras reinicio; disponer de temporizadores de animación no resuelve esa cuestión.

## Candidatas para un aula

Selección didáctica propuesta, condicionada a luz disponible, época, maceta y seguimiento de riego. Una ventana luminosa o iluminación adecuada puede ser necesaria; germinar dentro no garantiza completar el cultivo allí.

| Candidata | Interés para el aula | Diferencia útil para el perfil |
|---|---|---|
| Judía enana/frijol — *Phaseolus vulgaris* | Semilla, hojas y después flores/vainas; elegir variedad enana y espacio de maceta suficiente | Lugar cálido y soleado; riego regular, especialmente al florecer. [RHS](https://www.rhs.org.uk/vegetables/french-beans/grow-your-own) |
| Rábano de ensalada — *Raphanus sativus* | Ciclo relativamente corto y raíz visible al cosechar | Humedad regular; evitar calor y sequedad; sombra ligera con calor. [RHS](https://www.rhs.org.uk/vegetables/radishes/grow-your-own) |
| Albahaca — *Ocimum basilicum* | Aromas, hojas, poda y cosechas sucesivas | Luz abundante, contexto cálido y sustrato más uniformemente húmedo que romero/tomillo. [Kew](https://www.kew.org/read-and-watch/windowsill-veg-herbs), [UMN](https://extension.umn.edu/garden-and-home/yard-and-garden/gardening-in-minnesota/growing-herbs), [UMD](https://extension.umd.edu/resource/growing-vegetables-containers-and-salad-tables) |
| Lechuga de hoja — *Lactuca sativa* | Hojas y cosechas; contraste con albahaca | Estación fresca y humedad uniforme; reducir sol en condiciones muy cálidas. [UMD](https://extension.umd.edu/resource/growing-vegetables-containers-and-salad-tables) |
| Berro de jardín — *Lepidium sativum* | Experimento rápido de brotes, aproximadamente una semana | Puede crecer inicialmente en papel/algodón húmedo; no es berro de agua. El preset de sustrato no se transfiere a algodón sin validación. [Kew](https://www.kew.org/read-and-watch/windowsill-veg-herbs) |
| Haworthia, candidata *H. cooperi* | Mascota ornamental compacta y comparación de ciclos de secado | Luz brillante indirecta; dejar acercarse el sustrato a sequedad entre riegos. Seleccionar especie/cultivar concreto antes de crear el preset. [RHS](https://www.rhs.org.uk/plants/haworthia) |

Empezaría a validar **judía enana, rábano y albahaca en sustrato**. Berro queda como actividad introductoria; lechuga y haworthia como ampliaciones. La prioridad es una elección de producto, no una clasificación oficial de las fuentes. Las macetas se secan de manera distinta según tamaño, crecimiento y ambiente; no prometer autonomía durante un fin de semana sin observar la combinación real. [UMD, cultivo en recipientes](https://extension.umd.edu/resource/growing-vegetables-containers-and-salad-tables).

## Revisión de expresividad y referencia general — 2026-10-11

El usuario aprobó en P-Bit los colores de la planta y los medidores y valoró favorablemente el efecto del suelo. Pide ampliar el sol con la iluminación, simplificar la luna a un círculo solo en oscuridad marcada, aumentar las ondas de calor progresivamente y facilitar la expresión feliz. Esta aprobación es puntual: no completa todas las poses, idiomas, transiciones, flicker ni Demo.

### Problema del perfil anterior y corrección térmica

Antes del ajuste térmico, Content exigía Suelo 30..70 %, temperatura 18..26 °C, aire 40..70 % y Luz 200..8000 lux simultáneos. 27 °C o 39 % de aire bastaban para otra pose. El usuario atribuye su dificultad a cambios de un grado aun con buena lectura de Suelo. Se amplió únicamente la franja térmica a 15..30 °C, con histéresis y pose/rostro de intensidad gradual; los otros canales siguen pudiendo impedir Content. Sin necesidad conocida, datos ausentes continúan produciendo Unknown.

### Referencias hortícolas: alcance interior, no óptimos universales

[University of Georgia, B1318](https://fieldreport.caes.uga.edu/publications/B1318/growing-indoor-plants-with-success/) indica aproximadamente 21..27 °C de día y 18..21 °C de noche como orientación para plantas de interior, y un intervalo amplio de tolerancia de aproximadamente 14..30 °C para muchas de ellas; reconoce especies con preferencias distintas. No se transfiere como óptimo de germinación, cactus, lechuga o cualquier especie.

[Colorado State University](https://cmg.extension.colostate.edu/wp-content/uploads/sites/59/2020/01/Get-Your-Green-On-Houseplants-Part-I-handouts.pdf), p. 7, ofrece 40..60 % de humedad relativa como referencia ambiental doméstica en su material sobre plantas de interior. No convierte 39 % o 61 % en daño. Temperatura/aire/agua interactúan: la transpiración aumenta en condiciones cálidas y secas y depende del suministro de agua, según [Oregon State](https://extension.oregonstate.edu/catalog/em-9903-environmental-factors-affecting-plant-growth).

| Canal | Referencia útil | Límite de aplicación al P-Bit |
|---|---|---|
| Temperatura | Aproximadamente 18..27 °C como envolvente orientativa de día/noche para muchas plantas de interior | Aire medido, no hoja; especie/etapa pueden necesitar otros valores |
| Humedad del aire | 40..60 % como objetivo doméstico orientativo | No define por sí sola bienestar ni daño; considerar temperatura y especie |
| Luz | Intensidad y duración compatibles con la especie | No hay un intervalo universal en lux; 8000 es techo instrumental y una muestra baja no demuestra carencia diaria |
| Suelo | Riego/secado según especie y sustrato | El porcentaje del P-Bit es relativo, no humedad volumétrica; 30/70 son cortes ilustrativos heredados, no umbrales agronómicos generales |

### Decisiones V1 y propuestas de producto posteriores

Separar cuatro salidas: lectura, efecto ambiental, expresión principal y aviso sostenido. Una planta puede sonreír mientras una onda indica ambiente cálido; las barras conservan su información y no se oculta una condición. Content pasa a expresar ausencia de necesidad importante detectada, sin certificar salud real. La propuesta no resuelve nutrición, enfermedades ni crecimiento.

- Temperatura: núcleo orientativo 18..27 °C; 15..30 °C ya implementado como intervalo flexible para el personaje, con histéresis y reacción progresiva. Este último es una decisión provisional de UI, apoyada en la amplitud de tolerancias publicada, no un óptimo universal ni un límite de seguridad biológico.
- Aire: V1 conserva 40..70 % y añade intensidad gradual, de modo que un desvío suave no fuerza la pose completa. La propuesta anterior de 30..75 % sigue sin adoptar/validar; no es una cifra prescrita por las fuentes.
- Luz: reposo tranquilo sin penalización está implementado; no anunciar suficiencia diaria sin historial/cobertura. La temperatura o el agua siguen teniendo prioridad si hay una necesidad independiente.
- Suelo: efecto progresivo aprobado conservado, referencias General/Personalizado ligadas a calibración y gesto gradual implementados. SUELO SECO/HÚMEDO describe la lectura. Futuros avisos por estrés requieren calibración, especie y duración; no ampliar números como si fueran un óptimo de cultivo.
- Extremos y combinaciones: considerar calor con aire seco y secado del suelo, pero no exigir esa combinación para reconocer un extremo térmico. La duración para un aviso no se copia de los 1200 ms de estabilización UI: queda por diseñar/validar. Datos faltantes mantienen incertidumbre.

Detalles visuales ya implementados: sol amarillo con radios 1/2/3/4 desde 20/200/800/3000 lux, variación de brillo hasta 8000; luna circular pálida <20 lux como corte ilustrativo, sin detectar noche. Ondas magenta 1/2/3/4 desde 27/30/33/36 °C, no de daño. Se conserva la paleta aprobada, sin introducir rojo/coral. Temperatura 15..30 °C, histéresis ±0.5 °C y `thermal_intensity` suavizada; sonrisa/neutral/reacción completa según intensidad visual. Preview y build comprobados, observación general favorable del usuario en P-Bit el 2026-10-12; comprobaciones físicas específicas pendientes.

El usuario conserva el diseño el 2026-10-12. La [revisión de comportamiento](PLANTA_LAB_BEHAVIOR_REVIEW.md) propone corregir semántica/frescura/continuidad y separar referencias/animación antes de General/Personalizado. El alcance completo fue autorizado: reposo contextual, intensidad gradual de Suelo/Aire, identidad/frescura, continuidad y General/Personalizado están implementados en V1. Los avisos sostenidos/combinados y la propuesta numérica de Aire 30..75 % siguen sin adoptar. No se diagnostica salud real.

### Tiempo de sesión y combinaciones — siguiente revisión

El usuario plantea que humedad del suelo muy baja/alta puede causar estrés, que no hay reloj para saber duración/exposición diaria y que temperatura podría incluir calor del propio P-Bit. Separar observación, hipótesis y capacidades técnicas:

- La mascota actual sí reacciona a Suelo bajo/alto, junto al efecto de tierra aprobado; el porcentaje relativo no mide disponibilidad de agua ni oxígeno. Mantener esos números como ilustración hasta validar perfil/calibración, no como umbrales universales de estrés.
- Sin hora del día se puede medir **duración de sesión** con `millis()`. El firmware ya usa diferencias de tiempo para evidencia UI/animación, incluidas pruebas de rollover. Añadir duración observada de humedad no requiere RTC; sí exige diseñar cobertura de lecturas válidas y reinicios.
- No existe hoy historial que registre exposición diaria o persistencia de suelo, ni persistencia de ese historial. Un apagado no es evidencia de oscuridad ni humedad; RTC/hora y continuidad de observación son problemas distintos. No prometer diagnóstico diario por tener un temporizador.
- Calor propio del P-Bit es una posible fuente de sesgo planteada por el usuario, no cuantificada en esta unidad. No aplicar una resta térmica arbitraria. Antes de una corrección se necesitaría comparar montaje/lectura con referencia ambiental y comprobar la dependencia del uso/alimentación.
- Luz alta + aire cálido justifica observar la ubicación como hipótesis de zona luminosa/cálida, sin confirmar sol directo fuerte ni quemadura. El LDR se satura a 8000, y el DHT11 mide aire: la fuente de luz y temperatura de hoja siguen desconocidas.
- Luz/calor con aire seco y suelo que se seca puede dar contexto de mayor demanda de agua; duración y especie condicionan la interpretación. Una lectura alta de suelo tras riego tampoco demuestra daño. Los avisos por combinaciones sostenidas no se implementan en V1.

## Presets, personajes y catálogo web — futuro

Un preset debería incluir especie/variedad, etapa, contexto de luz, sustrato/maceta, método de calibración, fuentes, unidades originales, reglas observables y grado de validación con P-Bit. Los valores que no se puedan justificar quedan pendientes; una especie no aporta automáticamente cuatro porcentajes/mínimos/máximos.

El personaje puede depender de la especie mediante una familia de dibujo — brote, aromática, roseta o suculenta — y compartir las mismas salidas del modelo. La apariencia no define las necesidades. Mantener el espacio de las cuatro lecturas y la estrategia de buffer actual.

La web podría publicar fichas y perfiles con versión, fuentes y compatibilidad de firmware. Comenzar con fichas para consulta o ajuste manual; diseñar una importación después de definir formato y validación. No existe hoy descarga/importación de presets. Separar datos del perfil, reglas de interpretación y arte del personaje permite ampliar el catálogo sin rehacer todo el renderer.

## Secuencia vigente — revisada 2026-10-12

1. Diseño probado conservado por decisión del usuario; validaciones exhaustivas continúan abiertas.
2. Revisión acotada completada y alcance autorizado: comportamiento contextual, nueva evidencia/frescura, continuidad e intensidad gradual implementados.
3. General/Personalizado V1 implementado con referencias de Suelo/Temperatura/Aire, Luz contextual y persistencia/Reset propios. Probar el nuevo candidato en P-Bit sin atribuirle la aprobación de la versión visual anterior.
4. Validar fichas/presets por especie, etapa y calibración; judía, rábano y albahaca son candidatas, no presets listos.
5. Diseñar historial/cobertura y combinaciones para futuros consejos por duración. Personajes por especie y catálogo web posteriores.

El requisito de historial diario aplica a interpretaciones sostenidas; no bloquea referencias educativas personalizables. Esta implementación no añade ni valida nuevos umbrales agronómicos. No inicia prueba final de 24 h ni validación por unidad, aplazadas. Evidencia del candidato en [PRODUCTION_RELEASE](PRODUCTION_RELEASE.md); investigación no equivale a prueba física.
