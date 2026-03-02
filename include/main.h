/**
 * @file main.h
 * @brief Main header file for Space2Corps Actuator Control System
 * 
 * This file contains the primary data structures, definitions, and function
 * prototypes for the CubeSat actuator control system.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "actuator.h"
#include "wifi.h"
#include "status.h"
#include "sensors.h"

// #include <EEPROM.h>

/**
 * @brief ANSI color codes for console output
 */
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define GREEN   "\033[32m"

/**
 * @brief Boolean value definitions
 */
#define TRUE 1
#define FALSE 0

/**
 * @brief Maximum length for mission name string
 */
#define MAX_SIZE_NAME 64

/**
 * @brief Mission context structure
 * 
 * Contains all mission-critical data and state information
 */
typedef struct Mission_Ctx{
    char mission_name[MAX_SIZE_NAME];      /**< Mission identifier string */
    SystemStatus current_status;           /**< Current mission state */
    MotionData motion_data;                /**< IMU sensor data */
    GPSData gps_data;                      /**< GPS position data */
} Mission_Ctx;

/**
 * @brief Global mission context pointer
 * 
 * Accessible throughout the application for mission data
 */
extern Mission_Ctx* mission_Ctx;  

/**
 * @brief Main application entry point
 * 
 * Initializes the system and creates main tasks
 */
void app_main();

/**
 * @brief Main control task
 * 
 * Handles actuator control, state management, and sensor monitoring
 * 
 * @param pvParameters Task parameters (unused)
 */
void control_task(void *pvParameters);

/**
 * @brief Initialize mission context
 * 
 * Allocates memory and sets up initial mission parameters
 */
void init_context();

/**
 * @brief Handle mission status transitions
 * 
 * Manages state transitions and associated actions
 * 
 * @param status The target status to handle
 * @return int 0 on success, 1 on failure
 */
int handle_misson_status(SystemStatus status);

#endif // MAIN_H
