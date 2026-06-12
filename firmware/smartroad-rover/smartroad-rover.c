/**
 * @file smartroad-rover.c
 * @brief Punto de entrada principal — SmartRoad Rover
 *
 * Inicializa el sistema y ejecuta el loop principal
 * que despacha la FSM. La FSM gestiona tres modos:
 *
 *   AUTO — Navegacion autonoma reactiva (modo principal)
 *   WEB  — Control manual via WiFi
 *   GPS  — Navegacion hacia coordenada objetivo
 *
 * Seleccion de modo por serial USB:
 *   Al arrancar: menu con 5s de espera, default AUTO
 *   En cualquier momento: escribir AUTO, WEB o GPS
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "fsm.h"
#include "web_server.h"
#include "gps.h"

int main(void) {
    stdio_init_all();
    sleep_ms(2000);

    printf("=== SmartRoad Rover ===\n");
    printf("Iniciando sistema...\n\n");

    /* Inicializar WiFi — Access Point siempre activo
     * para permitir cambio a modo WEB en cualquier momento */
    web_server_init();

    /* Inicializar GPS */
    gps_init();

    /* Inicializar FSM — muestra menu, espera seleccion,
     * arranca en AUTO por defecto si no hay input */
    fsm_init();

    /* Loop principal */
    while (true) {
        fsm_run();
    }

    return 0;
}