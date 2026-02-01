#include "main.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE


static const char *TAG = "ESP32-C6_MAIN";

Mission_Ctx*  mission_Ctx;


void init_context(){
    mission_Ctx = (Mission_Ctx*) malloc(sizeof(Mission_Ctx));
    if (mission_Ctx == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        return;
    }
    mission_Ctx->current_status = SYSTEM_CHECKOUT;
    strncpy(mission_Ctx->mission_name, "Space2Corps", MAX_SIZE_NAME);

}



int handle_misson_status(SystemStatus status){
    switch (status)
    {
        case LIMIT_SWITCH_ON:
            set_servo_position(HINGE_OPEN);
            return 0;

        default:
            return 1;
    }

}

void control_task(void *pvParameters) {
    init_context();
    init_actuator();
    set_servo_position(HINGE_CLOSE); // Tourner à 0°

    ESP_LOGI(TAG, "Début du contrôle du servo moteur");



    while (1) {

        display_current_status(mission_Ctx->current_status);
        printf("%s\n", mission_Ctx->mission_name);
        check_limit_switch();

        handle_misson_status(mission_Ctx->current_status);
        vTaskDelay(10 / portTICK_PERIOD_MS);

    }
}

void app_main(void)
{
    // Create tasks for each core
    ESP_LOGI(TAG, "app main");

    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
}