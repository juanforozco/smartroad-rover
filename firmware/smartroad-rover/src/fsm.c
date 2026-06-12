/**
 * @file fsm.c
 * @brief Implementacion de la FSM del sistema SmartRoad Rover
 *
 * La FSM despacha la logica de cada modo y gestiona las
 * transiciones entre estados. Solo un estado activo a la vez.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "fsm.h"
#include "motors.h"
#include "sensors.h"
#include "navigation.h"
#include "web_server.h"
#include "pico/stdlib.h"
#include <stdio.h>

/* =========================================================
 * Variable de estado global
 * ========================================================= */

/** @brief Estado actual del sistema */
volatile system_state_t current_state = STATE_INIT;

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void fsm_init(void) {
    current_state = STATE_INIT;
    printf("[FSM] Inicializando sistema...\n");

    /* Inicializar todos los modulos de hardware */
    motors_init();
    sensors_init();
    navigation_init();

    printf("[FSM] Sistema listo.\n");

    /* Transicion automatica al modo autonomo por defecto */
    fsm_request_transition(STATE_AUTO_REACTIVE);
}

void fsm_run(void) {
    switch (current_state) {

        case STATE_INIT:
            /* Estado transitorio — fsm_init() ya manejo la transicion */
            fsm_request_transition(STATE_AUTO_REACTIVE);
            break;

        case STATE_AUTO_REACTIVE:
            /* Disparar sensores y ejecutar navegacion autonoma */
            sensors_trigger();
            sleep_ms(SENSOR_TRIGGER_WAIT_MS);
            navigation_step();

            /* Verificar si web_server solicito cambio de modo */
            if (web_server_mode_requested() == WEB_MODE_MANUAL) {
                fsm_request_transition(STATE_MANUAL);
            }
            break;

        case STATE_MANUAL:
            /* Servir peticiones HTTP y ejecutar comandos */
            web_server_poll();

            /* Verificar si se solicito cambio a autonomo */
            if (web_server_mode_requested() == WEB_MODE_AUTO) {
                fsm_request_transition(STATE_AUTO_REACTIVE);
            }

            /* Verificar watchdog WiFi (RNF-5) */
            if (web_server_wifi_timeout()) {
                printf("[FSM] WiFi timeout — transicion a SAFE_STOP\n");
                fsm_request_transition(STATE_SAFE_STOP);
            }
            break;

        case STATE_SAFE_STOP:
            /* Detener motores y esperar recuperacion */
            motors_stop();
            sleep_ms(100);

            /* Recuperacion automatica al modo autonomo */
            printf("[FSM] SAFE_STOP — recuperando en modo autonomo\n");
            sleep_ms(1000);
            fsm_request_transition(STATE_AUTO_REACTIVE);
            break;
    }
}

void fsm_request_transition(system_state_t new_state) {
    if (new_state == current_state) return;

    printf("[FSM] %s → %s\n",
           fsm_state_name(),
           (new_state == STATE_INIT)          ? "INIT" :
           (new_state == STATE_AUTO_REACTIVE)  ? "AUTO_REACTIVE" :
           (new_state == STATE_MANUAL)         ? "MANUAL" :
           (new_state == STATE_SAFE_STOP)      ? "SAFE_STOP" : "?");

    /* Acciones de salida del estado actual */
    if (current_state == STATE_AUTO_REACTIVE ||
        current_state == STATE_MANUAL) {
        motors_stop();
    }

    current_state = new_state;

    /* Acciones de entrada al nuevo estado */
    if (new_state == STATE_MANUAL) {
        web_server_reset_watchdog();
    }
}

const char *fsm_state_name(void) {
    switch (current_state) {
        case STATE_INIT:          return "INIT";
        case STATE_AUTO_REACTIVE: return "AUTO_REACTIVE";
        case STATE_MANUAL:        return "MANUAL";
        case STATE_SAFE_STOP:     return "SAFE_STOP";
        default:                  return "UNKNOWN";
    }
}