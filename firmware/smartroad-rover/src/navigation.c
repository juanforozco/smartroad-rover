/**
 * @file navigation.c
 * @brief Implementacion de la navegacion autonoma reactiva v3
 *
 * Cambios respecto a v2:
 *   - Deteccion lateral activa: si sensor izq o der detecta obstaculo
 *     cercano, el rover frena y evade hacia el lado contrario
 *   - Umbral frontal: 15cm (mas ajustado)
 *   - Umbral lateral: 20cm (mayor para reaccionar antes del choque)
 *   - Limpieza de buffer post-giro: se ejecutan NAV_BUFFER_FLUSH_CYCLES
 *     ciclos de trigger+espera para que el filtro de mediana se actualice
 *     con lecturas frescas antes de volver a evaluar
 *   - Velocidad avance (650) > velocidad giro (520) para movimiento
 *     mas natural y controlado
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
 * @brief Ejecuta N ciclos de trigger para limpiar el buffer de mediana
 *
 * Despues de una maniobra, el buffer puede contener lecturas del
 * obstaculo anterior. Esta funcion fuerza N nuevas lecturas para
 * que la mediana refleje la situacion actual antes de decidir.
 *
 * @param cycles Numero de ciclos de trigger a ejecutar
 */
static void flush_sensor_buffer(uint8_t cycles) {
    for (uint8_t i = 0; i < cycles; i++) {
        sensors_trigger();
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void navigation_init(void) {
    obstacle_confirm_count = 0;
    printf("[NAV] Modulo de navegacion v3 inicializado.\n");
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
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* --- Deteccion lateral izquierda --- */
    if (dist_left < NAV_OBSTACLE_SIDE_CM) {
        obstacle_confirm_count = 0;
        printf("[NAV] Obstaculo lateral izquierdo (%d cm) — girando derecha\n",
               dist_left);
        motors_stop();
        sleep_ms(50);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_TIME_MS);
        motors_stop();
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* --- Deteccion lateral derecha --- */
    if (dist_right < NAV_OBSTACLE_SIDE_CM) {
        obstacle_confirm_count = 0;
        printf("[NAV] Obstaculo lateral derecho (%d cm) — girando izquierda\n",
               dist_right);
        motors_stop();
        sleep_ms(50);
        motors_turn_left(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_TIME_MS);
        motors_stop();
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
        return true;
    }

    /* --- Confirmacion de obstaculo frontal --- */
    if (dist_center < NAV_OBSTACLE_FRONT_CM) {
        obstacle_confirm_count++;
    } else {
        obstacle_confirm_count = 0;
    }

    /* --- Actuar solo si se confirmo el obstaculo frontal --- */
    if (obstacle_confirm_count >= NAV_CONFIRM_COUNT) {
        obstacle_confirm_count = 0;
        motors_stop();
        sleep_ms(50);

        /* Girar hacia lado con mas espacio */
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

        /* Limpiar buffer antes de re-evaluar */
        flush_sensor_buffer(NAV_BUFFER_FLUSH_CYCLES);
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