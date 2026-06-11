/**
 * @file navigation.c
 * @brief Implementacion de la navegacion autonoma reactiva v4
 *
 * Arquitectura de decision (por orden de prioridad):
 *
 *   PRIORIDAD 1: Todos bloqueados → contingencia (retroceder + girar)
 *   PRIORIDAD 2: Frontal bloqueado → retroceder + girar 90° al lado libre
 *   PRIORIDAD 3: Frontal LIBRE + lateral muy cerca → correccion suave
 *   PRIORIDAD 4: Todo libre → avanzar
 *
 * Razon de la prioridad frontal:
 *   Los sensores laterales apuntan a 90° respecto al frontal.
 *   Una pared lateral NO bloquea el avance frontal. Si el camino
 *   adelante esta libre, el rover puede avanzar aunque detecte
 *   algo a los costados. Solo se corrige lateralmente cuando el
 *   obstaculo esta a menos de 10cm (contacto inminente).
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

/** @brief Contador de confirmaciones de obstaculo frontal */
static uint8_t obstacle_confirm_count = 0;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Limpia el buffer de mediana ejecutando N ciclos de trigger
 *
 * Fuerza N lecturas nuevas para que la mediana refleje
 * la situacion actual tras una maniobra.
 *
 * @param cycles Numero de ciclos
 */
static void flush_sensor_buffer(uint8_t cycles) {
    for (uint8_t i = 0; i < cycles; i++) {
        sensors_trigger_all_sequential();
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void navigation_init(void) {
    obstacle_confirm_count = 0;
    printf("[NAV] Navegacion autonoma v4 lista.\n");
    printf("[NAV] Frontal: %dcm | Lateral: %dcm\n",
           NAV_OBSTACLE_FRONT_CM, NAV_OBSTACLE_SIDE_CM);
}

bool navigation_step(void) {

    /* --- Leer distancias filtradas --- */
    uint16_t dist_center = sensors_get_distance(SENSOR_CENTER);
    uint16_t dist_left   = sensors_get_distance(SENSOR_LEFT);
    uint16_t dist_right  = sensors_get_distance(SENSOR_RIGHT);

    /* ======================================================
     * PRIORIDAD 1: Contingencia — todos los sensores bloqueados
     * ====================================================== */
    if (sensors_all_blocked()) {
        obstacle_confirm_count = 0;
        printf("[NAV] Bloqueado total — retrocediendo y girando\n");
        motors_reverse(NAV_SPEED_REVERSE);
        sleep_ms(NAV_REVERSE_TIME_MS);
        motors_stop();
        sleep_ms(50);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_90_MS);
        motors_stop();
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* ======================================================
     * PRIORIDAD 2: Obstaculo frontal confirmado
     * ====================================================== */
    if (dist_center < NAV_OBSTACLE_FRONT_CM) {
        obstacle_confirm_count++;
    } else {
        obstacle_confirm_count = 0;
    }

    if (obstacle_confirm_count >= NAV_CONFIRM_COUNT) {
        obstacle_confirm_count = 0;

        /* Retroceder antes de girar para tener espacio */
        printf("[NAV] Frontal bloqueado (%dcm) — retrocediendo\n", dist_center);
        motors_reverse(NAV_SPEED_REVERSE);
        sleep_ms(NAV_REVERSE_TIME_MS);
        motors_stop();
        sleep_ms(50);

        /* Girar ~90° hacia el lado con mas espacio */
        if (dist_left >= dist_right) {
            printf("[NAV] Girando izquierda (izq:%d der:%d)\n",
                   dist_left, dist_right);
            motors_turn_left(NAV_SPEED_TURN);
        } else {
            printf("[NAV] Girando derecha (izq:%d der:%d)\n",
                   dist_left, dist_right);
            motors_turn_right(NAV_SPEED_TURN);
        }
        sleep_ms(NAV_TURN_90_MS);
        motors_stop();

        /* Limpiar buffer para re-evaluar con lecturas frescas */
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* ======================================================
     * PRIORIDAD 3: Frontal LIBRE — correccion lateral suave
     * Solo actua si el obstaculo lateral esta muy cerca (10cm)
     * El rover puede avanzar: solo corrige la trayectoria
     * ====================================================== */
    if (dist_left < NAV_OBSTACLE_SIDE_CM) {
        /* Pared muy cerca a la izquierda: corregir levemente a la derecha */
        printf("[NAV] Correccion derecha por pared izq (%dcm)\n", dist_left);
        motors_turn_right(NAV_SPEED_CORRECTION);
        sleep_ms(NAV_CORRECTION_TIME_MS);
        motors_stop();
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    if (dist_right < NAV_OBSTACLE_SIDE_CM) {
        /* Pared muy cerca a la derecha: corregir levemente a la izquierda */
        printf("[NAV] Correccion izquierda por pared der (%dcm)\n", dist_right);
        motors_turn_left(NAV_SPEED_CORRECTION);
        sleep_ms(NAV_CORRECTION_TIME_MS);
        motors_stop();
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* ======================================================
     * PRIORIDAD 4: Camino libre — avanzar
     * ====================================================== */
    motors_forward(NAV_SPEED_FORWARD);
    return true;
}

void navigation_stop(void) {
    motors_stop();
    obstacle_confirm_count = 0;
}