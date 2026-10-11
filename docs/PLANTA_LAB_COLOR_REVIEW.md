# Planta Lab — propuestas de color

Fecha: 2026-10-11. Estado: adaptación B + verde A implementada; comparativa original conservada como antecedente. Usuario aprueba colores de planta/medidores y valora favorablemente el suelo en P-Bit; validación TFT completa pendiente.

El usuario pide una mascota amigable y bonita, con verdes vivos, cyan, azules, violetas y magenta. Rechaza los acentos amarillos/naranjas de las hojas y la presencia general de ámbar/coral en las barras. Esta preferencia prevalece sobre la orientación semafórica genérica de la habilidad TFT para esta pantalla.

## Diagnóstico del diseño anterior a esta adaptación

- `src/plant_pet_art.cpp` añade amarillo a sed/aire seco y naranja a calor. También usa una luz lima amarillenta en bienestar. Esos acentos ocupan corona y hojas, y pueden parecer deterioro.
- `src/ui_lab_plant.cpp` dibuja ámbar/verde/coral en todas las barras, incluso dentro del estado `BIEN`. Los colores cálidos ocupan gran parte de la UI y no dependen de que exista una necesidad activa.
- Textos como `AHOGADA` permanecen en la comparativa. Su semántica se revisará según la investigación biológica; esta pasada compara únicamente colores.

## Alternativas históricas propuestas

Los RGB de las tablas son entradas del renderer; se cuantizan a RGB565, por lo que el hex RGB decodificado de un PNG puede variar ligeramente.

| Variante | Planta: base / sombra / luz / brillo | Barras | Carácter |
|---|---|---|---|
| **A — Verde + cyan** | `#30E858` / `#007848` / `#70FFA0` / `#C8FFD8` | Bajo `#A068FF`, centro `#40FF40`, alto `#00E8FF` | Verde natural y bandas muy visibles |
| **B — Marca por sensor** | `#18E088` / `#006850` / `#60FFB8` / `#C8FFE8` | Identidad por fila; extremos atenuados y centro luminoso | Menta, lectura ordenada y menos competencia entre colores |
| **C — Verde + magenta** | `#40FF60` / `#087840` / `#80FFA0` / `#D8FFE0` | Bajo `#00C8FF`, centro `#40FF40`, alto `#FF40D8` | Más eléctrico y contrastado |

En B se reutilizan las identidades existentes de Suelo `PB_SOIL_P1`, temperatura `PB_TEMP_P1` y aire `PB_HUM_P1`. Luz usa violeta local en su barra; el pequeño icono técnico conserva su amarillo canónico. No se modifica `include/palette.h` ni las otras vistas.

| Fila de B | Centro | Extremos |
|---|---|---|
| Suelo | `#40FF40` (`0x47E8`) | `#106030` |
| Temperatura | `PB_TEMP_P1` (`0xF817`) | `#601050` |
| Aire | `PB_HUM_P1` (`0x075F`) | `#005060` |
| Luz | `#B078FF` | `#402070` |

Reglas comunes de las tres propuestas:

- La planta conserva la misma paleta verde en todas las poses, incluidos datos desconocidos; sus hojas no pasan a amarillo, naranja, cyan o violeta por estado.
- Sed/aire seco/poca luz usan violeta en A/B y cyan en C para badge/borde; humedad alta usa cyan, frío azul y calor magenta. Desconocido conserva texto/borde neutro.
- Las marcas ambientales usan la nueva familia cromática: calor magenta, aire seco violeta/cyan y símbolo ambiental de luz violeta. La tierra cambia a tonos apagados de ciruela, sin competir con el personaje.
- Valores blancos, iconos compactos, marcador blanco, shell, coordenadas, fuentes, estados, umbrales y gestos se conservan. Las bandas siguen representando posición respecto al perfil vigente; no son barras de progreso.
- La prioridad del estado continúa expresándose con texto, postura y borde de fila, además del color.

Recomendación de diseño: **B** por su relación con las identidades de sensor y menor peso visual de las bandas. A es la alternativa si se prefiere un verde menos turquesa y tres zonas más explícitas. El usuario eligió adaptar B: mantener sus barras, usar el verde A y recuperar tierra café/sol amarillo con progresión ambiental.

## Evidencia de la comparativa original

