#ifndef CONFIG_H
#define CONFIG_H

// Network configuration
#define ROUTER_IP_ADDR      "192.168.2.1"
#define WIFI_SSID           "SomeSSID"
#define WIFI_PASSWORD       "SomePassword"

// Measurememnt configuration
#define PING_TIMEOUT_MS     2000
#define MAX_PING_COUNT       5

// InfluxDB configuration
#define INFLUX_RETRY_DELAY_MS   1000
#define INFLUXDB_IP     "SomeIP"
#define INFLUXDB_PORT   8086
#define INFLUXDB_ORG    "Wi-Fi%20Latency"
#define INFLUXDB_BUCKET "Data"
#define INFLUXDB_TOKEN  "FjH5x9Z1qTgmxPChyd2Av8zVTrDsfI-BAfmadWYOViST-1BW40koPQoXupU5oNAmE8-4CIDNBBf8kWskdNRo7Q=="

#define DEBUG 1
#ifdef DEBUG
    #define DBG(fmt, ...) printf("DBG: " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)
#endif

#endif /* CONFIG_H */