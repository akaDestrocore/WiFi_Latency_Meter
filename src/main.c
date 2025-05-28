#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/bootrom.h"
#include "lwip/netif.h"
#include "sensors.h"
#include "ping.h"
#include "wifi.h"
#include "influxdb.h"

#define MAX_WIFI_REINIT_TRIES    15

/**
 * @brief  The application entry point.
 * @return int
 */
int main()
{
    // Initialize standard I/O
    stdio_init_all();
    // delay to allow the serial port to initialize
    sleep_ms(2000);
    printf("Wi-Fi Latency Meter Starting...\r\n");

    // Initialize ADC to measure temperature
    temperature_init();

    if (!wifi_init()) {
        printf("Fatal: Wi-Fi init failed, halting.\r\n");
        while (true) tight_loop_contents();
    }

    uint32_t retry_c = 0;
    bool wifi_reinit_success = false;
    uint32_t retry_delay = INITIAL_RETRY_DELAY_MS;

    while (true) {
        Ping_Handle_t ping;
        bool ping_ok = ping_measure(&ping, ROUTER_IP_ADDR);

        float temperature = temperature_read_celsius();
        printf("\r\nTemperature: %.2f°C\r\n", temperature);

        if (ping_ok && ping.received > 0) {
            uint64_t avg, min, max, jit;
            uint8_t loss;
            ping_calculate_stats(&ping, &avg, &min, &max, &jit, &loss);

            DBG("Packets: sent=%u, received=%u, loss=%u%%\r\n",
                ping.sent, ping.received, loss);
            DBG("RTT: avg=%llu us, min=%llu us, max=%llu us, jitter=%llu us\r\n",
                avg, min, max, jit);

            if (influxdb_send_measurements(avg, min, max, jit, loss, temperature)) {
                retry_c	= 0;
                retry_delay = INITIAL_RETRY_DELAY_MS;
            } else {
                DBG("Failed to send data to InfluxDB\r\n");
                retry_delay = calculate_backoff_delay(&retry_c, &retry_delay);
            }
        } else {
            DBG("Ping measurement failed\r\n");
            if (influxdb_send_failure(temperature)) {
                retry_c	= 0;
                retry_delay = INITIAL_RETRY_DELAY_MS;
            } else {
                DBG("Failed to send failure report to InfluxDB\r\n");
                retry_delay = calculate_backoff_delay(&retry_c, &retry_delay);
            }
        }

        // Check if Wi-Fi is still working
        struct netif *wifi_if = netif_default;
        if (!netif_is_up(wifi_if) || !netif_is_link_up(wifi_if)) {
            printf("Wi-Fi link down! Reinitializing…\r\n");
            cyw43_arch_deinit();

            bool reinit_ok = false;
            uint32_t backoff = INITIAL_RETRY_DELAY_MS;
            for (uint32_t i = 0; i < MAX_WIFI_REINIT_TRIES; ++i) {
                sleep_ms(backoff);
                if (wifi_init()) {
                    printf("Wi-Fi back online after %u retries\r\n", i + 1);
                    reinit_ok = true;
                    break;
                }
                backoff = calculate_backoff_delay(&i, &backoff);
            }

            if (!reinit_ok) {
                printf("Failed %u reinit attempts, rebooting…\r\n", MAX_WIFI_REINIT_TRIES);
                reset_usb_boot(0, 0);
            }
        }

        sleep_ms(MEASUREMENT_INTERVAL_MS);
    }
    return 0;
}
