/**
 * @file gps.h
 * @brief Modulo GPS para navegacion hacia coordenada objetivo
 *
 * Recibe tramas NMEA por UART0 (GP1) usando DMA para
 * transferencia sin intervencion del CPU. Parsea sentencias
 * $GPRMC y $GPGGA para extraer latitud, longitud y validez.
 *
 * Hardware:
 *   UART: UART0
 *   RX pin: GP1 (pin 2)
 *   Baud: 9600
 *   DMA: DREQ_UART0_RX → buffer circular en RAM
 *
 * Watchdog GPS (RNF-6):
 *   Si no se recibe trama valida en GPS_TIMEOUT_MS,
 *   gps_signal_valid() retorna false y la FSM transita
 *   a STATE_AUTO_REACTIVE.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef GPS_H
#define GPS_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Configuracion de hardware
 * ========================================================= */

#define GPS_UART_ID     uart0   ///< UART0 para el GPS
#define GPS_RX_PIN      1       ///< GP1 — UART0 RX
#define GPS_BAUD        9600    ///< Baudrate estandar NMEA

/** @brief Tamano del buffer DMA para recepcion NMEA */
#define GPS_BUF_SIZE    256

/** @brief Timeout de señal GPS en ms (RNF-6) */
#define GPS_TIMEOUT_MS  50000

/** @brief Radio de llegada al objetivo en metros (RF-G8) */
#define GPS_TARGET_RADIUS_M  3.0f

/* =========================================================
 * Tipos
 * ========================================================= */

/**
 * @brief Posicion GPS
 */
typedef struct {
    float lat;      ///< Latitud en grados decimales
    float lon;      ///< Longitud en grados decimales
    bool  valid;    ///< true si la posicion es valida
} gps_position_t;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el modulo GPS con DMA
 *
 * Configura UART0 a 9600 baud, reclama un canal DMA
 * con DREQ_UART0_RX y habilita DMA_IRQ_0 para señalizar
 * cuando hay trama disponible.
 */
void gps_init(void);

/**
 * @brief Retorna la posicion GPS actual
 *
 * @param pos Puntero donde se escribe la posicion
 * @return true si la posicion es valida
 */
bool gps_get_position(gps_position_t *pos);

/**
 * @brief Verifica si la señal GPS es valida
 *
 * @return true si se recibio trama valida en los ultimos GPS_TIMEOUT_MS
 */
bool gps_signal_valid(void);

/**
 * @brief Define la coordenada objetivo para navegacion GPS
 *
 * @param lat Latitud objetivo en grados decimales
 * @param lon Longitud objetivo en grados decimales
 */
void gps_set_target(float lat, float lon);

/**
 * @brief Ejecuta un paso de navegacion hacia el objetivo GPS
 *
 * Calcula rumbo hacia el objetivo, compara con orientacion
 * actual y actua sobre los motores. Mantiene deteccion de
 * obstaculos activa (RF-G5).
 *
 * Debe llamarse desde el loop principal en STATE_GPS.
 */
void gps_navigation_step(void);

/**
 * @brief Calcula distancia en metros entre dos coordenadas
 *
 * Usa formula de Haversine para distancia geodesica.
 *
 * @param lat1 Latitud punto 1
 * @param lon1 Longitud punto 1
 * @param lat2 Latitud punto 2
 * @param lon2 Longitud punto 2
 * @return Distancia en metros
 */
float gps_distance_m(float lat1, float lon1, float lat2, float lon2);

#endif /* GPS_H */