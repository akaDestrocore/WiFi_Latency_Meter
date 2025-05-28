#include "influxdb.h"

/* Private variables ---------------------------------------------------------*/
static HTTP_Handle_t http_handle;
static uint32_t retry_c = 0;
static uint32_t retry_delay = INFLUX_RETRY_DELAY_MS;

/* Private function prototypes -----------------------------------------------*/
static bool send_http_post(const char *query);
static void tcp_error_callback(void *arg, err_t err);
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err);

/**
 * @brief Send Wi-Fi measurement results to InfluxDB
 * @param rtt_avg_us Average round trip time in microseconds
 * @param rtt_min_us Minimum round trip time in microseconds
 * @param rtt_max_us Maximum round trip time in microseconds
 * @param jitter_us Jitter in microseconds
 * @param loss_pct Packet loss percentage
 * @param temperature_c Temperature in Celsius
 * @return true on success, false otherwise
 */
bool influxdb_send_measurements(uint64_t rtt_avg_us, uint64_t rtt_min_us, uint64_t rtt_max_us, 
                            uint64_t jitter_us, uint8_t loss_pct, float temperature_c) {
    char influx_query[512];

    snprintf(influx_query, sizeof(influx_query),
        "wifi_measurements,host=PicoW "
        "rtt_avg=%llu,"
        "rtt_min=%llu,"
        "rtt_max=%llu,"
        "jitter=%llu,"
        "loss=%u,"
        "temperature=%.2f",
        (uint64_t)rtt_avg_us,
        (uint64_t)rtt_min_us,
        (uint64_t)rtt_max_us,
        (uint64_t)jitter_us,
        loss_pct,
        temperature_c);
    
        DBG("Sending measurements to InfluxDB: %s\r\n", influx_query);
        
        // Send the HTTP POST request to InfluxDB
        bool request_res = send_http_post(influx_query);
        return request_res;
}

/**
 * @brief Helper function for sending HTTP POST request to InfluxDB
 * @param data Line protocol data to send
 * @return true on success, false otherwise
 */
static bool send_http_post(const char *data) {
    if (NULL == data || strlen(data) == 0) {
        DBG("Invalid data for HTTP POST request\n");
        return false;
    }

    ip_addr_t influxdb_ip;
    // Validate IP
    const char *scan = INFLUXDB_IP;
    int dot_c = 0;
    while (*scan) {
        if (*scan == '.') {
            dot_c++;
        } else if (*scan < '0' || *scan > '9') {
            DBG("Invalid IP address format: %s\n", INFLUXDB_IP);
            return false;
        }
        scan++;
    }
    if (dot_c != 3) {
        DBG("Invalid target IP address format: %s\n", INFLUXDB_IP);
        return false;
    }

    influxdb_ip.addr = ipaddr_addr(INFLUXDB_IP);

    http_handle.pcb = tcp_new();
    if(!http_handle.pcb){
        DBG("Failed to create TCP control block\n");
        return false;
    }

    // Set up the TCP control block
    tcp_arg(http_handle.pcb, NULL);
    tcp_err(http_handle.pcb, tcp_error_callback);
    tcp_recv(http_handle.pcb, tcp_recv_callback);

    http_handle.request_len = snprintf(http_handle.request, sizeof(http_handle.request),
        "POST /api/v2/write?org=%s&bucket=%s&precision=ms HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Authorization: Token %s\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        INFLUXDB_ORG,
        INFLUXDB_BUCKET,
        INFLUXDB_IP,
        INFLUXDB_PORT,
        INFLUXDB_TOKEN,
        strlen(data),
        data);

    // Connect to InfluxDB server
    err_t err = tcp_connect(http_handle.pcb, &influxdb_ip, INFLUXDB_PORT, tcp_connected_callback);

    if (err != ERR_OK) {
        DBG("TCP connect error: %d", err);
        tcp_close(http_handle.pcb);
        return false;
    }

    http_handle.complete = false;
    http_handle.success = false;

    // 5 sec
    uint64_t timeout = time_us_64() + 5000000;

    while ( !http_handle.complete && time_us_64() < timeout) {
        cyw43_arch_poll();
        sleep_ms(1);
    }

    if (!http_handle.complete) {
        DBG("HTTP request timed out");
        tcp_close(http_handle.pcb);
        return false;
    }

    return http_handle.success;
}

