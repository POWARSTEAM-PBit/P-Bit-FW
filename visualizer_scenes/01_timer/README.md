# 01 Timer

Escenas previstas en este grupo:

- `00_timer_idle.cpp`
- `01_timer_running_lt1h.cpp`
- `02_timer_paused_lt1h.cpp`
- `03_timer_running_ge1h.cpp`
- `04_timer_paused_ge1h.cpp`
- `05_timer_countdown_running.cpp`
- `06_timer_countdown_finished.cpp`
- `07_timer_editor_select.cpp`
- `08_timer_editor_edit_hours.cpp`
- `09_timer_editor_edit_minutes.cpp`
- `10_timer_editor_edit_seconds.cpp`

Notas de estado:

- `TIMER_SCREEN` sigue siendo parada visible top-level.
- Textos e instrucciones deben venir de `LangKey` (`TIT_TIMER`, `ST_TIMER_*`, `ST_PUSH_*`).
- Mantener los estados de cronómetro/editor como producción actual, no como laboratorio.
- Revisar clears acotados del valor de tiempo: el timer refresca más rápido que la mayoría de pantallas.
