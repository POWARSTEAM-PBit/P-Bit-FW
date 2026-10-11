# P-Bit Firmware

Firmware PlatformIO/Arduino para el P-Bit: placa educativa ambiental portátil basada en ESP32 con TFT ST7735, encoder, LED RGB, buzzer, sensores ambientales y conectividad inalámbrica del ESP32 reservada para configuración futura/fábrica.

Alimentación recomendada del producto: `3 baterías AAA`. El puerto USB-C se usa principalmente para programación y puede alimentar el equipo desde PC o powerbank como recurso auxiliar, pero no es el modo recomendado de uso diario.

## Build y flash rápido

```bash
# Compilar
platformio run -e esp32dev

# Compilar y flashear
platformio run -t upload

# Monitor serie
platformio device monitor
```

En Windows, si `platformio` no está en PATH:

```powershell
& 'C:\.platformio\penv\Scripts\platformio.exe' run -e esp32dev
```

## Estado actual

- Build verificada el `2026-10-11` sobre base `3744f04` + cambios locales de mascota y ajuste visual de Planta Lab: `esp32dev` SUCCESS, RAM estática `59580` (18.2 %), Flash `971189` (74.1 %), con flags de producción sin cambios.
- Planta Lab: mascota verde persistente, barras por sensor y tierra café progresiva, aprobados puntualmente por el usuario. Último ajuste: expresión térmica flexible 15..30 °C con histéresis y postura gradual; ondas magenta repartidas y escalonadas desde 27 °C, sol creciente y luna circular <20 lux. Copos 1..3 con descenso y efectos de aire 1..2 con deriva/pausas ya integrados, limitados a cuatro grupos ambientales conjuntos. Pruebas nativas/preview comprobados; esta revisión todavía requiere TFT real. Historial, combinaciones y presets siguen pendientes.
- Carrusel de producción: `Inicio`, `Clima Lab`, `Planta Lab` si Suelo está conectado, `Termo Lab`, 6 sensores en Sensor Zone, `Timer`, `Sistema`.
- Sensor Zone: `Temperatura`, `Humedad`, `Luz`, `Sonido`, `Suelo` y `Termómetro`; pulsación corta cambia `Principal -> Rango -> Ficha -> Dato -> Curva`. `Sonido` añade `Sonido VU` y `Sonido Onda` tras `Principal`.
- Alimentación de producto: `3 baterías AAA`; USB-C para programación/alimentación auxiliar.
- Idiomas: Español, Catalán, English.
- WiFi y Bluetooth forman parte del ESP32, pero el firmware inicial no ofrece configuración WiFi ni flujo público de Bluetooth. BLE queda desactivado de fábrica.

La validación final de producción sigue abierta. La prioridad actual es cerrar el refinamiento visual de `Planta Lab` en TFT real; configuración/presets después; Demo ya está implementado y anti-flicker requiere regresión visual tras este cambio. La prueba final de 24 h y la validación física completa por unidad se mantienen pendientes y aplazadas. Ver `docs/ROADMAP.md` y `docs/PRODUCTION_RELEASE.md` para distinguir estado actual de evidencia histórica.

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
