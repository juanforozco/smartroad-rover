/**
 * @file sensors.c
 * @brief Implementacion de sensores HC-SR04 con disparo secuencial
 *
 * v2 - Disparo secuencial para eliminar ecos cruzados.
 *
 * Problema identificado con disparo simultaneo:
 *   Con 3 sensores en el mismo TRIG, el eco del sensor A puede
 *   llegar al sensor B antes que su propio eco, generando lecturas
 *   falsas cortas que confunden al algoritmo de navegacion.
 *
 * Solucion implementada:
 *   Disparar un sensor a la vez. Esperar SENSOR_INTER_TRIGGER_MS
 *   para que los ecos se disipen antes del siguiente disparo.
 *   A 30ms por sensor, el ciclo completo es ~90ms — suficientemente
 *   rapido para navegacion reactiva a velocidad moderada.
 *
 * La medicion sigue siendo por IRQ: no hay polling bloqueante
 * esperando el ECHO. El CPU espera el tiempo de disipacion pero
 * las IRQ capturan el flanco exacto del ECHO con precision de us.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "sensors.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

/* =========================================================
 * Estructura interna de canal
 * ========================================================= */

typedef struct {
    uint     echo_pin;
    uint32_t t_rise;
    uint16_t buffer[SENSOR_FILTER_SIZE];
    uint8_t  buf_idx;
    bool     valid;
} sensor_channel_t;

/* =========================================================
 * Variables privadas
 * ========================================================= */

static sensor_channel_t sensors[SENSOR_COUNT] = {
    [SENSOR_RIGHT]  = { .echo_pin = SENSOR_ECHO_RIGHT,  .buf_idx = 0, .valid = false },
    [SENSOR_CENTER] = { .echo_pin = SENSOR_ECHO_CENTER, .buf_idx = 0, .valid = false },
    [SENSOR_LEFT]   = { .echo_pin = SENSOR_ECHO_LEFT,   .buf_idx = 0, .valid = false },
};

/** @brief Sensor actualmente esperando respuesta ECHO */
static volatile sensor_id_t active_sensor = SENSOR_COUNT;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Convierte pin GPIO a indice de sensor
 */
static sensor_id_t pin_to_sensor(uint gpio) {
    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        if (sensors[i].echo_pin == gpio) return i;
    }
    return SENSOR_COUNT;
}

/**
 * @brief Calcula mediana de tres valores uint16_t
 */
static uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
    uint16_t tmp;
    if (a > b) { tmp = a; a = b; b = tmp; }
    if (b > c) { tmp = b; b = c; c = tmp; }
    if (a > b) { tmp = a; a = b; b = tmp; }
    (void)a; (void)c;
    return b;
}

/**
 * @brief ISR compartida para los tres pines ECHO
 *
 * Solo procesa el sensor activo para evitar que ecos
 * residuales de disparos anteriores contaminen la lectura.
 */
static void echo_irq_handler(uint gpio, uint32_t events) {
    sensor_id_t idx = pin_to_sensor(gpio);

    /* Solo procesar el sensor que fue disparado activamente */
    if (idx != active_sensor) return;

    if (events & GPIO_IRQ_EDGE_RISE) {
        sensors[idx].t_rise = time_us_32();

    } else if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t pulse_us = time_us_32() - sensors[idx].t_rise;

        /* dist_cm = pulse_us * 17 / 1000
         * (velocidad sonido 340m/s → 0.017 cm/us → *17/1000) */
        uint16_t dist_cm = (uint16_t)((pulse_us * 17UL) / 1000UL);

        if (dist_cm > SENSOR_MAX_DISTANCE_CM) {
            dist_cm = SENSOR_MAX_DISTANCE_CM;
        }

        sensors[idx].buffer[sensors[idx].buf_idx] = dist_cm;
        sensors[idx].buf_idx = (sensors[idx].buf_idx + 1) % SENSOR_FILTER_SIZE;
        sensors[idx].valid = true;
    }
}

/**
 * @brief Dispara un sensor individual y espera su eco
 *
 * Activa el sensor como activo, genera pulso TRIG de 10us,
 * espera el tiempo de eco y luego desactiva.
 *
 * @param id Sensor a disparar
 */
static void trigger_single(sensor_id_t id) {
    /* Marcar sensor activo para que la ISR lo procese */
    active_sensor = id;

    /* Pulso TRIG de 10us */
    gpio_put(SENSOR_TRIG_PIN, 1);
    busy_wait_us(10);
    gpio_put(SENSOR_TRIG_PIN, 0);

    /* Esperar tiempo de eco — IRQ captura el resultado */
    sleep_ms(SENSOR_TRIGGER_WAIT_MS);

    /* Desactivar sensor activo */
    active_sensor = SENSOR_COUNT;
}

/* =========================================================
 * API publica
 * ========================================================= */

void sensors_init(void) {
    active_sensor = SENSOR_COUNT;

    /* Configurar TRIG como salida */
    gpio_init(SENSOR_TRIG_PIN);
    gpio_set_dir(SENSOR_TRIG_PIN, GPIO_OUT);
    gpio_put(SENSOR_TRIG_PIN, 0);

    /* Configurar cada ECHO con IRQ en ambos flancos */
    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        gpio_init(sensors[i].echo_pin);
        gpio_set_dir(sensors[i].echo_pin, GPIO_IN);

        gpio_set_irq_enabled_with_callback(
            sensors[i].echo_pin,
            GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
            true,
            &echo_irq_handler
        );

        /* Inicializar buffer con distancia maxima */
        for (uint8_t j = 0; j < SENSOR_FILTER_SIZE; j++) {
            sensors[i].buffer[j] = SENSOR_MAX_DISTANCE_CM;
        }
    }
}

void sensors_trigger_all_sequential(void) {
    /* Disparar cada sensor con pausa entre ellos
     * para que los ecos del anterior se disipen */
    trigger_single(SENSOR_CENTER);
    sleep_ms(SENSOR_INTER_TRIGGER_MS);

    trigger_single(SENSOR_RIGHT);
    sleep_ms(SENSOR_INTER_TRIGGER_MS);

    trigger_single(SENSOR_LEFT);
    /* No hace falta esperar al final — navigation_step lo maneja */
}

uint16_t sensors_get_distance(sensor_id_t id) {
    if (id >= SENSOR_COUNT) return SENSOR_MAX_DISTANCE_CM;
    return median3(
        sensors[id].buffer[0],
        sensors[id].buffer[1],
        sensors[id].buffer[2]
    );
}

bool sensors_obstacle_detected(sensor_id_t id) {
    return sensors_get_distance(id) < SENSOR_OBSTACLE_THRESHOLD_CM;
}

bool sensors_all_blocked(void) {
    return sensors_obstacle_detected(SENSOR_RIGHT)  &&
           sensors_obstacle_detected(SENSOR_CENTER) &&
           sensors_obstacle_detected(SENSOR_LEFT);
}