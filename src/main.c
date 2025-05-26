#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "sensors.h"


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

    /* Read temperature */
    float temperature = temperature_read_celsius();
    printf("Temperature: %.2f°C", temperature);
  
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
