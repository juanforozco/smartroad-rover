/**
 * @file fsm.c
 * @brief Implementacion de la FSM del sistema SmartRoad Rover
 *
 * Selector de modo por serial USB integrado directamente
 * en el loop principal, sin timers adicionales que puedan
 * interferir con el timing de las IRQ de sensores.
 *
 * La lectura serial es no bloqueante usando getchar_timeout_us(0).
 * Se ejecuta al final de cada ciclo de navegacion.
 *
 * Comandos seriales validos:
 *   AUTO → STATE_AUTO_REACTIVE
 *   WEB  → STATE_MANUAL
 *   GPS  → STATE_GPS
 *   MENU → muestra el menu
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "fsm.h"
#include "motors.h"
#include "sensors.h"
#include "navigation.h"
#include "web_server.h"
#include "gps.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* =========================================================
 * Variable de estado global
 * ========================================================= */

volatile system_state_t current_state = STATE_INIT;

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Buffer acumulador de comando serial */
static char serial_buf[16];

/** @brief Indice actual en el buffer serial */
static uint8_t serial_idx = 0;

/** @brief Flag: WiFi ya inicializado */
static bool wifi_initialized = false;

/** @brief Flag: GPS ya inicializado */
static bool gps_initialized = false;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Imprime el menu de seleccion de modo
 */
static void print_menu(void) {
    printf("\n========================================\n");
    printf("       SmartRoad Rover — Menu\n");
    printf("========================================\n");
    printf("  AUTO → Modo autonomo reactivo\n");
    printf("  WEB  → Modo manual via WiFi\n");
    printf("  GPS  → Modo navegacion GPS\n");
    printf("  MENU → Mostrar este menu\n");
    printf("========================================\n");
    printf("Modo actual: %s\n", fsm_state_name());
    printf("> ");
}

/**
 * @brief Procesa un comando serial completo
 * @param cmd String con el comando en mayusculas
 */
static void process_serial_command(const char *cmd) {
    if (strcmp(cmd, "AUTO") == 0) {
        printf("\n[FSM] Comando: AUTO\n");
        fsm_request_transition(STATE_AUTO_REACTIVE);
    } else if (strcmp(cmd, "WEB") == 0) {
        printf("\n[FSM] Comando: WEB\n");
        fsm_request_transition(STATE_MANUAL);
    } else if (strcmp(cmd, "GPS") == 0) {
        printf("\n[FSM] Comando: GPS\n");
        fsm_request_transition(STATE_GPS);
    } else if (strcmp(cmd, "MENU") == 0) {
        print_menu();
    } else {
        printf("\n[FSM] Comando no reconocido: '%s'\n", cmd);
        printf("Comandos: AUTO, WEB, GPS, MENU\n> ");
    }
}

/**
 * @brief Verifica y procesa un caracter serial disponible
 *
 * No bloqueante — usa getchar_timeout_us(0).
 * Acumula caracteres hasta recibir Enter.
 */
