/**
 * @file navigation.h
 * @brief Logica de navegacion autonoma reactiva
 *
 * Arquitectura de decision:
 *
 *   REGLA DE ORO: Si frontal libre → avanzar SIEMPRE.
 *   Los sensores laterales NUNCA detienen el avance frontal.
 *
 *   1. Todos bloqueados → contingencia completa
 *   2. Frontal bloqueado (3 confirmaciones) → maniobra evasiva completa
 *      frenado → pausa → retroceso lento → pausa → giro 90° lento → pausa
 *   3. Frontal libre + lateral muy cerca → correccion suave SIN frenar
 *      el rover ajusta trayectoria mientras sigue avanzando
 *   4. Todo libre → avanzar
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Velocidades
 * ========================================================= */

/** @brief Velocidad de avance normal (0-999) */
#define NAV_SPEED_FORWARD        650

/** @brief Velocidad durante maniobra evasiva — lento y controlado */
#define NAV_SPEED_MANEUVER       380

/** @brief Velocidad de giro en maniobra evasiva */
#define NAV_SPEED_TURN           400

/** @brief Velocidad de retroceso en maniobra evasiva */
#define NAV_SPEED_REVERSE        380

/* =========================================================
 * Umbrales de deteccion
 * ========================================================= */

/** @brief Umbral frontal en cm */
#define NAV_OBSTACLE_FRONT_CM     20

/** @brief Umbral lateral en cm — solo correccion suave sin frenar */
#define NAV_OBSTACLE_SIDE_CM       8

/* =========================================================
 * Tiempos de maniobra
 * ========================================================= */

/** @brief Pausa tras frenar — estabiliza lecturas (ms) */
#define NAV_PAUSE_AFTER_STOP_MS   150

/** @brief Tiempo de retroceso (ms) */
#define NAV_REVERSE_TIME_MS       450

/** @brief Pausa entre retroceso y giro (ms) */
#define NAV_PAUSE_BEFORE_TURN_MS  150

/** @brief Tiempo de giro 90 grados (ms) */
#define NAV_TURN_90_MS            750

/** @brief Pausa post-giro antes de re-evaluar (ms) */
#define NAV_PAUSE_AFTER_TURN_MS   150

/** @brief Confirmaciones consecutivas para actuar (frontal) */
#define NAV_CONFIRM_COUNT           3

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de navegacion
 */
void navigation_init(void);

/**
 * @brief Ejecuta un paso del algoritmo de navegacion
 * @return true si debe continuar
 */
bool navigation_step(void);

/**
 * @brief Detiene el rover inmediatamente
 */
void navigation_stop(void);

#endif /* NAVIGATION_H */