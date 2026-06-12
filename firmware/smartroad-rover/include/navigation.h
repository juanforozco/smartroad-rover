/**
 * @file navigation.h
 * @brief Logica de navegacion autonoma reactiva v9
 *
 * REGLA DE ORO: frontal libre = avanzar siempre,
 * excepto cuando lateral critico detecta contacto inminente.
 *
 * Prioridades:
 *   1.  Todos bloqueados → contingencia completa
 *   1.5 Lateral critico (< NAV_OBSTACLE_SIDE_CRITICAL_CM) →
 *       frena y corrige, se sobrepone a prioridad frontal
 *   2.  Frontal bloqueado (x NAV_CONFIRM_COUNT) → maniobra evasiva
 *       frenar → retroceder → girar 90° → limpiar buffer → arrancar
 *   3.  Frontal libre + lateral cercano → correccion proporcional
 *       sin frenar, diferencial de velocidad segun distancia
 *   4.  Todo libre → avanzar
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Velocidades (rango 0-999)
 * ========================================================= */

/** @brief Velocidad de avance normal */
#define NAV_SPEED_FORWARD        450

/** @brief Velocidad de giro en maniobra evasiva */
#define NAV_SPEED_TURN           400

/** @brief Velocidad de retroceso en maniobra evasiva */
#define NAV_SPEED_REVERSE        380

/* =========================================================
 * Umbrales de deteccion
 * ========================================================= */

/** @brief Umbral frontal en cm — confirmar obstaculo */
#define NAV_OBSTACLE_FRONT_CM        12

/** @brief Umbral lateral normal en cm — correccion proporcional suave */
#define NAV_OBSTACLE_SIDE_CM         10

/** @brief Umbral lateral critico en cm — frena y corrige inmediatamente
 *  Se sobrepone a la prioridad frontal para evitar choque lateral */
#define NAV_OBSTACLE_SIDE_CRITICAL_CM  4

/** @brief Confirmaciones consecutivas para actuar en frontal */
#define NAV_CONFIRM_COUNT              1

/* =========================================================
 * Tiempos de maniobra
 * ========================================================= */

/** @brief Pausa tras frenar — estabiliza lecturas (ms) */
#define NAV_PAUSE_AFTER_STOP_MS     150

/** @brief Duracion del retroceso (ms) */
#define NAV_REVERSE_TIME_MS         400

/** @brief Pausa entre retroceso y giro (ms) */
#define NAV_PAUSE_BEFORE_TURN_MS    150

/** @brief Duracion del giro ~90 grados (ms) */
#define NAV_TURN_90_MS              750

/** @brief Pausa post-giro antes de limpiar buffer (ms) */
#define NAV_PAUSE_AFTER_TURN_MS     150

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
 * @brief Detiene el rover y resetea estado interno
 */
void navigation_stop(void);

#endif /* NAVIGATION_H */