# Planta Lab — revisión de comportamiento y contrato de perfiles

Fecha: 2026-10-12. Auditoría inicial sobre `23fdb57`, árbol limpio al iniciar. Correcciones y setup V1 autorizados después por el usuario: implementados, compilados y cargados en COM8 sobre esa base + cambios locales. Prueba funcional/visual V1 en P-Bit pendiente.

El usuario conserva el diseño probado en P-Bit. Esta revisión no reabre paleta, ilustración, distribución ni efectos aprobados. Dos revisiones paralelas contrastaron modelo/pruebas y contrato/investigación; la revisión principal verificó adquisición, integración UI y dependencias del perfil. Alcance: `plant_pet.h/.cpp`, integración en `ui_lab_plant.cpp/tft_display.cpp`, fragmentos de adquisición/validez y documentos canónicos. No se auditó el firmware completo.

## Resultado de la implementación V1

Las referencias editables ya están separadas de datos instrumentales, escala y efectos ambientales. La oscuridad instantánea es contexto de reposo; las copias UI dejaron de confirmar cambios y las recuperaciones interrumpidas conservan la mezcla visible. El setup General/Personalizado está implementado sin exigir historial diario, crecimiento, RTC ni catálogo de especies. Diseño y paleta aprobados conservados.

| Hallazgo | Resolución en V1 | Verificación software |
|---|---|---|
| F1 | Luz excluida de atención adversa; REPOSO si los demás datos son conocidos y están dentro de referencia. Suelo/T/Aire conservan su gesto con poca luz. | Tests oscuridad + necesidad y reposo/recuperación PASS. |
| F2 | Identidad, fecha y calidad por canal; confirmación con adquisiciones nuevas y caducidad 1000 ms Suelo/Luz, 3000 ms T/Aire. Servicio del modelo independiente de copias UI. | 28 checks adquisición; suites de modelo/rollover/frescura y pantalla PASS. |
| F3 | Perfil versionado de seis referencias, amplitudes positivas y anclajes ambientales fijos. Barras/modelo/tierra comparten revisión; barra T amplía escala para referencias fuera de 0..40 °C. | Validación de perfil/NVS, límites y cambio de revisión PASS; referencia 50..60 °C comprobada en host. |
| F4 | Intensidad gradual también para Suelo/Aire; textos SUELO SECO / SUELO HÚMEDO en ES/CAT/EN. General conserva 30..70 / 15..30 / 40..70. | Suites modelo y raster/pantalla PASS; no validación agronómica. |
| F5 | Snapshot con mezcla efectiva de nueve poses; una transición nueva parte de la mezcla actual. | Pesos normalizados y continuidad exacta de píxeles de planta ante interrupción PASS. |

## Hallazgos históricos de la auditoría inicial

Los apartados F1–F5 siguientes describen **el código base antes de la implementación V1**. Sus líneas/trazas son referencias históricas, no una lista de defectos vigentes.

### F1 — Oscuridad como necesidad dominante: corregir antes del setup

`src/plant_pet.cpp`:17, 68–81, 142–162 considera adversa la luz bajo 200 lux. Una muestra de 0 lux tiene intensidad normalizada 1 y puede dominar Suelo 15 %, Temperatura 35 °C y Aire 20 % (0.5 cada uno). El caso reproducido devuelve Light como causa y Dim como gesto, con los cuatro canales afectados. Es una decisión heredada del producto, no un fallo de ejecución.

Recomendación: poca luz instantánea como contexto/reposo. Mantener lectura, símbolo y referencia visual de iluminación, pero excluirla de necesidades adversas y de la selección de atención principal. Con agua/temperatura fuera de referencia, conservar su gesto; con otros datos válidos y sin necesidad, permitir reposo tranquilo. No inferir noche, horas de sol ni suficiencia diaria. Distinguir contexto, banda de referencia y atención en el snapshot; modificar solo la causa sin revisar bordes/máscaras dejaría la fila de Luz tratada como alerta.

### F2 — Las copias UI no identifican mediciones nuevas: resolver el contrato temporal

`Reading` no tiene identidad/fecha de muestra por canal (`include/io.h`:4–15). `src/io.cpp`:75–101 publica snapshots a ritmo rápido, mientras DHT se intenta cada 1000 ms y Suelo aproximadamente cada 180 ms. DHT conserva el valor ante TOO_SOON o un primer fallo transitorio (211–237). `tft_display.cpp`:291–310 alimenta el modelo con cada copia UI y también al redibujar/cambiar pantalla; su cadencia depende de la vista (100 ms / 1000 ms, líneas 67–68, 694–705).

`PlantPetModel::update` cuenta esas llamadas como evidencia (121–129), sin distinguir una medición nueva de una copia de la anterior. Reproducción: una única muestra de 31 °C en t=1000 ms, copiada cada 100 ms hasta t=2200, confirma High; alimentando solo esa muestra física, sigue Normal. La escena representa un DHT que conserva el último valor tras un fallo; no se observó esta secuencia en hardware durante la auditoría.

