# Reglas de snippets para el visualizador TFT

## Objetivo

Estos archivos son escenas auxiliares para el visualizador web del P-Bit.
No forman parte del firmware y pueden borrarse más adelante.

## Formato obligatorio

- Un archivo `.cpp` por escena.
- Solo draw calls planas y declaraciones mínimas necesarias para el visualizador.
- No incluir `#include`, `setup()`, `loop()`, funciones auxiliares ni lógica de compilación.
- Mantener el orden real de dibujo del firmware.
- Si una escena necesita `TFT_eSprite`, declararlo explícitamente en el propio snippet.
- Usar textos, colores, fuentes y posiciones actuales del firmware siempre que sea posible.

## Cabecera recomendada

Cada archivo debe empezar con comentarios breves:

```cpp
// Scene: home_cards_es
// Source: src/ui_lab_home_cards.cpp
// Variant: production-current / visualizer-safe
// Lang: ES / key TIT_LAB_HOME_CARDS
```

`Variant` debe ser una de estas categorías:

- `production-current`: parada visible actual.
- `sensor-zone-renderer`: renderer usado dentro de `SENSOR_ZONE_SCREEN`.
- `config-menu`: menú abierto por pulsación larga desde Sensor Zone o Sistema.
- `hidden-debug`: pantalla compilada pero oculta en producto, como BLE.
- `legacy-lab`: escena antigua útil solo como referencia visual.

## Criterios de fidelidad

- Priorizar geometría, colores, fuentes y orden de render del firmware actual.
- Cuando el visualizador no soporte bien un patrón real del firmware, preferir una variante `visualizer-safe` y dejarlo indicado en comentarios.
- No inventar layouts nuevos.
- Mantener nombres de escena coherentes con los README de cada carpeta.
- Para Inicio, cards, Sensor Zone, dials y VU, reflejar la separación shell/data y los clears acotados aunque el snippet sea estático.
- Para luz, usar lux `0..8000` como escala de usuario; raw ADC `0..4095` solo en escenas de calibración/debug.
- Para i18n, usar el texto de `LangKey` ES/CAT/EN o anotar qué idioma simula la escena.
- BLE debe marcarse como `hidden-debug`; está factory-off y fuera del carrusel.

## Estilo

- ASCII por defecto salvo textos de UI que ya existan traducidos.
- Comentarios cortos y solo si aportan contexto.
- Código limpio, fácil de copiar y pegar en el visualizador.

## Revisión

- Cada carpeta agrupa una familia visual del P-Bit.
- Revisar primero que existan todos los archivos esperados según el `README.md` de cada carpeta.
- Antes de promover una escena, comprobar que representa la UI visible actual: Inicio, Clima Lab, Planta Lab condicional, Termo Lab, Sensor Zone, Timer o Sistema. `Sonido VU/Onda` son modos de Sensor Zone, no paradas globales.
