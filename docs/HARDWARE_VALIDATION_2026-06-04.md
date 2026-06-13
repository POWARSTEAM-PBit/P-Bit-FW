# Validación HW P-Bit — sesión 2026-06-04

Checklist práctico para validar el firmware tras la auditoría profunda 2026-06-03. Sigue el orden propuesto; cada fase puede ejecutarse en una sesión corta. Marca `[ ]` → `[x]` al completar. Anota valores en los espacios `___`.

> **Fuente del firmware:** rama `main` en `C:\POWAR-GIT\P-Bit-FW - edit\`.
> **Build producción:** `py -m platformio run -e esp32dev` — RAM `14.9%` (`48924` bytes), Flash `72.1%` (`945565` bytes).
> **Build depuración (recomendada para esta sesión):** `py -m platformio run -e esp32dev_debug -t upload` — activa `FIRMWARE_DEBUG` y emite Stack HWM por Serial cada `60 s` o cuando empeora el peor caso. Ver `docs/TECHNICAL.md` § A.5.

### Recomendación para esta sesión

Usa **`esp32dev_debug`** para flashear. Mientras pruebas funcionalmente con el encoder, el firmware va emitiendo HWM por Serial automáticamente. Al final tienes:
- Validación funcional cubierta (este checklist).
- Mediciones de Stack HWM capturadas en el `.log` del monitor.
- Reset reasons capturadas si hay reinicios.

Tras la sesión, para volver a producción: reflashea con `py -m platformio run -e esp32dev -t upload`.

---

## Metadatos de la sesión

- Fecha de inicio: `___________`
- Operador: `___________`
- Unidad probada (serial/etiqueta): `___________`
- Cable USB usado (marca/calidad): `___________`
- Fuente de alimentación (PC USB / cargador X mA): `___________`
- Hash del commit flasheado (`git log -1 --oneline`): `c743fdd feat: add debug build environment esp32dev_debug with stack HWM instrumentation`
- Build flasheado: RAM `48964` bytes, Flash `947861` bytes
- Idioma seleccionado en boot: `Español por defecto; selector apareció en primer boot post-flash, pero se omitió tras reset sin confirmación`

---

## Pre-vuelo — Serial monitor en background (recomendado)

> No es obligatorio para validación funcional, pero **muy recomendado**: mientras tú haces las pruebas, el monitor captura automáticamente cualquier reinicio o crash a un archivo de log.
>
> **Recomendación:** usar la build `esp32dev_debug` para esta sesión. Con ese env activo, el firmware emite Stack HWM por Serial cada `60 s` o cuando el peor caso empeora. Si flasheas la build de producción (`esp32dev`) en vez de la debug, el log solo captura boot, reset reasons, y crashes — sigue siendo útil para detectar reinicios pero sin las muestras HWM.

### Identificar puerto COM del P-Bit

```powershell
py -m platformio device list
```

Localiza el dispositivo con descripción tipo `CP210x` o `CH340`. Anota el puerto: `COM___`

### Lanzar el monitor con captura a archivo

En un terminal PowerShell de VSCode separado (Ctrl+`), ejecuta:

```powershell
py -m platformio device monitor --baud 115200 --port COMx | Tee-Object -FilePath C:\POWAR-GIT\pbit-serial-2026-06-04.log
```

Sustituye `COMx` por tu puerto real. Verás el output en pantalla y simultáneamente se guarda en el `.log`. Mientras esto corre, puedes usar VSCode para todo lo demás.

- [x] Puerto COM identificado: `COM8 — USB-SERIAL CH340`
- [x] Comando ejecutado en terminal separado de VSCode.
- [x] Verificado primer arranque del ESP32 visible en Serial (`rst:0x..` y boot trace).
- [x] Reset reason del primer boot observado: `rst:0x1 (POWERON_RESET)` / firmware `Reset reason: 1`

### Al terminar la sesión

- Ctrl+C en el terminal del monitor para cerrarlo.
- El archivo `.log` queda en `C:\POWAR-GIT\pbit-serial-2026-06-04.log`.
- Abrir el `.log` en VSCode y buscar (`Ctrl+F`) ocurrencias de `rst:` — anotar cada una en la sección "Reinicios inesperados" más abajo.

### Alternativa más simple

Si prefieres no ver el output en pantalla:

```powershell
py -m platformio device monitor --baud 115200 --port COMx > C:\POWAR-GIT\pbit-serial-2026-06-04.log 2>&1
```

