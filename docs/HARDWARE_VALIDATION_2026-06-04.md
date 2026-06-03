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
- Hash del commit flasheado (`git log -1 --oneline`): `___________`
- Build flasheado: RAM `_____` bytes, Flash `_____` bytes
- Idioma seleccionado en boot: `___________`

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

- [ ] Puerto COM identificado: `___________`
- [ ] Comando ejecutado en terminal separado de VSCode.
- [ ] Verificado primer arranque del ESP32 visible en Serial (`rst:0x..` y boot trace).
- [ ] Reset reason del primer boot observado: `___________`

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

Anomalías observadas: `___________`

### 1.2 Carrusel completo

> Orden esperado con `PBIT_ENABLE_GRAPH_LAB=1`:
> `HOME → CLIMA → MULTI → SONIDO VU → TEMPERATURA → HUMEDAD → LUZ → SONIDO → SUELO → TERMÓMETRO → TIMER → SISTEMA`
> 12 posiciones, circular.

- [ ] HOME se ve correctamente (cards 2×2 con T/H/L/S).
- [ ] CLIMA se ve correctamente (LAB_DUAL_TH, temp + hum).
- [ ] MULTI se ve correctamente (LAB_HOME_CARDS o equivalente).
- [ ] SONIDO VU se ve correctamente (animación reactiva al micro).
- [ ] TEMPERATURA visible, datos plausibles.
- [ ] HUMEDAD visible, datos plausibles.
- [ ] LUZ visible, datos plausibles.
- [ ] SONIDO visible, datos plausibles.
- [ ] SUELO visible (con sensor → datos; sin sensor → `Revisa IO35`).
- [ ] TERMÓMETRO visible (con sonda → datos; sin sonda → `Revisa IO33`).
- [ ] TIMER visible.
- [ ] SISTEMA visible.
- [ ] El carrusel da la vuelta completa (de SISTEMA → HOME).
- [ ] Ningún texto se sale del borde de la pantalla en ningún idioma probado.

Anomalías observadas: `___________`

### 1.3 Modos de cada sensor (SENSOR_ZONE_SCREEN)

> Pulsación corta en cualquier sensor cicla:
> `FOCUS(0) → VALOR(1) → GRAPH(2) → GAUGE/DIAL(3) → CARD(4) → FOCUS(0)`

Para cada sensor (TEMP, HUM, LUZ, SONIDO, SUELO, DS18):

- [ ] Pulsación corta cambia de modo. Modos cubiertos por sensor: `___________`
- [ ] Pulsación larga abre el menú clásico del sensor.
- [ ] Cada modo redibuja sin flicker visible.

Sensor con problema de flicker (si alguno): `___________`

### 1.4 Timer

- [ ] Pulsación corta entra en modo cronómetro o cuenta atrás (según diseño).
- [ ] Pulsación larga abre menú del timer.
- [ ] El dígito se actualiza sin flicker.
- [ ] Reset del timer funciona.

Anomalías observadas: `___________`

---

## Fase 2 — Sensores físicos (~45 min)

### 2.1 DHT11 (temperatura ambiente + humedad)

- [ ] Temperatura ambiente plausible (rango razonable según habitación).
- [ ] Humedad ambiente plausible.
- [ ] Soplar aire caliente: temperatura sube en pocos segundos.
- [ ] Soplar aire húmedo: humedad sube en pocos segundos.
- [ ] Si DHT11 está desconectado: la pantalla muestra estado de fallo o valores `---`.

Lectura en condiciones controladas (si tienes referencia):
- T habitación medida con otro termómetro: `___ °C`
- T mostrada por P-Bit: `___ °C`
- H habitación medida con otro higrómetro: `___ %`
- H mostrada por P-Bit: `___ %`

### 2.2 LDR (luz)

- [ ] Modo `lux` muestra valores en rango `0..8000`.
- [ ] Modo `FC` muestra valores derivados (`lux / 10.764`).
- [ ] Modo `raw` muestra valores ADC.
- [ ] Tapar el LDR baja la lectura visiblemente.
- [ ] Lámpara directa sube la lectura visiblemente.
- [ ] **RGB LED apagado en LIGHT_SCREEN** (no debe iluminarse — distorsiona la lectura del LDR).

