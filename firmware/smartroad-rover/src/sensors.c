/**
 * @file sensors.c
 * @brief Implementacion de lectura de sensores ultrasonicos HC-SR04
 *
 * Arquitectura de medicion:
 *   - TRIG: disparo sincrono desde el loop principal (polling)
 *   - ECHO: medicion mediante IRQ en flancos de subida y bajada
 *
 * En el flanco de SUBIDA se captura time_us_32() como t_rise.
 * En el flanco de BAJADA se calcula:
 *   distancia_cm = (t_fall - t_rise) * 0.017
 *   (velocidad del sonido = 340 m/s = 0.034 cm/us, dividido 2 por ida y vuelta)
 *
 * El filtro de mediana sobre 3 muestras descarta lecturas espurias
 * sin introducir latencia significativa.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "sensors.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

/* =========================================================
 * Estructuras privadas
 * ========================================================= */

/**
 * @brief Estado interno de un canal de sensor
 */
typedef struct {
    uint     echo_pin;                          ///< Pin GPIO del ECHO
    uint32_t t_rise;                            ///< Timestamp flanco subida (us)
    uint16_t buffer[SENSOR_FILTER_SIZE];        ///< Buffer circular de mediciones
    uint8_t  buf_idx;                           ///< Indice actual del buffer
    bool     valid;                             ///< Nueva medicion disponible
} sensor_channel_t;

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Estado de los tres canales de sensor */
static sensor_channel_t sensors[SENSOR_COUNT] = {
    [SENSOR_RIGHT]  = { .echo_pin = SENSOR_ECHO_RIGHT,  .buf_idx = 0, .valid = false },
    [SENSOR_CENTER] = { .echo_pin = SENSOR_ECHO_CENTER, .buf_idx = 0, .valid = false },
    [SENSOR_LEFT]   = { .echo_pin = SENSOR_ECHO_LEFT,   .buf_idx = 0, .valid = false },
};

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Retorna el indice del sensor dado su pin ECHO
 *
 * @param gpio Pin GPIO del ECHO
 * @return Indice del sensor, o SENSOR_COUNT si no se encuentra
 */
static sensor_id_t pin_to_sensor(uint gpio) {
    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        if (sensors[i].echo_pin == gpio) return i;
    }
    return SENSOR_COUNT; /* No encontrado */
}

/**
 * @brief Calcula la mediana de tres valores uint16_t
 *
 * Ordena tres valores y retorna el del medio.
 * Implementacion de red de comparacion optima para n=3.
 *
 * @param a Primer valor
 * @param b Segundo valor
 * @param c Tercer valor
 * @return Mediana de los tres valores
 */
static uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
    uint16_t tmp;
    /* Ordenar a <= b <= c con minimo de comparaciones */
    if (a > b) { tmp = a; a = b; b = tmp; }
    if (b > c) { tmp = b; b = c; c = tmp; }
    if (a > b) { tmp = a; a = b; b = tmp; }
    (void)a; (void)c; /* Suprimir warning de variables no usadas */
    return b;
}

/**
 * @brief ISR compartida para los tres pines ECHO
 *
 * Llamada por el hardware en cada flanco de los pines ECHO.
 * En flanco de subida: registra el tiempo de inicio del pulso.
 * En flanco de bajada: calcula la distancia y actualiza el buffer.
 *
 * @param gpio   Pin GPIO que genero la interrupcion
 * @param events Mascara de eventos (GPIO_IRQ_EDGE_RISE / FALL)
 */
static void echo_irq_handler(uint gpio, uint32_t events) {
    sensor_id_t idx = pin_to_sensor(gpio);
    if (idx == SENSOR_COUNT) return; /* Pin desconocido, ignorar */

    if (events & GPIO_IRQ_EDGE_RISE) {
        /* Flanco de subida: inicio del pulso ECHO */
        sensors[idx].t_rise = time_us_32();

    } else if (events & GPIO_IRQ_EDGE_FALL) {
        /* Flanco de bajada: fin del pulso ECHO */
        uint32_t t_fall = time_us_32();
        uint32_t pulse_us = t_fall - sensors[idx].t_rise;

        /* dist_cm = pulse_us * velocidad_sonido / 2
         * = pulse_us * 340m/s * 100cm/m / 2 / 1e6
         * = pulse_us * 0.017 cm/us
         * Multiplicamos por 17 y dividimos por 1000 para evitar float */
        uint16_t dist_cm = (uint16_t)((pulse_us * 17UL) / 1000UL);

        /* Descartar mediciones fuera de rango */
        if (dist_cm > SENSOR_MAX_DISTANCE_CM) {
            dist_cm = SENSOR_MAX_DISTANCE_CM;
        }

        /* Guardar en buffer circular */
        sensors[idx].buffer[sensors[idx].buf_idx] = dist_cm;
        sensors[idx].buf_idx = (sensors[idx].buf_idx + 1) % SENSOR_FILTER_SIZE;
        sensors[idx].valid = true;
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void sensors_init(void) {
    /* Configurar pin TRIG como salida, iniciar en bajo */
    gpio_init(SENSOR_TRIG_PIN);
    gpio_set_dir(SENSOR_TRIG_PIN, GPIO_OUT);
    gpio_put(SENSOR_TRIG_PIN, 0);

    /* Configurar pines ECHO como entradas con IRQ en ambos flancos */
    for (sensor_id_t i = 0; i < SENSOR_COUNT; i++) {
        gpio_init(sensors[i].echo_pin);
        gpio_set_dir(sensors[i].echo_pin, GPIO_IN);

        /* Registrar IRQ: flanco de subida Y bajada */
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

void sensors_trigger(void) {
    /* Pulso TRIG de 10us — disparo simultaneo para los 3 sensores */
    gpio_put(SENSOR_TRIG_PIN, 1);
    busy_wait_us(10);
    gpio_put(SENSOR_TRIG_PIN, 0);
    /* No se espera aqui — las IRQ capturan el ECHO de forma asincrona */
}

uint16_t sensors_get_distance(sensor_id_t id) {
    if (id >= SENSOR_COUNT) return SENSOR_MAX_DISTANCE_CM;

    /* Aplicar filtro de mediana sobre el buffer circular */
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