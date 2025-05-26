#ifndef CONFIG_H
#define CONFIG_H

// Measurememnt configuration
#define PING_TIMEOUT_MS     2000

#define DEBUG 1
#ifdef DEBUG
    #define DBG(fmt, ...) printf("DBG: " fmt "\n", ##__VA_ARGS__)
#else
    #define DBG(fmt, ...)
#endif

#endif /* CONFIG_H */