El terminal queda silencioso. Todo va directo al archivo.

---

## Fase 1 — Boot + navegación básica (~30 min)

### 1.1 Selector de idioma

Procedimiento: borrar NVS si fuese necesario (flashear de nuevo) y arrancar en frío.

- [ ] El selector aparece en frío con `Español` / `Catalán` / `English`.
- [ ] Encoder navega entre las 3 opciones, ciclo 0→1→2→0.
- [ ] Pulsar encoder confirma el idioma.
- [ ] Tras confirmar, la UI principal aparece con el idioma elegido.
- [ ] Reiniciar y comprobar que el idioma queda persistido (no vuelve a salir el selector).

Anomalías observadas: `2026-06-04: el selector de idioma apareció tras flashear, pero al reiniciar sin confirmar idioma ya no volvió a aparecer; la UI continuó en Inicio con idioma español por defecto. Posible edge case: fw_stamp queda guardado antes de confirmar idioma. Diagnosticar al final.`

### 1.2 Carrusel completo

> Orden esperado con `PBIT_ENABLE_GRAPH_LAB=1`:
> `HOME → CLIMA → MULTI → SONIDO VU → TEMPERATURA → HUMEDAD → LUZ → SONIDO → SUELO → TERMÓMETRO → TIMER → SISTEMA`
> 12 posiciones, circular.

- [ ] HOME se ve correctamente (cards 2×2 con T/H/L/S). **Anomalía:** valor de lux con más de 3 dígitos se monta sobre barra/card y, al volver a menos dígitos, deja fantasmas.
- [x] CLIMA se ve correctamente (LAB_DUAL_TH, temp + hum).
- [x] MULTI/TEMP LAB se ve correctamente. Nota visual menor: en card de DS18B20 desconectado, subir `--` y `Revisa IO33` aprox. `Y-3 px`.
- [x] SONIDO VU se ve correctamente (animación reactiva al micro). Nota: parpadeo leve con valores muy altos, posiblemente propio de la animación VU.
- [x] TEMPERATURA visible, datos plausibles. Vista con card superior y gráfica inferior se ve bien.
- [x] HUMEDAD visible, datos plausibles. Modos por pulsación: `Humedad → Hum Lab → Hum gráfica → Hum dial → Hum tarjeta → Humedad`.
- [x] LUZ visible, datos plausibles. Pantalla dedicada se ve bien.
- [x] SONIDO visible, datos plausibles.
- [x] SUELO visible sin sensor → `Revisa IO35` con estado atenuado/desaturado.
- [x] TERMÓMETRO visible sin sonda → `Revisa IO33`; se ve bien, con ajuste visual pendiente ya anotado.
- [x] TIMER visible.
- [x] SISTEMA visible. Se observan Idioma, uptime/tiempo corriendo, ID, Bip OFF y Alarmas OFF.
- [x] El carrusel da la vuelta completa (de SISTEMA → HOME).
- [ ] Ningún texto se sale del borde de la pantalla en ningún idioma probado.

Anomalías observadas: `HOME/Luz: valor lux de 4+ dígitos invade barra/card y no limpia completamente al reducir dígitos; probable clear dinámico insuficiente o ancho reservado demasiado pequeño.`

### 1.3 Modos de cada sensor (SENSOR_ZONE_SCREEN)

> Pulsación corta en cualquier sensor cicla:
> `FOCUS(0) → VALOR(1) → GRAPH(2) → GAUGE/DIAL(3) → CARD(4) → FOCUS(0)`

Para cada sensor (TEMP, HUM, LUZ, SONIDO, SUELO, DS18):

- [x] Pulsación corta cambia de modo. Modos cubiertos por sensor: `HUMEDAD: Humedad → Hum Lab → Hum gráfica → Hum dial → Hum tarjeta → Humedad`; `LDR/LUZ: rota por todas las pantallas/modos`; `SONIDO: rota por sus pantallas/modos correctamente`; `SONIDO LAB: cambia de pantalla/modo al presionar`
- [ ] Pulsación larga abre el menú clásico del sensor.
- [x] Cada modo redibuja sin flicker visible. Humedad probada con soplido al sensor: valores cambian bien, sin flicker/fantasmas.

Sensor con problema de flicker (si alguno): `___________`

### 1.4 Timer

- [x] Pulsación corta entra en modo cronómetro o cuenta atrás (según diseño).
- [ ] Pulsación larga abre menú del timer.
- [x] El dígito se actualiza sin flicker.
- [x] Reset del timer funciona.