Lectura comparada con luxómetro (si tienes):
- Lectura luxómetro: `___ lux`
- Lectura P-Bit: `___ lux`
- Diferencia razonable (sí/no): `___`

### 2.3 Micrófono

- [ ] `SONIDO VU` reacciona al ruido del entorno.
- [ ] Aplaudir cerca: barra/animación reacciona claramente.
- [ ] Silencio en habitación: barra/animación cae al mínimo.
- [ ] Modo SONIDO clásico también reacciona.

Anomalías observadas: `___________`

### 2.4 Sensor de Suelo (IO35)

Procedimiento: probar con sensor desconectado, después conectarlo en caliente.

- [ ] Sin sensor conectado: pantalla muestra `Revisa IO35` con paleta atenuada (no gris plano).
- [ ] Conectar sensor en caliente: aparece **splash verde** `CONECTADO / Sensor Suelo / IO35` durante ~1.5 s.
- [ ] Tras el splash, la pantalla muestra valor de humedad de suelo.
- [ ] Desconectar el sensor en caliente: aparece **splash rojo** `DESCONECTADO / Sensor Suelo / IO35` durante ~1.5 s.
- [ ] Tras el splash, vuelve a estado `Revisa IO35`.
- [ ] Splash NO aparece en boot (solo en cambios de estado posteriores).
- [ ] Splash NO aparece durante Demo Mode.

Calibración:
- [ ] Menú largo desde sensor SUELO abre menú de calibración.
- [ ] Calibración seco (sensor al aire) registra valor.
- [ ] Calibración mojado (sensor en agua) registra valor.
- [ ] Umbrales `seco / óptimo / húmedo` editables y persistentes.

Anomalías observadas: `___________`

### 2.5 Termómetro DS18B20 (IO33)

Procedimiento: probar con sonda desconectada, después conectarla en caliente.

- [ ] Sin sonda conectada: pantalla muestra `Revisa IO33` con paleta atenuada.
- [ ] Conectar sonda en caliente: aparece **splash verde** `CONECTADO / Sensor DS18B20 / IO33` durante ~1.5 s.
- [ ] Tras el splash, muestra temperatura.
- [ ] Desconectar sonda: aparece **splash rojo** `DESCONECTADO / Sensor DS18B20 / IO33` durante ~1.5 s.
- [ ] Tras el splash, vuelve a `Revisa IO33`.
- [ ] Splash NO aparece en boot.
- [ ] Lectura plausible (T ambiente o T agua tibia según prueba).

Anomalías observadas: `___________`

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

- [ ] `Sistema > Bip OFF` silencia beeps de UI (rotary, navegación).
- [ ] Con `Bip OFF`, `Alarmas ON`: alertas siguen sonando.
- [ ] Con `Bip ON`, `Alarmas OFF`: navegación sigue beep, pero alertas y timer mudos.
- [ ] Visual de alerta (RGB + jewel) **siempre activo** independiente de `Alarmas`.

### 3.2 Unidad Celsius / Fahrenheit

- [ ] Cambiar a Fahrenheit afecta TEMPERATURA y TERMÓMETRO simultáneamente.
- [ ] Reiniciar el P-Bit: la unidad persiste (NVS `sys_unit_f`).
- [ ] Volver a Celsius funciona igual y persiste.

### 3.3 Reposo

- [ ] Sin tocar el encoder durante el tiempo configurado, la pantalla entra en modo reposo (mensaje `ZZZ` visible o pantalla apagada).
- [ ] Tocar el encoder despierta el P-Bit a la pantalla previa.
- [ ] Demo Mode NO entra en reposo mientras está activo.

### 3.4 Idioma

- [ ] Cambiar idioma desde `Sistema > Idioma` aplica el cambio sin reinicio (full redraw).
- [ ] Reiniciar: el idioma persiste.
- [ ] Textos en CAT y EN no se solapan ni se salen del borde en ninguna pantalla probada.

### 3.5 Reset

