/**
 * @file main.c
 * @brief Main application implementation for Space2Corps
 * 
 * This file contains the core application logic including mission initialization,
 * state management, and task coordination for the CubeSat actuator control system.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#include "main.h"

/**
 * @brief Local logging tag for this module
 */
static const char *TAG = "MASTER_MAIN";

/**
 * @brief Global mission context pointer
 * 
 * Contains all mission-critical data and state information
 */
Mission_Ctx* mission_Ctx;

/**
 * @brief Initialize mission context
 * 
 * Allocates memory for the mission context structure and sets up initial parameters.
 * Initializes sensor interfaces and sets the initial mission state.
 * 
 * @note This function must be called before any mission operations begin.
 */
void init_context(){
    // Allocate memory for mission context
    mission_Ctx = (Mission_Ctx*) malloc(sizeof(Mission_Ctx));
    if (mission_Ctx == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        return;
    }
    
    // Set initial mission state
    mission_Ctx->current_status = SYSTEM_CHECKOUT;
    
    // Initialize mission name
    strncpy(mission_Ctx->mission_name, "Space2Corps", MAX_SIZE_NAME);
    
    // Initialize sensor data structures
    mission_Ctx->motion_data = (MotionData){0};
    mission_Ctx->motion_data.motion_initialized = false;
    mission_Ctx->gps_data = (GPSData){0};
    
    // Initialize sensor interfaces
    init_i2c();
    init_motion_sensors();
}

/**
 * @brief Handle mission status transitions
 * 
 * Manages state transitions and executes associated actions based on the current
 * mission status. Implements the core state machine logic.
 * 
 * @param status The target status to handle
 * @return int 0 on successful transition, 1 on failure
 * 
 * @note This function implements the mission state machine logic and should be called
 *        periodically to process state transitions.
 */
int handle_misson_status(SystemStatus status){

    if (mission_Ctx->current_status < LIMIT_SWITCH_ON) {
            ESP_LOGI(TAG, "Début rotation anti-horaire");
            transition_to_status(&mission_Ctx->current_status, COIL_WINDING);
            rotate_steps(STEPS_PER_REVOLUTION, 1);

            return 0;
    }    
    switch (status)
    {
        case LIMIT_SWITCH_ON:
            ESP_LOGI(TAG, "Fin de course détectée, ouverture du servo");
            vTaskDelay(pdMS_TO_TICKS(1000));
            set_servo_position(HINGE_OPEN);
            transition_to_status(&mission_Ctx->current_status, OPEN_HINGE);

            return 0;

        default:
            return 1;
    }   

}

/**
 * @brief Main control task
 * 
 * The primary FreeRTOS task that handles actuator control, state management,
 * and sensor monitoring throughout the mission.
 * 
 * @param pvParameters Task parameters (unused)
 * 
 * @note This task runs continuously and should not return.
 */
void control_task(void *pvParameters) {
    // Initialize mission context and actuators
    init_context();
    init_actuator();
    
    // Initial delay to allow system stabilization
    for (int i = 0; i < 5; i++) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // Set initial servo position (closed hinge)
    set_servo_position(HINGE_CLOSE); // Tourner à 0°
 
    ESP_LOGI(TAG, "Début du contrôle du servo moteur");

    ESP_LOGI(TAG, "Attente initiale de %d secondes", INITIAL_DELAY/1000);
    
    // Additional initialization delay
    for (int i = 0; i < 5; i++) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Main control loop
    while (1) {

        // Display current mission status
        display_current_status(mission_Ctx->current_status);
        
        // Process state transitions (commented out in current implementation)
        //handle_misson_status(mission_Ctx->current_status);
        
        // Display sensor data if available
        if (mission_Ctx->motion_data.motion_initialized) {
            display_motion_data();
        }else{
            ESP_LOGE(TAG, "NO IMU DATA");
        }

        // Short delay to prevent watchdog timeout
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Application main entry point
 * 
 * Initializes the system and creates the main FreeRTOS tasks.
 * This is the first function called when the application starts.
 * 
 * @note This function should not return.
 */
void app_main(void)
{
    // Log system start
    ESP_LOGI(TAG, "app main");

    // Create main tasks
    // wifi_task: Handles WiFi communication and command processing
    xTaskCreate(wifi_task, "wifi_task", 4096, NULL, 5, NULL);
    
    // control_task: Handles actuator control and state management
    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
}
