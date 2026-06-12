/**
 * @file smartroad-rover.c
 * @brief Punto de entrada principal — SmartRoad Rover
 *
 * Modo activo: autonomo reactivo.
 *
 * El sistema arranca directamente en modo autonomo reactivo,
 * que es el modo principal del proyecto (RF-A1 a RF-A10).
 *
 * Modulos de FSM y servidor web estan implementados en
 * src/fsm.c y src/web_server.c como parte de la arquitectura
 * disenada, pero la integracion completa queda como trabajo
 * futuro (ver README).
 *
 * Flujo de programa:
 *   1. Inicializar stdio USB para debug
 *   2. Inicializar modulos: motors, sensors, navigation
 *   3. Loop principal: trigger sensores + navigation_step()
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
    /* Inicializar stdio por USB para debug */
    stdio_init_all();
    sleep_ms(2000);

    printf("=== SmartRoad Rover ===\n");
    printf("Modo: Autonomo Reactivo\n\n");

    /* Inicializar modulos de hardware */
    motors_init();
    sensors_init();
    navigation_init();

    printf("Sistema listo. Iniciando navegacion...\n\n");

    /* Loop principal — polling + IRQ */
    while (true) {
        sensors_trigger();
        sleep_ms(SENSOR_TRIGGER_WAIT_MS);
        navigation_step();
    }

    return 0;
}