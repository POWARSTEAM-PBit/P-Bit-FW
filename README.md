# P-Bit Firmware

Firmware PlatformIO/Arduino para el P-Bit: placa educativa ambiental portátil basada en ESP32 con TFT ST7735, encoder, LED RGB, buzzer, sensores ambientales y conectividad inalámbrica del ESP32 reservada para configuración futura/fábrica.

Alimentación recomendada del producto: `3 baterías AAA`. El puerto USB-C se usa principalmente para programación y puede alimentar el equipo desde PC o powerbank como recurso auxiliar, pero no es el modo recomendado de uso diario.

## Build y flash rápido

```bash
# Compilar
platformio run -e esp32dev

# Compilar y flashear
platformio run -e esp32dev -t upload

# Monitor serie
platformio device monitor
```

En Windows, si `platformio` no está en PATH:

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run -e esp32dev
```

## Estado actual

- Build verificada el `2026-10-12`, base `23fdb57` + cambios locales V1 de Planta Lab: `esp32dev` SUCCESS (22.711 s), RAM estática `60372/327680` (18.4 %), Flash `979701/1310720` (74.7 %), flags de producción sin cambios.
- Candidato V1 cargado en COM8: SUCCESS (34.229 s), hashes verificados y reinicio RTS enviado. La carga no cierra la comprobación física del candidato.
- Planta Lab conserva el diseño visual aceptado provisionalmente por el usuario el 2026-10-12: verde persistente, barras por sensor, tierra café progresiva, sol/luna y efectos ambientales distribuidos. La nueva V1 añade REPOSO contextual por luz baja, mediciones identificadas/frescura y gesto gradual de Suelo/Temperatura/Aire con continuidad en recuperación. No diagnostica salud ni daño biológico.
- Configuración propia: pulsación larga ~1.2 s, menú `Perfil/Ajustar/Reset/Salir`, General/Personalizado y seis referencias en borrador. Guardar confirma/persiste; Cancelar es inicial y una larga cancela todo. Temperatura se edita en C/F global y guarda °C. General conserva personalizado; Reset afecta únicamente Planta Lab, sin modificar alarmas ni calibración. Una calibración distinta solicita revisar las referencias personalizadas de Suelo.
- Modelos real/Demo separados; Demo usa General y no escribe perfiles. Planta Lab todavía no pertenece a las escenas Demo. Presets por especie, historial diario y consejos sostenidos/combinaciones no implementados.
- Carrusel de producción: `Inicio`, `Clima Lab`, `Planta Lab` si Suelo está conectado, `Termo Lab`, 6 sensores en Sensor Zone, `Timer`, `Sistema`.
- Sensor Zone: `Temperatura`, `Humedad`, `Luz`, `Sonido`, `Suelo` y `Termómetro`; pulsación corta cambia `Principal -> Rango -> Ficha -> Dato -> Curva`. `Sonido` añade `Sonido VU` y `Sonido Onda` tras `Principal`.
- Alimentación de producto: `3 baterías AAA`; USB-C para programación/alimentación auxiliar.
- Idiomas: Español, Catalán, English.
- WiFi y Bluetooth forman parte del ESP32, pero el firmware inicial no ofrece configuración WiFi ni flujo público de Bluetooth. BLE queda desactivado de fábrica.

La validación final de producción sigue abierta. El diseño previo se conserva; el siguiente paso es probar en P-Bit el comportamiento y la configuración V1, incluidas persistencia, cancelación, calibración, idiomas y regresión de flicker/Demo. La valoración física de la versión visual anterior no verifica estos cambios nuevos. La prueba final de 24 h y la validación física completa por unidad se mantienen pendientes y aplazadas. Ver `docs/ROADMAP.md` y `docs/PRODUCTION_RELEASE.md` para distinguir candidato actual de evidencia histórica.

## Documentación

| Documento | Contenido |
|---|---|
| `docs/PBIT_KNOWLEDGE_BASE.md` | Guía base completa: producto, uso, pantallas, sensores, pinout, energía y soporte |
| `docs/PROJECT.md` | Qué es el P-Bit, hardware, capacidades, usos |
| `docs/USER_GUIDE.md` | Manual de usuario: uso, menús, seguridad, configuración |
| `docs/TECHNICAL.md` | Arquitectura, pinout, código, BLE, NVS, menús completos |
| `docs/DESIGN_SYSTEM.md` | Paleta, iconos, fuentes, reglas visuales TFT |
| `docs/TFT_RENDER_RULES.md` | Protocolo anti-flicker, sprites, chrome/data |
| `docs/ROADMAP.md` | Pendientes y mejoras futuras |
| `docs/PRODUCTION_CHECKLIST.md` | Checklist antes de entregar build o unidad |
| `docs/PRODUCTION_RELEASE.md` | Última build comprobada, evidencia histórica y límites de validación |
| `docs/ADR-003-planta-lab.md` | Diseño de Planta Lab, comportamiento implementado y preguntas abiertas |
| `CHANGELOG.md` | Historial de cambios |
| `AGENTS.md` | Guía de entrada para agentes IA |

## Producción

Antes de entregar builds o unidades: seguir `docs/PRODUCTION_CHECKLIST.md`.
Verificar: funcionamiento con `3 baterías AAA`, USB-C tratado como programación/auxiliar, BLE no anuncia, `PBIT_ENABLE_SERIAL_PLOTTER=0`, sin `FIRMWARE_DEBUG`, carrusel correcto.
