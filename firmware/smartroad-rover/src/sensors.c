/**
 * @file sensors.c
 * @brief Implementacion de sensores HC-SR04
 *
 * Disparo simultaneo desde pin TRIG compartido.
 * Medicion por IRQ en flancos de subida y bajada del ECHO.
 * Filtro de mediana sobre 3 muestras.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "sensors.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

/* =========================================================
 * Estructura interna
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

/* =========================================================
 * Funciones privadas
 * ========================================================= */

static sensor_id_t pin_to_sensor(uint gpio) {
    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        if (sensors[i].echo_pin == gpio) return i;
    }
    return SENSOR_COUNT;
}

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
 * Flanco subida: registra tiempo de inicio del pulso.
 * Flanco bajada: calcula distancia y actualiza buffer circular.
 */
static void echo_irq_handler(uint gpio, uint32_t events) {
    sensor_id_t idx = pin_to_sensor(gpio);
    if (idx == SENSOR_COUNT) return;

    if (events & GPIO_IRQ_EDGE_RISE) {
        sensors[idx].t_rise = time_us_32();

    } else if (events & GPIO_IRQ_EDGE_FALL) {
        uint32_t pulse_us = time_us_32() - sensors[idx].t_rise;

        /* dist_cm = pulse_us * 0.017
         * velocidad sonido 340m/s = 0.034 cm/us / 2 (ida y vuelta) */
        uint16_t dist_cm = (uint16_t)((pulse_us * 17UL) / 1000UL);

        if (dist_cm > SENSOR_MAX_DISTANCE_CM) {
            dist_cm = SENSOR_MAX_DISTANCE_CM;
        }

        sensors[idx].buffer[sensors[idx].buf_idx] = dist_cm;
        sensors[idx].buf_idx = (sensors[idx].buf_idx + 1) % SENSOR_FILTER_SIZE;
        sensors[idx].valid = true;
    }
}

/* =========================================================
 * API publica
 * ========================================================= */

void sensors_init(void) {
    gpio_init(SENSOR_TRIG_PIN);
    gpio_set_dir(SENSOR_TRIG_PIN, GPIO_OUT);
    gpio_put(SENSOR_TRIG_PIN, 0);

    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        gpio_init(sensors[i].echo_pin);
        gpio_set_dir(sensors[i].echo_pin, GPIO_IN);

        gpio_set_irq_enabled_with_callback(
            sensors[i].echo_pin,
            GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
            true,
            &echo_irq_handler
        );

        for (uint8_t j = 0; j < SENSOR_FILTER_SIZE; j++) {
            sensors[i].buffer[j] = SENSOR_MAX_DISTANCE_CM;
        }
    }
}

void sensors_trigger(void) {
    gpio_put(SENSOR_TRIG_PIN, 1);
    busy_wait_us(10);
    gpio_put(SENSOR_TRIG_PIN, 0);
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