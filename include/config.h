#ifndef CONFIG_H
#define CONFIG_H

// Network configuration
#define ROUTER_IP_ADDR      "192.168.2.1"
#define WIFI_SSID           "SomeSSID"
#define WIFI_PASSWORD       "SomePassword"

// Measurememnt configuration
#define PING_TIMEOUT_MS     2000
#define MAX_PING_COUNT       5

#define DEBUG 1
#ifdef DEBUG
    #define DBG(fmt, ...) printf("DBG: " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)
#endif

#endif /* CONFIG_H */