Recomendación: identidad y tiempo de adquisición por canal, junto a calidad/frescura; confirmar con muestras nuevas, no navegación ni frames. Valores idénticos pueden ser mediciones nuevas: filtrar solo cambios numéricos sería incorrecto. Mantener la tolerancia actual a errores DHT como política de adquisición, separada de nueva evidencia. El modelo debe evolucionar independientemente de la cadencia de dibujo. Diseñar caducidad/cobertura antes de interpretar duración sostenida; los 1200 ms siguen siendo estabilización UI, no estrés biológico.

### F3 — Referencias editables están acopladas a escalas y animación: requisito del setup

`PlantPetRange` mezcla escala de barra y referencias (`include/plant_pet.h`:17–23). La escala térmica 0..40 °C no es el dominio de validez del DHT: adquisición acepta -20..80 °C (`io.cpp`:221). En Luz, 8000 es techo instrumental de la estimación; no un máximo saludable. No persistir la estructura completa como perfil.

La rampa térmica (`plant_pet.cpp`:57–65) divide por 36−referencia_alta y referencia_baja−8: hacer editables esos límites sin redefinir la rampa permite denominadores cero o invertidos. También hay una diferencia de anclaje ambiental: los copos dependen de la banda Low del perfil, mientras las ondas calientes usan temperatura absoluta ≥27 °C (`plant_pet_art.cpp`, surroundings). Con un mínimo personalizado de 29 °C, 28 °C podría generar copos y suprimir ondas por la rama cold/else. Es un riesgo futuro; los valores fijos vigentes son seguros.

Recomendación: referencias del perfil, escala UI, validez instrumental y anclajes de animación separados. Las bandas, barras y modelo deben usar la misma revisión de perfil. Acordar explícitamente qué efecto representa ambiente medido y qué gesto representa desviación respecto al perfil. Mantener los anclajes ambientales actuales en la primera implementación, evitando reinterpretarlos como daño por especie. Para la pose relativa, decidir cómo trasladar amplitudes visuales al cambiar referencias; offsets positivos versionados son una opción, sin atribución biológica.

### F4 — Suelo/aire pasan a pose completa cerca del corte: decisión de producto

`plant_pet.cpp`:46–48, 84–90 selecciona la misma pose por Suelo 29.99 % que por sequedad mucho mayor, o por Aire 70.01 % que por humedad extrema, después de confirmar la banda. Solo temperatura tiene intensidad de pose gradual. Esto no contradice el contrato actual, pero dificulta una expresión flexible.

Recomendación: conservar inicialmente referencias 30..70 % / 15..30 °C / 40..70 %; no incorporar por arrastre la propuesta de Aire 30..75 %. Separar intensidad visual de clasificación para desvíos suaves de Suelo/Aire, reutilizando poses y paleta. Acordar qué desviaciones merecen atención antes de fijar nuevos cortes. `AHOGADA` sugiere un diagnóstico que la lectura no mide: se recomienda un texto como `SUELO HÚMEDO` o `SUELO ALTO`, pendiente de elección y prueba ES/CAT/EN.

La histéresis térmica mantiene Hot a 30 °C después de 31 °C, aunque la intensidad visual sea cero y la cara sonría. Está probado e intencionado; no es un nuevo bug. Revisar el significado de `CALOR` en esa zona como descripción de contexto, sin obligar a que una sonrisa implique cuatro valores exactamente centrales.

### F5 — Recuperación interrumpida: defecto de continuidad reproducido

`plant_pet.cpp`:182–190 conserva el mood lógico anterior, no la mezcla visual que estaba dibujándose. Traza: Suelo 10 % en t=0; 50 % en t=100/1300 inicia recuperación de 3000 ms; 90 % en t=1400/2600 la interrumpe. Justo antes hay Thirsty→Content con transición 110/255 (~43 %). Después, el origen pasa a Content completo con transición 0 hacia Soggy. El renderer empieza desde una pose distinta de la mezcla previa.

Recomendación: conservar el punto visual efectivo al reiniciar una transición, añadir una regresión de continuidad y mantener arte/layout aprobados. Es un caso lógico confirmado, no un fallo observado por el usuario en TFT. No bloquea acordar el perfil; conviene corregirlo al ajustar el modelo antes del setup.

## Contrato implementado — V1

Contrato autorizado e implementado después de la auditoría:

| Capa | Contenido | Alcance |
|---|---|---|
| Medición | Valor, unidad base, validez, identidad/fecha de muestra y calidad | Dato ausente/antiguo y techo LDR separados; la escala gráfica no determina validez física |
| Perfil | General / Personalizado; referencias baja/alta de Suelo, Temperatura y Aire; versión | Referencias educativas, no óptimos universales ni alarmas de sensor |
| Modelo | Bandas de referencia, incertidumbre, atención/gesto e intensidad | Luz contextual; prioridades estables de representación, no gravedad fisiológica comparada |
| Render | Barras, símbolos ambientales, poses, transiciones y caché de revisión del perfil | Diseño vigente conservado; un cambio de referencias invalida campos afectados aunque valor/gesto no cambien |
| Consejos sostenidos | Fuera de V1 | Requieren especie, cobertura, duración y fundamento propio |