Anomalías observadas: `Mejora visual menor: en estado LISTO, subir el texto aprox. 2 px (Y-2). Funcional OK: inicia, cambia color/estado, pausa con tonos rojizos e instrucciones actualizadas, continúa, se detiene y pulsación larga reinicia.`

---

## Fase 2 — Sensores físicos (~45 min)

### 2.1 DHT11 (temperatura ambiente + humedad)

- [x] Temperatura ambiente plausible (rango razonable según habitación).
- [x] Humedad ambiente plausible.
- [x] Soplar aire caliente: temperatura sube en pocos segundos.
- [x] Soplar aire húmedo: humedad sube en pocos segundos.
- [ ] Si DHT11 está desconectado: la pantalla muestra estado de fallo o valores `---`.

Lectura en condiciones controladas (si tienes referencia):
- T habitación medida con otro termómetro: `___ °C`
- T mostrada por P-Bit: `___ °C`
- H habitación medida con otro higrómetro: `___ %`
- H mostrada por P-Bit: `___ %`

### 2.2 LDR (luz)

- [x] Modo `lux` muestra valores en rango `0..8000`.
- [ ] Modo `FC` muestra valores derivados (`lux / 10.764`).
- [ ] Modo `raw` muestra valores ADC.
- [x] Tapar el LDR baja la lectura visiblemente.
- [x] Lámpara directa sube la lectura visiblemente.
- [ ] **RGB LED apagado en LIGHT_SCREEN** (no debe iluminarse — distorsiona la lectura del LDR).

Lectura comparada con luxómetro (si tienes):
- Lectura luxómetro: `___ lux`
- Lectura P-Bit: `___ lux`
- Diferencia razonable (sí/no): `___`

### 2.3 Micrófono

- [x] `SONIDO VU` reacciona al ruido del entorno.
- [x] Aplaudir/soplar cerca: barra/animación reacciona claramente.
- [ ] Silencio en habitación: barra/animación cae al mínimo.
- [ ] Modo SONIDO clásico también reacciona.

Anomalías observadas: `Micrófono funcional pero parece poco sensible/mal calibrado: reacciona más claramente al soplar que al hablar. Evaluar opción de calibrar "silencio" y "ruido" o ajustar thresholds/gain.`

### 2.4 Sensor de Suelo (IO35)

Procedimiento: probar con sensor desconectado, después conectarlo en caliente.

- [x] Sin sensor conectado: pantalla muestra `Revisa IO35` con paleta atenuada (no gris plano).
- [x] Conectar sensor en caliente: aparece **splash verde** `CONECTADO / Sensor Suelo / IO35` durante ~1.5 s.
- [x] Tras el splash, la pantalla muestra valor de humedad de suelo.
- [x] Desconectar el sensor en caliente: aparece **splash rojo** `DESCONECTADO / Sensor Suelo / IO35` durante ~1.5 s.
- [ ] Tras el splash, vuelve a estado `Revisa IO35`.
- [ ] Splash NO aparece en boot (solo en cambios de estado posteriores).
- [ ] Splash NO aparece durante Demo Mode.

Calibración:
- [x] Menú largo desde sensor SUELO abre menú de calibración.
- [ ] Calibración seco (sensor al aire) registra valor.
- [ ] Calibración mojado (sensor en agua) registra valor.
- [ ] Límites `seco / óptimo / húmedo` editables y persistentes.

Anomalías observadas: `Ajuste visual solicitado para estado desconectado en Suelo y DS18B20: card superior solo "Sin Sensor", con color más vivo/entonado a la paleta y Y+2; card inferior solo "Revisa IOxx" y Y-4 para centrar mejor. Valores de suelo se ven plausibles y correctos. BUG render/UX bloqueante: pantalla/menú de calibración de Suelo genera mucho flicker cada vez que cambian valores, y cambian constantemente; afecta todo el cuerpo salvo título/línea. Al cargar el menú de configuración/calibración no dibuja fondo propio, queda el fondo de la pantalla del sensor; otros menús sí tienen fondo. No hay salida/cancelación segura sin guardar: para salir hay que seleccionar obligatoriamente el valor a calibrar y luego el siguiente, quedando guardados; Pablo tuvo que resetear para no guardar. Propuesta: pulsación larga vuelve al menú anterior conservando calibración previa para evitar reset si se entra o calibra mal. Pendiente posterior: calibración general con raw al aire y sumergido en agua en ~5 P-Bits para promediar defaults de firmware.`

