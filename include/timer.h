#pragma once

extern bool userTimerRunning;
extern unsigned long userTimerStart;
extern unsigned long userTimerElapsed;

/**
 * @brief Bandera 'volatile' para avisar a la pantalla que el timer se acaba de resetear.
 */
extern volatile bool g_timer_just_reset;

/**
 * @brief Start the user timer.
 */
void startUserTimer();

/**
 * @brief Stop the user timer and record the elapsed time.
 */
void stopUserTimer();

/**
 * @brief Resets the user timer to zero.
 */
void resetUserTimer();

/**
 * @brief Receive the elapsed time in MM:SS:CC (Minutos:Segundos:Centésimas).
 */
char * getTimeHMS();