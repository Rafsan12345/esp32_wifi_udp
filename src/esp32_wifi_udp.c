#include "esp32_wifi_udp.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_smartconfig.h"

static const char *TAG = "wifi_udp";

// WiFi state
static bool wifi_connected = false;

// UDP state
static int udp_socket = -1;
static struct sockaddr_in broadcast_addr;
static TaskHandle_t broadcast_task_handle = NULL;

// ---------------- WiFi Provisioner ----------------
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_smartconfig_start(SC_TYPE_ESPTOUCH_AIRKISS);
        ESP_LOGI(TAG, "SmartConfig started");
    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        esp_wifi_connect();
    } else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        wifi_connected = true;
        ESP_LOGI(TAG, "WiFi connected");
    }
}

esp_err_t wifi_init(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_t any_id;
    esp_event_handler_instance_t got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &got_ip);

    wifi_config_t wifi_config = {0};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    return ESP_OK;
}

esp_err_t wifi_start_provisioning(void)
{
    esp_smartconfig_start(SC_TYPE_ESPTOUCH_AIRKISS);
    return ESP_OK;
}

bool wifi_is_connected(void)
{
    return wifi_connected;
}

// ---------------- UDP Broadcast ----------------
static void udp_broadcast_task(void *pvParameters)
{
    char msg[128];
    while(1) {
        if(wifi_connected) {
            snprintf(msg, sizeof(msg), "{\"device\":\"ESP32\"}");
            sendto(udp_socket, msg, strlen(msg), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

esp_err_t udp_init(void)
{
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    int broadcastEnable = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    return ESP_OK;
}

esp_err_t udp_start_broadcast(uint16_t port)
{
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    xTaskCreate(udp_broadcast_task, "udp_broadcast", 2048, NULL, 5, &broadcast_task_handle);
    return ESP_OK;
}

void udp_stop_broadcast(void)
{
    if(broadcast_task_handle) {
        vTaskDelete(broadcast_task_handle);
        broadcast_task_handle = NULL;
    }
}
