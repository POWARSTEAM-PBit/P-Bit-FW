# TFT Render Rules — P-Bit

Reglas permanentes para todas las pantallas TFT del P-Bit.
Cualquier pantalla nueva debe seguir este documento desde el primer commit, no como corrección posterior.

---

## 1. Sistema de coordenadas

La pantalla es **160 × 128 píxeles**, orientación landscape (rotation = 1).

```text
X: 0 ──────────────────────────────────── 159
Y:
 0  ┌─────────────────────────────────────┐
    │  HEADER  (título, fuente interna)   │
28  ├─────────────────────────────────────┤  ← L_HEADER_LINE
    │  franja de respiración              │
32  ├─────────────────────────────────────┤  ← L_CONTENT_TOP
    │                                     │
    │  ÁREA DE CONTENIDO                  │
    │                                     │
118 ├─────────────────────────────────────┤  ← zona de alerta/footer
    │  FOOTER / HINT / ALERT JEWEL        │
127 └─────────────────────────────────────┘
```

**Zonas reservadas:**

| Zona    | Y inicio | Y fin | Notas                                  |
| ------- | -------- | ----- | -------------------------------------- |
| Header  | 0        | 27    | Solo titulo de pantalla                |
| Divider | 28       | 31    | L_HEADER_LINE, no dibujar encima       |
| Content | 32       | 117   | Toda la UI de datos vive aqui          |
| Footer  | 118      | 127   | Hints, alert jewel, labels secundarios |

**Márgenes laterales obligatorios:**

- Margen mínimo desde X=0 o X=159: **2 px**
- Para contenido centrado: usar `L_MARGIN_SIDE = 10`
- Para cards: definir `CARD_X` ≥ 2, `CARD_X + CARD_W` ≤ 158

---

## 2. Protocolo anti-flicker

El flicker casi siempre viene de borrar más de lo necesario, o de borrar y redibujar cosas que no cambiaron. El protocolo es:

### 2.1 Tres niveles de redraw

| Nivel               | Activar cuando                   | Redibujar                               |
| ------------------- | -------------------------------- | --------------------------------------- |
| screen_changed      | Cambio de pantalla o reset de UI | Shell completo: header, card, labels    |
| sensor_data_changed | Nuevo valor de sensor            | Solo los campos dinamicos que cambiaron |
| Nada                | Valor igual al ultimo dibujado   | No redibujar nada                       |

**Regla:** nunca usar `sensor_data_changed` como señal para repintar el shell. El shell se pinta una vez y solo vuelve a pintarse si `screen_changed` o si un cambio de estado fuerza un layout diferente.

### 2.2 Caché obligatoria por campo dinámico

Cada campo que cambia con el sensor debe tener su propia variable de caché:

```cpp
// Caché de último valor dibujado
static int   last_value_drawn  = -9999;
static int   last_unit_drawn   = -1;
static int   last_state_drawn  = -1;

// Antes de dibujar, comparar
int new_value = (int)roundf(reading * 10.0f); // para 1 decimal
if (!screen_changed && new_value == last_value_drawn) return; // salir si no hay cambio
last_value_drawn = new_value;
```

**Reglas de la caché:**

- Para valores `%.0f`: clave = `(int)roundf(value)` — nunca `(int)value`
- Para valores `%.1f`: clave = `(int)roundf(value * 10.0f)`
- Para strings: comparar con `strcmp`, no con punteros
- Para estados bool/enum: comparar directamente

### 2.3 Nunca `fillScreen()` dentro del loop de render

`fillScreen()` solo está permitido en:

- inicialización de hardware
- el bloque `screen_changed` de una transición de pantalla

En cualquier otro contexto, usar `fillRect(...)` acotado a la región exacta.

### 2.4 El shell se dibuja una vez

```cpp
void draw_screen_mysensor(bool screen_changed, bool data_changed) {
    if (screen_changed) {
        // Dibujar TODO lo estático aquí:
        // - header
        // - bordes de card
        // - iconos fijos
        // - labels que nunca cambian
        // - líneas divisoras
        draw_static_shell();
    }

    if (!screen_changed && !data_changed) return;

    // Solo actualizar campos dinámicos:
    update_value_field();
    update_bar_field();
    update_category_label();
}
```

