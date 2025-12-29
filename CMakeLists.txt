idf_component_register(
    SRCS "src/esp32_wifi_udp.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi esp_netif nvs_flash lwip
)
