#include "ping.h"

/* Private variables ---------------------------------------------------------*/
static struct raw_pcb *ping_pcb = NULL;
static volatile uint64_t ping_rtt_us = 0;
static volatile bool ping_done = false;
static volatile uint16_t echo_seq = 0;

/* Private function prototypes -----------------------------------------------*/
static uint8_t ping_recv_callback(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip4_addr_t *addr);
static void send_ping(const ip4_addr_t *dest, uint16_t seq);


/** @brief Ping measurement function using ICMP
 * @param ping_handle Pointer to Ping_Handle_t
 * @param ip_addr Target IP address to ping
 * @return true on success, false otherwise
 */
bool ping_measure(Ping_Handle_t *ping_handle, const char *ip_addr) {
    if ((NULL == ping_handle) ||(NULL == ip_addr)) {
        DBG("Invalid parameters\n");
        return false;
    }

    ip4_addr_t target_ip;
    uint8_t dot_c = 0;
    while (*ip_addr) {
        if (*ip_addr == '.') {
            dot_c++;
        } else if (*ip_addr < '0' || *ip_addr > '9') {
            DBG("Invalid IP address format\n");
            break;
        }
        ip_addr++;
    }
    if (dot_c == 3 && *ip_addr == '\0') {
        target_ip.addr = ipaddr_addr(ip_addr);
    } else {
        DBG("Invalid target IP address format: %s\n", ip_addr);
        return false;
    }

    ping_pcb = raw_new(IP_PROTO_ICMP);
    if (NULL == ping_pcb) {
        DBG("Failed to create raw protocol control block\n");
        return false;
    }

    raw_bind(ping_pcb, IP_ADDR_ANY);
    raw_recv(ping_pcb, ping_recv_callback, ping_handle);

    ping_handle->sent = MAX_PING_COUNT;
    ping_handle->received = 0;
    
    for ( int i = 0; i < MAX_PING_COUNT; i++) {
        ping_done = false;
        send_ping(&target_ip, ++echo_seq);
        
        uint64_t timeout = time_us_64() + PING_TIMEOUT_MS * 1000;
        while (!ping_done && time_us_64() < timeout) {
            cyw43_arch_poll();
            sleep_ms(1);
        }

        if( true == ping_done) {
            ping_handle->rtt_us[ping_handle->received] = ping_rtt_us;
            DBG("Ping %d: %llu us", i, ping_rtt_us);
        } else {
            DBG("Ping %d: timeout", i);
        }
    }

    raw_remove(ping_pcb);
    return ping_handle->received > 0 ? true : false;
}

/**
 * @brief ICMP receive callback
 * @param arg User-provided argument
 * @param pcb Raw protocol control block
 * @param p Packet buffer containing the ICMP packet
 * @param addr Source IP address
 * @return 1 if the packet was processed, 0 otherwise
 */
static uint8_t ping_recv_callback(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip4_addr_t *addr) {
    struct ip_hdr *ip_hdr = (struct ip_hdr *)p->payload;
    uint16_t hdr_len = IPH_HL(ip_hdr) * 4;

    if (p->tot_len < hdr_len + sizeof(ICMP_EchoHeader_t)) {
        // Dereference packet buffer because it is too small
        pbuf_free(p);
        return 0;
    }

    ICMP_EchoHeader_t *icmp_hdr = (ICMP_EchoHeader_t *)((uint8_t *)p->payload + hdr_len);
    // Validate checksum
    uint16_t checksum_recv = icmp_hdr->checksum;
    // Set checksum to 0 for calculation
    icmp_hdr->checksum = 0;
    uint16_t checksum_calc = inet_chksum(icmp_hdr, sizeof(ICMP_EchoHeader_t));
    if (icmp_hdr->type == ICMP_ER && lwip_ntohs(icmp_hdr->id) == 0xBADA && checksum_recv == checksum_calc) {
        uint64_t now_us = time_us_64();
        uint64_t timestamp = ntohl(icmp_hdr->timestamp);
        ping_rtt_us = now_us - timestamp;
        ping_done = true;
    } else {
        ping_done = false;
    }

    pbuf_free(p);
    return 1;
}

/**
 * @brief Send a single ICMP echo request
 * @param dest Destination IP addr
 * @param seq Sequence number
 */
static void send_ping(const ip4_addr_t *dest, uint16_t seq) {
    ICMP_EchoHeader_t icmp_hdr = {
        .type = ICMP_ECHO,
        .code = 0,
        .checksum = 0,
        .id = lwip_htons(0xBADA),
        .sequence = lwip_htons(seq),
        .timestamp = htonl(time_us_64() / 1000) // ms
    };

    // Calculate checksum
    icmp_hdr.checksum = inet_chksum(&icmp_hdr, sizeof(ICMP_EchoHeader_t));

    struct pbuf *p = pbuf_alloc(PBUF_IP, sizeof(ICMP_EchoHeader_t), PBUF_RAM);
    if (NULL == p) {
        DBG("Failed to allocate pbuf\n");
        return;
    }

    // Copy the ICMP header into the pbuf
    memcpy(p->payload, &icmp_hdr, sizeof(ICMP_EchoHeader_t));
    // Send the ICMP echo request
    raw_sendto(ping_pcb, p, (const ip_addr_t*)dest);
    pbuf_free(p);
}