/**
 * @file motors.h
 * @brief Control de motores DC via TB6612FNG
 *
 * Abstraccion del driver TB6612FNG para control de dos motores DC.
 * Usa PWM de hardware del RP2040 para control de velocidad y
 * señales digitales GPIO para control de direccion.
 *
 * Orientacion (viendo el rover como si se fuera montado en el):
 *   Motor A = llanta IZQUIERDA (GP7, GP8, GP9)
 *   Motor B = llanta DERECHA   (GP11, GP12, GP13)
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef MOTORS_H
#define MOTORS_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Definicion de pines
 * ========================================================= */

/** @brief Pines Motor A (izquierdo viendo el rover como si fuera montado) */
#define MOTOR_A_IN1   7   ///< GP7  - Control direccion A1
#define MOTOR_A_IN2   8   ///< GP8  - Control direccion A2
#define MOTOR_A_PWM   9   ///< GP9  - Control velocidad PWM

/** @brief Pines Motor B (derecho viendo el rover como si fuera montado) */
#define MOTOR_B_IN1  11   ///< GP11 - Control direccion B1
#define MOTOR_B_IN2  12   ///< GP12 - Control direccion B2
#define MOTOR_B_PWM  13   ///< GP13 - Control velocidad PWM

/* =========================================================
 * Parametros de PWM
 * ========================================================= */

/** @brief Frecuencia PWM en Hz (20kHz - inaudible para motores DC) */
#define MOTORS_PWM_FREQ_HZ   20000

/** @brief Valor maximo del wrap del PWM (resolucion 0-999) */
#define MOTORS_PWM_WRAP      999

/** @brief Velocidad maxima (equivale a 100% ciclo de trabajo) */
#define MOTORS_SPEED_MAX     999

/** @brief Velocidad de giro para maniobras de evasion */
#define MOTORS_SPEED_TURN    700

/** @brief Velocidad de avance normal */
#define MOTORS_SPEED_FULL    900

/* =========================================================
 * Tipos
 * ========================================================= */

/**
 * @brief Direccion de giro de un motor
 */
typedef enum {
    MOTOR_FORWARD,   ///< Giro hacia adelante
    MOTOR_REVERSE,   ///< Giro hacia atras
    MOTOR_STOP,      ///< Motor detenido (freno activo)
    MOTOR_COAST      ///< Motor libre (sin freno)
} motor_dir_t;

/**
 * @brief Identificador de motor
 */
typedef enum {
    MOTOR_A,    ///< Motor A - lado izquierdo
    MOTOR_B,    ///< Motor B - lado derecho
    MOTOR_BOTH  ///< Ambos motores simultaneamente
} motor_id_t;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de motores
 *
 * Configura los pines GPIO de direccion y los slices PWM
 * de hardware para ambos motores. Deja los motores detenidos.
 * Debe llamarse una sola vez al inicio del programa.
 */
void motors_init(void);

/**
 * @brief Controla velocidad y direccion de ambos motores
 *
 * @param dir_a   Direccion del motor A (izquierdo)
 * @param speed_a Velocidad del motor A (0 a MOTORS_PWM_WRAP)
 * @param dir_b   Direccion del motor B (derecho)
 * @param speed_b Velocidad del motor B (0 a MOTORS_PWM_WRAP)
 */
void motors_set(motor_dir_t dir_a, uint16_t speed_a,
                motor_dir_t dir_b, uint16_t speed_b);

/**
 * @brief Avanza el rover en linea recta
 * @param speed Velocidad (0 a MOTORS_PWM_WRAP)
 */
void motors_forward(uint16_t speed);

/**
 * @brief Retrocede el rover en linea recta
 * @param speed Velocidad (0 a MOTORS_PWM_WRAP)
 */
void motors_reverse(uint16_t speed);

/**
 * @brief Gira el rover a la izquierda (en su propio eje)
 * @param speed Velocidad de giro (0 a MOTORS_PWM_WRAP)
 */
void motors_turn_left(uint16_t speed);

/**
 * @brief Gira el rover a la derecha (en su propio eje)
 * @param speed Velocidad de giro (0 a MOTORS_PWM_WRAP)
 */
void motors_turn_right(uint16_t speed);

/**
 * @brief Detiene ambos motores con freno activo
 */
void motors_stop(void);

#endif /* MOTORS_H */