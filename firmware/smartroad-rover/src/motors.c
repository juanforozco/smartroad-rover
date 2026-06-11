/**
 * @file motors.c
 * @brief Implementacion del control de motores DC via TB6612FNG
 *
 * Usa PWM de hardware del RP2040 a 20kHz para control de velocidad.
 * Cada motor tiene su propio slice PWM independiente.
 * Las señales de direccion son GPIO digitales simples.
 *
 * Orientacion (viendo el rover como si se fuera montado en el):
 *   Motor A = llanta IZQUIERDA
 *   Motor B = llanta DERECHA
 *
 * Tabla de verdad TB6612FNG:
 *   IN1=1, IN2=0 → adelante
 *   IN1=0, IN2=1 → atras
 *   IN1=1, IN2=1 → freno (SHORT BRAKE)
 *   IN1=0, IN2=0 → libre (COAST)
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "motors.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Numero de slice PWM del motor A */
static uint slice_a;

/** @brief Numero de slice PWM del motor B */
static uint slice_b;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Configura un pin GPIO como salida digital para direccion
 * @param pin Numero del pin GPIO
 */
static void setup_dir_pin(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

/**
 * @brief Configura un pin como salida PWM de hardware
 * @param pin   Numero del pin GPIO
 * @param slice Puntero donde se guarda el numero de slice asignado
 */
static void setup_pwm_pin(uint pin, uint *slice) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(pin);

    pwm_config cfg = pwm_get_default_config();

    /* Divisor de reloj para obtener 20kHz
     * CLK_SYS = 125MHz
     * wrap = 999
     * divisor = 125MHz / (20000 x 1000) = 6.25 */
    pwm_config_set_clkdiv(&cfg, 6.25f);
    pwm_config_set_wrap(&cfg, MOTORS_PWM_WRAP);
    pwm_init(*slice, &cfg, true);

    /* Iniciar con ciclo de trabajo 0 (motor detenido) */
    pwm_set_gpio_level(pin, 0);
}

/**
 * @brief Aplica direccion y velocidad a un motor
 *
 * @param in1   Pin IN1 del motor
 * @param in2   Pin IN2 del motor
 * @param pwm   Pin PWM del motor
 * @param dir   Direccion deseada
 * @param speed Velocidad (0 a MOTORS_PWM_WRAP)
 */
static void apply_motor(uint in1, uint in2, uint pwm,
                        motor_dir_t dir, uint16_t speed,
                        bool invert) {
    /* Si el motor esta fisicamente invertido, swap de direccion */
    if (invert) {
        if (dir == MOTOR_FORWARD) dir = MOTOR_REVERSE;
        else if (dir == MOTOR_REVERSE) dir = MOTOR_FORWARD;
    }

    switch (dir) {
        case MOTOR_FORWARD:
            gpio_put(in1, 1);
            gpio_put(in2, 0);
            pwm_set_gpio_level(pwm, speed);
            break;
        case MOTOR_REVERSE:
            gpio_put(in1, 0);
            gpio_put(in2, 1);
            pwm_set_gpio_level(pwm, speed);
            break;
        case MOTOR_STOP:
            gpio_put(in1, 1);
            gpio_put(in2, 1);
            pwm_set_gpio_level(pwm, 0);
            break;
        case MOTOR_COAST:
            gpio_put(in1, 0);
            gpio_put(in2, 0);
            pwm_set_gpio_level(pwm, 0);
            break;
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void motors_init(void) {
    /* Configurar pines de direccion Motor A (izquierdo) */
    setup_dir_pin(MOTOR_A_IN1);
    setup_dir_pin(MOTOR_A_IN2);

    /* Configurar pines de direccion Motor B (derecho) */
    setup_dir_pin(MOTOR_B_IN1);
    setup_dir_pin(MOTOR_B_IN2);

    /* Configurar PWM Motor A y Motor B */
    setup_pwm_pin(MOTOR_A_PWM, &slice_a);
    setup_pwm_pin(MOTOR_B_PWM, &slice_b);

    /* Dejar motores detenidos al iniciar */
    motors_stop();
}

void motors_set(motor_dir_t dir_a, uint16_t speed_a,
                motor_dir_t dir_b, uint16_t speed_b) {
    /* Motor B esta fisicamente invertido, se invierte su direccion */
    motor_dir_t dir_b_real = dir_b;
    if (dir_b == MOTOR_FORWARD)  dir_b_real = MOTOR_REVERSE;
    else if (dir_b == MOTOR_REVERSE) dir_b_real = MOTOR_FORWARD;

    apply_motor(MOTOR_A_IN1, MOTOR_A_IN2, MOTOR_A_PWM, dir_a,      speed_a, false);
    apply_motor(MOTOR_B_IN1, MOTOR_B_IN2, MOTOR_B_PWM, dir_b_real, speed_b, false);
}

void motors_forward(uint16_t speed) {
    motors_set(MOTOR_FORWARD, speed, MOTOR_FORWARD, speed);
}

void motors_reverse(uint16_t speed) {
    motors_set(MOTOR_REVERSE, speed, MOTOR_REVERSE, speed);
}

void motors_turn_left(uint16_t speed) {
    /* Motor A (izquierdo) atras, Motor B (derecho) adelante */
    motors_set(MOTOR_REVERSE, speed, MOTOR_FORWARD, speed);
}

void motors_turn_right(uint16_t speed) {
    /* Motor A (izquierdo) adelante, Motor B (derecho) atras */
    motors_set(MOTOR_FORWARD, speed, MOTOR_REVERSE, speed);
}

void motors_stop(void) {
    motors_set(MOTOR_STOP, 0, MOTOR_STOP, 0);
}