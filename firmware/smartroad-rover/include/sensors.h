/**
 * @file sensors.h
 * @brief Lectura de sensores ultrasonicos HC-SR04
 *
 * Implementa la medicion de distancia mediante tres sensores HC-SR04.
 * El disparo TRIG es sincrono (polling) y compartido entre los tres sensores.
 * La medicion del pulso ECHO se realiza mediante interrupciones GPIO (IRQ)
 * en ambos flancos, capturando time_us_32() para calcular el tiempo de vuelo.
 *
 * Pines utilizados:
 *   TRIG (compartido): GP15
 *   ECHO derecho:      GP18
 *   ECHO centro:       GP20
 *   ECHO izquierdo:    GP19
 *
 * Cada señal ECHO pasa por un divisor resistivo (R1=1k, R2=2k)
 * que adapta 5V a 3.33V antes de llegar al GPIO del Pico W.
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

#define SENSOR_TRIG_PIN   15   ///< GP15 - TRIG compartido para los 3 sensores
#define SENSOR_ECHO_RIGHT 18   ///< GP18 - ECHO sensor derecho
#define SENSOR_ECHO_CENTER 20  ///< GP20 - ECHO sensor centro (frontal)
#define SENSOR_ECHO_LEFT  19   ///< GP19 - ECHO sensor izquierdo

/* =========================================================
 * Parametros
 * ========================================================= */

/** @brief Distancia umbral de deteccion de obstaculo en cm */
#define SENSOR_OBSTACLE_THRESHOLD_CM   20

/** @brief Distancia maxima valida en cm (timeout del sensor) */
#define SENSOR_MAX_DISTANCE_CM        400

/** @brief Numero de muestras para el filtro de mediana */
#define SENSOR_FILTER_SIZE              3

/** @brief Tiempo de espera entre disparo y lectura en ms */
#define SENSOR_TRIGGER_WAIT_MS         30

/* =========================================================
 * Tipos
 * ========================================================= */

/**
 * @brief Identificador de sensor
 */
typedef enum {
    SENSOR_RIGHT  = 0,   ///< Sensor derecho
    SENSOR_CENTER = 1,   ///< Sensor centro (frontal)
    SENSOR_LEFT   = 2,   ///< Sensor izquierdo
    SENSOR_COUNT  = 3    ///< Numero total de sensores
} sensor_id_t;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo de sensores
 *
 * Configura el pin TRIG como salida y los pines ECHO como entradas
 * con interrupciones en ambos flancos. Debe llamarse una sola vez
 * al inicio del programa, despues de motors_init().
 */
void sensors_init(void);

/**
 * @brief Dispara el pulso TRIG en los tres sensores simultaneamente
 *
 * Genera un pulso de 10us en GP15. Los tres sensores reciben el mismo
 * pulso ya que comparten el pin TRIG. Las IRQ de ECHO se encargan
 * de capturar el tiempo de vuelo de cada uno.
 * Esta funcion es no bloqueante: retorna inmediatamente despues
 * del pulso sin esperar la respuesta ECHO.
 */
void sensors_trigger(void);

/**
 * @brief Retorna la distancia filtrada de un sensor en cm
 *
 * Aplica filtro de mediana sobre las ultimas SENSOR_FILTER_SIZE
 * mediciones validas para descartar lecturas espurias.
 *
 * @param id Identificador del sensor (SENSOR_RIGHT, CENTER, LEFT)
 * @return Distancia en cm, o SENSOR_MAX_DISTANCE_CM si no hay lectura valida
 */
uint16_t sensors_get_distance(sensor_id_t id);

/**
 * @brief Verifica si hay un obstaculo detectado por un sensor
 *
 * @param id Identificador del sensor
 * @return true si la distancia es menor a SENSOR_OBSTACLE_THRESHOLD_CM
 */
bool sensors_obstacle_detected(sensor_id_t id);

/**
 * @brief Verifica si todos los sensores detectan obstaculo simultaneamente
 *
 * Caso de contingencia: rover completamente bloqueado.
 *
 * @return true si los tres sensores detectan obstaculo
 */
bool sensors_all_blocked(void);

#endif /* SENSORS_H */