/**
 * @file smartroad-rover.c
 * @brief Prueba del modo autonomo reactivo — SmartRoad Rover
 *
 * Integra sensores, motores y navegacion para probar
 * el modo autonomo reactivo completo.
 *
 * Ciclo principal:
 *   1. Disparar sensores (polling)
 *   2. Esperar ecos (IRQ en background)
 *   3. Ejecutar paso de navegacion
 *   4. Repetir
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

    printf("=== SmartRoad Rover — Modo autonomo reactivo ===\n");

    /* Inicializar modulos en orden correcto */
    motors_init();
    sensors_init();
    navigation_init();

    printf("Sistema listo. Iniciando navegacion autonoma...\n\n");

    while (true) {
        /* Disparar los tres sensores simultaneamente */
        sensors_trigger();

        /* Esperar ecos — las IRQ actualizan los buffers en background */
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);

        /* Ejecutar un paso del algoritmo de navegacion */
        navigation_step();
    }

    return 0;
}