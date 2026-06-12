/**
 * @file gps.c
 * @brief Implementacion del modulo GPS con DMA
 *
 * Arquitectura DMA:
 *   UART0 RX → DMA canal → buffer circular en RAM
 *   Sin intervencion del CPU durante la recepcion.
 *   DMA_IRQ_0 señaliza cuando el buffer esta listo.
 *
 * Parsing NMEA:
 *   Se procesan sentencias $GPRMC y $GPGGA.
 *   La posicion se actualiza solo cuando el campo
 *   de validez indica fix activo ('A' en GPRMC).
 *
 * Navegacion GPS:
 *   Calcula rumbo al objetivo usando atan2.
 *   Compara con orientacion estimada por odometria basica.
 *   Detiene el rover al llegar al radio objetivo (RF-G8).
 *
 * @author Juan Felipe Orozco
 * @date 2026
 */

#include "gps.h"
#include "motors.h"
#include "sensors.h"
#include "navigation.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* =========================================================
 * Variables privadas
 * ========================================================= */

/** @brief Buffer DMA para recepcion de tramas NMEA */
static char gps_dma_buf[GPS_BUF_SIZE];

/** @brief Buffer de linea NMEA en procesamiento */
static char gps_line_buf[128];

/** @brief Canal DMA asignado */
static int dma_channel_num = -1;

/** @brief Posicion GPS actual */
static gps_position_t current_pos = { .lat = 0, .lon = 0, .valid = false };

/** @brief Coordenada objetivo */
static gps_position_t target_pos  = { .lat = 0, .lon = 0, .valid = false };

/** @brief Timestamp de la ultima trama valida (us) */
static volatile uint64_t last_valid_us = 0;

/** @brief Flag: nueva trama disponible */
static volatile bool trama_disponible = false;

/* =========================================================
 * Funciones privadas
 * ========================================================= */

/**
 * @brief ISR del DMA — señaliza trama disponible
 */
static void gps_dma_irq_handler(void) {
    dma_hw->ints0 = (1u << dma_channel_num);
    trama_disponible = true;

    /* Recargar DMA para continuar recepcion */
    dma_channel_set_write_addr(dma_channel_num, gps_dma_buf, true);
}

/**
 * @brief Convierte campo NMEA de grados/minutos a grados decimales
 *
 * Formato NMEA: DDDMM.MMMM
 *
 * @param field String con el campo NMEA
 * @return Grados decimales
 */
static float nmea_to_degrees(const char *field) {
    if (!field || strlen(field) < 4) return 0.0f;
    float raw = atof(field);
    int deg = (int)(raw / 100);
    float min = raw - (deg * 100);
    return deg + min / 60.0f;
}

/**
 * @brief Parsea una sentencia GPRMC o GNRMC
 *
 * Formato: $GPRMC,hhmmss,A,llll.ll,N,yyyyy.yy,E,...
 * Campo 2: A=activo, V=invalido
 *
 * @param sentence Sentencia NMEA completa
 */
static void parse_gprmc(const char *sentence) {
    /* Copiar para tokenizar sin modificar el original */
    char buf[128];
    strncpy(buf, sentence, 127);
    buf[127] = '\0';

    char *fields[13] = {0};
    int   nf = 0;
    char *tok = strtok(buf, ",");
    while (tok && nf < 13) {
        fields[nf++] = tok;
        tok = strtok(NULL, ",");
    }

    if (nf < 7) return;

    /* Campo 2: validez */
    if (!fields[2] || fields[2][0] != 'A') return;

    /* Campos 3-6: latitud y longitud */
    float lat = nmea_to_degrees(fields[3]);
    float lon = nmea_to_degrees(fields[5]);

    /* Hemisferio */
    if (fields[4] && fields[4][0] == 'S') lat = -lat;
    if (fields[6] && fields[6][0] == 'W') lon = -lon;

    current_pos.lat   = lat;
    current_pos.lon   = lon;
    current_pos.valid = true;
    last_valid_us     = time_us_64();

    printf("[GPS] Pos: %.6f, %.6f\n", lat, lon);
}

/**
 * @brief Procesa el buffer DMA buscando sentencias NMEA completas
 */
