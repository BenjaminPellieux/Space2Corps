/**
 * @file wifi.h
 * @brief WiFi communication module for Space2Corps
 * 
 * This module provides WiFi connectivity in Access Point mode
 * and UDP communication interface for remote control.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#ifndef WIFI_H
#define WIFI_H

#include "main.h"

/**
 * @brief WiFi configuration
 */
#define SSID "ESP32-C6_AP"      /**< Access Point SSID */
#define PASSWORD "password123"  /**< WiFi password */
#define PORT 3333               /**< UDP port number */

/**
 * @brief Initialize WiFi in SoftAP mode
 * 
 * Configures ESP32 as Access Point
 */
void wifi_init_softap(void);

/**
 * @brief WiFi communication task
 * 
 * Handles UDP communication and command processing
 * 
 * @param pvParameters Task parameters (unused)
 */
void wifi_task(void *pvParameters);

#endif // WIFI_H
