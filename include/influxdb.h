#ifndef INFLUXDB_H
#define INFLUXDB_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"
#include "config.h"

typedef struct {
    struct tcp_pcb *pcb;
    bool complete;
    bool success;
    char request[1024];
    int request_len;
} HTTP_Handle_t;

/**
 * @brief InfluxDB function protoypes
 */
// Write Data via InfluxDB CLI
bool influxdb_send_measurements(uint64_t rtt_avg_us, uint64_t rtt_min_us, uint64_t rtt_max_us, 
                            uint64_t jitter_us, uint8_t loss_pct, float temperature_c);
// Separate function to send failed measurement attempt results
bool influxdb_send_failure(float temperature_c);
// Backoff delay calculation
uint32_t calculate_backoff_delay(uint32_t *retry_c, uint32_t *retry_delay);

#endif /* INFLUXDB_H */