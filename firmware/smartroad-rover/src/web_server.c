/**
 * @file web_server.c
 * @brief Implementacion del servidor HTTP embebido
 *
 * El Pico W opera como Access Point WiFi.
 * El servidor HTTP atiende comandos de control del rover
 * y sirve la interfaz web de control.
 *
 * Arquitectura no bloqueante:
 *   - lwIP con modelo polled (pico_cyw43_arch_lwip_poll)
 *   - web_server_poll() atiende callbacks de lwIP
 *   - Los comandos se procesan en el callback HTTP
 *   - El watchdog usa time_us_64() sin timers adicionales
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "web_server.h"
#include "motors.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include <stdio.h>
#include <string.h>
#include "lwip/netif.h"

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Timestamp del ultimo comando recibido (us) */
static volatile uint64_t last_cmd_us = 0;

/** @brief Solicitud de cambio de modo pendiente */
static volatile web_mode_request_t mode_request = WEB_MODE_NONE;

/** @brief PCB del servidor TCP */
static struct tcp_pcb *server_pcb = NULL;

/* =========================================================
 * Pagina HTML de control
 * ========================================================= */

static const char *HTML_PAGE =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n\r\n"
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>SmartRoad Rover</title>"
    "<style>"
    "body{font-family:Arial;text-align:center;background:#1a1a2e;color:#eee;padding:20px}"
    "h1{color:#e94560}h2{color:#aaa;font-size:1em}"
    ".btn{display:inline-block;margin:8px;padding:18px 28px;"
    "font-size:1.2em;border:none;border-radius:10px;"
    "cursor:pointer;color:white;min-width:120px}"
    ".fwd{background:#27ae60}.rev{background:#c0392b}"
    ".lft{background:#2980b9}.rgt{background:#8e44ad}"
    ".stp{background:#e67e22;font-weight:bold}"
    ".mode{background:#16a085;margin-top:20px}"
    ".grid{display:grid;grid-template-columns:repeat(3,1fr);gap:5px;max-width:360px;margin:auto}"
    ".span3{grid-column:span 3}"
    "</style></head><body>"
    "<h1>SmartRoad Rover</h1>"
    "<h2>Control Manual</h2>"
    "<div class='grid'>"
    "<div></div>"
    "<button class='btn fwd' onclick=\"fetch('/cmd?action=forward')\">&#9650; Adelante</button>"
    "<div></div>"
    "<button class='btn lft' onclick=\"fetch('/cmd?action=left')\">&#9668; Izquierda</button>"
    "<button class='btn stp' onclick=\"fetch('/cmd?action=stop')\">&#9646; Stop</button>"
    "<button class='btn rgt' onclick=\"fetch('/cmd?action=right')\">Derecha &#9658;</button>"
    "<div></div>"
    "<button class='btn rev' onclick=\"fetch('/cmd?action=reverse')\">&#9660; Atras</button>"
    "<div></div>"
    "<button class='btn mode span3' onclick=\"fetch('/cmd?action=mode_auto')\">&#9881; Modo Autonomo</button>"
    "</div></body></html>";

static const char *HTTP_OK =
    "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK";

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief Procesa un comando recibido por HTTP
 *
 * Extrae el parametro action de la URL y ejecuta
 * la accion correspondiente sobre los motores.
 *
 * @param request Buffer con la peticion HTTP
 */
static void process_command(const char *request) {
    /* Actualizar timestamp del watchdog */
    last_cmd_us = time_us_64();
    mode_request = WEB_MODE_NONE;

    if (strstr(request, "action=forward")) {
        printf("[WEB] Comando: ADELANTE\n");
        motors_forward(600);
    } else if (strstr(request, "action=reverse")) {
        printf("[WEB] Comando: ATRAS\n");
        motors_reverse(600);
    } else if (strstr(request, "action=left")) {
        printf("[WEB] Comando: IZQUIERDA\n");
        motors_turn_left(500);
    } else if (strstr(request, "action=right")) {
        printf("[WEB] Comando: DERECHA\n");
        motors_turn_right(500);
    } else if (strstr(request, "action=stop")) {
        printf("[WEB] Comando: STOP\n");
        motors_stop();
    } else if (strstr(request, "action=mode_auto")) {
        printf("[WEB] Solicitud modo autonomo\n");
        motors_stop();
        mode_request = WEB_MODE_AUTO;
    } else if (strstr(request, "action=mode_manual")) {
        printf("[WEB] Solicitud modo manual\n");
        mode_request = WEB_MODE_MANUAL;
    }
}

/**
 * @brief Callback de recepcion de datos TCP
 */
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *pcb,
                                struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(pcb);
        return ERR_OK;
    }

    char *data = (char *)p->payload;

    /* Determinar si es peticion de comando o de pagina */
    if (strstr(data, "GET /cmd")) {
        process_command(data);
        tcp_write(pcb, HTTP_OK, strlen(HTTP_OK), TCP_WRITE_FLAG_COPY);
    } else {
        tcp_write(pcb, HTML_PAGE, strlen(HTML_PAGE), TCP_WRITE_FLAG_COPY);
    }

    tcp_output(pcb);
    pbuf_free(p);
    tcp_close(pcb);
    return ERR_OK;
}

/**
 * @brief Callback de nueva conexion TCP
 */
static err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_recv_callback);
    return ERR_OK;
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

bool web_server_init(void) {
    printf("[WEB] Iniciando WiFi en modo Access Point...\n");

    if (cyw43_arch_init()) {
        printf("[WEB] Error: fallo inicializacion WiFi\n");
        return false;
    }

    /* Configurar como Access Point */
    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASSWORD,
                               CYW43_AUTH_WPA2_AES_PSK);
    
    /* Configurar IP estatica del Access Point */
    ip4_addr_t ip, mask, gw;
    IP4_ADDR(&ip,   192, 168, 4,   1);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    IP4_ADDR(&gw,   192, 168, 4,   1);
    netif_set_addr(netif_default, &ip, &mask, &gw);

    printf("[WEB] Red WiFi activa: %s\n", WIFI_SSID);
    printf("[WEB] Contrasena: %s\n", WIFI_PASSWORD);
    printf("[WEB] Acceso: http://192.168.4.1\n");

    /* Iniciar servidor TCP en puerto 80 */
    server_pcb = tcp_new();
    if (!server_pcb) {
        printf("[WEB] Error: no se pudo crear PCB\n");
        return false;
    }

    tcp_bind(server_pcb, IP_ADDR_ANY, 80);
    server_pcb = tcp_listen(server_pcb);
    tcp_accept(server_pcb, tcp_accept_callback);

    printf("[WEB] Servidor HTTP listo en puerto 80\n");

    /* Inicializar watchdog */
    last_cmd_us = time_us_64();
    return true;
}

void web_server_poll(void) {
    /* Atender callbacks pendientes de lwIP */
    cyw43_arch_poll();
}

bool web_server_wifi_timeout(void) {
    return (time_us_64() - last_cmd_us) >
           ((uint64_t)WEB_WIFI_TIMEOUT_MS * 1000ULL);
}

void web_server_reset_watchdog(void) {
    last_cmd_us = time_us_64();
}

web_mode_request_t web_server_mode_requested(void) {
    web_mode_request_t req = mode_request;
    mode_request = WEB_MODE_NONE;
    return req;
}