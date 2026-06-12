/**
 * @file fsm.h
 * @brief Maquina de estados finita (FSM) del sistema SmartRoad Rover
 *
 * Gestiona los modos de operacion del sistema garantizando que
 * solo un modo este activo en cualquier instante (RF-S2).
 *
 * Estados:
 *   STATE_INIT          — Inicializacion del sistema
 *   STATE_AUTO_REACTIVE — Navegacion autonoma reactiva (modo principal)
 *   STATE_MANUAL        — Control manual via WiFi (modo complementario)
 *   STATE_SAFE_STOP     — Detencion de emergencia
 *
 * Transiciones:
 *   INIT → AUTO_REACTIVE (por defecto al arrancar)
 *   Cualquier estado → MANUAL (comando web: "mode=manual")
 *   Cualquier estado → AUTO_REACTIVE (comando web: "mode=auto")
 *   MANUAL → SAFE_STOP (perdida de WiFi > 500ms, RNF-5)
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
    STATE_SAFE_STOP       ///< Detencion de emergencia
} system_state_t;

/* =========================================================
 * Variable de estado global
 * volatile: visible tanto desde el loop principal como desde IRQ
 * ========================================================= */

/** @brief Estado actual del sistema — accesible desde todos los modulos */
extern volatile system_state_t current_state;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa la FSM
 *
 * Establece el estado inicial como STATE_INIT y prepara
 * la transicion automatica al modo por defecto.
 */
void fsm_init(void);

/**
 * @brief Ejecuta un ciclo de la FSM
 *
 * Debe llamarse desde el loop principal. Despacha la logica
 * correspondiente al estado activo y evalua transiciones.
 */
void fsm_run(void);

/**
 * @brief Solicita una transicion de estado
 *
 * Valida que la transicion sea legal antes de aplicarla.
 * Puede llamarse desde cualquier modulo o desde la ISR de WiFi.
 *
 * @param new_state Estado destino
 */
void fsm_request_transition(system_state_t new_state);

/**
 * @brief Retorna el nombre del estado actual como string
 *
 * Para uso en mensajes de debug y pagina web.
 *
 * @return Puntero a string con el nombre del estado
 */
const char *fsm_state_name(void);

#endif /* FSM_H */