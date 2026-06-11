/**
 * @file smartroad-rover.c
 * @brief Prueba del modulo de sensores — SmartRoad Rover
 *
 * Lee los tres sensores HC-SR04 y muestra las distancias
 * por el monitor serial cada 500ms.
 * Verificar que las lecturas son coherentes con la realidad fisica.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "sensors.h"

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("=== SmartRoad Rover — Prueba de sensores ===\n");

    sensors_init();
    printf("Sensores inicializados.\n\n");

    while (true) {
        /* Disparar los tres sensores */
        sensors_trigger();

        /* Esperar a que lleguen los ecos (max ~30ms para 5m) */
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);

        /* Leer distancias filtradas */
        uint16_t dist_right  = sensors_get_distance(SENSOR_RIGHT);
        uint16_t dist_center = sensors_get_distance(SENSOR_CENTER);
        uint16_t dist_left   = sensors_get_distance(SENSOR_LEFT);

        /* Mostrar por serial */
        printf("IZQ: %3d cm | CENTRO: %3d cm | DER: %3d cm",
               dist_left, dist_center, dist_right);

        /* Indicar si hay obstaculo */
        if (sensors_all_blocked()) {
            printf("  [!!! BLOQUEADO TOTAL !!!]");
        } else {
            if (sensors_obstacle_detected(SENSOR_LEFT))   printf("  [OBS IZQ]");
            if (sensors_obstacle_detected(SENSOR_CENTER)) printf("  [OBS CENTRO]");
            if (sensors_obstacle_detected(SENSOR_RIGHT))  printf("  [OBS DER]");
        }
        printf("\n");

        sleep_ms(500);
    }

    return 0;
}