static void process_dma_buffer(void) {
    static int line_idx = 0;

    for (int i = 0; i < GPS_BUF_SIZE; i++) {
        char c = gps_dma_buf[i];
        if (c == '\n') {
            gps_line_buf[line_idx] = '\0';
            if (strncmp(gps_line_buf, "$GPRMC", 6) == 0 ||
                strncmp(gps_line_buf, "$GNRMC", 6) == 0) {
                parse_gprmc(gps_line_buf);
            }
            line_idx = 0;
        } else if (c != '\r' && line_idx < 127) {
            gps_line_buf[line_idx++] = c;
        }
    }
}

/* =========================================================
 * Implementacion de la API publica
 * ========================================================= */

void gps_init(void) {
    printf("[GPS] Inicializando UART0 a %d baud...\n", GPS_BAUD);

    /* Configurar UART0 */
    uart_init(GPS_UART_ID, GPS_BAUD);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);

    /* Reclamar canal DMA */
    dma_channel_num = dma_claim_unused_channel(true);
    printf("[GPS] Canal DMA: %d\n", dma_channel_num);

    /* Configurar canal DMA */
    dma_channel_config cfg = dma_channel_get_default_config(dma_channel_num);
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_dreq(&cfg, DREQ_UART0_RX);

    dma_channel_configure(
        dma_channel_num, &cfg,
        gps_dma_buf,
        &uart_get_hw(GPS_UART_ID)->dr,
        GPS_BUF_SIZE,
        true
    );

    /* Habilitar IRQ del DMA */
    dma_channel_set_irq0_enabled(dma_channel_num, true);
    irq_set_exclusive_handler(DMA_IRQ_0, gps_dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    printf("[GPS] DMA activo. Esperando tramas NMEA...\n");
}

bool gps_get_position(gps_position_t *pos) {
    if (trama_disponible) {
        process_dma_buffer();
        trama_disponible = false;
    }
    if (pos) *pos = current_pos;
    return current_pos.valid;
}

bool gps_signal_valid(void) {
    /* Verificar solo el timeout, no la validez de posicion
     * Permite que el modo GPS permanezca activo mientras
     * espera adquirir señal satelital */
    uint64_t now = time_us_64();
    uint64_t last = last_valid_us;
    uint64_t timeout = (uint64_t)GPS_TIMEOUT_MS * 1000ULL;
    return (now - last) < timeout;
}

void gps_set_target(float lat, float lon) {
    target_pos.lat   = lat;
    target_pos.lon   = lon;
    target_pos.valid = true;
    printf("[GPS] Objetivo: %.6f, %.6f\n", lat, lon);
}

float gps_distance_m(float lat1, float lon1, float lat2, float lon2) {
    /* Formula de Haversine */
    const float R = 6371000.0f; /* Radio de la Tierra en metros */
    float dlat = (lat2 - lat1) * (float)M_PI / 180.0f;
    float dlon = (lon2 - lon1) * (float)M_PI / 180.0f;
    float a = sinf(dlat/2) * sinf(dlat/2) +
              cosf(lat1 * (float)M_PI / 180.0f) *
              cosf(lat2 * (float)M_PI / 180.0f) *
              sinf(dlon/2) * sinf(dlon/2);
    float c = 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
    return R * c;
}

void gps_navigation_step(void) {
    /* Procesar buffer DMA si hay trama nueva */
    if (trama_disponible) {
        process_dma_buffer();
        trama_disponible = false;
    }

    /* Verificar señal GPS */
    if (!gps_signal_valid()) {
        motors_stop();
        return;
    }

    /* Verificar objetivo definido */
    if (!target_pos.valid) {
        printf("[GPS] Sin objetivo definido.\n");
        motors_stop();
        return;
    }

    /* Calcular distancia al objetivo */
    float dist = gps_distance_m(
        current_pos.lat, current_pos.lon,
        target_pos.lat,  target_pos.lon
    );

    printf("[GPS] Distancia al objetivo: %.1f m\n", dist);

    /* Verificar si llegamos al objetivo (RF-G8) */
    if (dist <= GPS_TARGET_RADIUS_M) {
        printf("[GPS] Objetivo alcanzado!\n");
        motors_stop();
        return;
    }

    /* Mantener deteccion de obstaculos activa (RF-G5) */
    if (sensors_obstacle_detected(SENSOR_CENTER) ||
        sensors_obstacle_detected(SENSOR_LEFT)   ||
        sensors_obstacle_detected(SENSOR_RIGHT)) {
        printf("[GPS] Obstaculo detectado — evasion\n");
        navigation_step();
        return;
    }

    /* Avanzar hacia el objetivo */
    motors_forward(450);
}