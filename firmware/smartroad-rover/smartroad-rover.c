/**
 * @file smartroad-rover.c
 * @brief Prueba del modulo de motores — SmartRoad Rover
 *
 * Prueba secuencial de todas las funciones del modulo motors:
 * adelante, atras, giro izquierda, giro derecha y stop.
 * Verificar visualmente que cada movimiento es correcto.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "motors.h"

int main(void) {
    /* Inicializar stdio para debug por USB */
    stdio_init_all();
    sleep_ms(2000); /* Esperar a que el puerto USB se estabilice */

    printf("=== SmartRoad Rover — Prueba de motores ===\n");

    /* Inicializar modulo de motores */
    motors_init();
    printf("Motores inicializados.\n");

    while (true) {

        /* --- Adelante --- */
        printf("ADELANTE...\n");
        motors_forward(MOTORS_SPEED_FULL);
        sleep_ms(2000);

        /* --- Stop --- */
        printf("STOP...\n");
        motors_stop();
        sleep_ms(1000);

        /* --- Atras --- */
        printf("ATRAS...\n");
        motors_reverse(MOTORS_SPEED_FULL);
        sleep_ms(2000);

        /* --- Stop --- */
        printf("STOP...\n");
        motors_stop();
        sleep_ms(1000);

        /* --- Giro izquierda --- */
        printf("GIRO IZQUIERDA...\n");
        motors_turn_left(MOTORS_SPEED_TURN);
        sleep_ms(1000);

        /* --- Stop --- */
        printf("STOP...\n");
        motors_stop();
        sleep_ms(1000);

        /* --- Giro derecha --- */
        printf("GIRO DERECHA...\n");
        motors_turn_right(MOTORS_SPEED_TURN);
        sleep_ms(1000);

        /* --- Stop --- */
        printf("STOP...\n");
        motors_stop();
        sleep_ms(2000);

        printf("--- Ciclo completo, repitiendo ---\n\n");
    }

    return 0;
}