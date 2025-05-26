#ifndef PING_H
#define PING_H

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/ip4_addr.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/pbuf.h"
#include "lwip/inet_chksum.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_PING_COUNT       5

typedef struct __attribute__((packed)) {
    uint8_t type;        // ICMP type
    uint8_t code;        // ICMP code
    uint16_t checksum;   // ICMP checksum
    uint16_t id;         // Identifier
    uint16_t sequence;   // Sequence number
    uint32_t timestamp;   // Timestamp in ms
} ICMP_EchoHeader_t;


/**
 * @brief Ping handle structure definition
 */
typedef struct {
    uint64_t rtt_us[MAX_PING_COUNT];
    uint16_t sent;
    uint16_t received;
} Ping_Handle_t;

/**
 * @brief Ping function protoypes
 */
// Ping measurement
bool ping_measure(Ping_Handle_t *ping_handle, const char *ip_addr);
// Ping statistics calculation
bool ping_calculate_stats(Ping_Handle_t *ping_handle, uint64_t *avg_rtt_us, 
                        uint64_t *min_rtt_us, uint64_t *max_rtt_us, 
                        uint64_t *jitt_us, uint8_t *loss_p);

#endif /* PING_H */