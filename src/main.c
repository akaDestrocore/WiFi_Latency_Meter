#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "sensors.h"
#include "ping.h"


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
	
    while (true) {

		Ping_Handle_t ping_handle;
		bool ping_success = ping_measure(&ping_handle, ROUTER_IP_ADDR);

		/* Read temperature */
		float temperature = temperature_read_celsius();
		printf("Temperature: %.2f°C\r\n", temperature);

		if(true == ping_success && ping_handle.received > 0) {
			uint64_t avg_rtt_us, min_rtt_us, max_rtt_us, jitter_us;
			uint8_t loss_p;

			ping_calculate_stats(&ping_handle, &avg_rtt_us, &min_rtt_us, &max_rtt_us, &jitter_us, &loss_p);

			DBG("Packets: sent=%u, received=%u, loss=%u%%\r\n", ping_handle.sent, ping_handle.received, loss_p);
			DBG("Avg RTT: %llu us, Min RTT: %llu us, Max RTT: %llu us, Jitter: %llu us\r\n", avg_rtt_us, min_rtt_us, max_rtt_us, jitter_us);
		}

		sleep_ms(2000); 
    }
}
