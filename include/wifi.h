#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "config.h"

/**
 * @brief Wi-Fi function protoypes
 */
// Wi-Fi initialization
bool wifi_init(void);
// Check Wi-Fi connection status
bool wifi_is_connected(void);
// Process Wi-Fi events
void wifi_process(void);
// Wi-Fi de-initialization
void wifi_deinit(void);

#endif /* WIFI_H */