---

## 3. Reglas de clear rect

Este es el origen más frecuente de artefactos visuales: borrar más de lo necesario y eliminar elementos vecinos.

### 3.1 Nunca usar ancho fijo para borrar texto

```cpp
// MAL: borra 80 px independientemente del texto anterior
tft.fillRect(VALUE_X, VALUE_Y - 14, 80, 16, TFT_BLACK);

// BIEN: borrar exactamente lo que se midió antes (ver 3.2)
```

### 3.2 Patrón correcto para borrar y redibujar texto

```cpp
// 1. Guardar el ancho del texto anterior (o el máximo posible del campo)
int clear_w = tft.textWidth(last_str_drawn) + 2; // +2 px de margen

// 2. Clampar al límite del card
clear_w = min(clear_w, CARD_X + CARD_W - CARD_PAD - VALUE_X);

// 3. Calcular la altura real de la fuente activa
int clear_h = tft.fontHeight() + 2;

// 4. Borrar solo esa región
tft.fillRect(VALUE_X, VALUE_Y - tft.fontHeight(), clear_w, clear_h, TFT_BLACK);

// 5. Dibujar el nuevo valor
tft.drawString(new_str, VALUE_X, VALUE_Y);

// 6. Guardar el nuevo string para el próximo ciclo
strlcpy(last_str_drawn, new_str, sizeof(last_str_drawn));
```

### 3.3 Alternativa: borrar por ancho máximo conocido del campo

Si el campo tiene un ancho máximo conocido (ej: siempre 3 dígitos), se puede usar ese máximo como ancho fijo, pero solo si ese máximo no supera el límite del card:

```cpp
// Solo válido si MAX_FIELD_W < CARD_X + CARD_W - CARD_PAD - FIELD_X
constexpr int MY_VALUE_CLEAR_W = 36; // ancho máximo del campo "999"
tft.fillRect(VALUE_X, VALUE_Y - font_h, MY_VALUE_CLEAR_W, font_h + 2, TFT_BLACK);
```

### 3.4 Clamping obligatorio antes de cualquier fillRect

Antes de ejecutar cualquier `fillRect`, verificar que no se sale de los límites del card o la región asignada:

```cpp
// Helper recomendado: asegurar que el rect no sale del card
int safe_x  = max(rect_x, CARD_X + CARD_PAD);
int safe_y  = max(rect_y, CARD_Y + CARD_PAD);
int safe_x2 = min(rect_x + rect_w, CARD_X + CARD_W - CARD_PAD);
int safe_y2 = min(rect_y + rect_h, CARD_Y + CARD_H - CARD_PAD);
int safe_w  = safe_x2 - safe_x;
int safe_h  = safe_y2 - safe_y;
if (safe_w > 0 && safe_h > 0)
    tft.fillRect(safe_x, safe_y, safe_w, safe_h, TFT_BLACK);
```

---

## 4. Cards y safe areas

### 4.1 Estructura obligatoria de todo card

Un card es un rectángulo con borde. Sus elementos viven dentro del área interior, nunca sobre el borde ni fuera de él.

```text
CARD_X ──────────────────── CARD_X + CARD_W
CARD_Y  ┌────────────────────────────┐
        │ padding = CARD_PAD (≥ 2)   │
        │  ┌──────────────────────┐  │
        │  │  área interna segura │  │
        │  └──────────────────────┘  │
        │ padding = CARD_PAD (≥ 2)   │
CARD_Y + CARD_H  └──────────────────────────┘
```

**Valores obligatorios al definir un card:**

```cpp
constexpr int MY_CARD_X   = 10;
constexpr int MY_CARD_Y   = 36;
constexpr int MY_CARD_W   = 140;
constexpr int MY_CARD_H   = 68;
constexpr int MY_CARD_PAD = 3;   // padding mínimo interior

// Límites calculados del área interna (para validaciones)
constexpr int MY_CARD_INNER_X1 = MY_CARD_X + MY_CARD_PAD;
constexpr int MY_CARD_INNER_Y1 = MY_CARD_Y + MY_CARD_PAD;
constexpr int MY_CARD_INNER_X2 = MY_CARD_X + MY_CARD_W - MY_CARD_PAD;
constexpr int MY_CARD_INNER_Y2 = MY_CARD_Y + MY_CARD_H - MY_CARD_PAD;
```

