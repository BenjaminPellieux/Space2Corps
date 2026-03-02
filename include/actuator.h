/**
 * @file actuator.h
 * @brief Actuator control module for Space2Corps
 * 
 * This module provides interfaces for controlling servo motors and stepper motors
 * used in the CubeSat deployment mechanisms.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "main.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_rom_gpio.h"
#include "esp_task_wdt.h"

/**
 * @brief Servo motor configuration
 */
// Configuration du PWM pour le servo
#define SERVO_PIN GPIO_NUM_18     /**< Signal pin for servo */
#define SERVO_MIN_PULSEWIDTH 500    /**< Pulse width for 0° (microseconds) */
#define SERVO_MAX_PULSEWIDTH 2500   /**< Pulse width for 180° (microseconds) */
#define SERVO_PULSEWIDTH SERVO_MAX_PULSEWIDTH - SERVO_MIN_PULSEWIDTH

#define SERVO_FREQ 50               /**< PWM frequency for servo (50 Hz) */
#define SERVO_MAX_DEGREE 180        /**< Maximum angle in degrees */
#define SEVOR_PERIOD_US 20000       /**< Period in microseconds (1/frequency * 1e6) */
#define LIMIT_SWITCH_PIN GPIO_NUM_19 /**< Limit switch input pin */
#define HINGE_CLOSE 0               /**< Servo position for closed hinge */
#define HINGE_OPEN 90              /**< Servo position for open hinge */

#define EN_PIN      GPIO_NUM_20    /**< Stepper motor enable pin */
#define DIR_PIN     GPIO_NUM_21    /**< Stepper motor direction pin */
#define STEP_PIN    GPIO_NUM_22    /**< Stepper motor step pin */
#define M0_PIN      GPIO_NUM_10    /**< Microstepping configuration pin */
#define M1_PIN      GPIO_NUM_11    /**< Microstepping configuration pin */
#define M2_PIN      GPIO_NUM_0     /**< Microstepping configuration pin */

/**
 * @brief Stepper motor configuration structure
 */
typedef struct {
    gpio_num_t step_pin;   /**< Step signal pin */
    gpio_num_t dir_pin;    /**< Direction control pin */
    // ... autres configurations
} StepperConfig;

/**
 * @brief Stepper motor configuration
 */
// Configuration for 1/8 step (1600 steps/rev)
#define STEPS_PER_REVOLUTION 1600   /**< Steps per full revolution */
#define STEP_DELAY_US 4000         /**< Delay between steps (4 ms) */

#define INITIAL_DELAY 5000        /**< Initial delay of 5 seconds */

/**
 * @brief Initialize all actuators
 * 
 * Configures servo, stepper motor, and limit switch
 */
void init_actuator();

/**
 * @brief Initialize servo motor
 * 
 * Configures PWM for servo control
 */
void init_servo();

/**
 * @brief Calculate PWM duty cycle for servo position
 * 
 * @param degree Target angle in degrees
 * @return uint16_t Duty cycle value
 */
uint16_t calculate_duty(uint8_t degree);

/**
 * @brief Set servo position
 * 
 * @param degree Target angle (0-180)
 */
void set_servo_position(uint8_t degree);

/**
 * @brief Initialize stepper motor GPIOs
 * 
 * Configures all stepper motor control pins
 */
void init_motor_gpio();

/**
 * @brief Setup limit switch
 * 
 * Configures limit switch GPIO with pull-up resistor
 */
void setup_limit_switch();

/**
 * @brief Check limit switch status
 * 
 * @return bool TRUE if limit switch is pressed, FALSE otherwise
 */
bool check_limit_switch();

/**
 * @brief Rotate stepper motor
 * 
 * @param steps Number of steps to rotate
 * @param direction Rotation direction (TRUE = forward, FALSE = backward)
 */
void rotate_steps(int steps, bool direction);

#endif // ACTUATOR_H
