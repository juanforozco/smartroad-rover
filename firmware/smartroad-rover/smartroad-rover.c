/**
 * @file smartroad-rover.c
 * @brief Modo autonomo reactivo — SmartRoad Rover
 *
 * Loop principal: disparo de sensores + paso de navegacion.
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

    motors_init();
    sensors_init();
    navigation_init();

    printf("Sistema listo. Iniciando navegacion...\n\n");

    while (true) {
        sensors_trigger();
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);
        navigation_step();
    }

    return 0;
}