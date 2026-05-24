# P-Bit Firmware

Firmware PlatformIO/Arduino para el P-Bit: placa educativa ambiental basada en ESP32 con TFT ST7735, encoder, LED RGB, buzzer, sensores ambientales y BLE opcional.

## Estado Actual

- Sensores: DHT11, LDR, micrófono analógico, humedad de suelo capacitiva y DS18B20.
- Build OK `esp32dev`: compilación local correcta con PlatformIO.
- Estado de compilación actual: `PBIT_ENABLE_GRAPH_LAB=1` y `PBIT_ENABLE_SERIAL_PLOTTER=0`.
- UI principal: carrusel con `Home`, `Clima`, `Multi`, `Sonido VU`, seis posiciones de sensor, `Timer` y `Sistema`.
- La familia de pantallas Lab incluye vistas como `Home Cards`, `Linear Dash` y `Sound VU`; el carrusel de usuario actual usa las vistas Lab estabilizadas para `Home`, `Clima`, `Multi` y `Sonido VU`.
- Con ese flag activo, las seis posiciones de sensor usan `SENSOR_ZONE_SCREEN`; la pulsación corta cambia el modo visual del sensor: `Focus`, `Valor`, `Gráfica`, `Dial` y `Card`.
- `GRAPH_SCREEN` existe en código, pero en el carrusel actual la gráfica se usa como modo por sensor dentro de `SENSOR_ZONE_SCREEN`.
- i18n centralizado para Español, Catalán e Inglés. El idioma se elige en el primer arranque y puede cambiarse desde `Sistema`; el cambio afecta a la interfaz completa.
- LDR corregido a la polaridad de la placa actual y limitado en firmware al rango `0..20000 lux`.
- `Sistema` separa `Bip` para beeps de UI y `Alarmas` para alertas/timer audibles.
- BLE está oculto y desactivado por defecto de fábrica; su activación queda documentada solo en material técnico interno.
- `Termómetro` usa `DS18B20` como identificador técnico; su icono de sonda sigue marcado como no final para revisión visual.
- La UI usa refresco acotado y cachés de dibujo para reducir parpadeos; `Timer`, `Sistema` y sensores tienen cadencias de actualización específicas.
- El reposo automático es visible: muestra `ZZZ` y despierta con interacción del encoder.

## Uso Rápido

```bash
platformio run -e esp32dev
platformio run -t upload
platformio device monitor
```

Si `platformio` no está en PATH en Windows, puede usarse la instalación local de PlatformIO, por ejemplo:

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run -e esp32dev
```

## Documentación

- `PBIT_FUNCIONAMIENTO_ACTUAL.md`: descripción funcional del firmware real.
- `MANUAL_DE_USUARIO_PBIT.md`: guía de uso.
- `MANUAL_TECNICO_PBIT.md`: arquitectura, NVS, BLE, energía y datos.
- `docs/PRODUCTION_CHECKLIST.md`: checklist antes de entregar una build.
- `docs/REPO_HYGIENE.md`: notas de higiene y artefactos grandes a mover en una pasada futura.
- `ROADMAP_PBIT.md`: trabajo pendiente y prioridades.

## Producción

Antes de flashear lotes o entregar unidades, seguir `docs/PRODUCTION_CHECKLIST.md`. En especial: confirmar que la build `esp32dev` compila, que BLE no anuncia, que `PBIT_ENABLE_SERIAL_PLOTTER` sigue en `0`, que no hay `FIRMWARE_DEBUG` activo y que el carrusel coincide con el flag elegido.

La corrección del LDR y su rango `0..20000 lux` están validados en firmware/build. Cualquier entrega física debe comprobar lecturas plausibles en la unidad concreta, sin asumir una calibración certificada de luxómetro.
