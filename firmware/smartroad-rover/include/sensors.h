/**
 * @file sensors.h
 * @brief Lectura de sensores ultrasonicos HC-SR04
 *
 * Disparo simultaneo de los tres sensores desde un pin TRIG compartido.
 * Medicion de distancia mediante IRQ en ambos flancos del pin ECHO.
 * Filtro de mediana sobre 3 muestras para descartar lecturas espurias.
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

/** @brief Tiempo de espera tras disparo para captura de ecos (ms) */
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
 */
void sensors_init(void);

/**
 * @brief Dispara los tres sensores simultaneamente
 *
 * Genera pulso TRIG de 10us. Las IRQ capturan los ecos
 * de forma asincrona. No bloqueante.
 */
void sensors_trigger(void);

/**
 * @brief Retorna distancia filtrada de un sensor en cm
 * @param id Identificador del sensor
 * @return Distancia en cm
 */
uint16_t sensors_get_distance(sensor_id_t id);

/**
 * @brief Verifica si un sensor detecta obstaculo
 * @param id Identificador del sensor
 * @return true si distancia < SENSOR_OBSTACLE_THRESHOLD_CM
 */
bool sensors_obstacle_detected(sensor_id_t id);

/**
 * @brief Verifica si los tres sensores detectan obstaculo
 * @return true si todos detectan obstaculo
 */
bool sensors_all_blocked(void);

#endif /* SENSORS_H */