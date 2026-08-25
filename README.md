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

- Build OK `esp32dev` con `PBIT_ENABLE_GRAPH_LAB=1` y `PBIT_ENABLE_SERIAL_PLOTTER=0`.
- Carrusel de producción: `Inicio`, `Clima Lab`, `Planta Lab` si Suelo está conectado, `Termo Lab`, 6 sensores en Sensor Zone, `Timer`, `Sistema`.
- Sensor Zone: `Temperatura`, `Humedad`, `Luz`, `Sonido`, `Suelo` y `Termómetro`; pulsación corta cambia `Principal -> Rango -> Ficha -> Dato -> Curva`. `Sonido` añade `Sonido VU` y `Sonido Onda` tras `Principal`.
- Alimentación de producto: `3 baterías AAA`; USB-C para programación/alimentación auxiliar.
- Idiomas: Español, Catalán, English.
- WiFi y Bluetooth forman parte del ESP32, pero el firmware inicial no ofrece configuración WiFi ni flujo público de Bluetooth. BLE queda desactivado de fábrica.

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
| `CHANGELOG.md` | Historial de cambios |
| `AGENTS.md` | Guía de entrada para agentes IA |

## Producción

Antes de entregar builds o unidades: seguir `docs/PRODUCTION_CHECKLIST.md`.
Verificar: funcionamiento con `3 baterías AAA`, USB-C tratado como programación/auxiliar, BLE no anuncia, `PBIT_ENABLE_SERIAL_PLOTTER=0`, sin `FIRMWARE_DEBUG`, carrusel correcto.
