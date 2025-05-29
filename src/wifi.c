#include "wifi.h"

/* Private variables ---------------------------------------------------------*/
static bool wifi_connected = false;


/**
 * @brief Initialize Wi-Fi in station mode and connect to router
 * @return true on success, false otherwise
 * @note `cyw43_arch_poll()` must be called periodically in the main loop for
 * this to work properly
 */
bool wifi_init(void) {
    DBG("Initializing Wi-Fi\n");
    
    if( cyw43_arch_init_with_country(CYW43_COUNTRY_TURKEY)) {
        DBG("Wi-Fi initialization failed!\n");
        return false;
    }

    // Disable Wi-Fi power management
    cyw43_wifi_pm(&cyw43_state, CYW43_PERFORMANCE_PM & ~0xf);

    // Set station mode
    cyw43_arch_enable_sta_mode();

    DBG("Connecting to %s\n", WIFI_SSID);
    if (0 != cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK)) {
        DBG("Wi-Fi connection failed!\n");
        cyw43_arch_deinit();
        return false;
    } else {
        wifi_connected = true;
        DBG("Connected to %s\n", WIFI_SSID);
    }

    return wifi_connected;

}

/**
 * @brief Check if Wi-Fi is connected
 * @return true if connected, false otherwise
 */
bool wifi_is_connected(void) {
    bool connected = false;
    cyw43_arch_lwip_begin();
    connected = wifi_connected;
    cyw43_arch_lwip_end();
    return connected;
}

/**
 * @brief Wrapper function for cyw43_arch_poll() that is used to process anything 
 *          required by the cyw43_driver or the TCP/IP stack
 * @note This function should be called regularly in the main loop
 */
void wifi_process(void) {
    cyw43_arch_poll();
}

/**
 * @brief Deinitialize Wi-Fi and free resources
 */
void wifi_deinit(void) {
    DBG("De-initializing Wi-Fi\n");
    cyw43_arch_deinit();
    wifi_connected = false;
}