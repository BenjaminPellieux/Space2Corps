/**
 * @file wifi.c
 * @brief WiFi communication implementation for Space2Corps
 * 
 * This file contains the implementation of WiFi connectivity in Access Point mode
 * and UDP communication interface for remote control of the CubeSat system.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#include "wifi.h"

/**
 * @brief Local logging tag for this module
 */
static const char *TAG = "MASTER_WIFI";

/**
 * @brief WiFi event handler
 * 
 * Handles WiFi events such as station connection and disconnection.
 * Logs client MAC addresses and connection status.
 * 
 * @param arg User argument (unused)
 * @param event_base Event base (WIFI_EVENT)
 * @param event_id Event identifier
 * @param event_data Event data pointer
 */
// Déclaration de la fonction de gestion des événements WiFi
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        printf("%s Station %02x:%02x:%02x:%02x:%02x:%02x joined, AID%d ", TAG, event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5],  event->aid);

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        printf("%s Station %02x:%02x:%02x:%02x:%02x:%02x left, AID%d ", TAG,  event->mac[0], event->mac[1], event->mac[2], event->mac[3], event->mac[4], event->mac[5],  event->aid);
    }
}

/**
 * @brief Initialize WiFi in SoftAP mode
 * 
 * Configures the ESP32 as a WiFi Access Point with the specified SSID and password.
 * Sets up the WiFi stack, event loop, and registers event handlers.
 * 
 * @note This function must be called before any WiFi communication can occur.
 */
void wifi_init_softap(void)
{
    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create default WiFi AP network interface
    esp_netif_create_default_wifi_ap();

    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register WiFi event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                    ESP_EVENT_ANY_ID, 
                                                    &wifi_event_handler, 
                                                    NULL, 
                                                    NULL));

    // Configure Access Point settings
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = SSID,
            .ssid_len = strlen(SSID),
            .channel = 6,
            .password = PASSWORD,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    // Set WiFi mode to Access Point
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    
    // Apply WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s", SSID, PASSWORD);
}

/**
 * @brief WiFi communication task
 * 
 * Main FreeRTOS task that handles WiFi communication and command processing.
 * Initializes NVS flash, sets up WiFi, creates a UDP socket, and processes
 * incoming commands.
 * 
 * @param pvParameters Task parameters (unused)
 * 
 * @note This task runs continuously and should not return.
 */
void wifi_task(void *pvParameters)
{
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi in SoftAP mode
    wifi_init_softap();

    // Create UDP socket for communication
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return;
    }

    // Configure server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to address
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket");
        close(sock);
        return;
    }

    // Main communication loop
    while (1) {
        ESP_LOGI(TAG, "WIFI task");
        char buffer[100];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Receive data from client
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            ESP_LOGE(TAG, "Failed to receive data");
            continue;
        }

        // Null-terminate received data
        buffer[len] = '\0';
        ESP_LOGI(TAG, "Received: %s", buffer);

        // Send ACK response
        char *response = "ACK";
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_addr_len);

        // Short delay to prevent watchdog timeout
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Close socket (unreachable in normal operation)
    close(sock);
}
