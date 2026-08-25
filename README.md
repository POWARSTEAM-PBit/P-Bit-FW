# P-Bit Firmware

Firmware PlatformIO/Arduino para el P-Bit: placa educativa ambiental basada en ESP32 con TFT ST7735, encoder, LED RGB, buzzer, sensores ambientales y BLE opcional.

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
- Idiomas: Español, Catalán, English.
- BLE desactivado de fábrica.

## Documentación

| Documento | Contenido |
|---|---|
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
Verificar: BLE no anuncia, `PBIT_ENABLE_SERIAL_PLOTTER=0`, sin `FIRMWARE_DEBUG`, carrusel correcto.
