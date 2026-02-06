
#include "wifi.h"

static const char *TAG = "MASTER_WIFI";

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

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, 
                                                    ESP_EVENT_ANY_ID, 
                                                    &wifi_event_handler, 
                                                    NULL, 
                                                    NULL));

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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s", SSID, PASSWORD);
}

void wifi_task(void *pvParameters)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();

    // Create a socket for communication
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Failed to bind socket");
        close(sock);
        return;
    }

    while (1) {
        ESP_LOGI(TAG, "WIFI task");
        char buffer[100];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len < 0) {
            ESP_LOGE(TAG, "Failed to receive data");
            continue;
        }

        buffer[len] = '\0';
        ESP_LOGI(TAG, "Received: %s", buffer);

        // Send a response
        char *response = "ACK";
        sendto(sock, response, strlen(response), 0, (struct sockaddr *)&client_addr, client_addr_len);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    close(sock);
}

