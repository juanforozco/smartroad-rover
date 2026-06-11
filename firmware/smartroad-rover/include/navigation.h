/**
 * @file navigation.h
 * @brief Logica de navegacion autonoma reactiva
 *
 * Implementa el algoritmo de evasion de obstaculos basado en
 * las lecturas de los tres sensores HC-SR04.
 *
 * Mejoras respecto a v1:
 *   - Confirmacion de obstaculo en 2 lecturas consecutivas
 *   - Avance obligatorio post-giro para salir del punto de decision
 *   - Velocidades diferenciadas: avance normal vs maniobra
 *   - Umbral de deteccion ajustado a 18cm
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
#define NAV_SPEED_FORWARD        500

/** @brief Velocidad durante maniobras de evasion (0-999) */
#define NAV_SPEED_MANEUVER       450

/** @brief Velocidad de giro (0-999) */
#define NAV_SPEED_TURN           600

/** @brief Velocidad de retroceso en contingencia (0-999) */
#define NAV_SPEED_REVERSE        450

/** @brief Umbral de deteccion de obstaculo en cm */
#define NAV_OBSTACLE_THRESHOLD_CM   18

/** @brief Tiempo de giro en maniobra de evasion (ms) */
#define NAV_TURN_TIME_MS            500

/** @brief Tiempo de retroceso en contingencia (ms) */
#define NAV_REVERSE_TIME_MS         400

/** @brief Tiempo de avance obligatorio post-giro (ms) */
#define NAV_POST_TURN_ADVANCE_MS    200

/** @brief Confirmaciones consecutivas requeridas para actuar */
#define NAV_CONFIRM_COUNT           2

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de navegacion
 */
void navigation_init(void);

/**
 * @brief Ejecuta un paso del algoritmo de navegacion autonoma
 *
 * @return true si el modo debe continuar
 */
bool navigation_step(void);

/**
 * @brief Detiene el rover inmediatamente
 */
void navigation_stop(void);

#endif /* NAVIGATION_H */