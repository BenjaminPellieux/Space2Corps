/**
 * @file status.h
 * @brief State machine module for Space2Corps
 * 
 * This module implements the finite state machine for mission status management
 * throughout the CubeSat mission lifecycle.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#ifndef STATUS_H
#define STATUS_H

#include "main.h"

/**
 * @brief Mission status enumeration
 * 
 * Defines all possible states in the mission state machine
 */
typedef enum {
    FLOOR,                    /**< Ground operations state */
    ASCENT,                   /**< Launch ascent phase */
    PRE_SEPARATION_WAKE_UP,   /**< Pre-separation wakeup */
    SEPARATION,               /**< Satellite separation */
    POST_SEPARATION_WAKE_UP,  /**< Post-separation wakeup */
    MECHANICAL_DEPLOYMENTS,   /**< Mechanical deployments */
    SYSTEM_CHECKOUT,          /**< System checkout phase */
    CHECK_COIL_CONTROL,       /**< Coil control verification */
    COIL_WINDING,             /**< Coil winding operation */
    LIMIT_SWITCH_ON,          /**< Limit switch activated */
    CHECK_HINGE_CONTROL,      /**< Hinge control verification */
    OPEN_HINGE,               /**< Hinge opening operation */
    CHECK_PROPULSION,         /**< Propulsion system check */
    PROPULSION_THRUST,        /**< Propulsion thrust operation */
    COIL_UNWINDING,           /**< Coil unwinding operation */
    STANDBY,                  /**< Low power standby mode */
    INSTRUMENT_CHECKOUT,      /**< Instrument checkout */
    EXPERIENCE_DATA,          /**< Experience data collection */
    DATA_DOWNLINK,            /**< Data downlink operation */
    SAFE_MODE,                /**< Safe mode for error recovery */
    SURVIVAL,                 /**< Survival mode */
    END_OF_MISSION            /**< Mission completion */
} SystemStatus;

/**
 * @brief Get status name string
 * 
 * @param status The status enum value
 * @return const char* Human-readable status name
 */
const char* get_status_name(SystemStatus status);

/**
 * @brief Get status color code
 * 
 * @param status The status enum value
 * @return const char* ANSI color code for display
 */
const char* get_status_color(SystemStatus status);

/**
 * @brief Transition to new status
 * 
 * @param current_status Pointer to current status
 * @param new_status Target status to transition to
 */
void transition_to_status(SystemStatus *current_status, SystemStatus new_status);

/**
 * @brief Display current status
 * 
 * @param status Current status to display
 */
void display_current_status(SystemStatus status);

#endif // STATUS_H
