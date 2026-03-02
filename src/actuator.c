/**
 * @file actuator.c
 * @brief Actuator control implementation for Space2Corps
 * 
 * This file contains the implementation of servo and stepper motor control
 * functions for the CubeSat deployment mechanisms.
 * 
 * @author Space2Corps Development Team
 * @version 1.0
 * @date 2024
 */

#include "actuator.h"

/**
 * @brief Local logging tag for this module
 */
static const char *TAG = "MASTER_ACTUATOR";

/**
 * @brief Limit switch state flag
 * 
 * Indicates whether the limit switch has been pressed
 */
volatile bool limit_switch_pressed = FALSE;

/**
 * @brief Initialize all actuators
 * 
 * Configures and initializes the servo motor, stepper motor, and limit switch.
 * This function should be called during system initialization.
 * 
 * @note All actuator initialization is performed in this function.
 */
void init_actuator(){
    init_servo();
    setup_limit_switch();
    init_motor_gpio();
}

/**
 * @brief Initialize stepper motor GPIOs
 * 
 * Configures all GPIO pins required for stepper motor control including
 * step, direction, enable, and microstepping configuration pins.
 * 
 * @note The DRV8825 driver is configured for 1/8 microstepping mode.
 */
void init_motor_gpio() {

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << STEP_PIN) | (1ULL << DIR_PIN) |
                        (1ULL << EN_PIN)   | (1ULL << M0_PIN)  |
                        (1ULL << M1_PIN)   | (1ULL << M2_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    esp_err_t ret = gpio_config(&io_conf);
     if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed: %d", ret);
        return;
    }


    // Enable the driver (EN active LOW)
    gpio_set_level(EN_PIN, 1);

    // Configure for 1/8 step microstepping
    gpio_set_level(M0_PIN, 1);
    gpio_set_level(M1_PIN, 1);
    gpio_set_level(M2_PIN, 0);

    ESP_LOGI(TAG, "DRV8825 initialisé en 1/8 step, EN activé");
}

/**
 * @brief Rotate stepper motor
 * 
 * Rotates the stepper motor by the specified number of steps in the given direction.
 * Monitors the limit switch during rotation and stops if activated.
 * 
 * @param steps Number of steps to rotate
 * @param direction Rotation direction (TRUE = forward, FALSE = backward)
 * 
 * @note The motor is enabled during rotation and disabled afterwards to save power.
 */
void rotate_steps(int steps, bool direction) {
    // Enable motor
    gpio_set_level(EN_PIN, 0);
    
    // Set rotation direction
    gpio_set_level(DIR_PIN, direction ? 1 : 0 );
    
    // Execute steps
    for (int i = 0; i < steps; i++) {
        
        // Generate step pulse
        gpio_set_level(STEP_PIN, 1);
        esp_rom_delay_us(10);
        gpio_set_level(STEP_PIN, 0);
        esp_rom_delay_us(STEP_DELAY_US);
         
        // Check limit switch after each step
        if (check_limit_switch()){
            return;
        }

    }
    
    // Disable motor to save power
    gpio_set_level(EN_PIN, 1);
}

/**
 * @brief Initialize servo motor
 * 
 * Configures the PWM timer and channel for servo motor control.
 * The servo is configured for 50Hz operation with 16-bit resolution.
 * 
 * @note The servo PWM signal is generated using the LEDC peripheral.
 */
void init_servo() {
    // Configure PWM timer
    ledc_timer_config_t pwm_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_16_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = SERVO_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&pwm_timer);

    // Configure PWM channel
    ledc_channel_config_t pwm_channel = {
        .gpio_num = SERVO_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 500,
        .hpoint = 0
    };
    ledc_channel_config(&pwm_channel);
}

/**
 * @brief Calculate PWM duty cycle for servo position
 * 
 * Converts a target angle in degrees to the corresponding PWM duty cycle.
 * 
 * @param degree Target angle in degrees (0-180)
 * @return uint16_t Duty cycle value for LEDC
 * 
 * @note The calculation is based on the servo's pulse width requirements.
 */
uint16_t calculate_duty(uint8_t degree) {

    // Calculate pulse width in microseconds
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH + ((SERVO_PULSEWIDTH) * degree) / SERVO_MAX_DEGREE;

    // Calculate duty cycle based on 16-bit resolution
    return (pulse_width * (1 << 16)) / SEVOR_PERIOD_US;

}

/**
 * @brief Set servo position
 * 
 * Positions the servo motor to the specified angle by setting the appropriate
 * PWM duty cycle.
 * 
 * @param degree Target angle (0-180 degrees)
 * 
 * @note Includes a 1-second delay after positioning to allow the servo to stabilize.
 */
void set_servo_position(uint8_t degree) {


    // Calculate duty cycle for target angle
    uint16_t duty = calculate_duty(degree);

    // Set PWM duty cycle
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    ESP_LOGI(TAG, "Position à %d° \t Duty  = %d", degree, duty);
    
    // Delay to allow servo to reach position
    vTaskDelay(1000 / portTICK_PERIOD_MS);

}

/**
 * @brief Setup limit switch
 * 
 * Configures the limit switch GPIO with pull-up resistor and interrupt
 * capabilities. The limit switch is used for end-of-travel detection.
 * 
 * @note Interrupt handling is currently commented out but available for future use.
 */
void setup_limit_switch() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LIMIT_SWITCH_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,  // Interrupt on rising edge
    };
    esp_err_t ret = gpio_config(&io_conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure motor GPIOs");
        return;
    }



    // Install interrupt service (currently commented out)
    // gpio_install_isr_service(0);

    // Add interrupt handler (currently commented out)
    // gpio_isr_handler_add(LIMIT_SWITCH_PIN, limit_switch_isr_handler, NULL);
}

/**
 * @brief Check limit switch status
 * 
 * Monitors the limit switch and handles end-of-travel detection.
 * If the limit switch is activated, transitions to the LIMIT_SWITCH_ON state
 * and stops the motor.
 * 
 * @return bool TRUE if limit switch is pressed, FALSE otherwise
 * 
 * @note Includes debouncing delay and state transition handling.
 */
bool check_limit_switch() {

    if ((mission_Ctx->current_status < LIMIT_SWITCH_ON) && (gpio_get_level(LIMIT_SWITCH_PIN) == FALSE)) {
        // Debounce delay
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
        // Transition to limit switch state
        transition_to_status(&mission_Ctx->current_status, LIMIT_SWITCH_ON);
        
        // Disable motor
        gpio_set_level(EN_PIN, 1);
        
        ESP_LOGI(TAG, "Limit switch déclenché, arrêt du moteur");
        return TRUE;
    }
    return FALSE;
}