Artefactos locales no versionados en `.pio/plant_pet_review/color_review/`:

- `current/all-states.png`: nueve estados del código vigente, regenerados.
- `proposals-comparison.png`: A/B/C con bienestar, sed, calor y poca luz.
- `a/all-states.png`, `b/all-states.png`, `c/all-states.png`: nueve estados por propuesta; PNG individuales y nativos en cada carpeta.
- `palettes.json` y `verification.json`: colores y comprobaciones de esta comparativa.

Se generaron 36 pantallas con el renderer C++ real y copias aisladas de los dos archivos visuales. Host `-Wall -Wextra -Werror`; 25 comparaciones de actualización parcial/completa por variante, 100 PASS. Los nueve framebuffers de la versión anterior coincidían byte a byte con su preview previo. En aquella comparativa las fuentes de producción se verificaron intactas.

Contraste calculado tras cuantización RGB565: mínimo de texto de badge/fondo 5.45:1; cara/base verde 10.64:1 en A, 10.01:1 en B y 12.72:1 en C. Es una comprobación matemática de software, no una validación del ST7735 real.

La comparativa original solo creó documentación y previews aislados, sin nueva build ESP32 ni flasheo. La implementación posterior se registra abajo; configuración, presets e historial siguen fuera de esta pasada.

## Adaptación elegida e implementada — evidencia anterior al ajuste térmico

- Barras B: identidad por sensor, tramo normal luminoso y extremos oscurecidos; marcador/cifras blancos. Planta A verde persistente en todos los estados, incluso Unknown.
- Tierra café: tono continuo con la lectura relativa; grietas progresivas en banda baja confirmada, pequeñas manchas de agua que crecen en banda alta confirmada. Ausentes/no finitos sin señales de sequedad/agua. No simula secado por tiempo ni volumen real de agua.
- Sol amarillo según lectura y luna pálida para baja iluminación, sin reloj ni confirmación de noche. Frío cyan/azul, calor magenta, aire seco violeta y aire húmedo cyan suspendido, con detalle progresivo.
- Movimientos, parpadeos, poses, recuperación, rangos y lógica biológica se conservan. `POCA LUZ` bajo 200 lux todavía está vigente; reposo y perfiles quedan posteriores al cierre visual.
- Renderer y UI reales: nueve estados, once niveles de suelo y GIF de 64 frames en `.pio/plant_pet_review/natural/`, sin versionar. 81 checks de modelo, 1215 casos de raster, 101 niveles de suelo y 4 ausentes/no finitos; 50 comparaciones parcial/completo PASS. Host `-Wall -Wextra -Werror` limpio.
- `esp32dev` SUCCESS: RAM 59564/327680 (18.2 %), Flash 970289/1310720 (74.0 %), base `3744f04` + cambios locales sin commit. Sin flasheo ni validación física de esta adaptación. ADR-003, DESIGN_SYSTEM, render, manuales y estado de producción sincronizados.

## Aprobación visual puntual del usuario

El 2026-10-11 el usuario aprueba colores de planta/medidores y valora favorablemente el efecto del suelo en el P-Bit. Pide como siguiente ajuste sol creciente, luna circular solo con luz muy baja, intensidad de ondas y una expresión feliz más flexible. No reabrir la paleta aprobada por inferencia de estas peticiones; posible acento cálido de calor se estudiará solo en el efecto. Propuesta y fuentes en [PLANTA_LAB_RESEARCH](PLANTA_LAB_RESEARCH.md#revisión-de-expresividad-y-referencia-general--2026-10-11); sol/luna y respuesta térmica flexible ya implementados después de esta aprobación, según ADR-003. Historial/combinaciones y presets siguen pendientes. La aceptación puntual no completa pruebas de poses, idiomas, recuperación, flicker ni Demo.

## Ajuste posterior de símbolos y temperatura

Se conservan hojas y medidores aprobados. Sol de cuatro tamaños y luna como disco pálido solo <20 lux; ondas magenta 1..4 y respuesta térmica gradual con margen 15..30 °C e histéresis. Sin rojo/coral nuevo. La evidencia vigente está en [PRODUCTION_RELEASE](PRODUCTION_RELEASE.md); previews actuales en `.pio/plant_pet_review/warmth/`. Aún sin validación física de este ajuste.
