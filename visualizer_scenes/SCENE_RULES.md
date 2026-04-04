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
// Scene: timer_idle
// Source: src/ui_timer.cpp
// Variant: visualizer-safe / exact-layout
```

## Criterios de fidelidad
- Priorizar geometría, colores, fuentes y orden de render del firmware actual.
- Cuando el visualizador no soporte bien un patrón real del firmware, preferir una variante `visualizer-safe` y dejarlo indicado en comentarios.
- No inventar layouts nuevos.
- Mantener nombres de escena coherentes con los README de cada carpeta.

## Estilo
- ASCII por defecto.
- Comentarios cortos y solo si aportan contexto.
- Código limpio, fácil de copiar y pegar en el visualizador.

## Revisión
- Cada carpeta agrupa una familia visual del P-Bit.
- Revisar primero que existan todos los archivos esperados según el `README.md` de cada carpeta.