### 2.5 Termómetro DS18B20 (IO33)

Procedimiento: probar con sonda desconectada, después conectarla en caliente.

- [x] Sin sonda conectada: pantalla muestra `Revisa IO33` con paleta atenuada.
- [x] Conectar sonda en caliente: aparece **splash verde** `CONECTADO / Sensor DS18B20 / IO33` durante ~1.5 s.
- [x] Tras conectar sonda, muestra temperatura correctamente.
- [x] Desconectar sonda: aparece **splash rojo** `DESCONECTADO / Sensor DS18B20 / IO33` durante ~1.5 s.
- [x] Tras el splash, vuelve a `Revisa IO33`.
- [ ] Splash NO aparece en boot.
- [x] Lectura plausible (T ambiente o T agua tibia según prueba). Conecta bien y muestra splash verde.

Anomalías observadas: `Mejora visual menor: en estado DS18B20 desconectado dentro de Temp Lab, subir "--" y "Revisa IO33" aprox. 3 px. Con sonda conectada, visual y lectura OK.`

### 2.6 Alertas visuales/RGB/Audio

Procedimiento: en al menos un sensor crítico, forzar condición de alerta.

Sensor elegido para probar: `___________`

- [ ] Al cruzar el umbral configurado, el RGB LED cambia de color según la severidad.
- [ ] El alert jewel (esquina inferior derecha) cambia de color.
- [ ] Si `Alarmas ON`: se oye un beep al disparar la alerta.
- [ ] Si `Alarmas OFF`: no se oye beep pero el visual sí cambia.

Anomalías observadas: `___________`

---

## Fase 3 — Sistema y persistencia (~20 min)

### 3.1 Bip vs Alarmas

> `Bip` = beeps de UI (rotary, navegación, confirmaciones).
> `Alarmas` = audio de alertas + timer.

- [x] `Sistema > Bip OFF` silencia beeps de UI (rotary, navegación). Observado inicialmente OFF; pulsar en Sistema activa Bip correctamente y volver a pulsar lo apaga.
- [ ] Con `Bip OFF`, `Alarmas ON`: alertas siguen sonando. Visualmente `Alarmas` queda ON al activarla.
- [x] Con `Bip ON`, `Alarmas OFF`: navegación sigue beep, pero alertas y timer mudos. Bip toggle funciona ON/OFF y persiste tras reset.
- [ ] Visual de alerta (RGB + jewel) **siempre activo** independiente de `Alarmas`.

Anomalías/observaciones: `Bip y Alarmas aparecieron OFF tras flashear debug; Pablo no recuerda haberlos apagado. Luego Bip y Alarmas se activaron y persistieron tras reset, por lo que NVS de estos toggles funciona. Revisar al final si OFF inicial era default intencional o efecto del reset por build-hash.`

### 3.2 Unidad Celsius / Fahrenheit

- [x] Cambiar a Fahrenheit afecta TEMPERATURA y TERMÓMETRO simultáneamente, y se propaga a otras pantallas donde aparece temperatura.
- [x] Reiniciar el P-Bit: la unidad persiste (NVS `sys_unit_f`).
- [x] Volver a Celsius funciona igual y persiste.

### 3.3 Reposo

- [x] Sin tocar el encoder durante el tiempo configurado, la pantalla entra en modo reposo (mensaje `ZZZ` visible o pantalla apagada). Serial: `[Power] Entering IDLE mode.`
- [x] Tocar el encoder despierta el P-Bit a la pantalla previa. Serial: `[Power] Leaving IDLE mode.`
- [x] Demo Mode NO entra en reposo mientras está activo. Probado con sleep configurado a 30 s durante varios minutos.

### 3.4 Idioma

- [x] Cambiar idioma desde `Sistema > Idioma` aplica el cambio sin reinicio (full redraw). Probado en los 3 idiomas.
- [x] Reiniciar: el idioma persiste.
- [x] Textos en CAT y EN no se solapan ni se salen del borde en las pantallas probadas.

### 3.5 Reset

- [ ] `Sistema > Reset` pide confirmación.
- [ ] Confirmar: limpia NVS y reinicia mostrando selector de idioma.
- [ ] Reset general devuelve todo al estado inicial de firmware: pantallas principales seleccionadas en todos los sensores, idioma/selector inicial, Bip/Alarmas/defaults, unidades, reposo, calibraciones y demás valores NVS.