static void check_serial(void) {
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT) return;

    if (c == '\r' || c == '\n') {
        if (serial_idx > 0) {
            serial_buf[serial_idx] = '\0';
            for (int i = 0; serial_buf[i]; i++) {
                serial_buf[i] = toupper(serial_buf[i]);
            }
            process_serial_command(serial_buf);
            serial_idx = 0;
        }
    } else if (serial_idx < 15 && isprint(c)) {
        serial_buf[serial_idx++] = (char)c;
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void fsm_init(void) {
    current_state = STATE_INIT;

    motors_init();
    sensors_init();
    navigation_init();

    print_menu();
    printf("(Arrancando en AUTO en 5 segundos...)\n");

    /* Esperar seleccion — leer serial manualmente durante la espera */
    for (int i = 0; i < 50; i++) {
        check_serial();
        sleep_ms(100);
        if (current_state != STATE_INIT) break;
    }

    if (current_state == STATE_INIT) {
        printf("[FSM] Sin seleccion — modo AUTO por defecto\n");
        fsm_request_transition(STATE_AUTO_REACTIVE);
    }
}

void fsm_run(void) {
    switch (current_state) {

        case STATE_INIT:
            fsm_request_transition(STATE_AUTO_REACTIVE);
            break;

        case STATE_AUTO_REACTIVE:
            sensors_trigger();
            sleep_ms(SENSOR_TRIGGER_WAIT_MS);
            navigation_step();
            /* Verificar serial al final del ciclo — no bloqueante */
            check_serial();
            break;

        case STATE_MANUAL:
            web_server_poll();
            check_serial();

            if (web_server_mode_requested() == WEB_MODE_AUTO) {
                fsm_request_transition(STATE_AUTO_REACTIVE);
            }
            if (web_server_wifi_timeout()) {
                printf("[FSM] WiFi timeout — SAFE_STOP\n");
                fsm_request_transition(STATE_SAFE_STOP);
            }
            break;

        case STATE_GPS:
            sensors_trigger();
            sleep_ms(SENSOR_TRIGGER_WAIT_MS);
            gps_navigation_step();
            check_serial();

            if (!gps_signal_valid()) {
                printf("[FSM] GPS perdido — AUTO_REACTIVE\n");
                fsm_request_transition(STATE_AUTO_REACTIVE);
            }
            break;

        case STATE_SAFE_STOP:
            motors_stop();
            sleep_ms(100);
            printf("[FSM] SAFE_STOP — recuperando...\n");
            sleep_ms(1000);
            fsm_request_transition(STATE_AUTO_REACTIVE);
            break;
    }
}

void fsm_request_transition(system_state_t new_state) {
    if (new_state == current_state) return;

    printf("[FSM] %s → %s\n", fsm_state_name(),
           (new_state == STATE_INIT)          ? "INIT"          :
           (new_state == STATE_AUTO_REACTIVE)  ? "AUTO_REACTIVE" :
           (new_state == STATE_MANUAL)         ? "MANUAL"        :
           (new_state == STATE_GPS)            ? "GPS"           :
           (new_state == STATE_SAFE_STOP)      ? "SAFE_STOP"     : "?");

    if (current_state == STATE_AUTO_REACTIVE ||
        current_state == STATE_MANUAL        ||
        current_state == STATE_GPS) {
        motors_stop();
    }

    current_state = new_state;

    switch (new_state) {

        case STATE_AUTO_REACTIVE:
            printf("[FSM] Modo autonomo activo.\n");
            printf("[FSM] Escribe AUTO/WEB/GPS para cambiar.\n");
            break;

        case STATE_MANUAL:
            if (!wifi_initialized) {
                printf("[FSM] Iniciando WiFi...\n");
                if (web_server_init()) {
                    wifi_initialized = true;
                } else {
                    printf("[FSM] Error WiFi — volviendo a AUTO\n");
                    current_state = STATE_AUTO_REACTIVE;
                    return;
                }
            }
            web_server_reset_watchdog();
            printf("[FSM] Conectar a: %s / %s\n", WIFI_SSID, WIFI_PASSWORD);
            printf("[FSM] Acceso: http://192.168.4.1\n");
            break;

        case STATE_GPS:
            if (!gps_initialized) {
                printf("[FSM] Iniciando GPS...\n");
                gps_init();
                gps_initialized = true;
            }
            printf("[FSM] Esperando señal satelital...\n");
            break;

        case STATE_SAFE_STOP:
            motors_stop();
            break;

        default:
            break;
    }
}

const char *fsm_state_name(void) {
    switch (current_state) {
        case STATE_INIT:          return "INIT";
        case STATE_AUTO_REACTIVE: return "AUTO_REACTIVE";
        case STATE_MANUAL:        return "MANUAL";
        case STATE_GPS:           return "GPS";
        case STATE_SAFE_STOP:     return "SAFE_STOP";
        default:                  return "UNKNOWN";
    }
}

void fsm_check_serial_command(void) {
    check_serial();
}