/**
 * @brief TCP error callback for InfluxDB
 * @param arg User provided argument (needed by LWIP API)
 * @param err Error code
 */
static void tcp_error_callback(void *arg, err_t err) {
    DBG("TCP error callback: %d\n", err);
    http_handle.complete = true;
    http_handle.success = false;
}

/**
 * @brief TCP receive callback for InfluxDB response
 * @param arg User provided argument (needed by LWIP API)
 * @param tpcb TCP protocol control block
 * @param p Packet buffer
 * @param err Error code
 * @return ERR_OK on success
 */
static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (NULL == p) {
        tcp_close(tpcb);
        http_handle.complete = true;
        return ERR_OK;
    }

    char response[128];
    pbuf_copy_partial(p, response, sizeof(response) - 1, 0);
    // Add null char at the end of the response
    response[sizeof(response) - 1] = '\0';
    
    http_handle.success = false;

    if (response[9] == '2' && response[10] == '0' && response[11] == '4') {
        http_handle.success = true;
        DBG("InfluxDB response: 204 No Content (success)");
    } else {
         DBG("InfluxDB bad response: %s", response);
    }

    pbuf_free(p);
    tcp_recved(tpcb, p->tot_len);
    tcp_close(tpcb);
    http_handle.complete = true;
    return ERR_OK;
}

/**
 * @brief TCP connected callback for InfluxDB
 * @param arg User provided argument (unused)
 * @param tpcb TCP protocol control block
 * @param err Error code
 * @return ERR_OK on success
 */
static err_t tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        DBG("TCP connection error: %d\n", err);
        http_handle.complete = true;
        http_handle.success = false;
        return err;
    }

    // Send HTTP request
    err_t write_err = tcp_write(tpcb, http_handle.request, http_handle.request_len, TCP_WRITE_FLAG_COPY);

    if (write_err != ERR_OK) {
        DBG("TCP write error: %d\n", write_err);
        http_handle.complete = true;
        return write_err;
    }

    tcp_output(tpcb);
    return ERR_OK;
}

/**
 * @brief Send a packet with no successful pings to InfluxDB
 * @param[in] temperature_c Temperature in Celsius
 * @return true on success, false otherwise
 */
bool influxdb_send_failure(float temperature_c) {
    char influx_query[128];

    snprintf(influx_query, sizeof(influx_query), "wifi_measurements,host=PicoW "
        "loss=100,"
        "temperature=%.2f",
        temperature_c);

    // Send the HTTP POST request to InfluxDB
    DBG("Sending failure query anyway: %s\n", influx_query);
    bool request_res = send_http_post(influx_query);
    return request_res;
}

/**
 * @brief Calculate delay for retrying failed operations
 * @param retry_count Pointer to retry counter
 * @param retry_delay Pointer to current retry delay
 * @note Retry counter will be incremented
 * @return Delay time in ms
 */
uint32_t calculate_backoff_delay(uint32_t *retry_c, uint32_t *retry_delay) {
    (*retry_c)++;

    if (*retry_c > MAX_RETRY_COUNT) {
        DBG("Max retries reached (%d). Resetting counter", MAX_RETRY_COUNT);
        *retry_c = 0;
        *retry_delay = INITIAL_RETRY_DELAY_MS;
        return MEASUREMENT_INTERVAL_MS;
    }

    // Exponential delay with a limitation
    uint32_t delay = INITIAL_RETRY_DELAY_MS * (1 << (*retry_c - 1));
    uint32_t max_delay = MEASUREMENT_INTERVAL_MS * 4;
    
    *retry_delay = (delay > max_delay) ? max_delay : delay;
    DBG("Backoff delay: %lu ms (retry %lu/%d)", *retry_delay, *retry_c, MAX_RETRY_COUNT);
    
    return *retry_delay;
}