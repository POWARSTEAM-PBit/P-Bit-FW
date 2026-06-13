# ADR-002 - Taxonomía visible de pantallas P-Bit

Fecha: 2026-06-13

Estado: Aceptado

## Contexto

El P-Bit está pensado como una herramienta educativa ambiental para niños y jóvenes de 10 a 15 años. Sus pantallas deben ayudar a observar datos reales, entender cambios del entorno y usar vocabulario científico sin convertir la interfaz en una lista de términos técnicos.

Antes de esta decisión, varias pantallas de un solo sensor usaban nombres visibles como `Lab`, `Valor`, `Dial` o `Tarjeta`. Esos nombres generaban tres problemas:

- `Lab` aparecía en pantallas de un solo sensor, diluyendo su significado como laboratorio o experimento.
- `Valor`, `Dial` y `Tarjeta` describen componentes o conceptos de interfaz, no experiencias de aprendizaje.
- La nomenclatura no siempre ayudaba a dar instrucciones de aula claras, como "ve a Luz Curva" o "mira Suelo Rango".

La arquitectura interna ya soporta los modos actuales y no requiere cambios para resolver el problema visible. Por eso esta decisión se limita a taxonomía de producto, strings visibles y documentación.

## Decisión

Las pantallas de un solo sensor usarán esta familia conceptual:

```text
Principal
Dato
Curva
Rango
Ficha
```

La pantalla `Principal` no lleva sufijo visible: usa solo el nombre del sensor.

Ejemplos:

```text
TEMPERATURA
TEMP. DATO
TEMP. CURVA
TEMP. RANGO
TEMP. FICHA

LUZ
LUZ DATO
LUZ CURVA
LUZ RANGO
LUZ FICHA
```

## Regla de Lab

`Lab` queda reservado para pantallas multisensor, experimentos o actividades educativas.

Regla de oro:

```text
Lab = pantalla multisensor, experimento o actividad educativa.
Un sensor individual nunca usa Lab como sufijo visible.
```

Pantallas con `Lab` visible permitidas:

- `CLIMA LAB`
- `TERMO LAB`
- `PLANT LAB` / `PLANTAS LAB` futuro

Pantallas que dejan de usar `Lab` visible como modo de un solo sensor:

- `TEMP. LAB`
- `HUM. LAB`
- `LUZ LAB`
- `SONIDO LAB`
- `SUELO LAB`
- `TERMÓMETRO LAB`

`SONIDO VU` se mantiene como nombre de una visualización concreta, no como Lab. En una fase posterior podrá bajar al stack de `Sonido` como modo propio, sin mezclar ese refactor con esta decisión.

## Razonamiento de nombres

### Dato

Reemplaza a `Valor`.

`Dato` refuerza el lenguaje de ciencia y alfabetización de datos. El P-Bit no solo muestra números: ayuda a entender que una lectura de sensor es un dato que se puede observar, comparar y usar para decidir.

### Curva

Reemplaza a `Gráfica` como nombre visible preferido para el modo histórico.

`Curva` es más corto en TFT 160x128 y conecta mejor con cambio en el tiempo: curva de temperatura, curva de luz, curva de humedad. Sigue siendo comprensible y habilita actividades de aula centradas en tendencias.

### Rango

Reemplaza a `Dial` / `Gauge`.

El gauge del P-Bit muestra un arco con bandas de color, mínimo, máximo y aguja. Esa pantalla no solo mide: sitúa el dato dentro de una zona. `Rango` nombra el concepto que se quiere enseñar:

- rango seguro
- rango de alerta
- rango saludable
- rango de temperatura

Se descartan alternativas:

- `Medidor`: describe el componente de UI, no el concepto.
- `Nivel`: usa una metáfora vertical que encaja mejor en tanques, VU o barras.
- `Escala`: es correcta, pero más ambigua para niños y puede evocar mapa o música.

`Nivel` queda disponible para visualizaciones verticales reales futuras.

### Ficha

Reemplaza a `Tarjeta` / `Card`.

`Ficha` suena escolar, breve y útil para una pantalla resumen. Evita el olor a componente de interfaz que tiene `Card` o `Tarjeta`.

En ingles se usara `Info` en vez de `Card`, porque `Info` comunica al usuario lo que encontrara ahi y evita jerga de UI.

## Traducciones

| Concepto | Español | Català | English |
|---|---|---|---|
| Dato | Dato | Dada | Data |
| Curva | Curva | Corba | Curve |
| Rango | Rango | Rang | Range |
| Ficha | Ficha | Fitxa | Info |

## Prefijos abreviados

Por el límite de 160x128 px, los headers de modos usarán el formato:

```text
[PREFIJO_ABREVIADO_SENSOR] [SUFIJO_MODO_COMPLETO]
```

Contrato inicial para español:

| Sensor | Prefijo |
|---|---|
| Temperatura | `TEMP.` |
| Humedad | `HUM.` |
| Luz | `LUZ` |
| Sonido | `SON.` |
| Suelo | `SUE.` |
| Termómetro / DS18B20 | `TERMO` |

Las equivalencias en catalán e inglés deben verificarse antes del cierre visual. Especial atención a palabras largas en catalán y a combinaciones como `TERMOMETRE RANG` / `THERMO RANGE`.

## Alcance

### Sí se toca en la fase de implementación

- Strings visibles de i18n.
- Documentación de usuario, técnica y de proyecto.
- Comentarios que contradigan claramente la taxonomia visible.
- CHANGELOG del lote de implementacion.

### No se toca en esta decision

- Enums internos como `SZ_VIZ_VALOR`, `SZ_VIZ_GAUGE` o `SZ_VIZ_CARD`.
- Identificadores `LAB_*_SCREEN`.
- Archivos `ui_lab_*.cpp` por razones de nombre solamente.
- Orden del carrusel.
- Persistencia NVS.
- Demo Mode.
- Arquitectura de Sensor Zone.

## Consecuencias

El cambio mejora la claridad del producto sin forzar un refactor técnico. La interfaz visible queda más coherente con el propósito educativo:

- Los sensores individuales hablan de datos, curvas, rangos y fichas.
- Los Labs quedan como espacios de investigacion multisensor.
- Los nombres son mas utiles para actividades de aula y documentacion.

La deuda técnica de nombres internos se mantiene diferida. Si en el futuro se renombra `SZ_VIZ_VALOR` o `LAB_*_SCREEN`, deberá hacerse como refactor separado, después de validar que no afecta NVS, Demo Mode ni el orden de navegación.

## Plan de aplicación

1. Mantener este ADR como fuente de decisión.
2. Hacer un commit separado con los strings visibles y documentación.
3. Compilar `esp32dev`.
4. Verificar en hardware real headers en ES/CAT/EN.
5. Evaluar en una fase posterior si `SONIDO VU` baja al stack de `Sonido`.
