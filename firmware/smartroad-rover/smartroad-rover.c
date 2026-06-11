/**
 * @file smartroad-rover.c
 * @brief Modo autonomo reactivo — SmartRoad Rover
 *
 * Loop principal con disparo secuencial de sensores.
 * Ciclo completo: ~90ms (3 sensores x 35ms) + tiempo de maniobra.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "motors.h"
#include "sensors.h"
#include "navigation.h"

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("=== SmartRoad Rover — Modo autonomo reactivo v2 ===\n");
    printf("Disparo secuencial de sensores activo.\n\n");

    motors_init();
    sensors_init();
    navigation_init();

    printf("Sistema listo. Iniciando navegacion...\n\n");

    while (true) {
        /* Disparar sensores de forma secuencial
         * evita ecos cruzados entre sensores */
        sensors_trigger_all_sequential();

        /* Ejecutar paso de navegacion con lecturas limpias */
        navigation_step();
    }

    return 0;
}