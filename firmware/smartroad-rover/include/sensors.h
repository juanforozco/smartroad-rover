/**
 * @file sensors.h
 * @brief Lectura de sensores ultrasonicos HC-SR04
 *
 * v2 - Disparo secuencial para evitar interferencia de ecos cruzados.
 *
 * Segun el datasheet del HC-SR04, con multiples sensores el eco de
 * uno puede ser capturado por otro generando lecturas falsas cortas.
 * La solucion es disparar cada sensor individualmente esperando
 * que los ecos se disipen antes del siguiente disparo.
 *
 * Arquitectura:
 *   - TRIG compartido (GP15): se activa para cada sensor por separado
 *   - ECHO por sensor: IRQ en ambos flancos captura tiempo de vuelo
 *   - Ciclo completo: ~90ms (3 sensores x 30ms)
 *
 * Pines:
 *   TRIG compartido: GP15
 *   ECHO derecho:    GP18
 *   ECHO centro:     GP20
 *   ECHO izquierdo:  GP19
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Definicion de pines
 * ========================================================= */

#define SENSOR_TRIG_PIN    15  ///< GP15 - TRIG compartido
#define SENSOR_ECHO_RIGHT  18  ///< GP18 - ECHO sensor derecho
#define SENSOR_ECHO_CENTER 20  ///< GP20 - ECHO sensor centro
#define SENSOR_ECHO_LEFT   19  ///< GP19 - ECHO sensor izquierdo

/* =========================================================
 * Parametros
 * ========================================================= */

/** @brief Distancia umbral de obstaculo en cm */
#define SENSOR_OBSTACLE_THRESHOLD_CM   20

/** @brief Distancia maxima valida en cm */
#define SENSOR_MAX_DISTANCE_CM        400

/** @brief Numero de muestras para filtro de mediana */
#define SENSOR_FILTER_SIZE              3

/** @brief Tiempo de espera entre disparos secuenciales (ms)
 *  >= 30ms para disipar ecos antes del siguiente disparo */
#define SENSOR_INTER_TRIGGER_MS        35

/** @brief Tiempo de espera para eco tras disparo (ms) */
#define SENSOR_TRIGGER_WAIT_MS         30

/* =========================================================
 * Tipos
 * ========================================================= */

/**
 * @brief Identificador de sensor
 */
typedef enum {
    SENSOR_RIGHT  = 0,  ///< Sensor derecho
    SENSOR_CENTER = 1,  ///< Sensor centro (frontal)
    SENSOR_LEFT   = 2,  ///< Sensor izquierdo
    SENSOR_COUNT  = 3   ///< Total de sensores
} sensor_id_t;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de sensores
 *
 * Configura TRIG como salida y los tres ECHO como entradas
 * con IRQ en ambos flancos. Inicializa buffers con distancia maxima.
 */
void sensors_init(void);

/**
 * @brief Dispara y lee los tres sensores de forma secuencial
 *
 * Dispara cada sensor individualmente esperando SENSOR_INTER_TRIGGER_MS
 * entre cada uno para evitar interferencia de ecos cruzados.
 * Bloquea ~90ms en total (tiempo necesario para 3 lecturas limpias).
 * Debe llamarse desde el loop principal antes de leer distancias.
 */
void sensors_trigger_all_sequential(void);

/**
 * @brief Retorna la distancia filtrada de un sensor en cm
 *
 * @param id Identificador del sensor
 * @return Distancia en cm filtrada por mediana
 */
uint16_t sensors_get_distance(sensor_id_t id);

/**
 * @brief Verifica si un sensor detecta obstaculo
 *
 * @param id Identificador del sensor
 * @return true si distancia < SENSOR_OBSTACLE_THRESHOLD_CM
 */
bool sensors_obstacle_detected(sensor_id_t id);

/**
 * @brief Verifica si los tres sensores detectan obstaculo
 *
 * @return true si todos detectan obstaculo simultaneamente
 */
bool sensors_all_blocked(void);

#endif /* SENSORS_H */