Observaciones Reset: `Parece limpiar también idioma/NVS, pero el selector se ve correctamente tras forzar un reinicio adicional. Revisar si, tras Reset general desde Sistema, conviene forzar reinicio completo tipo BLE para que el estado inicial sea visible inmediatamente.`

### 3.6 Demo Mode

- [ ] Encender el P-Bit manteniendo el encoder pulsado **durante el logo de boot**: entra en Demo Mode.
- [ ] Pulsación larga desde `HOME`: también activa Demo Mode.
- [ ] Splash breve `Modo Demo` al entrar.
- [x] Sensores cambian con coreografía smooth.
- [ ] Gráficas dibujan datos sintéticos.
- [x] Cualquier interacción del encoder sale del Demo Mode.
- [x] Demo Mode no persiste tras reinicio.
- [x] No se ven flicker ni transiciones bruscas evidentes en la coreografía.

Anomalías observadas en Demo Mode: `Demo Mode visualmente está muy bien. Hallazgos: Timer se ve parado, debería simular actividad aunque sea falsa; Termo Lab/DS18 gráfica no muestra valores; al salir de Demo Mode quedan persistidas/seleccionadas las pantallas/modos por los que pasó el demo, y no debería afectar las selecciones principales reales del usuario. Demo Mode debe ser transitorio y restaurar modos/pantallas previos.`

---

## Fase 4 — BLE (~15 min)

### 4.1 Estado de fábrica — BLE OFF

> El P-Bit sale `ble_en=false`. Cada nuevo flash limpia NVS y resetea a `false`.

- [ ] Escanear con app BLE externa (nRF Connect, LightBlue, etc.): **NO debe aparecer** publicidad `PBIT-XXXX`.
- [ ] La fila `BLE` **NO aparece** en la pantalla `Sistema`.

App de escaneo usada: `___________`

### 4.2 Activar BLE — gesto secreto

> Mantener encoder pulsado **30 segundos** estando en SISTEMA activa la pantalla oculta BLE.

- [ ] Mantener 30 s el encoder en SISTEMA hace aparecer la pantalla `BLE Toggle`. Observación: actualmente parece tardar ~60 s; objetivo acordado: 30 s.
- [x] Activar BLE → la fila `BLE` aparece ahora en SISTEMA. Al seleccionar Bluetooth se reinicia, comportamiento correcto.
- [x] Escanear: **AHORA SÍ** debe aparecer el dispositivo como `PBIT-XXXX` por nombre (valida el fix de scan response del commit `2a51810`).
- [x] Conectar desde la app BLE: la conexión se establece o queda disponible para conexión desde el celular.
- [ ] Una vez conectado, recibir lecturas de sensores periódicamente.

Nombre del dispositivo visto en el escáner: `___________`

### 4.3 Desactivar BLE

- [x] Volver a desactivar BLE desde la pantalla oculta.
- [x] Reiniciar: el escaneo NO ve `PBIT-XXXX`.
- [x] La fila BLE desaparece de SISTEMA.

Anomalías observadas: `___________`

---

## Anomalías globales observadas

### Reinicios inesperados durante la sesión

| Cuándo | Estado del P-Bit antes del reinicio | Reset reason en Serial (si capturado) |
|---|---|---|
| `Pre-vuelo` | `Monitor serial reportó PermissionError(13) y reconectó; no se observó crash firmware asociado` | `No aplica — reconexión del monitor, después boot normal rst:0x1` |
| `Durante calibración Suelo` | `Usuario reseteó manualmente porque no había cancelación sin guardar; tras reset vuelve a HOME/Inicio` | `Reset manual; sin evidencia de crash` |
| `___________` | `___________` | `___________` |

> Reset reasons frecuentes:
> - `rst:0x1` = power-on normal
> - `rst:0x3` o `0xC` = software (`esp_restart`)
> - `rst:0x4 / 0x7 / 0x8` = watchdog timeout (problema)
> - `rst:0x6` = stack overflow (problema)
> - `rst:0xF` = brownout (alimentación insuficiente)

### Stack HWM capturados durante la sesión

> Solo aplica si flasheaste con `esp32dev_debug` y tenías el Serial monitor capturando. Busca en el `.log` líneas tipo `[Stack] DisplayTask HWM free: X bytes (worst: Y)`.

- DisplayTask — HWM free observado al inicio: `2272 bytes`
- DisplayTask — HWM free observado al final: `2080 bytes` (última/peor muestra observada en log)
- DisplayTask — peor caso (worst) observado: `2080 bytes`
- SensorTask — HWM free observado al inicio: `3152 bytes`
- SensorTask — HWM free observado al final: `2572 bytes` (última muestra vista hasta ahora)
- SensorTask — peor caso (worst) observado: `2440 bytes` antes de reset manual; `2564 bytes` tras último boot observado

