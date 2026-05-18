#pragma once
// P-Bit sensor identity palette — retro arcade edition 2026-05-17
// Fuente de verdad para colores de sensores en todas las viz del sensor zone.
// Aplicar SOLO a: FOCUS, CARD, VALOR, GRAPH, GAUGE (sensor zone screens).
// NO aplicar a: HOME cards, CLIMA LAB, DUAL TH — esos tienen lógica responsiva propia.
//
// Estética objetivo: retro arcade / Gameboy Color — colores saturados al máximo,
// neón, ácidos, máximo contraste sobre fondo negro. Sin pasteles, sin tonos tierra.
// Cada sensor tiene una identidad de color única e inconfundible.
//
// Reglas de aplicación:
//   P1  → borde de card, línea de gráfica, icono activo, arco gauge, identidad visual
//   P2  → indicador de unidad (°C, %, lux), ring exterior gauge, sparkline, label sensor
//   P3  → label de máximo en gráfica, segmento pico VU, acento cálido
//   P4  → label de mínimo en gráfica, referencia de cero, contraste frío
//   WHITE → dato numérico principal en TODAS las visualizaciones (universal)

#include <stdint.h>

// ── P1: Primary — identidad del sensor ──────────────────────────────────────
// 6 hues únicos e inconfundibles. Máxima saturación, alta luminosidad.
// TEMP=naranja, HUM=cian, LUZ=amarillo, SOUND=magenta, SOIL=lima, DS18=violeta
constexpr uint16_t PB_TEMP_P1    = 0xFA80;  // naranja ácido      rgb(255,  80,   0)
constexpr uint16_t PB_HUM_P1     = 0x075F;  // cian eléctrico     rgb(  0, 232, 255)
constexpr uint16_t PB_LUZ_P1     = 0xFFE0;  // amarillo puro      rgb(255, 255,   0)
constexpr uint16_t PB_SOUND_P1   = 0xF81F;  // magenta punk       rgb(255,   0, 255)
constexpr uint16_t PB_SOIL_P1    = 0x47E8;  // lima ácido         rgb( 64, 255,  64)
constexpr uint16_t PB_DS18_P1    = 0xA01F;  // violeta eléctrico  rgb(160,   0, 255)

// ── P2: Secondary — complemento, contrasta con P1 ───────────────────────────
// Usado en: indicador de unidad (°C/%), ring exterior gauge, sparkline, label banda.
// Cada P2 es distinto de su P1 para crear tensión visual.
constexpr uint16_t PB_TEMP_P2    = 0xF814;  // rosa eléctrico     rgb(255,   0, 160)
constexpr uint16_t PB_HUM_P2     = 0x2A9F;  // cobalto láser      rgb( 40,  80, 255)
constexpr uint16_t PB_LUZ_P2     = 0xFC40;  // ámbar eléctrico    rgb(255, 136,   0)
constexpr uint16_t PB_SOUND_P2   = 0x07E8;  // verde ácido        rgb(  0, 255,  64)
constexpr uint16_t PB_SOIL_P2    = 0xCC04;  // tierra cálida      rgb(200, 128,  32)
constexpr uint16_t PB_DS18_P2    = 0x045F;  // azul láser         rgb(  0, 136, 255)

// ── P3: Acento cálido — pico, máximo, highlight ──────────────────────────────
// Usado en: label máximo en gráfica, segmento de pico en VU, valor extremo.
constexpr uint16_t PB_TEMP_P3    = 0xFE45;  // oro eléctrico      rgb(255, 200,  40)
constexpr uint16_t PB_HUM_P3     = 0x8FFF;  // aqua brillante     rgb(140, 255, 255)
constexpr uint16_t PB_LUZ_P3     = 0xFE40;  // oro neón           rgb(255, 200,   0)
constexpr uint16_t PB_SOUND_P3   = 0xF8A0;  // rojo neón (pico)   rgb(255,  20,   0)
constexpr uint16_t PB_SOIL_P3    = 0x87F0;  // menta claro        rgb(128, 255, 128)
constexpr uint16_t PB_DS18_P3    = 0xCC5F;  // amatista claro     rgb(200, 136, 255)

// ── P4: Contraste frío — mínimo, referencia, fondo ───────────────────────────
// Usado en: label mínimo en gráfica, tick de cero, contraste con P1.
constexpr uint16_t PB_TEMP_P4    = 0x055F;  // azul hielo         rgb(  0, 170, 255)
constexpr uint16_t PB_HUM_P4     = 0x01F4;  // azul océano        rgb(  0,  60, 160)
constexpr uint16_t PB_LUZ_P4     = 0x7A40;  // dorado oscuro      rgb(120,  72,   0)
constexpr uint16_t PB_SOUND_P4   = 0x4011;  // púrpura oscuro     rgb( 64,   0, 136)
constexpr uint16_t PB_SOIL_P4    = 0x0300;  // verde oscuro       rgb(  0,  96,   0)
constexpr uint16_t PB_DS18_P4    = 0x0659;  // cian frío          rgb(  0, 200, 200)

// ── Helpers — acceso por índice SzSensorId ───────────────────────────────────
// SzSensorId: TEMP=0, HUM=1, LIGHT=2, SOUND=3, SOIL=4, DS18=5
inline uint16_t pb_primary(uint8_t sensor_id) {
    static const uint16_t kP1[] = {
        PB_TEMP_P1, PB_HUM_P1, PB_LUZ_P1,
        PB_SOUND_P1, PB_SOIL_P1, PB_DS18_P1
    };
    return (sensor_id < 6) ? kP1[sensor_id] : 0x8410;
}

inline uint16_t pb_secondary(uint8_t sensor_id) {
    static const uint16_t kP2[] = {
        PB_TEMP_P2, PB_HUM_P2, PB_LUZ_P2,
        PB_SOUND_P2, PB_SOIL_P2, PB_DS18_P2
    };
    return (sensor_id < 6) ? kP2[sensor_id] : 0x4208;
}

inline uint16_t pb_accent_warm(uint8_t sensor_id) {
    static const uint16_t kP3[] = {
        PB_TEMP_P3, PB_HUM_P3, PB_LUZ_P3,
        PB_SOUND_P3, PB_SOIL_P3, PB_DS18_P3
    };
    return (sensor_id < 6) ? kP3[sensor_id] : 0xFFFF;
}

inline uint16_t pb_contrast_cool(uint8_t sensor_id) {
    static const uint16_t kP4[] = {
        PB_TEMP_P4, PB_HUM_P4, PB_LUZ_P4,
        PB_SOUND_P4, PB_SOIL_P4, PB_DS18_P4
    };
    return (sensor_id < 6) ? kP4[sensor_id] : 0x4208;
}

// ── Shared backgrounds — no cambiar ──────────────────────────────────────────
// Panel navy principal:  tft.color565(8,  12, 18)
// Fondo de gráfica:      tft.color565(4,  8,  20)
// Card interior:         0x0841
// Card border idle:      0x2945