- [ ] `Sistema > Reset` pide confirmación.
- [ ] Confirmar: limpia NVS y reinicia mostrando selector de idioma.

### 3.6 Demo Mode

- [ ] Encender el P-Bit manteniendo el encoder pulsado **durante el logo de boot**: entra en Demo Mode.
- [ ] Pulsación larga desde `HOME`: también activa Demo Mode.
- [ ] Splash breve `Modo Demo` al entrar.
- [ ] Sensores cambian con coreografía smooth.
- [ ] Gráficas dibujan datos sintéticos.
- [ ] Cualquier interacción del encoder sale del Demo Mode.
- [ ] Demo Mode no persiste tras reinicio.
- [ ] No se ven flicker ni transiciones bruscas evidentes en la coreografía.

Anomalías observadas en Demo Mode: `___________`

---

## Fase 4 — BLE (~15 min)

### 4.1 Estado de fábrica — BLE OFF

> El P-Bit sale `ble_en=false`. Cada nuevo flash limpia NVS y resetea a `false`.

- [ ] Escanear con app BLE externa (nRF Connect, LightBlue, etc.): **NO debe aparecer** publicidad `PBIT-XXXX`.
- [ ] La fila `BLE` **NO aparece** en la pantalla `Sistema`.

App de escaneo usada: `___________`

### 4.2 Activar BLE — gesto secreto

> Mantener encoder pulsado **30 segundos** estando en SISTEMA activa la pantalla oculta BLE.

- [ ] Mantener 30 s el encoder en SISTEMA hace aparecer la pantalla `BLE Toggle`.
- [ ] Activar BLE → la fila `BLE` aparece ahora en SISTEMA.
- [ ] Escanear: **AHORA SÍ** debe aparecer el dispositivo como `PBIT-XXXX` por nombre (valida el fix de scan response del commit `2a51810`).
- [ ] Conectar desde la app BLE: la conexión se establece.
- [ ] Una vez conectado, recibir lecturas de sensores periódicamente.

Nombre del dispositivo visto en el escáner: `___________`

### 4.3 Desactivar BLE

- [ ] Volver a desactivar BLE desde la pantalla oculta.
- [ ] Reiniciar: el escaneo NO ve `PBIT-XXXX`.
- [ ] La fila BLE desaparece de SISTEMA.

Anomalías observadas: `___________`

---

## Anomalías globales observadas

### Reinicios inesperados durante la sesión

| Cuándo | Estado del P-Bit antes del reinicio | Reset reason en Serial (si capturado) |
|---|---|---|
| `___________` | `___________` | `___________` |
| `___________` | `___________` | `___________` |
| `___________` | `___________` | `___________` |

> Reset reasons frecuentes:
> - `rst:0x1` = power-on normal
> - `rst:0x3` o `0xC` = software (`esp_restart`)
> - `rst:0x4 / 0x7 / 0x8` = watchdog timeout (problema)
> - `rst:0x6` = stack overflow (problema)
> - `rst:0xF` = brownout (alimentación insuficiente)

### Stack HWM capturados durante la sesión

> Solo aplica si flasheaste con `esp32dev_debug` y tenías el Serial monitor capturando. Busca en el `.log` líneas tipo `[Stack] DisplayTask HWM free: X bytes (worst: Y)`.

- DisplayTask — HWM free observado al inicio: `___ bytes`
- DisplayTask — HWM free observado al final: `___ bytes`
- DisplayTask — peor caso (worst) observado: `___ bytes`
- SensorTask — HWM free observado al inicio: `___ bytes`
- SensorTask — HWM free observado al final: `___ bytes`
- SensorTask — peor caso (worst) observado: `___ bytes`

> Interpretación rápida:
> - `> ~2400 bytes libres` (≥60% del stack 4096): holgado.
> - `~1200..2400`: razonable.
> - `~400..1200`: apretado, considerar ampliar stack.
> - `< 400`: ampliar stack ya.

### Otras observaciones

- Texto recortado / solapado: `___________`
- Flicker visible: `___________`
- Color/contraste raro: `___________`
- Comportamientos inesperados: `___________`

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
