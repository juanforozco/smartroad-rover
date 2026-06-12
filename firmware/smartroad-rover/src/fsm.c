/**
 * @file fsm.c
 * @brief Implementacion de la FSM del sistema SmartRoad Rover
 *
 * Incluye selector de modo por serial USB usando un
 * repeating timer de hardware del RP2040. El timer verifica
 * stdin cada 100ms sin bloquear el loop principal.
 *
 * Comandos seriales validos (en cualquier momento):
 *   AUTO → STATE_AUTO_REACTIVE
 *   WEB  → STATE_MANUAL
 *   GPS  → STATE_GPS
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
#include "hardware/timer.h"
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

/** @brief Timer para verificacion periodica de serial */
static repeating_timer_t serial_timer;

/** @brief Buffer acumulador de comando serial */
static char serial_buf[16];

/** @brief Indice actual en el buffer serial */
static uint8_t serial_idx = 0;

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
    printf("========================================\n");
    printf("Modo actual: %s\n", fsm_state_name());
    printf("Escribe un comando y presiona Enter:\n> ");
}

/**
 * @brief Procesa un comando serial recibido
 *
 * @param cmd String con el comando (ya en mayusculas)
 */
static void process_serial_command(const char *cmd) {
    if (strcmp(cmd, "AUTO") == 0) {
        printf("\n[FSM] Comando serial: AUTO\n");
        fsm_request_transition(STATE_AUTO_REACTIVE);
    } else if (strcmp(cmd, "WEB") == 0) {
        printf("\n[FSM] Comando serial: WEB\n");
        fsm_request_transition(STATE_MANUAL);
    } else if (strcmp(cmd, "GPS") == 0) {
        printf("\n[FSM] Comando serial: GPS\n");
        fsm_request_transition(STATE_GPS);
    } else if (strcmp(cmd, "MENU") == 0) {
        print_menu();
    } else {
        printf("\n[FSM] Comando no reconocido: '%s'\n", cmd);
        printf("Comandos validos: AUTO, WEB, GPS, MENU\n> ");
    }
}

/**
 * @brief Callback del repeating timer — verifica serial cada 100ms
 *
 * Lee caracteres disponibles en stdin y acumula en buffer.
 * Al recibir Enter, procesa el comando completo.
 *
 * @param t Puntero al timer (no usado)
 * @return true para mantener el timer activo
 */
static bool serial_timer_callback(repeating_timer_t *t) {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\r' || c == '\n') {
            if (serial_idx > 0) {
                serial_buf[serial_idx] = '\0';
                /* Convertir a mayusculas */
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
    return true; /* Mantener timer activo */
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void fsm_init(void) {
    current_state = STATE_INIT;

    /* Inicializar modulos de hardware */
    motors_init();
    sensors_init();
    navigation_init();

    /* Iniciar timer de hardware para verificacion serial
     * Periodo: 100ms — no bloqueante, corre en background */
    add_repeating_timer_ms(100, serial_timer_callback, NULL, &serial_timer);

    /* Mostrar menu y esperar seleccion */
    print_menu();
    printf("(Arrancando en AUTO en 5 segundos...)\n");

    /* Esperar 5 segundos — el timer atiende input serial */
    sleep_ms(5000);

    /* Si no hubo transicion, arrancar en AUTO por defecto */
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

            if (web_server_mode_requested() == WEB_MODE_MANUAL) {
                fsm_request_transition(STATE_MANUAL);
            }
            break;

        case STATE_MANUAL:
            web_server_poll();

            if (web_server_mode_requested() == WEB_MODE_AUTO) {
                fsm_request_transition(STATE_AUTO_REACTIVE);
            }

            if (web_server_wifi_timeout()) {
                printf("[FSM] WiFi timeout — SAFE_STOP\n");
                fsm_request_transition(STATE_SAFE_STOP);
            }
            break;

        case STATE_GPS:
            /* Ejecutar paso de navegacion GPS
             * Los sensores siguen activos durante GPS (RF-G5) */
            sensors_trigger();
            sleep_ms(SENSOR_TRIGGER_WAIT_MS);
            gps_navigation_step();

            /* Verificar perdida de señal GPS (RNF-6) */
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

    /* Acciones de salida */
    if (current_state == STATE_AUTO_REACTIVE ||
        current_state == STATE_MANUAL ||
        current_state == STATE_GPS) {
        motors_stop();
    }

    current_state = new_state;

    /* Acciones de entrada */
    if (new_state == STATE_MANUAL) {
        web_server_reset_watchdog();
        printf("[FSM] WiFi activo. Conectar a: %s\n", WIFI_SSID);
        printf("[FSM] Acceso web: http://192.168.4.1\n");
    }
    if (new_state == STATE_AUTO_REACTIVE) {
        printf("[FSM] Navegacion autonoma activa.\n");
        printf("[FSM] Escribe AUTO/WEB/GPS para cambiar modo.\n");
    }
    if (new_state == STATE_GPS) {
        printf("[FSM] Modo GPS activo.\n");
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
    /* La verificacion serial la maneja el repeating timer
     * Esta funcion existe para compatibilidad de API */
}