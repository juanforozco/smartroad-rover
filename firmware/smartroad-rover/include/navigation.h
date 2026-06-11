/**
 * @file navigation.h
 * @brief Logica de navegacion autonoma reactiva
 *
 * v3 - Mejoras:
 *   - Deteccion lateral activa con umbral propio
 *   - Umbral frontal reducido a 15cm
 *   - Umbral lateral en 20cm
 *   - Limpieza de buffer post-giro (3 ciclos de trigger)
 *   - Velocidad avance mayor que velocidad giro
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Parametros de navegacion
 * ========================================================= */

/** @brief Velocidad de avance normal (0-999) */
#define NAV_SPEED_FORWARD           650

/** @brief Velocidad durante maniobras de evasion (0-999) */
#define NAV_SPEED_MANEUVER          480

/** @brief Velocidad de giro (0-999) — menor que avance */
#define NAV_SPEED_TURN              520

/** @brief Velocidad de retroceso en contingencia (0-999) */
#define NAV_SPEED_REVERSE           450

/** @brief Umbral de deteccion frontal en cm */
#define NAV_OBSTACLE_FRONT_CM       15

/** @brief Umbral de deteccion lateral en cm */
#define NAV_OBSTACLE_SIDE_CM        20

/** @brief Tiempo de giro en maniobra de evasion (ms) */
#define NAV_TURN_TIME_MS            500

/** @brief Tiempo de retroceso en contingencia (ms) */
#define NAV_REVERSE_TIME_MS         400

/** @brief Ciclos de trigger para limpiar buffer post-giro */
#define NAV_BUFFER_FLUSH_CYCLES      3

/** @brief Confirmaciones consecutivas requeridas para actuar */
#define NAV_CONFIRM_COUNT            2

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de navegacion
 */
void navigation_init(void);

/**
 * @brief Ejecuta un paso del algoritmo de navegacion autonoma
 * @return true si el modo debe continuar
 */
bool navigation_step(void);

/**
 * @brief Detiene el rover inmediatamente
 */
void navigation_stop(void);

#endif /* NAVIGATION_H */