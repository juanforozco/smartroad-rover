/**
 * @file fsm.h
 * @brief Maquina de estados finita (FSM) del sistema SmartRoad Rover
 *
 * Gestiona los modos de operacion garantizando que solo
 * un modo este activo en cualquier instante (RF-S2).
 *
 * Estados:
 *   STATE_INIT          — Inicializacion del sistema
 *   STATE_AUTO_REACTIVE — Navegacion autonoma reactiva (modo principal)
 *   STATE_MANUAL        — Control manual via WiFi (complementario)
 *   STATE_GPS           — Navegacion GPS hacia coordenada objetivo
 *   STATE_SAFE_STOP     — Detencion de emergencia
 *
 * Seleccion de modo:
 *   Por serial USB en cualquier momento:
 *     "AUTO" → STATE_AUTO_REACTIVE
 *     "WEB"  → STATE_MANUAL
 *     "GPS"  → STATE_GPS
 *   Al arrancar: espera 5s, default AUTO si no hay input
 *
 * Transiciones de contingencia:
 *   MANUAL → SAFE_STOP (WiFi timeout > 500ms, RNF-5)
 *   GPS    → AUTO_REACTIVE (señal GPS perdida > 3s, RNF-6)
 *   SAFE_STOP → AUTO_REACTIVE (recuperacion automatica)
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef FSM_H
#define FSM_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Estados de la FSM
 * ========================================================= */

/**
 * @brief Estados del sistema
 */
typedef enum {
    STATE_INIT,           ///< Inicializacion — transitorio al arrancar
    STATE_AUTO_REACTIVE,  ///< Modo autonomo reactivo (principal)
    STATE_MANUAL,         ///< Modo manual via WiFi (complementario)
    STATE_GPS,            ///< Modo GPS hacia coordenada objetivo
    STATE_SAFE_STOP       ///< Detencion de emergencia
} system_state_t;

/* =========================================================
 * Variable de estado global
 * volatile: visible desde loop principal e IRQ
 * ========================================================= */

/** @brief Estado actual del sistema */
extern volatile system_state_t current_state;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa la FSM y todos los modulos de hardware
 *
 * Muestra el menu de seleccion de modo por serial,
 * espera 5 segundos y arranca en AUTO si no hay input.
 */
void fsm_init(void);

/**
 * @brief Ejecuta un ciclo de la FSM
 *
 * Despacha la logica del estado activo y evalua transiciones.
 * Debe llamarse desde el loop principal.
 */
void fsm_run(void);

/**
 * @brief Solicita una transicion de estado
 *
 * @param new_state Estado destino
 */
void fsm_request_transition(system_state_t new_state);

/**
 * @brief Retorna el nombre del estado actual
 *
 * @return String con el nombre del estado
 */
const char *fsm_state_name(void);

/**
 * @brief Verifica y procesa comandos seriales pendientes
 *
 * Llamada periodicamente por un repeating timer de hardware.
 * Lee stdin y actualiza el estado si hay un comando valido.
 */
void fsm_check_serial_command(void);

#endif /* FSM_H */