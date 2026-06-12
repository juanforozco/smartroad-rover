/**
 * @file navigation.c
 * @brief Implementacion de la navegacion autonoma reactiva v9
 *
 * Prioridades de decision:
 *   1. Todos bloqueados → contingencia completa
 *   1.5. Lateral critico (< 4cm) → frena y corrige, se sobrepone a todo
 *   2. Frontal bloqueado confirmado → maniobra evasiva completa
 *   3. Frontal libre + lateral cercano → correccion proporcional sin frenar
 *   4. Todo libre → avanzar
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "navigation.h"
#include "sensors.h"
#include "motors.h"
#include "pico/stdlib.h"
#include <stdio.h>

static uint8_t obstacle_confirm_count = 0;

/**
 * @brief Maniobra evasiva completa
 *
 * frenar → pausa → retroceder → pausa → girar 90° →
 * limpiar buffer → arrancar si frontal libre
 *
 * @param turn_left true = girar izquierda, false = girar derecha
 */
static void evasive_maneuver(bool turn_left) {
    motors_stop();
    sleep_ms(NAV_PAUSE_AFTER_STOP_MS);

    motors_reverse(NAV_SPEED_REVERSE);
    sleep_ms(NAV_REVERSE_TIME_MS);
    motors_stop();
    sleep_ms(NAV_PAUSE_BEFORE_TURN_MS);

    if (turn_left) {
        motors_turn_left(NAV_SPEED_TURN);
    } else {
        motors_turn_right(NAV_SPEED_TURN);
    }
    sleep_ms(NAV_TURN_90_MS);
    motors_stop();
    sleep_ms(NAV_PAUSE_AFTER_TURN_MS);

    /* Limpiar buffer con 5 lecturas frescas */
    for (uint8_t i = 0; i < 5; i++) {
        sensors_trigger();
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);
    }
    obstacle_confirm_count = 0;

    /* Arrancar inmediatamente si frontal libre */
    if (sensors_get_distance(SENSOR_CENTER) >= NAV_OBSTACLE_FRONT_CM) {
        //printf("[NAV] Via libre post-maniobra — arrancando\n");
        motors_forward(NAV_SPEED_FORWARD);
    }
}

void navigation_init(void) {
    obstacle_confirm_count = 0;
    printf("[NAV] Navegacion autonoma v9 lista.\n");
    printf("[NAV] Frontal: %dcm | Lateral: %dcm | Critico: %dcm\n",
           NAV_OBSTACLE_FRONT_CM, NAV_OBSTACLE_SIDE_CM,
           NAV_OBSTACLE_SIDE_CRITICAL_CM);
}

bool navigation_step(void) {

    uint16_t dist_center = sensors_get_distance(SENSOR_CENTER);
    uint16_t dist_left   = sensors_get_distance(SENSOR_LEFT);
    uint16_t dist_right  = sensors_get_distance(SENSOR_RIGHT);

    /* ======================================================
     * PRIORIDAD 1: Todos bloqueados — contingencia
     * ====================================================== */
    if (sensors_all_blocked()) {
        obstacle_confirm_count = 0;
        printf("[NAV] BLOQUEADO TOTAL — contingencia\n");
        motors_stop();
        sleep_ms(NAV_PAUSE_AFTER_STOP_MS);
        motors_reverse(NAV_SPEED_REVERSE);
        sleep_ms(NAV_REVERSE_TIME_MS + 200);
        motors_stop();
        sleep_ms(NAV_PAUSE_BEFORE_TURN_MS);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(NAV_TURN_90_MS);
        motors_stop();
        for (uint8_t i = 0; i < 5; i++) {
            sensors_trigger();
            sleep_ms(SENSOR_TRIGGER_WAIT_MS);
        }
        obstacle_confirm_count = 0;
        return true;
    }

    /* ======================================================
     * PRIORIDAD 1.5: Lateral critico — se sobrepone a todo
     * Actua incluso si frontal esta libre
     * ====================================================== */
    if (dist_left < NAV_OBSTACLE_SIDE_CRITICAL_CM) {
        obstacle_confirm_count = 0;
        //printf("[NAV] CRITICO izq (%dcm) — corrigiendo\n", dist_left);
        motors_stop();
        sleep_ms(100);
        motors_turn_right(NAV_SPEED_TURN);
        sleep_ms(300);
        motors_stop();
        return true;
    }

    if (dist_right < NAV_OBSTACLE_SIDE_CRITICAL_CM) {
        obstacle_confirm_count = 0;
        //printf("[NAV] CRITICO der (%dcm) — corrigiendo\n", dist_right);
        motors_stop();
        sleep_ms(100);
        motors_turn_left(NAV_SPEED_TURN);
        sleep_ms(300);
        motors_stop();
        return true;
    }

    /* ======================================================
     * PRIORIDAD 2: Obstaculo frontal — confirmar antes de actuar
     * ====================================================== */
    if (dist_center < NAV_OBSTACLE_FRONT_CM) {
        obstacle_confirm_count++;
    } else {
        obstacle_confirm_count = 0;

        /* ======================================================
         * PRIORIDAD 3: Frontal libre — correccion lateral proporcional
         * Motor del lado del obstaculo va mas lento segun distancia
         * ====================================================== */
        if (dist_left < NAV_OBSTACLE_SIDE_CM && dist_right >= NAV_OBSTACLE_SIDE_CM) {
            uint16_t slow = (dist_left < 6) ? 150 :
                            (dist_left < 8) ? 250 : 330;
            //printf("[NAV] Correccion derecha (izq:%dcm)\n", dist_left);
            motors_set(MOTOR_FORWARD, NAV_SPEED_FORWARD,
                       MOTOR_FORWARD, slow);
            return true;
        }

        if (dist_right < NAV_OBSTACLE_SIDE_CM && dist_left >= NAV_OBSTACLE_SIDE_CM) {
            uint16_t slow = (dist_right < 6) ? 150 :
                            (dist_right < 8) ? 250 : 330;
            //printf("[NAV] Correccion izquierda (der:%dcm)\n", dist_right);
            motors_set(MOTOR_FORWARD, slow,
                       MOTOR_FORWARD, NAV_SPEED_FORWARD);
            return true;
        }

        /* ======================================================
         * PRIORIDAD 4: Todo libre — avanzar
         * ====================================================== */
        motors_forward(NAV_SPEED_FORWARD);
        return true;
    }

    /* ======================================================
     * Maniobra evasiva con confirmacion suficiente
     * ====================================================== */
    if (obstacle_confirm_count >= NAV_CONFIRM_COUNT) {
        obstacle_confirm_count = 0;

        bool turn_left = (dist_left >= dist_right);
        printf("[NAV] Frontal bloqueado (%dcm) — maniobra %s (izq:%d der:%d)\n",
               dist_center,
               turn_left ? "izquierda" : "derecha",
               dist_left, dist_right);

        evasive_maneuver(turn_left);
    }

    /* Aun confirmando — frenar y actualizar lectura */
    motors_stop();
    sensors_trigger();
    sleep_ms(SENSOR_TRIGGER_WAIT_MS);
    return true;
}

void navigation_stop(void) {
    motors_stop();
    obstacle_confirm_count = 0;
}