#pragma once
// P-Bit Layout System — 160x128 landscape (rotation=1)
// Grid: 4px

// ── Global ───────────────────────────────────────────────
constexpr int L_HEADER_Y    = 2;    // Posicion Y del titulo principal de cada pantalla. Cambiarlo sube o baja TODOS los headers: por ejemplo "TEMPERATURA", "SONIDO", "INFO SISTEMA" o "TEMPORIZADOR".
constexpr int L_HEADER_LINE = 28;   // Posicion Y de la linea horizontal bajo el titulo. Si la bajas, dejas mas aire entre titulo y linea; si la subes, el bloque de cabecera queda mas compacto.
constexpr int L_CONTENT_TOP = 32;   // Primera Y util del contenido debajo del header. Sirve como referencia para donde empiezan tanques, tarjetas y bloques de datos.
constexpr int L_MARGIN_SIDE = 10;  // Margen lateral comun. Afecta a elementos horizontales centrados como la linea del header y las barras de luz/sonido.

// ── Familia A: tank vertical (Temp, Humidity, Soil, DS18) ─
constexpr int LA_LEFT_CX    = 60;   // Centro horizontal del panel izquierdo en pantallas con tanque. Mueve el centrado de hints, numeros grandes y textos inferiores en TEMP, HUMEDAD, SUELO y TERMOMETRO.
constexpr int LA_HINT_Y     = 34;   // Posicion Y del texto pequeño de ayuda. Ejemplos: "Pulsa > F" o "Push > C". Subirlo lo acerca al header; bajarlo lo acerca al valor grande.
constexpr int LA_VALUE_TOP  = 50;   // Posicion Y superior del valor principal grande. Afecta a textos como "24.5", "63%" o "---" en todas las pantallas de esta familia.
constexpr int LA_UNIT_Y     = 92;   // Reserva historica para unidades o textos medios. Ahora casi no se usa directamente, pero sigue siendo util como referencia para futuros ajustes.
constexpr int LA_CATEGORY_Y = 108;  // Posicion Y del texto inferior del panel izquierdo. Afecta a categorias o unidades como "Óptimo", "Celsius", "No sensor", etc.
constexpr int LA_TANK_X     = 120;  // Posicion X izquierda del tanque vertical. Si la cambias, acercas o alejas el tanque del bloque de texto de la izquierda.
constexpr int LA_TANK_Y     = 36;   // Posicion Y superior del tanque vertical. Mueve hacia arriba o abajo el "deposito" o "termometro" grafico.
constexpr int LA_TANK_W     = 28;   // Ancho del tanque vertical. Aumentarlo lo hace mas protagonista; reducirlo deja mas espacio visual entre texto y tanque.
constexpr int LA_TANK_H     = 88;   // Alto del tanque vertical. Cambia cuanto ocupa el grafico y tambien el margen inferior que deja respecto al borde de pantalla.

// ── Familia B: barra horizontal (Sound, Light) ───────────
constexpr int LB_VALUE_TOP  = 38;   // Posicion Y superior del valor grande en pantallas de barra. Ejemplos: lux en LIGHT o porcentaje en SOUND.
constexpr int LB_BAR_X      = 20;   // Posicion X izquierda de la barra horizontal. Normalmente coincide con el margen lateral global para que la composicion quede equilibrada.
constexpr int LB_BAR_Y      = 91;   // Posicion Y superior de la barra. Si la subes, la barra queda mas cerca del valor; si la bajas, queda mas separada.
constexpr int LB_BAR_W      = 120;  // Ancho de la barra horizontal. Cambia la longitud visible de la escala en pantallas como LIGHT y SOUND.
constexpr int LB_BAR_H      = 14;   // Alto de la barra horizontal. Si lo aumentas, la barra pesa mas visualmente; si lo reduces, se ve mas fina.
constexpr int LB_CATEGORY_Y = 114;  // Posicion Y del texto de categoria bajo la barra. Ejemplos: "Bright", "Ruidoso", "Luz solar".

// ── Timer ────────────────────────────────────────────────
constexpr int LT_HINT_Y     = 32;   // Posicion Y de la instruccion del timer sobre la tarjeta. Ejemplos: "Pulsa > Iniciar" o "Hold > Rst | Push > Run".
constexpr int LT_CARD_X     = 15;   // Posicion X izquierda de la tarjeta del temporizador. Cambia el margen lateral del card completo.
constexpr int LT_CARD_Y     = 50;   // Posicion Y superior de la tarjeta del temporizador. Si la bajas, separas mas la instruccion del card; si la subes, compactas el bloque.
constexpr int LT_CARD_W     = 130;  // Ancho de la tarjeta del temporizador. Afecta al marco que contiene el estado y el tiempo.
constexpr int LT_CARD_H     = 68;   // Alto de la tarjeta del temporizador. Cambiarlo altera el espacio vertical disponible para estado, tiempo y respiracion interna.
constexpr int LT_STATE_Y    = 64;   // Posicion Y del texto de estado dentro del card. Ejemplos: "Listo", "Corriendo" o "Pausado".
constexpr int LT_TIME_Y     = 90;   // Posicion Y del tiempo grande dentro del card. Mueve solo el cronometro sin tocar el header ni la instruccion.

// ── System Info ──────────────────────────────────────────
constexpr int LS_CARD_X     = 10;   // Posicion X izquierda del card de informacion del sistema. Mueve el marco que contiene DEV, UP, BLE y LAN.
constexpr int LS_CARD_Y     = 36;   // Posicion Y superior del card de informacion del sistema. Si la cambias, desplazas todo el bloque de filas.
constexpr int LS_CARD_W     = 140;  // Ancho del card de informacion del sistema. Cambia el espacio horizontal disponible para etiquetas y valores.
constexpr int LS_CARD_H     = 68;   // Alto del card de informacion del sistema. Afecta la respiracion vertical interna entre sus filas.
constexpr int LS_LABEL_X    = 20;   // Posicion X de las etiquetas fijas de la izquierda: "DEV", "UP", "BLE" y "LAN".
constexpr int LS_VALUE_X    = 52;   // Posicion X de los valores de la derecha: nombre del dispositivo, uptime, estado BLE e idioma.
constexpr int LS_ROW_DEV    = 40;   // Posicion Y de la fila DEV. Solo afecta a esa linea dentro de System Info.
constexpr int LS_ROW_UP     = 54;   // Posicion Y de la fila UP. Mueve el uptime sin tocar las otras filas.
constexpr int LS_ROW_BLE    = 68;   // Posicion Y de la fila BLE. Mueve el texto "Connected/Disconnected" o su traduccion equivalente.
constexpr int LS_ROW_LAN    = 82;   // Posicion Y de la fila LAN. Mueve el codigo de idioma: ESP, CAT o ENG.
constexpr int LS_FOOTER_Y   = 108;  // Posicion Y del texto inferior fuera del card. Ejemplo: "Sonido: ON/OFF (Pulsa)" en la parte baja de System Info.
