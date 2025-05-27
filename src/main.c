#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "sensors.h"
#include "ping.h"
#include "wifi.h"
#include "influxdb.h"


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

	// Initialize Wi-Fi
	if (!wifi_init()) {
		printf("Wi-Fi initialization failed!\r\n");
		while (true) {
			tight_loop_contents();
		}
	}
	
	uint32_t retry_c = 0;
    uint32_t retry_delay = INITIAL_RETRY_DELAY_MS;

    while (true) {

		Ping_Handle_t ping_handle;
		bool ping_success = ping_measure(&ping_handle, ROUTER_IP_ADDR);

		// Read temperature
		float temperature = temperature_read_celsius();
		printf("\r\nTemperature: %.2f°C\r\n", temperature);

		if(true == ping_success && ping_handle.received > 0) {
			uint64_t avg_rtt_us, min_rtt_us, max_rtt_us, jitter_us;
			uint8_t loss_p;

			ping_calculate_stats(&ping_handle, &avg_rtt_us, &min_rtt_us, &max_rtt_us, &jitter_us, &loss_p);

			DBG("Packets: sent=%u, received=%u, loss=%u%%\r\n", ping_handle.sent, ping_handle.received, loss_p);
			DBG("Avg RTT: %llu us, Min RTT: %llu us, Max RTT: %llu us, Jitter: %llu us\r\n", avg_rtt_us, min_rtt_us, max_rtt_us, jitter_us);


			// Send measurements to InfluxDB
			if (influxdb_send_measurements(avg_rtt_us, min_rtt_us, max_rtt_us, jitter_us, loss_p, temperature)) {
				DBG("Data sent to InfluxDB");
				retry_c = 0;
				retry_delay = INITIAL_RETRY_DELAY_MS;
			} else {
				DBG("Failed to send data to InfluxDB");
                retry_delay = calculate_backoff_delay(&retry_c, &retry_delay);
			}
		} else {
			// Ping measurement failed
			DBG("Ping measurement failed");

			// Send temeprature and packet loss to InfluxDB
			if (influxdb_send_failure(temperature)) {
                retry_c = 0;
                retry_delay = INITIAL_RETRY_DELAY_MS;
            } else {
                DBG("Failed to send failure report to InfluxDB");
                retry_delay = calculate_backoff_delay(&retry_c, &retry_delay);
            }
		}

		sleep_ms(MEASUREMENT_INTERVAL_MS); 
    }
}