> Interpretación rápida:
> - `> ~2400 bytes libres` (≥60% del stack 4096): holgado.
> - `~1200..2400`: razonable.
> - `~400..1200`: apretado, considerar ampliar stack.
> - `< 400`: ampliar stack ya.

### Otras observaciones

- Texto recortado / solapado: `___________`
- Flicker visible: `HOME/Luz deja fantasmas con lux de 4+ dígitos; menú/calibración Suelo genera mucho flicker en todo el cuerpo salvo título/línea al actualizar valores constantes; SONIDO VU tiene parpadeo leve con valores altos.`
- Color/contraste raro: `Estados desconectados de Suelo/DS18: usar "Sin Sensor" más vivo/entonado en card superior.`
- Comportamientos inesperados: `Selector de idioma no reaparece tras reset sin confirmar en primer boot post-flash; Bip/Alarmas arrancaron OFF.`
- Pulido visual transversal: `En cards principales de todos los sensores, centrar horizontalmente icono, nombre del sensor (ej. "Aire") y valor+unidad (%/lux/etc.).`
- Pulido visual LDR: `En LDR dial, aplicar oscurecido más real del ícono: a 0 lux que desaparezca o se vea gris; aumentar brillo progresivamente con la luz, similar al gauge.`
- Mejora funcional sonido: `Micrófono parece poco sensible a voz normal; evaluar calibración de silencio/ruido o ajuste de thresholds/gain.`
- Calibración producción Suelo: `Tomar raw seco/aire y raw mojado/agua en varias unidades (~5 P-Bits), promediar y actualizar defaults de firmware si conviene.`
- UX calibración Suelo: `Menú debe dibujar fondo propio al entrar y ofrecer cancelación sin guardar, por ejemplo pulsación larga para volver conservando calibración anterior.`
- Bloqueante producción: `Calibración Suelo no permite salir sin guardar; obliga a capturar pasos de calibración y guardar, o resetear.`
- Análisis futuro i18n: `Evaluar facilidad y coste de memoria/flash de añadir más idiomas (francés, chino, alemán, etc.) antes de ampliar languages.h.`
- Demo Mode: `No debe persistir ni contaminar selección de pantallas/modos al salir; Timer debería simular actividad; Termo Lab/DS18 debe alimentar gráfica sintética.`
- Reset general: `Debe restaurar estado inicial de firmware completo: modos/pantallas principales por sensor, idioma/selector inicial, Bip/Alarmas/defaults, unidad, sleep, calibraciones y todos los valores NVS.`
- Validación prolongada pendiente: `Dejar P-Bit conectado corriendo 24 h con esp32dev_debug y Serial log activo para detectar resets, WDT/brownout, drift de HWM o degradación visual.`

---

## Veredicto final

- Fase 1 (Boot + navegación): ✅ OK / ⚠️ Con notas / ❌ Bloquea — `___________`
- Fase 2 (Sensores físicos): ✅ OK / ⚠️ Con notas / ❌ Bloquea — `___________`
- Fase 3 (Sistema y persistencia): ✅ OK / ⚠️ Con notas / ❌ Bloquea — `___________`
- Fase 4 (BLE): ✅ OK / ⚠️ Con notas / ❌ Bloquea — `___________`

### Decisión

- [ ] Firmware aprobado para producción → marcar checkboxes correspondientes en `docs/PRODUCTION_CHECKLIST.md` y actualizar fecha.
- [ ] Hay anomalías menores → documentar en el ROADMAP como deuda y seguir adelante.
- [ ] Hay anomalías bloqueantes → traer este checklist completo a Claude/Codex para análisis y plan de fix.
- [ ] Prueba prolongada 24 h pendiente con `esp32dev_debug` + Serial log.

### Si hay que escalar

Copia esta sección y pásala a Claude:

```
Estoy de vuelta tras validación HW del firmware (commit ___________).
Fases superadas: ___________.
Bloqueantes: ___________.
Reinicios observados: ___________.
Notas adicionales: ___________.
Adjunto este checklist completado.
```

---

> Este documento no necesita commitearse. Se mantiene como anotación de sesión. Si se commitea, hacerlo con mensaje `docs: validación HW 2026-06-04 [estado]`.
