#ifndef ESP32_WIFI_UDP_H
#define ESP32_WIFI_UDP_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// ---------------- WiFi Provisioning ----------------
esp_err_t wifi_init(void);                 // Initialize WiFi
esp_err_t wifi_start_provisioning(void);   // Start ESP-Touch / AirKiss
bool wifi_is_connected(void);              // Check WiFi status

// ---------------- UDP Broadcast ----------------
esp_err_t udp_init(void);                  // Initialize UDP broadcast
esp_err_t udp_start_broadcast(uint16_t port); // Start broadcasting on user-defined port
void udp_stop_broadcast(void);             // Stop broadcasting

#endif // ESP32_WIFI_UDP_H
