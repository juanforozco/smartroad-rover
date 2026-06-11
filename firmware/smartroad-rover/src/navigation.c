/**
 * @file navigation.c
 * @brief Implementacion de la navegacion autonoma reactiva v6
 *
 * REGLA DE ORO: frontal libre = avanzar siempre.
 *
 * Maniobra evasiva completa cuando frontal bloqueado:
 *   [Frenar] → [Pausa 150ms] → [Retroceder 450ms] →
 *   [Pausa 150ms] → [Girar 90° 750ms] → [Pausa 150ms]
 *
 * Cada pausa permite que las lecturas de sensores se estabilicen
 * antes de la siguiente accion, evitando decisiones sobre señales
 * en transicion.
 *
 * Correccion lateral: solo cuando frontal libre y lateral < 8cm.
 * No detiene el rover — ajusta trayectoria continuando el avance.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "navigation.h"
#include "sensors.h"
#include "motors.h"
#include "pico/stdlib.h"
#include <stdio.h>

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Contador de confirmaciones de obstaculo frontal consecutivas */
static uint8_t obstacle_confirm_count = 0;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Ejecuta la maniobra evasiva completa
 *
 * Secuencia controlada con pausas entre cada accion:
 * frenar → estabilizar → retroceder → estabilizar → girar → estabilizar
 *
 * @param turn_left true = girar izquierda, false = girar derecha
 */
static void evasive_maneuver(bool turn_left) {
    /* Paso 1: Frenar */
    motors_stop();
    sleep_ms(NAV_PAUSE_AFTER_STOP_MS);

    /* Paso 2: Retroceder despacio */
    motors_reverse(NAV_SPEED_REVERSE);
    sleep_ms(NAV_REVERSE_TIME_MS);
    motors_stop();
    sleep_ms(NAV_PAUSE_BEFORE_TURN_MS);

    /* Paso 3: Girar 90° despacio */
    if (turn_left) {
        motors_turn_left(NAV_SPEED_TURN);
    } else {
        motors_turn_right(NAV_SPEED_TURN);
    }
    sleep_ms(NAV_TURN_90_MS);
    motors_stop();

    /* Paso 4: Pausa post-giro — dejar que lecturas se estabilicen */
    sleep_ms(NAV_PAUSE_AFTER_TURN_MS);
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void navigation_init(void) {
    obstacle_confirm_count = 0;
    printf("[NAV] Navegacion autonoma v6 lista.\n");
    printf("[NAV] Frontal: %dcm | Lateral: %dcm | Confirmaciones: %d\n",
           NAV_OBSTACLE_FRONT_CM, NAV_OBSTACLE_SIDE_CM, NAV_CONFIRM_COUNT);
}

bool navigation_step(void) {

    /* Leer distancias filtradas */
    uint16_t dist_center = sensors_get_distance(SENSOR_CENTER);
    uint16_t dist_left   = sensors_get_distance(SENSOR_LEFT);
    uint16_t dist_right  = sensors_get_distance(SENSOR_RIGHT);

    /* ======================================================
     * PRIORIDAD 1: Contingencia — todos bloqueados
     * ====================================================== */
    if (sensors_all_blocked()) {
        obstacle_confirm_count = 0;
        printf("[NAV] BLOQUEADO TOTAL — contingencia\n");

        /* Maniobra de contingencia: retroceder mas y girar derecha */
        motors_stop();
        sleep_ms(NAV_PAUSE_AFTER_STOP_MS);
        motors_reverse(NAV_SPEED_REVERSE);
        sleep_ms(NAV_REVERSE_TIME_MS + 200); /* Retroceso extra */
        motors_stop();
        sleep_ms(NAV_PAUSE_BEFORE_TURN_MS);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_90_MS);
        motors_stop();
        sleep_ms(NAV_PAUSE_AFTER_TURN_MS);
        return true;
    }

    /* ======================================================
     * PRIORIDAD 2: Obstaculo frontal — confirmar antes de actuar
     * ====================================================== */
    if (dist_center < NAV_OBSTACLE_FRONT_CM) {
        obstacle_confirm_count++;
    } else {
        /* Frontal libre: reset contador y priorizar avance */
        obstacle_confirm_count = 0;

        /* ======================================================
         * PRIORIDAD 3: Frontal libre — correccion lateral suave
         * Solo si lateral muy cerca (8cm) — SIN frenar el rover
         * ====================================================== */
        if (dist_left < NAV_OBSTACLE_SIDE_CM && dist_right >= NAV_OBSTACLE_SIDE_CM) {
            /* Pared izquierda muy cerca: avanzar con leve desvio derecha */
            printf("[NAV] Correccion derecha suave (izq:%dcm)\n", dist_left);
            motors_set(MOTOR_FORWARD, NAV_SPEED_FORWARD,
                       MOTOR_FORWARD, NAV_SPEED_FORWARD - 200);
            return true;
        }

        if (dist_right < NAV_OBSTACLE_SIDE_CM && dist_left >= NAV_OBSTACLE_SIDE_CM) {
            /* Pared derecha muy cerca: avanzar con leve desvio izquierda */
            printf("[NAV] Correccion izquierda suave (der:%dcm)\n", dist_right);
            motors_set(MOTOR_FORWARD, NAV_SPEED_FORWARD - 200,
                       MOTOR_FORWARD, NAV_SPEED_FORWARD);
            return true;
        }

        /* ======================================================
         * PRIORIDAD 4: Todo libre — avanzar a velocidad normal
         * ====================================================== */
        motors_forward(NAV_SPEED_FORWARD);
        return true;
    }

    /* ======================================================
     * Ejecutar maniobra evasiva solo con confirmacion suficiente
     * ====================================================== */
    if (obstacle_confirm_count >= NAV_CONFIRM_COUNT) {
        obstacle_confirm_count = 0;

        /* Decidir direccion segun lado con mas espacio */
        bool turn_left = (dist_left >= dist_right);

        printf("[NAV] Frontal bloqueado (%dcm) — maniobra %s (izq:%d der:%d)\n",
               dist_center,
               turn_left ? "izquierda" : "derecha",
               dist_left, dist_right);

        evasive_maneuver(turn_left);
    }

    /* Si aun no confirma: frenar y esperar siguiente lectura */
    motors_stop();
    return true;
}

void navigation_stop(void) {
    motors_stop();
    obstacle_confirm_count = 0;
}