- **General**: valores iniciales actuales, sin afirmar especie/etapa. **Personalizado**: copia editable de referencias Suelo/Temperatura/Aire. Luz contextual; no configurar un supuesto mínimo diario ni máximo saludable.
- Root settings: grid 2×3 con `Perfil` / `Ajustar`, `Reset` abajo izquierda y `Salir` abajo derecha. PRIMARY_COUNT=2, RESET_INDEX=2, EXIT_INDEX=3 y máximo encoder=3, conforme a AGENTS. No llamar Preset al selector si solo hay General/Personalizado.
- Guardar un borrador solo al confirmar; cancelar no cambia perfil activo ni NVS. Volver a General mediante el selector puede conservar el personalizado para reutilizarlo; Reset restaura únicamente el estado de Planta Lab a General y elimina su personalización.
- Validar finitos, orden y separación útil de referencias; amplitudes de pose positivas y límites compatibles con medición. Guardar temperatura en °C; °F es presentación/conversión de entrada. Versión de esquema y carga validada; datos corruptos/incompatibles vuelven a General de forma controlada.
- No modificar alarmas de sensor, calibración RAW ni otras pantallas. El perfil no es un `Reading` ni un crecimiento guardado.
- Un cambio de perfil invalida candidatos/causa pendientes y caché de barras; recalcular con datos frescos, sin heredar una falsa `MEJORANDO`. Distinguir ausencia de dato de referencia pendiente de revisión.
- Suelo personalizado depende de referencias de calibración. Registrar su vinculación y, al cambiar calibración, pedir revisar/confirmar esa referencia; no asumir portabilidad entre macetas ni restaurar silenciosamente alarmas/calibración. V1 guarda la pareja RAW seco/húmedo en el registro del perfil; al cambiarla muestra `REVISAR SUELO`, mantiene la lectura y neutraliza su interpretación hasta guardar de nuevo desde Ajustar.
- Mantener instancias real/Demo separadas y no escribir perfil desde Demo. Demo usa General para demostraciones reproducibles.
- Sin presets genéricos `cactus` / `tropical` / `flor` todavía. General/Personalizado sirve también para plantas compradas/adultas sin inferir edad, crecimiento o especie. Presets posteriores: especie/etapa/contexto concreto, fuentes y calibración compatible.

## Secuencia de trabajo y próximo paso

Los pasos 1–3 ya están implementados y la carga V1 terminó; falta la prueba de uso del usuario. El paso 4 sigue futuro.

1. Acordar reposo/atención/textos y los campos de este contrato; no volver a diseñar la mascota.
2. Ajustar el modelo de forma acotada: luz contextual, evidencia/frescura y continuidad; acordar intensidad suave de Suelo/Aire sin nuevos umbrales agronómicos.
3. Separar referencias de escala/anclajes e incorporar General/Personalizado + confirmación/NVS/Reset. Validar después en P-Bit.
4. Preparar presets concretos y cobertura/historial para futuros consejos. Personajes por especie y catálogo web posteriores.

La secuencia anterior de RESEARCH exigía historial/combinaciones antes de cualquier personalización. Esa exigencia se conserva para consejos sostenidos, pero no bloquea un setup de referencias educativas.

## Evidencia y límites

Durante la auditoría inicial: cuatro trazas dirigidas compiladas contra el modelo base `23fdb57` con C++11 `-Wall -Wextra -Werror`: oscuridad dominante, copias frente a muestras, recuperación interrumpida y Hot con intensidad cero en histéresis. Se reprodujeron los resultados descritos. Harness/binario ignorados en `.pio/plant_pet_review/behavior_audit/`; no forman parte del firmware.

La fase inicial fue solo revisión/documentación, sin build ESP32 ni carga. La fase posterior autorizada sí cambió firmware: suites nativas de adquisición/modelo/setup PASS; raster 1215 combinaciones +101 niveles de Suelo +4 ausentes/no finitos y continuidad PASS; 143 comparaciones de pantalla principal y 30 de setup ES/CAT/EN PASS. Build `esp32dev` final SUCCESS (22.711 s), RAM60372/327680 (18.4 %) y Flash979701/1310720 (74.7 %). Carga COM8 SUCCESS (34.229 s), hashes verificados y reinicio RTS. Logs/previews ignorados en `.pio/plant_pet_review/profile_v1/`; registro completo en [producción](PRODUCTION_RELEASE.md).

No se activó debug ni se cambiaron dependencias/flags. La evidencia host/build/carga no valida rangos biológicos, presets ni uso físico del nuevo setup. Las pruebas de 24 h/unidad siguen aplazadas; el cierre visual provisional anterior se conserva.

Documentos relacionados: [ADR-003](ADR-003-planta-lab.md), [investigación](PLANTA_LAB_RESEARCH.md), [roadmap](ROADMAP.md), [producción](PRODUCTION_RELEASE.md).