### 4.2 Reglas de posicionamiento dentro del card

- Ningún elemento puede tener su origen X < `CARD_INNER_X1`
- Ningún elemento puede terminar en X > `CARD_INNER_X2`
- Ningún baseline puede estar por encima de `CARD_INNER_Y1 + font_height`
- Ningún elemento puede terminar en Y > `CARD_INNER_Y2`
- Los bordes del card no se redibujan en cada frame; solo en `screen_changed`

### 4.3 Separación mínima entre elementos dentro del mismo card

- Entre icono y texto: ≥ 3 px
- Entre valor y unidad: ≥ 2 px
- Entre texto y borde del card: ≥ `CARD_PAD`
- Entre barra y texto encima/debajo: ≥ 3 px
- Entre dos labels adyacentes: ≥ 2 px

### 4.4 Elementos fuera del card

Si un elemento está fuera de un card (ej: label de footer, hint), debe definir sus propios límites explícitos y no puede invadir el espacio de ningún card vecino.

---

## 5. Texto

### 5.1 Medir antes de dibujar

Siempre calcular el ancho del texto antes de posicionarlo o de definir el área de borrado:

```cpp
int w = tft.textWidth(str);
int x = CENTER_X - w / 2; // centrado real, no estimado
```

### 5.2 Texto centrado: siempre calcular, nunca hardcodear

```cpp
// MAL: asumir que el texto encaja centrado en X=80
tft.drawCentreString(str, 80, y, font);

// BIEN para posiciones críticas: verificar que no se sale
int text_w = tft.textWidth(str);
if (text_w > MAX_FIELD_W) {
    // reducir precisión, truncar o cambiar fuente antes de dibujar
}
tft.drawCentreString(str, center_x, y, font);
```

### 5.3 Datum consistente

Todos los campos equivalentes en la misma pantalla deben usar el mismo datum (`TL`, `ML`, `TR`, etc.). No mezclar datums sin motivo explícito.

### 5.4 No superponer texto sobre texto

Si el valor puede variar en número de caracteres (ej: "25.3" vs "100.0"), el área de borrado debe cubrir el ancho del texto más largo posible dentro de los límites del campo.

---

## 6. Iconos

### 6.1 Zona exclusiva

Cada icono tiene asignada una zona exclusiva (x, y, w, h). Ningún otro elemento dibuja dentro de esa zona.

### 6.2 No redibujar si no cambió

```cpp
static int last_icon_type = -1;
if (!screen_changed && sensor_icon_type == last_icon_type) {
    // no redibujar el icono
} else {
    draw_icon(icon_x, icon_y, sensor_icon_type);
    last_icon_type = sensor_icon_type;
}
```

### 6.3 El icono no puede pisar el borde del card

El icono más su margen (≥ 2 px por lado) debe caber completamente dentro del área interna del card.

---

## 7. Barras e indicadores

### 7.1 La barra vive en su propia banda

La barra tiene coordenadas fijas. Ningún texto se dibuja sobre la barra ni en la banda que ocupa.

### 7.2 Actualizar la barra solo si cambió el valor

```cpp
static int last_bar_fill = -1;
int new_fill = map(value, val_min, val_max, 0, BAR_W);
new_fill = constrain(new_fill, 0, BAR_W);
if (!screen_changed && new_fill == last_bar_fill) return;
last_bar_fill = new_fill;
// redibujar barra
```

### 7.3 La barra no puede salir de su contenedor

```cpp
// Borde del contenedor de la barra
tft.drawRect(BAR_X - 1, BAR_Y - 1, BAR_W + 2, BAR_H + 2, BORDER_COLOR);
// Relleno: nunca excede BAR_W
int fill = constrain(computed_fill, 0, BAR_W);
tft.fillRect(BAR_X, BAR_Y, fill, BAR_H, fill_color);
// Fondo de la parte vacía
tft.fillRect(BAR_X + fill, BAR_Y, BAR_W - fill, BAR_H, TFT_BLACK);
```

