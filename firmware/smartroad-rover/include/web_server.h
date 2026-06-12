/**
 * @file web_server.h
 * @brief Servidor HTTP embebido para control manual via WiFi
 *
 * El Pico W genera su propia red WiFi (Access Point) a la que
 * el usuario se conecta desde un smartphone o PC. El servidor
 * sirve una pagina web con controles de movimiento.
 *
 * Credenciales de la red:
 *   SSID: SmartRoad-Rover
 *   Password: rover1234
 *
 * Acceso: http://192.168.4.1
 *
 * Comandos disponibles (RF-M3):
 *   /cmd?action=forward
 *   /cmd?action=reverse
 *   /cmd?action=left
 *   /cmd?action=right
 *   /cmd?action=stop
 *   /cmd?action=mode_auto
 *   /cmd?action=mode_manual
 *
 * Watchdog WiFi (RNF-5):
 *   Si no se recibe comando en WEB_WIFI_TIMEOUT_MS,
 *   web_server_wifi_timeout() retorna true y la FSM
 *   transita a STATE_SAFE_STOP.
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * Configuracion WiFi
 * ========================================================= */

/** @brief SSID de la red WiFi generada por el Pico W */
#define WIFI_SSID        "SmartRoad-Rover"

/** @brief Contrasena de la red WiFi */
#define WIFI_PASSWORD    "rover1234"

/** @brief Timeout del watchdog WiFi en ms (RNF-5) */
#define WEB_WIFI_TIMEOUT_MS   500

/* =========================================================
 * Tipos
 * ========================================================= */

/**
 * @brief Modos solicitados desde la interfaz web
 */
typedef enum {
    WEB_MODE_NONE,    ///< Sin solicitud de cambio de modo
    WEB_MODE_AUTO,    ///< Solicitud de modo autonomo
    WEB_MODE_MANUAL   ///< Solicitud de modo manual
} web_mode_request_t;

/* =========================================================
 * API publica
 * ========================================================= */

/**
 * @brief Inicializa el servidor web y la red WiFi
 *
 * Configura el Pico W como Access Point, inicia lwIP
 * y abre el servidor HTTP en el puerto 80.
 *
 * @return true si la inicializacion fue exitosa
 */
bool web_server_init(void);

/**
 * @brief Atiende peticiones HTTP pendientes (polling)
 *
 * Debe llamarse repetidamente desde el loop principal
 * cuando el sistema esta en STATE_MANUAL.
 * No bloqueante — retorna inmediatamente si no hay peticiones.
 */
void web_server_poll(void);

/**
 * @brief Verifica si hubo timeout del watchdog WiFi
 *
 * @return true si han pasado mas de WEB_WIFI_TIMEOUT_MS
 *         desde el ultimo comando recibido
 */
bool web_server_wifi_timeout(void);

/**
 * @brief Resetea el watchdog WiFi
 *
 * Debe llamarse al entrar al estado MANUAL para
 * evitar timeout inmediato.
 */
void web_server_reset_watchdog(void);

/**
 * @brief Retorna la solicitud de cambio de modo pendiente
 *
 * @return WEB_MODE_MANUAL, WEB_MODE_AUTO o WEB_MODE_NONE
 */
web_mode_request_t web_server_mode_requested(void);

#endif /* WEB_SERVER_H */