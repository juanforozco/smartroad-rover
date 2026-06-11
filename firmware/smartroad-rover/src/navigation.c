/**
 * @file navigation.c
 * @brief Implementacion de la navegacion autonoma reactiva v2
 *
 * Mejoras respecto a v1:
 *   - Confirmacion de 2 lecturas consecutivas antes de actuar
 *     evita reaccionar a lecturas espurias o transitorias
 *   - Avance obligatorio de 200ms post-giro para salir del
 *     punto de decision antes de re-evaluar
 *   - Velocidades diferenciadas: avance normal (500) vs
 *     maniobra (450) para mayor control en evasion
 *   - Umbral reducido a 18cm para reaccionar solo cuando
 *     el obstaculo esta realmente cerca
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
 * Implementacion de la API publica
 * ========================================================= */

void navigation_init(void) {
    obstacle_confirm_count = 0;
    printf("[NAV] Modulo de navegacion v2 inicializado.\n");
}

bool navigation_step(void) {

    /* --- Leer distancias filtradas --- */
    uint16_t dist_center = sensors_get_distance(SENSOR_CENTER);
    uint16_t dist_left   = sensors_get_distance(SENSOR_LEFT);
    uint16_t dist_right  = sensors_get_distance(SENSOR_RIGHT);

    /* --- Caso de contingencia: todos los sensores bloqueados --- */
    if (sensors_all_blocked()) {
        obstacle_confirm_count = 0;
        printf("[NAV] Bloqueado total — retrocediendo\n");
        motors_reverse(NAV_SPEED_REVERSE);
        sleep_ms(NAV_REVERSE_TIME_MS);
        motors_stop();
        sleep_ms(50);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_TIME_MS);
        motors_stop();
        sleep_ms(50);
        /* Avance obligatorio para salir del punto */
        motors_forward(NAV_SPEED_MANEUVER);
        sleep_ms(NAV_POST_TURN_ADVANCE_MS);
        motors_stop();
        return true;
    }

    /* --- Confirmacion de obstaculo frontal --- */
    if (dist_center < NAV_OBSTACLE_THRESHOLD_CM) {
        obstacle_confirm_count++;
    } else {
        /* Resetear contador si el camino esta libre */
        obstacle_confirm_count = 0;
    }

    /* --- Actuar solo si se confirmo el obstaculo --- */
    if (obstacle_confirm_count >= NAV_CONFIRM_COUNT) {
        obstacle_confirm_count = 0;
        motors_stop();
        sleep_ms(50); /* Pausa breve para estabilizar */

        /* Seleccionar direccion de giro segun lado libre */
        if (dist_left >= dist_right) {
            printf("[NAV] Obstaculo frontal — girando izquierda (izq:%d der:%d)\n",
                   dist_left, dist_right);
            motors_turn_left(NAV_SPEED_TURN);
        } else {
            printf("[NAV] Obstaculo frontal — girando derecha (izq:%d der:%d)\n",
                   dist_left, dist_right);
            motors_turn_right(NAV_SPEED_TURN);
        }
        sleep_ms(NAV_TURN_TIME_MS);
        motors_stop();
        sleep_ms(50);

        /* Avance obligatorio post-giro para salir del punto de decision */
        motors_forward(NAV_SPEED_MANEUVER);
        sleep_ms(NAV_POST_TURN_ADVANCE_MS);
        motors_stop();
        return true;
    }

    /* --- Camino libre: avanzar --- */
    motors_forward(NAV_SPEED_FORWARD);
    return true;
}

void navigation_stop(void) {
    motors_stop();
    obstacle_confirm_count = 0;
}