---

## 8. Sprites

### 8.1 Usar sprite solo cuando aporta estabilidad real

Usar sprite cuando:

- Hay gráficos con muchos píxeles cambiando por frame (líneas, curvas)
- Una región vibra con repintado parcial directo en TFT
- Se necesita componer sobre fondo dinámico

No usar sprite cuando:

- El dato es un número simple con clear rect acotado
- El layout ya es estable sin buffering extra
- El sprite solo añade coste de RAM sin reducir flicker

### 8.2 El sprite debe tener las mismas dimensiones que su región destino

```cpp
TFT_eSprite spr(&tft);
spr.createSprite(SPRITE_W, SPRITE_H);
// ... dibujar en sprite ...
spr.pushSprite(DEST_X, DEST_Y);
spr.deleteSprite();
```

### 8.3 El sprite se empuja siempre en la misma posición

La posición de push debe ser constante o depender de constantes definidas en `layout.h`. No calcularla en cada frame con lógica variable.

---

## 9. Pantalla nueva: checklist de arranque

Antes de escribir la primera línea de código de una pantalla nueva, definir y anotar:

```text
□ Dimensiones del card principal (X, Y, W, H)
□ Padding interior del card (CARD_PAD ≥ 2)
□ Límites internos calculados (INNER_X1, INNER_Y1, INNER_X2, INNER_Y2)
□ Zona del icono (icon_x, icon_y, icon_w, icon_h) dentro del card
□ Zona del valor principal (value_x, value_y, max_w)
□ Zona de la unidad (unit_x, unit_y)
□ Zona de la categoría/label (label_x, label_y)
□ Zona de la barra (bar_x, bar_y, bar_w, bar_h) si aplica
□ Zona del footer/hint (hint_y) dentro de [118, 127]
□ Lista de variables de caché necesarias (una por campo dinámico)
□ Lista de qué elementos van en shell y qué elementos son dinámicos
```

Solo después de responder todo lo anterior se empieza a codificar.

---

## 10. Checklist de validación antes de entregar una pantalla

### Flicker

- [ ] ¿La pantalla tiene shell estático separado del contenido dinámico?
- [ ] ¿Cada campo dinámico tiene su propia variable de caché?
- [ ] ¿El shell nunca se repinta cuando solo cambia un dato?
- [ ] ¿No hay `fillScreen()` fuera del bloque `screen_changed`?
- [ ] ¿Los clear rects están acotados al campo, no al ancho de pantalla?

### Límites y overflow

- [ ] ¿Todos los elementos están dentro del área interna del card (`INNER_X1..X2`, `INNER_Y1..Y2`)?
- [ ] ¿Los textos más largos posibles caben sin salirse?
- [ ] ¿El clear rect de cada campo está clampado a los límites del card?
- [ ] ¿La barra está contenida en su `BAR_X..BAR_X+BAR_W`?
- [ ] ¿El icono tiene zona exclusiva y no invade texto ni borde?

### Superposición

- [ ] ¿Ningún texto se superpone con otro texto?
- [ ] ¿Ningún elemento pisa una línea divisora o el borde del card?
- [ ] ¿La unidad y el valor tienen separación ≥ 2 px?
- [ ] ¿El footer/hint no invade el área de contenido (Y < 118)?
- [ ] ¿El alert jewel (Y=118, X=14) no está cubierto por otro elemento?

### Hardware

- [ ] ¿La pantalla no vibra al cambiar datos en hardware real?
- [ ] ¿Los textos son legibles a distancia y en movimiento?
- [ ] ¿Los bordes del card no se repintan en cada frame?

---

## 11. Regla de aceptación

Una pantalla TFT del P-Bit solo se considera lista cuando pasa el checklist completo de la sección 10. No antes.

Si una pantalla "se ve bien" en el visualizador pero vibra en hardware, **no está lista**.
Si una pantalla funciona pero algún elemento sale del card o pisa una línea, **no está lista**.
Si una pantalla depende de un `fillScreen()` para parecer estable, **no está lista**.
