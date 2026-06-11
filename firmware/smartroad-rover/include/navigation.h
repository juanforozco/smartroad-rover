/**
 * @file navigation.h
 * @brief Logica de navegacion autonoma reactiva
 *
 * v4 - Prioridad absoluta al sensor frontal:
 *   - Si frontal libre → avanzar siempre
 *   - Si frontal bloqueado → retroceder + giro 90°
 *   - Laterales solo corrigen levemente cuando frontal esta libre
 *     y el obstaculo lateral esta muy cerca (10cm)
 *   - Sensores a 90° del frontal: detectan paredes paralelas
 *     al movimiento, no obstaculos que bloqueen el avance
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

/** @brief Velocidad de giro principal (0-999) */
#define NAV_SPEED_TURN              520

/** @brief Velocidad de retroceso (0-999) */
#define NAV_SPEED_REVERSE           450

/** @brief Velocidad de correccion lateral suave (0-999) */
#define NAV_SPEED_CORRECTION        480

/** @brief Umbral de deteccion FRONTAL en cm */
#define NAV_OBSTACLE_FRONT_CM       15

/** @brief Umbral de deteccion LATERAL en cm
 *  Bajo (10cm) porque los sensores laterales estan a 90 grados
 *  y solo deben actuar cuando hay contacto inminente */
#define NAV_OBSTACLE_SIDE_CM        10

/** @brief Tiempo de retroceso antes de girar (ms) */
#define NAV_REVERSE_TIME_MS         350

/** @brief Tiempo de giro principal ~90 grados (ms) */
#define NAV_TURN_90_MS              600

/** @brief Tiempo de correccion lateral suave (ms) */
#define NAV_CORRECTION_TIME_MS      200

/** @brief Ciclos de flush del buffer post-maniobra */
#define NAV_BUFFER_FLUSH_CYCLES      3

/** @brief Confirmaciones para obstaculo frontal */
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